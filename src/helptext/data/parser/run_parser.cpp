/////////////////////////////////////////////////////////////////////////////
// Purpose:   Documentation parser pipeline implementation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <atomic>
#include <expected>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "doc_types.h"
#include "file_parser.h"
#include "file_scanner.h"
#include "ftsrch.h"
#include "markdown_writer.h"
#include "parser_reporter.h"
#include "stemmer.h"
#include "symbol_table.h"
#include "utils.h"
#include "zip_writer.h"

namespace fs = std::filesystem;

// Helper for packaging generated parser output into the ZIP archive
// Called by: RunParser()
static bool PackageOutputArchive(const ParseOptions& opts, const fs::path& output_dir,
                                 const std::vector<fs::path>& generated_files,
                                 const fs::path& kfts_path, const bool has_fts)
{
    if (!opts.zip_path)
    {
        return true;
    }

    const fs::path zip_file_path = *opts.zip_path;

    if (const fs::path zip_parent = zip_file_path.parent_path(); !zip_parent.empty())
    {
        std::error_code error_code;
        fs::create_directories(zip_parent, error_code);
        if (error_code)
        {
            parser::AddErrorMessage("Error: cannot create ZIP output directory '" +
                                    zip_parent.string() + "': " + error_code.message());
            return false;
        }
    }

    ZipWriter zipper;
    const std::expected<void, std::string> open_result = zipper.Open(zip_file_path);
    if (!open_result)
    {
        parser::AddErrorMessage("Error: " + open_result.error());
        return false;
    }

    bool zip_ok = true;

    for (const fs::path& out_rel: generated_files)
    {
        const fs::path out_path = output_dir / out_rel;

        if (fs::exists(out_path))
        {
            const std::string archive_name = out_rel.generic_string();
            const std::expected<void, std::string> add_result =
                zipper.AddDiskFile(archive_name, out_path);
            if (!add_result)
            {
                parser::AddErrorMessage("ZIP error: " + add_result.error());
                zip_ok = false;
                break;
            }
        }
    }

    if (zip_ok && !opts.single_file)
    {
        const fs::path index_path = output_dir / "index.md";
        if (fs::exists(index_path))
        {
            const std::expected<void, std::string> add_result =
                zipper.AddDiskFile("index.md", index_path);
            if (!add_result)
            {
                parser::AddErrorMessage("ZIP error: " + add_result.error());
                zip_ok = false;
            }
        }
    }

    if (zip_ok && !kfts_path.empty() && fs::exists(kfts_path))
    {
        std::ifstream kfts_file(kfts_path, std::ios::binary | std::ios::ate);
        if (kfts_file)
        {
            const std::streamsize kfts_size = kfts_file.tellg();
            kfts_file.seekg(0, std::ios::beg);
            std::vector<char> kfts_data(static_cast<std::size_t>(kfts_size));
            if (kfts_file.read(kfts_data.data(), kfts_size))
            {
                const std::expected<void, std::string> add_result = zipper.AddBinaryFile(
                    "data/search_index.kfts", kfts_data.data(), kfts_data.size());
                if (!add_result)
                {
                    parser::AddErrorMessage("ZIP error: " + add_result.error());
                    zip_ok = false;
                }
            }
        }
    }

    if (zip_ok && has_fts)
    {
        const fs::path map_path = output_dir / "data" / "doc_map.json";
        if (fs::exists(map_path))
        {
            const std::expected<void, std::string> add_result =
                zipper.AddDiskFile("data/doc_map.json", map_path);
            if (!add_result)
            {
                parser::AddErrorMessage("ZIP error: " + add_result.error());
                zip_ok = false;
            }
        }
    }

    if (!zip_ok)
    {
        return false;
    }

    const std::expected<void, std::string> finalize_result = zipper.Finalize();
    if (!finalize_result)
    {
        parser::AddErrorMessage("Error finalizing ZIP: " + finalize_result.error());
        return false;
    }

    const std::uintmax_t zip_size = fs::file_size(zip_file_path);
    const std::string zip_saved_message = "ZIP archive saved: " + zip_file_path.string() + " (" +
                                          std::to_string(zip_size) + " bytes)";
    parser::AddResultMessage(zip_saved_message);

    return true;
}

// ---------------------------------------------------------------------------
// File-scope result structs shared between RunParser and GenerateLists
// ---------------------------------------------------------------------------

struct GeneratedFile
{
    fs::path out_rel;     // relative path within output_dir (e.g. "wxButton.md")
    fs::path out_path;    // absolute path on disk
    std::string title;    // document title for FTS
    std::string md_text;  // populated only when FTS indexing is enabled
};

struct PerFileResult
{
    fs::path rel_path;                                       // source .h relative path
    std::vector<GeneratedFile> output_files;                 // one entry per generated .md
    std::vector<std::pair<std::string, std::string>> pages;  // (page id, title)
    std::vector<std::string> alias_pages;                    // @class names with no class
    std::size_t class_count { 0 };
    std::size_t enum_count { 0 };
    std::size_t function_count { 0 };
    bool parsed_ok { false };
    std::string error_message;
};

// ---------------------------------------------------------------------------
// GenerateLists
// ---------------------------------------------------------------------------

// Generate data/classes.md, data/events.md, data/overviews.md, data/functions.md.
// Each file contains one entry per line: class name (no extension) for class/event
// files, or relative path without extension for non-class content files.
// Returns relative paths (under "data/") of the files written, for ZIP inclusion.
static std::vector<fs::path> GenerateLists(const std::vector<PerFileResult>& results,
                                           const docparser::SymbolTable& symbols,
                                           const fs::path& output_dir)
{
    std::vector<std::string> classes_list;
    std::vector<std::string> events_list;
    std::vector<std::string> overviews_list;
    std::vector<std::string> functions_list;

    for (const PerFileResult& slot: results)
    {
        if (!slot.parsed_ok)
        {
            continue;
        }

        for (const GeneratedFile& gen_file: slot.output_files)
        {
            const std::string stem = gen_file.out_rel.stem().string();

            if (symbols.IsPageName(stem))
            {
                // @page topic — always listed under overviews regardless of how
                // many free functions its header happens to define.
                fs::path entry_path = gen_file.out_rel;
                entry_path.replace_extension();
                overviews_list.push_back(entry_path.generic_string());
                continue;
            }

            if (symbols.IsClassPageStem(stem))
            {
                // Class output file — classify as event or regular class.
                if (symbols.IsEventClass(stem))
                {
                    events_list.push_back(stem);
                }
                else
                {
                    classes_list.push_back(stem);
                }
            }
            else
            {
                // Non-class content file (enums, typedefs, defines, free functions).
                // Build the entry as the relative path without extension.
                fs::path entry_path = gen_file.out_rel;
                entry_path.replace_extension();
                const std::string entry = entry_path.generic_string();

                if (slot.function_count > 0)
                {
                    functions_list.push_back(entry);
                }
                else
                {
                    overviews_list.push_back(entry);
                }
            }
        }
    }

    std::ranges::sort(classes_list);
    std::ranges::sort(events_list);
    std::ranges::sort(overviews_list);
    std::ranges::sort(functions_list);

    // Ensure the data/ directory exists (may already exist if FTS is enabled).
    {
        std::error_code error_code;
        std::ignore = fs::create_directories(output_dir / "data", error_code);
    }

    std::vector<fs::path> written_files;

    auto write_list = [&](const std::string& filename, const std::vector<std::string>& entries)
    {
        const fs::path list_path = output_dir / "data" / filename;
        std::ofstream list_out(list_path);
        if (!list_out)
        {
            parser::AddErrorMessage("Error: could not write " + list_path.string());
            return;
        }
        for (const std::string& entry: entries)
        {
            list_out << entry << "\n";
        }
        written_files.emplace_back(fs::path("data") / filename);
    };

    write_list("classes.md", classes_list);
    write_list("events.md", events_list);
    write_list("overviews.md", overviews_list);
    write_list("functions.md", functions_list);

    return written_files;
}

// Copy the wx logo SVG to the output directory so it gets packaged into the
// documentation ZIP archive.
static void CopyLogoSvg(const std::filesystem::path& output_dir)
{
    static constexpr std::string_view kSvgFile = "src/art_src/wxlogo.svg";
    static const fs::path candidates[] = {
        fs::path(kSvgFile),
        fs::path("..") / kSvgFile,
        fs::path("..") / ".." / kSvgFile,
        fs::path("..") / ".." / ".." / kSvgFile,
    };
    for (const fs::path& candidate: candidates)
    {
        if (fs::exists(candidate))
        {
            std::error_code error_code;
            const fs::path dest = output_dir / fs::path(kSvgFile).filename();
            fs::copy_file(candidate, dest, fs::copy_options::overwrite_existing, error_code);
            return;
        }
    }
}

// Generate index.md — a welcome/home page modeled after the wxWidgets
// documentation introduction at https://docs.wxwidgets.org/latest/ .
static void GenerateWelcomePage(const docparser::SymbolTable& symbols,
                                const std::vector<PerFileResult>& results,
                                const std::filesystem::path& output_dir)
{
    // Copy wxlogo.svg to the output directory so it gets packaged into the ZIP.
    CopyLogoSvg(output_dir);

    // -- Compute statistics ------------------------------------------------

    const std::size_t class_count = symbols.ClassCount();

    std::size_t event_count = 0;
    for (const auto& [name, _path]: symbols.AllClasses())
    {
        if (symbols.IsEventClass(name))
        {
            ++event_count;
        }
    }

    std::size_t overview_count = 0;
    std::size_t function_count = 0;
    for (const PerFileResult& slot: results)
    {
        if (!slot.parsed_ok)
        {
            continue;
        }
        function_count += slot.function_count;
        for (const GeneratedFile& gen_file: slot.output_files)
        {
            const std::string stem = gen_file.out_rel.stem().string();
            if (!symbols.IsClassPageStem(stem))
            {
                ++overview_count;
            }
        }
    }

    // -- Collect overview paths and their display titles -------------------

    std::vector<std::pair<std::string, std::string>> overview_entries;  // (path, title)
    for (const PerFileResult& slot: results)
    {
        if (!slot.parsed_ok)
        {
            continue;
        }
        for (const GeneratedFile& gen_file: slot.output_files)
        {
            const std::string stem = gen_file.out_rel.stem().string();
            if (symbols.IsClassPageStem(stem))
            {
                continue;
            }

            fs::path overview_rel = gen_file.out_rel;
            overview_rel.replace_extension();

            // Prefer the @page title; fall back to the readable file stem for
            // non-class pages that are not topic pages.
            std::string display_name = stem;
            for (const auto& [page_id, page_title]: slot.pages)
            {
                if (page_id == stem && !page_title.empty())
                {
                    display_name = page_title;
                    break;
                }
            }
            if (display_name == stem)
            {
                std::ranges::replace(display_name, '/', ' ');
                if (!display_name.empty())
                {
                    display_name[0] = static_cast<char>(
                        std::toupper(static_cast<unsigned char>(display_name[0])));
                }
            }
            overview_entries.emplace_back(overview_rel.generic_string(), display_name);
        }
    }

    std::ranges::sort(overview_entries);

    // -- Write index.md ----------------------------------------------------

    const fs::path index_path = output_dir / "index.md";
    std::ofstream index_file(index_path);
    if (!index_file)
    {
        parser::AddErrorMessage("Error: could not write index.md");
        return;
    }

    index_file << "# wxWidgets Documentation\n\n";

    index_file
        << R"(<p style="text-align: center;"><img src="memory:wxlogo.svg" width="64" height="64" alt="wxWidgets logo"></p>)"
        << "\n\n";

    index_file << "Welcome to wxWidgets, a stable and powerful open source "
               << "framework for developing native cross-platform GUI "
               << "applications in C++!\n\n";

    index_file << "## Reference\n\n";
    index_file << "Use the **side panel** (left) to search and navigate:\n\n";
    index_file << "- **Classes** (" << class_count << " total) — "
               << "full alphabetical list of all wxWidgets classes.\n";
    index_file << "- **Events** (" << event_count << " total) — "
               << "all event classes and their documentation.\n";
    index_file << "- **Overviews** (" << overview_count << " total) — "
               << "topic-based documentation sections.\n";
    index_file << "- **Functions** (" << function_count << " total) — "
               << "free functions, macros, and global constants.\n\n";

    index_file << "## Overviews\n\n";
    index_file << "Key topic-based documentation sections:\n\n";
    for (const auto& [overview_path, display_name]: overview_entries)
    {
        index_file << "- [" << display_name << "](" << overview_path << ".md)\n";
    }

    index_file << "\n---\n*Generated documentation home page*\n";
}

// Write data/inheritance.json: per-class direct base and direct derived classes.
// The viewer consumes this to render inheritance graphs on the fly. Classes with
// neither bases nor derived are omitted to keep the file small.
static void WriteInheritanceJson(const docparser::SymbolTable& symbols, const fs::path& output_dir)
{
    {
        std::error_code error_code;
        std::ignore = fs::create_directories(output_dir / "data", error_code);
    }
    const fs::path json_path = output_dir / "data" / "inheritance.json";
    std::ofstream out_stream(json_path);
    if (!out_stream)
    {
        parser::AddErrorMessage("Error: could not write inheritance.json");
        return;
    }

    const auto write_array = [&out_stream](const std::vector<std::string>& items)
    {
        out_stream << '[';
        for (std::size_t i = 0; i < items.size(); ++i)
        {
            if (i > 0)
            {
                out_stream << ',';
            }
            out_stream << '"' << items[i] << '"';
        }
        out_stream << ']';
    };

    out_stream << "{\n";
    bool first_entry = true;
    for (const auto& [name, _path]: symbols.AllClasses())
    {
        const std::vector<std::string> bases = symbols.DirectBases(name);
        const std::vector<std::string> derived = symbols.DirectDerived(name);
        if (bases.empty() && derived.empty())
        {
            continue;
        }
        if (!first_entry)
        {
            out_stream << ",\n";
        }
        first_entry = false;
        out_stream << "  \"" << name << "\": {\"bases\": ";
        write_array(bases);
        out_stream << ", \"derived\": ";
        write_array(derived);
        out_stream << '}';
    }
    out_stream << "\n}\n";
}

int RunParser(const ParseOptions& opts)
{
    const fs::path input_dir = opts.input_dir;
    const fs::path output_dir = opts.output_dir;

    if (!fs::exists(input_dir) || !fs::is_directory(input_dir))
    {
        parser::AddErrorMessage("Error: Input directory does not exist: " + input_dir.string());
        return 1;
    }

    if (opts.verbose)
    {
        parser::AddResultMessage("Input:  " + input_dir.string() +
                                 "\nOutput: " + output_dir.string());
    }

    // Pass 1: Scan files and build symbol table
    const std::vector<fs::path> files = docparser::ScanDirectory(input_dir);
    parser::AddResultMessage("Found " + std::to_string(files.size()) + " header files.");

    docparser::SymbolTable symbols;
    symbols.BuildFromFiles(input_dir, files);
    parser::AddResultMessage("Symbol table: " + std::to_string(symbols.ClassCount()) +
                             " classes, " + std::to_string(symbols.EnumCount()) + " enums.");

    if (opts.verbose)
    {
        for (const auto& [name, path]: symbols.AllClasses())
        {
            parser::AddResultMessage("  " + name + " -> " + path.generic_string());
        }
    }

    // Pass 2: Parse files and generate markdown
    std::error_code error_code;
    const bool created_output_dir = fs::create_directories(output_dir, error_code);
    if (!created_output_dir && error_code)
    {
        parser::AddErrorMessage("Error: cannot create output directory '" + output_dir.string() +
                                "': " + error_code.message());
        return 1;
    }

    std::vector<fs::path> files_to_process;
    if (opts.single_file)
    {
        files_to_process.emplace_back(*opts.single_file);
    }
    else
    {
        files_to_process = files;
    }

    // Non-class output files are named after the source basename. Two headers in
    // different directories may share a basename, so every file after the
    // top-level one gets a deterministic name that prefixes its source directory
    // components joined with '_' (persist/toplevel.h -> persist_toplevel.md).
    std::unordered_map<std::string, std::string> non_class_stems;
    {
        std::unordered_map<std::string, std::vector<fs::path>> files_by_name;
        for (const fs::path& rel_path: files_to_process)
        {
            files_by_name[rel_path.filename().string()].push_back(rel_path);
        }

        for (const auto& file_group: files_by_name)
        {
            if (file_group.second.size() < 2)
            {
                continue;
            }

            std::vector<fs::path> sorted_group = file_group.second;
            std::ranges::sort(sorted_group,
                              [](const fs::path& lhs, const fs::path& rhs)
                              {
                                  const bool lhs_is_root = lhs.parent_path().empty();
                                  const bool rhs_is_root = rhs.parent_path().empty();
                                  if (lhs_is_root != rhs_is_root)
                                  {
                                      return lhs_is_root;
                                  }
                                  return lhs.generic_string() < rhs.generic_string();
                              });

            for (std::size_t index = 1; index < sorted_group.size(); ++index)
            {
                const fs::path& rel_path = sorted_group[index];
                std::string stem;
                for (const fs::path& component: rel_path.parent_path())
                {
                    if (!stem.empty())
                    {
                        stem += '_';
                    }
                    stem += component.string();
                }
                if (!stem.empty())
                {
                    stem += '_';
                }
                stem += rel_path.stem().string();
                non_class_stems[rel_path.generic_string()] = stem;
            }
        }
    }

    // FTS index setup
    ftsrch::IndexPtr fts_index;
    ftsrch::DocId next_doc_id = 0;
    std::vector<std::pair<ftsrch::DocId, std::string>> doc_map;  // doc_id -> relative .md path
    std::optional<ftsrch::SnowballStemmer> stemmer;

    if (!opts.no_fts)
    {
        stemmer.emplace("english");
        ftsrch::IndexOptions index_opts;
        index_opts.stemmer = stemmer->AsFunction();
        fts_index = ftsrch::CreateIndex(std::move(index_opts));
    }

    std::size_t success_count = 0;
    std::size_t error_count = 0;
    std::size_t warning_count = 0;
    const std::size_t total = files_to_process.size();

    // Per-file result collected by the parallel workers. The heavy lifting
    // (ParseFile + markdown generation + disk write) runs on worker threads;
    // the ftsrch index is populated serially afterwards so doc_ids remain
    // deterministic and no locking is needed around AddDocument.

    std::vector<PerFileResult> results(total);

    const unsigned hw_threads = std::thread::hardware_concurrency();
    const unsigned worker_count = [hw_threads, total]() -> unsigned
    {
        if (total == 0)
        {
            return 1U;
        }
        const unsigned capped = static_cast<unsigned>(std::min<std::size_t>(total, 32));
        if (hw_threads <= 2U)
        {
            return std::min(2U, capped);
        }
        return std::min(hw_threads - 2U, capped);
    }();

    std::atomic<std::size_t> next_index { 0 };
    std::atomic<std::size_t> completed { 0 };

    const bool want_fts = (fts_index != nullptr);
    const bool verbose = opts.verbose;

    auto worker = [&]()
    {
        while (true)
        {
            const std::size_t idx = next_index.fetch_add(1, std::memory_order_relaxed);
            if (idx >= total)
            {
                return;
            }

            PerFileResult& slot = results[idx];
            slot.rel_path = files_to_process[idx];
            const fs::path abs_path = input_dir / slot.rel_path;

            try
            {
                docparser::FileContent content = docparser::ParseFile(abs_path, slot.rel_path);

                // An @class doc that no class in this header claims becomes a
                // doc-only class page, unless pass 1 already knows the name as a
                // class page from another header.
                for (docparser::ClassInfo& alias_page: content.alias_pages)
                {
                    if (symbols.IsClassPageStem(alias_page.name))
                    {
                        continue;
                    }
                    slot.alias_pages.push_back(alias_page.name);
                    content.classes.push_back(std::move(alias_page));
                }

                slot.pages.reserve(content.pages.size());
                for (const docparser::PageInfo& page_info: content.pages)
                {
                    slot.pages.emplace_back(page_info.id, page_info.title);
                }

                std::string non_class_stem;
                const std::unordered_map<std::string, std::string>::const_iterator stem_iter =
                    non_class_stems.find(slot.rel_path.generic_string());
                if (stem_iter != non_class_stems.end())
                {
                    non_class_stem = stem_iter->second;
                }

                // WriteFile creates per-class files, one file per @page, plus a
                // source-derived file for any non-class content; returns all
                // generated relative paths.
                const std::vector<fs::path> generated = docparser::WriteFile(
                    content, symbols, output_dir, slot.rel_path, input_dir, non_class_stem);

                slot.class_count = content.classes.size();
                slot.enum_count = content.enums.size();
                slot.function_count = content.free_functions.size();

                for (const fs::path& out_rel: generated)
                {
                    GeneratedFile gen_file;
                    gen_file.out_rel = out_rel;
                    gen_file.out_path = output_dir / out_rel;
                    // Use stem as title (class name for class files, page id for
                    // topic pages, source stem for other non-class files)
                    gen_file.title = out_rel.stem().string();

                    if (want_fts)
                    {
                        std::ifstream md_file(gen_file.out_path);
                        gen_file.md_text.assign(std::istreambuf_iterator<char>(md_file),
                                                std::istreambuf_iterator<char>());
                    }

                    slot.output_files.emplace_back(std::move(gen_file));
                }

                slot.parsed_ok = true;
            }
            catch (const std::exception& ex)
            {
                slot.parsed_ok = false;
                slot.error_message = ex.what();
            }

            const std::size_t done = completed.fetch_add(1, std::memory_order_relaxed) + 1;

            if (verbose)
            {
                parser::AddResultMessage("[" + std::to_string(done) + "/" + std::to_string(total) +
                                         "] " + slot.rel_path.generic_string());
                if (slot.parsed_ok)
                {
                    const std::string output_summary =
                        "  -> " + std::to_string(slot.output_files.size()) + " file(s) (" +
                        std::to_string(slot.class_count) + " classes, " +
                        std::to_string(slot.enum_count) + " enums, " +
                        std::to_string(slot.function_count) + " functions)";
                    parser::AddResultMessage(output_summary);
                }
            }

            if (!slot.parsed_ok)
            {
                parser::AddErrorMessage("Error parsing " + slot.rel_path.generic_string() + ": " +
                                        slot.error_message);
            }
        }
    };

    if (opts.verbose)
    {
        const std::string worker_message = "Parsing with " + std::to_string(worker_count) +
                                           " worker thread" + (worker_count == 1 ? "" : "s") +
                                           " (hw_concurrency=" + std::to_string(hw_threads) + ")";
        parser::AddResultMessage(worker_message);
    }

    {
        std::vector<std::jthread> pool;
        pool.reserve(worker_count);
        for (unsigned worker_id = 0; worker_id < worker_count; ++worker_id)
        {
            pool.emplace_back(worker);
        }
        // std::jthread joins on scope exit
    }

    // Register alias pages after the pool closes so every worker sees the same
    // pass-1 symbol table and the registration order stays deterministic.
    for (const PerFileResult& slot: results)
    {
        for (const std::string& alias_name: slot.alias_pages)
        {
            symbols.RegisterClassPageStem(alias_name);
        }
    }

    // Output-name invariant: two source files must never share an output name.
    // Non-class names were disambiguated before the worker pass; this verifies the
    // result and resolves anything left (for example two headers declaring a class
    // with the same name) deterministically.
    {
        struct OutputOwner
        {
            std::size_t slot_index = 0;
            std::size_t file_index = 0;
        };

        std::unordered_map<std::string, OutputOwner> owners;
        for (std::size_t slot_index = 0; slot_index < results.size(); ++slot_index)
        {
            PerFileResult& slot = results[slot_index];
            for (std::size_t file_index = 0; file_index < slot.output_files.size(); ++file_index)
            {
                GeneratedFile& gen_file = slot.output_files[file_index];
                const std::string output_name = gen_file.out_rel.generic_string();
                const auto [owner_iter, inserted] =
                    owners.try_emplace(output_name, OutputOwner { slot_index, file_index });
                if (inserted)
                {
                    continue;
                }

                const OutputOwner existing = owner_iter->second;
                const fs::path& existing_path = results[existing.slot_index].rel_path;
                if (existing_path == slot.rel_path)
                {
                    // A single header may declare the same class twice (for example
                    // the macro-based wxScopedPtr and its template counterpart). Both
                    // declarations map to one page; the later write replaces the
                    // earlier one in the same worker, which is deterministic.
                    continue;
                }

                parser::AddErrorMessage("Error: output name collision: " + output_name + " from " +
                                        existing_path.generic_string() + " and " +
                                        slot.rel_path.generic_string());

                // Keep class page names stable; rename the non-class output when
                // the colliding entry is one.
                const std::string stem = gen_file.out_rel.stem().string();
                if (symbols.IsClassPageStem(stem))
                {
                    continue;
                }

                std::string new_stem;
                for (const fs::path& component: slot.rel_path.parent_path())
                {
                    if (!new_stem.empty())
                    {
                        new_stem += '_';
                    }
                    new_stem += component.string();
                }
                if (!new_stem.empty())
                {
                    new_stem += '_';
                }
                new_stem += stem;

                const fs::path new_rel = fs::path(new_stem + ".md");
                std::error_code rename_error;
                fs::rename(gen_file.out_path, output_dir / new_rel, rename_error);
                if (!rename_error)
                {
                    gen_file.out_rel = new_rel;
                    gen_file.out_path = output_dir / new_rel;
                }
            }
        }
    }

    // Serial FTS pass: adds one document per generated .md file in deterministic
    // source-file order so doc_ids remain stable across runs.
    // Also collects all generated file paths for ZIP packaging.
    std::vector<fs::path> all_generated_files;
    for (std::size_t i = 0; i < total; ++i)
    {
        const PerFileResult& slot = results[i];
        if (!slot.parsed_ok)
        {
            ++error_count;
            continue;
        }

        for (const GeneratedFile& gen_file: slot.output_files)
        {
            all_generated_files.emplace_back(gen_file.out_rel);

            if (fts_index)
            {
                const ftsrch::DocId doc_id = next_doc_id++;
                const std::expected<void, ftsrch::Error> fts_result =
                    ftsrch::AddDocument(*fts_index, doc_id, gen_file.title, gen_file.md_text);
                if (fts_result.has_value())
                {
                    doc_map.emplace_back(doc_id, gen_file.out_rel.generic_string());
                }
                else
                {
                    parser::AddErrorMessage("Warning: FTS indexing failed for " +
                                            gen_file.out_rel.generic_string());
                    ++warning_count;
                }
            }
        }

        ++success_count;
    }

    // Generate index.md (welcome page)
    if (!opts.single_file)
    {
        GenerateWelcomePage(symbols, results, output_dir);
        all_generated_files.emplace_back("wxlogo.svg");
        if (opts.verbose)
        {
            parser::AddResultMessage("Generated index.md");
        }
    }

    // Save FTS index
    fs::path kfts_path;  // stashed for ZIP packaging below
    if (fts_index)
    {
        {
            std::error_code error_code;
            std::ignore = fs::create_directories(output_dir / "data", error_code);
        }
        kfts_path = output_dir / "data" / "search_index.kfts";
        const std::expected<void, ftsrch::Error> save_result =
            ftsrch::SaveIndex(*fts_index, kfts_path);
        if (save_result.has_value())
        {
            parser::AddResultMessage("FTS index saved: " + kfts_path.generic_string());
        }
        else
        {
            parser::AddErrorMessage("Error saving FTS index");
            ++error_count;
        }

        // Write doc_map.json
        const fs::path map_path = output_dir / "data" / "doc_map.json";
        std::ofstream map_out(map_path);
        if (map_out)
        {
            map_out << "{\n";
            for (std::size_t j = 0; j < doc_map.size(); ++j)
            {
                const auto& [doc_id, doc_path] = doc_map[j];
                map_out << "  \"" << doc_id << "\": \"" << doc_path << "\"";
                if (j + 1 < doc_map.size())
                {
                    map_out << ",";
                }
                map_out << "\n";
            }
            map_out << "}\n";
            if (opts.verbose)
            {
                const std::string doc_map_message = "Doc map saved: " + map_path.generic_string() +
                                                    " (" + std::to_string(doc_map.size()) +
                                                    " entries)";
                parser::AddResultMessage(doc_map_message);
            }
        }
        else
        {
            parser::AddErrorMessage("Error: could not write doc_map.json");
            ++error_count;
        }
    }

    // Write data/inheritance.json (independent of FTS) for on-the-fly graphs.
    if (!opts.single_file)
    {
        WriteInheritanceJson(symbols, output_dir);
        all_generated_files.emplace_back("data/inheritance.json");
    }

    // Generate list files (classes, events, overviews, functions) in data/.
    if (!opts.single_file)
    {
        const std::vector<fs::path> list_files = GenerateLists(results, symbols, output_dir);
#if defined(_WIN32)
        all_generated_files.append_range(list_files);
#else
        // clang-20 on Ubuntu 24 (GitHub runner) does not support std::vector::append_range (C++23).
        all_generated_files.insert(all_generated_files.end(), list_files.begin(), list_files.end());
#endif
        if (opts.verbose)
        {
            parser::AddResultMessage("Generated list files: data/classes.md, data/events.md, "
                                     "data/overviews.md, data/functions.md");
        }
    }

    // Package into ZIP archive
    if (!PackageOutputArchive(opts, output_dir, all_generated_files, kfts_path,
                              (fts_index != nullptr)))
    {
        ++error_count;
    }

    // Summary
    const std::string error_suffix =
        error_count > 0 ? " (" + std::to_string(error_count) + " errors)" : "";
    const std::string warning_suffix =
        warning_count > 0 ? " (" + std::to_string(warning_count) + " warnings)" : "";
    const std::string summary_message = "Processed " + std::to_string(success_count) +
                                        " source files -> " +
                                        std::to_string(all_generated_files.size()) +
                                        " markdown files" + error_suffix + warning_suffix + ".";
    parser::AddResultMessage(summary_message);

    return error_count > 0 ? 1 : 0;
}
