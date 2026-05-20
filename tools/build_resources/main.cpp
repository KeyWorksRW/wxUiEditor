/////////////////////////////////////////////////////////////////////////////
// Purpose:   build_resources — preprocess and zip XML + SVG files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "utils.h"

namespace fs = std::filesystem;

// ###########################################################################
//  Forward declarations
// ###########################################################################

enum class ExitCode : std::uint8_t
{
    success = 0,
    bad_arg = 1,
    missing_input = 2,
    io_error = 3,
    compression_error = 4,
};

struct CommandLine
{
    std::string zip_output_path;
    std::string source_dir;
    std::string docs_source_dir;
    bool show_help { false };
    bool create_ui_zip { false };
    bool build_docs_zip { false };

    // Parser / zip-test modes (migrated from wxViewer)
    bool parser_mode { false };
    bool zip_test_mode { false };
    std::string zip_test_path;

    // Parser options
    std::string output_dir;
    bool verbose { false };
    bool no_fts { false };
    std::optional<std::string> single_file;
    std::optional<std::string> zip_path;
};

std::expected<CommandLine, ExitCode> parse_args(std::span<const char* const> args);

[[nodiscard]] ExitCode create_ui_zip(const CommandLine& cmd);
[[nodiscard]] ExitCode build_docs_zip(const CommandLine& cmd);

// ###########################################################################
//  Entry point
// ###########################################################################

int main(int argc, char* argv[])
{
    std::expected<CommandLine, ExitCode> result =
        parse_args({ argv + 1, static_cast<size_t>(argc - 1) });
    if (!result)
    {
        return std::to_underlying(result.error());
    }

    const CommandLine& cmd = *result;

    if (cmd.show_help)
    {
        std::print("Usage: build_resources [options]\n"
                   "\n"
                   "Options:\n"
                   "  --help, -h             Show this help message\n"
                   "  --parse                Run documentation parser pipeline\n"
                   "  --zip-test <path>      Verify a parser-produced ZIP archive\n"
                   "  --srcdir <path>        Root of the source tree (default: .)\n"
                   "  --outdir <path>        Output directory for parser results\n"
                   "  --verbose              Enable verbose parser output\n"
                   "  --no-fts               Skip full-text search indexing\n"
                   "  --single-file <path>   Process only a single file\n"
                   "  --zip-path <path>      Path to an existing zip archive\n"
                   "  --create-ui-zip <path> Compress all XML + SVG files into a zip archive\n"
                   "  --build-docs-zip <path> Build wxWidgets documentation zip\n"
                   "  --docdir <path>        Path to wxViewer/tests docs source\n");
        return std::to_underlying(ExitCode::success);
    }

    if (cmd.zip_test_mode)
    {
        return RunZipTest(fs::path(cmd.zip_test_path));
    }

    if (cmd.parser_mode)
    {
        ParseOptions opts;
        opts.input_dir = fs::path(cmd.source_dir);
        opts.output_dir = fs::path(cmd.output_dir);
        opts.verbose = cmd.verbose;
        opts.no_fts = cmd.no_fts;
        if (cmd.single_file.has_value())
        {
            opts.single_file = fs::path(cmd.single_file.value());
        }
        if (cmd.zip_path.has_value())
        {
            opts.zip_path = fs::path(cmd.zip_path.value());
        }
        return RunParser(opts);
    }

    if (cmd.create_ui_zip)
    {
        return std::to_underlying(create_ui_zip(cmd));
    }

    if (cmd.build_docs_zip)
    {
        return std::to_underlying(build_docs_zip(cmd));
    }

    std::print("No action specified. Use --help for usage information.\n");
    return std::to_underlying(ExitCode::bad_arg);
}

// ###########################################################################
//  Argument parsing
// ###########################################################################

std::expected<CommandLine, ExitCode> parse_args(std::span<const char* const> args)
{
    CommandLine cmd;

    for (size_t i = 0; i < args.size(); ++i)
    {
        const std::string_view current_arg(args[i]);

        if (current_arg == "--help" || current_arg == "-h")
        {
            cmd.show_help = true;
            return cmd;
        }

        if (current_arg == "--create-ui-zip")
        {
            if (++i >= args.size())
            {
                std::print(stderr, "ERROR: --create-ui-zip requires a path argument\n");
                return std::unexpected(ExitCode::bad_arg);
            }
            cmd.create_ui_zip = true;
            cmd.zip_output_path = args[i];
            continue;
        }

        if (current_arg == "--srcdir")
        {
            if (++i >= args.size())
            {
                std::print(stderr, "ERROR: --srcdir requires a path argument\n");
                return std::unexpected(ExitCode::bad_arg);
            }
            cmd.source_dir = args[i];
            continue;
        }

        if (current_arg == "--build-docs-zip")
        {
            if (++i >= args.size())
            {
                std::print(stderr, "ERROR: --build-docs-zip requires a path argument\n");
                return std::unexpected(ExitCode::bad_arg);
            }
            cmd.build_docs_zip = true;
            cmd.zip_output_path = args[i];
            continue;
        }

        if (current_arg == "--docdir")
        {
            if (++i >= args.size())
            {
                std::print(stderr, "ERROR: --docdir requires a path argument\n");
                return std::unexpected(ExitCode::bad_arg);
            }
            cmd.docs_source_dir = args[i];
            continue;
        }

        if (current_arg == "--parse")
        {
            cmd.parser_mode = true;
            continue;
        }

        if (current_arg == "--zip-test")
        {
            if (++i >= args.size())
            {
                std::print(stderr, "ERROR: --zip-test requires a path argument\n");
                return std::unexpected(ExitCode::bad_arg);
            }
            cmd.zip_test_mode = true;
            cmd.zip_test_path = args[i];
            continue;
        }

        if (current_arg == "--outdir" || current_arg == "--output-dir")
        {
            if (++i >= args.size())
            {
                std::print(stderr, "ERROR: --outdir requires a path argument\n");
                return std::unexpected(ExitCode::bad_arg);
            }
            cmd.output_dir = args[i];
            continue;
        }

        if (current_arg == "--verbose")
        {
            cmd.verbose = true;
            continue;
        }

        if (current_arg == "--no-fts")
        {
            cmd.no_fts = true;
            continue;
        }

        if (current_arg == "--single-file")
        {
            if (++i >= args.size())
            {
                std::print(stderr, "ERROR: --single-file requires a path argument\n");
                return std::unexpected(ExitCode::bad_arg);
            }
            cmd.single_file = args[i];
            continue;
        }

        if (current_arg == "--zip-path")
        {
            if (++i >= args.size())
            {
                std::print(stderr, "ERROR: --zip-path requires a path argument\n");
                return std::unexpected(ExitCode::bad_arg);
            }
            cmd.zip_path = args[i];
            continue;
        }

        std::print(stderr, "ERROR: unrecognized option: {}\n", current_arg);
        return std::unexpected(ExitCode::bad_arg);
    }

    return cmd;
}

// ###########################################################################
//  Command implementations (stubs — will be filled in later)
// ###########################################################################

[[nodiscard]] ExitCode create_ui_zip(const CommandLine& /* cmd */)
{
    // TODO: implement XML preprocessing + miniz zip creation + libdeflate compression
    std::print("create_ui_zip not yet implemented\n");
    return ExitCode::success;
}

[[nodiscard]] ExitCode build_docs_zip(const CommandLine& /* cmd */)
{
    // TODO: implement markdown -> zip using cppmark + miniz + libdeflate
    std::print("build_docs_zip not yet implemented\n");
    return ExitCode::success;
}
