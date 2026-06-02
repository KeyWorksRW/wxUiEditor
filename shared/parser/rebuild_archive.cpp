/////////////////////////////////////////////////////////////////////////////
// Purpose:   Edit/Delete/Add workflow for documentation ZIP archives
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "ftsrch.h"
#include "mainapp.h"
#include "stemmer.h"
#include "utils.h"
#include "zip_writer.h"

#include <algorithm>
#include <charconv>
#include <expected>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <vector>

#include <miniz.h>

namespace fs = std::filesystem;

// ============================================================================
//  Minimal JSON helper functions (flat structure only)
// ============================================================================

static void SkipWhitespace(std::string_view json, std::size_t& pos) noexcept
{
    while (pos < json.size() &&
           (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r'))
        ++pos;
}

static auto ParseJsonString(std::string_view json, std::size_t& pos) -> std::string
{
    if (pos >= json.size() || json[pos] != '"')
        return {};
    ++pos;  // skip opening quote
    std::string result;
    while (pos < json.size() && json[pos] != '"')
    {
        if (json[pos] == '\\')
        {
            ++pos;
            if (pos >= json.size())
                break;
            switch (json[pos])
            {
                case '"':
                    result += '"';
                    break;
                case '\\':
                    result += '\\';
                    break;
                case '/':
                    result += '/';
                    break;
                case 'n':
                    result += '\n';
                    break;
                case 'r':
                    result += '\r';
                    break;
                case 't':
                    result += '\t';
                    break;
                default:
                    result += json[pos];
                    break;
            }
        }
        else
        {
            result += json[pos];
        }
        ++pos;
    }
    if (pos < json.size())
        ++pos;  // skip closing quote
    return result;
}

// Parse a JSON object { "key": "value", ... } into a map.
static auto ParseStringMap(std::string_view json, std::size_t& pos)
    -> std::unordered_map<std::string, std::string>
{
    std::unordered_map<std::string, std::string> result;
    SkipWhitespace(json, pos);
    if (pos >= json.size() || json[pos] != '{')
        return result;
    ++pos;  // skip '{'
    SkipWhitespace(json, pos);
    if (pos < json.size() && json[pos] == '}')
    {
        ++pos;
        return result;  // empty object
    }
    while (pos < json.size())
    {
        SkipWhitespace(json, pos);
        if (pos >= json.size() || json[pos] != '"')
            break;
        const std::string key = ParseJsonString(json, pos);
        SkipWhitespace(json, pos);
        if (pos >= json.size() || json[pos] != ':')
            break;
        ++pos;  // skip ':'
        SkipWhitespace(json, pos);
        const std::string value = ParseJsonString(json, pos);
        result.emplace(std::move(key), std::move(value));
        SkipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == ',')
        {
            ++pos;
            continue;
        }
        if (pos < json.size() && json[pos] == '}')
        {
            ++pos;
            break;
        }
        break;
    }
    return result;
}

// Parse a JSON array of strings: ["item1", "item2", ...]
static auto ParseStringArray(std::string_view json, std::size_t& pos) -> std::vector<std::string>
{
    std::vector<std::string> result;
    SkipWhitespace(json, pos);
    if (pos >= json.size() || json[pos] != '[')
        return result;
    ++pos;  // skip '['
    SkipWhitespace(json, pos);
    if (pos < json.size() && json[pos] == ']')
    {
        ++pos;
        return result;  // empty array
    }
    while (pos < json.size())
    {
        SkipWhitespace(json, pos);
        const std::string item = ParseJsonString(json, pos);
        result.push_back(std::move(item));
        SkipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == ',')
        {
            ++pos;
            continue;
        }
        if (pos < json.size() && json[pos] == ']')
        {
            ++pos;
            break;
        }
        break;
    }
    return result;
}

// Top-level manifest structure parsed from JSON.
struct Manifest
{
    std::unordered_map<std::string, std::string> edits;
    std::vector<std::string> deletes;
    std::unordered_map<std::string, std::string> adds;
};

static auto ParseManifest(std::string_view json) -> Manifest
{
    Manifest manifest;
    std::size_t pos = 0;
    SkipWhitespace(json, pos);
    if (pos >= json.size() || json[pos] != '{')
        return manifest;
    ++pos;  // skip '{'
    while (pos < json.size())
    {
        SkipWhitespace(json, pos);
        if (pos >= json.size())
            break;
        if (json[pos] == '}')
        {
            ++pos;
            break;
        }
        const std::string key = ParseJsonString(json, pos);
        SkipWhitespace(json, pos);
        if (pos >= json.size() || json[pos] != ':')
            break;
        ++pos;
        SkipWhitespace(json, pos);

        if (key == "edit" || key == "add")
        {
            auto map = ParseStringMap(json, pos);
            if (key == "edit")
                manifest.edits = std::move(map);
            else
                manifest.adds = std::move(map);
        }
        else if (key == "delete")
        {
            manifest.deletes = ParseStringArray(json, pos);
        }
        else
        {
            // Skip unknown value
            if (pos < json.size() && json[pos] == '{')
            {
                int depth = 1;
                ++pos;
                while (pos < json.size() && depth > 0)
                {
                    if (json[pos] == '{')
                        ++depth;
                    else if (json[pos] == '}')
                        --depth;
                    ++pos;
                }
            }
            else if (pos < json.size() && json[pos] == '[')
            {
                int depth = 1;
                ++pos;
                while (pos < json.size() && depth > 0)
                {
                    if (json[pos] == '[')
                        ++depth;
                    else if (json[pos] == ']')
                        --depth;
                    ++pos;
                }
            }
            else
            {
                while (pos < json.size() && json[pos] != ',' && json[pos] != '}' &&
                       json[pos] != ' ')
                    ++pos;
            }
        }

        SkipWhitespace(json, pos);
        if (pos < json.size() && json[pos] == ',')
        {
            ++pos;
            continue;
        }
        if (pos < json.size() && json[pos] == '}')
        {
            ++pos;
            break;
        }
        break;
    }
    return manifest;
}

// ============================================================================
//  RebuildArchive implementation
// ============================================================================

int RebuildArchive(const fs::path& zip_path, const fs::path& manifest_path)
{
    // ---------------------------------------------------------------
    // 1. Open existing archive
    // ---------------------------------------------------------------
    const auto archive_result = OpenDocArchive(zip_path);
    if (!archive_result)
    {
        wxGetApp().AddErrorMessage("Error: " + archive_result.error());
        return 1;
    }
    const auto& archive = *archive_result;

    // ---------------------------------------------------------------
    // 2. Read and parse existing doc_map.json
    // ---------------------------------------------------------------
    const auto doc_map_result = ReadMarkdown(*archive, "data/doc_map.json");
    if (!doc_map_result)
    {
        wxGetApp().AddErrorMessage("Error: cannot read data/doc_map.json from archive: " +
                                   doc_map_result.error());
        return 1;
    }

    // Parse doc_map.json: { "0": "path.md", "1": "path.md", ... }
    std::size_t parse_pos = 0;
    const auto raw_doc_map = ParseStringMap(doc_map_result.value(), parse_pos);

    // Build reverse map: path -> doc_id
    std::unordered_map<std::string, ftsrch::DocId> path_to_id;
    for (const auto& [key_str, path]: raw_doc_map)
    {
        ftsrch::DocId doc_id = 0;
        std::from_chars(key_str.data(), key_str.data() + key_str.size(), doc_id);
        path_to_id[path] = doc_id;
    }

    // ---------------------------------------------------------------
    // 3. Extract all markdown entries from the archive
    // ---------------------------------------------------------------
    // Open the ZIP file directly with miniz to enumerate entries
    mz_zip_archive reader {};
    const std::string zip_path_str = zip_path.string();
    if (!mz_zip_reader_init_file(&reader, zip_path_str.c_str(), 0))
    {
        wxGetApp().AddErrorMessage("Error: cannot open ZIP archive for enumeration: " +
                                   zip_path_str);
        return 1;
    }

    std::unordered_map<std::string, std::string> md_files;  // archive_name -> content
    const mz_uint num_files = mz_zip_reader_get_num_files(&reader);
    for (mz_uint i = 0; i < num_files; ++i)
    {
        char filename_buf[512];
        const mz_uint filename_len =
            mz_zip_reader_get_filename(&reader, i, filename_buf, sizeof(filename_buf));
        if (filename_len == 0)
            continue;
        const std::string_view filename(filename_buf, filename_len);

        // Skip data/ entries
        if (filename.starts_with("data/"))
            continue;

        // Only process .md files
        if (!filename.ends_with(".md"))
            continue;

        mz_zip_archive_file_stat stat {};
        if (!mz_zip_reader_file_stat(&reader, i, &stat))
            continue;

        std::string content;
        if (stat.m_uncomp_size > 0)
        {
            content.resize(static_cast<std::size_t>(stat.m_uncomp_size));
            if (!mz_zip_reader_extract_to_mem(&reader, i, content.data(), content.size(), 0))
            {
                wxGetApp().AddErrorMessage("Error: failed to extract '" + std::string(filename) +
                                           "'");
                mz_zip_reader_end(&reader);
                return 1;
            }
        }
        md_files[std::string(filename)] = std::move(content);
    }
    mz_zip_reader_end(&reader);

    // ---------------------------------------------------------------
    // 4. Read and parse the manifest JSON
    // ---------------------------------------------------------------
    std::ifstream manifest_file(manifest_path);
    if (!manifest_file)
    {
        wxGetApp().AddErrorMessage("Error: cannot open manifest file: " + manifest_path.string());
        return 1;
    }
    const std::string manifest_json((std::istreambuf_iterator<char>(manifest_file)),
                                    std::istreambuf_iterator<char>());

    const Manifest manifest = ParseManifest(manifest_json);

    // ---------------------------------------------------------------
    // 5. Apply edits
    // ---------------------------------------------------------------
    for (const auto& [name, content]: manifest.edits)
    {
        md_files[name] = content;
    }

    // ---------------------------------------------------------------
    // 6. Apply deletes
    // ---------------------------------------------------------------
    for (const auto& name: manifest.deletes)
    {
        md_files.erase(name);
    }

    // ---------------------------------------------------------------
    // 7. Apply adds
    // ---------------------------------------------------------------
    for (const auto& [name, content]: manifest.adds)
    {
        md_files[name] = content;
    }

    // ---------------------------------------------------------------
    // 8. Collect file paths in deterministic order
    // ---------------------------------------------------------------
    std::vector<std::string> sorted_paths;
    sorted_paths.reserve(md_files.size());
    for (const auto& [path, _]: md_files)
    {
        sorted_paths.push_back(path);
    }
    std::ranges::sort(sorted_paths);

    // ---------------------------------------------------------------
    // 9. Create a new FTS index
    // ---------------------------------------------------------------
    std::optional<ftsrch::SnowballStemmer> stemmer;
    stemmer.emplace("english");
    ftsrch::IndexOptions index_opts;
    index_opts.stemmer = stemmer->AsFunction();
    ftsrch::IndexPtr fts_index = ftsrch::CreateIndex(std::move(index_opts));

    // ---------------------------------------------------------------
    // 10. Add documents to the index
    // ---------------------------------------------------------------
    ftsrch::DocId next_doc_id = 0;
    std::vector<std::pair<ftsrch::DocId, std::string>> new_doc_map;
    for (const auto& path: sorted_paths)
    {
        const auto& content = md_files[path];
        const auto add_result = ftsrch::AddDocument(*fts_index, next_doc_id, path, content);
        if (!add_result)
        {
            wxGetApp().AddErrorMessage("Error: FTS AddDocument failed for '" + path + "'");
            return 1;
        }
        new_doc_map.emplace_back(next_doc_id, path);
        ++next_doc_id;
    }

    // ---------------------------------------------------------------
    // 11. Save index to a temporary file and read back
    // ---------------------------------------------------------------
    const fs::path temp_kfts = zip_path.parent_path() / "temp.kfts";
    const auto save_result = ftsrch::SaveIndex(*fts_index, temp_kfts);
    if (!save_result)
    {
        wxGetApp().AddErrorMessage("Error: failed to save temporary FTS index");
        return 1;
    }

    std::ifstream kfts_file(temp_kfts, std::ios::binary | std::ios::ate);
    if (!kfts_file)
    {
        wxGetApp().AddErrorMessage("Error: cannot read temporary FTS index");
        std::error_code ec;
        fs::remove(temp_kfts, ec);
        return 1;
    }
    const std::streamsize kfts_size = kfts_file.tellg();
    kfts_file.seekg(0, std::ios::beg);
    std::vector<char> kfts_data(static_cast<std::size_t>(kfts_size));
    if (!kfts_file.read(kfts_data.data(), kfts_size))
    {
        wxGetApp().AddErrorMessage("Error: failed to read temporary FTS index");
        std::error_code ec;
        fs::remove(temp_kfts, ec);
        return 1;
    }
    kfts_file.close();
    {
        std::error_code ec;
        fs::remove(temp_kfts, ec);
    }

    // ---------------------------------------------------------------
    // 12. Open ZipWriter on zip_path + ".new"
    // ---------------------------------------------------------------
    const fs::path new_zip_path = zip_path.string() + ".new";
    ZipWriter zipper;
    const auto open_result = zipper.Open(new_zip_path);
    if (!open_result)
    {
        wxGetApp().AddErrorMessage("Error: " + open_result.error());
        return 1;
    }

    // ---------------------------------------------------------------
    // 13. Add each markdown file
    // ---------------------------------------------------------------
    for (const auto& path: sorted_paths)
    {
        const auto& content = md_files[path];
        const auto add_result = zipper.AddFile(path, content);
        if (!add_result)
        {
            wxGetApp().AddErrorMessage("Error: " + add_result.error());
            return 1;
        }
    }

    // ---------------------------------------------------------------
    // 14. Add data/doc_map.json
    // ---------------------------------------------------------------
    std::string doc_map_json = "{\n";
    for (std::size_t j = 0; j < new_doc_map.size(); ++j)
    {
        const auto& [doc_id, doc_path] = new_doc_map[j];
        doc_map_json += "  \"" + std::to_string(doc_id) + "\": \"" + doc_path + "\"";
        if (j + 1 < new_doc_map.size())
            doc_map_json += ",";
        doc_map_json += "\n";
    }
    doc_map_json += "}\n";

    {
        const auto add_result = zipper.AddFile("data/doc_map.json", doc_map_json);
        if (!add_result)
        {
            wxGetApp().AddErrorMessage("Error: " + add_result.error());
            return 1;
        }
    }

    // ---------------------------------------------------------------
    // 15. Add data/search_index.kfts (compressed via AddBinaryFile)
    // ---------------------------------------------------------------
    {
        const auto add_result =
            zipper.AddBinaryFile("data/search_index.kfts", kfts_data.data(), kfts_data.size());
        if (!add_result)
        {
            wxGetApp().AddErrorMessage("Error: " + add_result.error());
            return 1;
        }
    }

    // ---------------------------------------------------------------
    // 16. Finalize
    // ---------------------------------------------------------------
    const auto finalize_result = zipper.Finalize();
    if (!finalize_result)
    {
        wxGetApp().AddErrorMessage("Error: " + finalize_result.error());
        std::error_code ec;
        fs::remove(new_zip_path, ec);
        return 1;
    }

    // ---------------------------------------------------------------
    // 18. Atomic rename: .new -> original
    // ---------------------------------------------------------------
    {
        std::error_code ec;
        fs::rename(new_zip_path, zip_path, ec);
        if (ec)
        {
            wxGetApp().AddErrorMessage("Error: cannot replace original ZIP: " + ec.message());
            return 1;
        }
    }

    wxGetApp().AddResultMessage("Archive rebuilt: " + zip_path.string());
    return 0;
}
