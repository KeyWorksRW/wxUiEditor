/////////////////////////////////////////////////////////////////////////////
// Purpose:   build_resources — preprocess and zip XML + SVG files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   MIT — see the LICENSE file
/////////////////////////////////////////////////////////////////////////////

#include <array>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <vector>

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
    bool create_ui_zip { false };
    bool build_docs_zip { false };
};

auto parse_args(std::span<const char* const> args) -> std::expected<CommandLine, ExitCode>;

auto create_ui_zip(const CommandLine& cmd) -> ExitCode;
auto build_docs_zip(const CommandLine& cmd) -> ExitCode;

// ###########################################################################
//  Entry point
// ###########################################################################

auto main(int argc, char* argv[]) -> int
{
    auto result = parse_args({ argv + 1, static_cast<size_t>(argc - 1) });
    if (!result)
    {
        return std::to_underlying(result.error());
    }

    const auto& cmd = *result;

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

auto parse_args(std::span<const char* const> args) -> std::expected<CommandLine, ExitCode>
{
    CommandLine cmd;

    for (size_t i = 0; i < args.size(); ++i)
    {
        std::string_view current_arg(args[i]);

        if (current_arg == "--help" || current_arg == "-h")
        {
            std::print("Usage: build_resources [options]\n"
                       "\n"
                       "Options:\n"
                       "  --help, -h             Show this help message\n"
                       "  --create-ui-zip <path> Compress all XML + SVG files into a zip archive\n"
                       "  --srcdir <path>        Root of the source tree (default: .)\n"
                       "  --build-docs-zip <path> Build wxWidgets documentation zip\n"
                       "  --docdir <path>        Path to wxViewer/tests docs source\n");
            std::exit(0);
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

        std::print(stderr, "ERROR: unrecognized option: {}\n", current_arg);
        return std::unexpected(ExitCode::bad_arg);
    }

    return cmd;
}

// ###########################################################################
//  Command implementations (stubs — will be filled in later)
// ###########################################################################

auto create_ui_zip(const CommandLine& /* cmd */) -> ExitCode
{
    // TODO: implement XML preprocessing + miniz zip creation + libdeflate compression
    std::print("create_ui_zip not yet implemented\n");
    return ExitCode::success;
}

auto build_docs_zip(const CommandLine& /* cmd */) -> ExitCode
{
    // TODO: implement markdown -> zip using cppmark + miniz + libdeflate
    std::print("build_docs_zip not yet implemented\n");
    return ExitCode::success;
}
