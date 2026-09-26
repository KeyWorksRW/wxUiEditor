//////////////////////////////////////////////////////////////////////////
// Purpose:   Verify that importing a project file did not change
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/**
 * @file verify_import.cpp
 * @brief Import a project file and save or verify its serialized .wxui form
 *
 * This code is called from the mainapp command line to verify that importing a project file
 * (WinResource, wxFormBuilder, wxCrafter, wxSmith, wxGlade, or DialogBlocks) is deterministic.
 * It performs no code generation, so no language compiler is required and the results are
 * identical on every platform.
 *
 *   --save_import <source>    serialize the imported project to <source>.wxui (or --golden)
 *   --verify_import <source>  import <source> and compare it against the golden .wxui file
 *
 * Importers are lossy and approximate: this verifies *determinism* (same input produces the same
 * output), not that the import matches the original file.
 *
 * Exit codes (see verify_codegen.h):
 *   0 = Success (no differences found)
 *   1 = Failure (differences detected - see the .log file)
 *   2 = Source or golden file not found
 *   3 = Invalid arguments
 */

#include <cctype>
#include <format>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>  // std::ignore

#include <wx/cmdline.h>   // wxCmdLineParser
#include <wx/filename.h>  // wxFileName
#include <wx/string.h>    // wxString

#include "verify_import.h"

#include "../../tools/compare/diff.h"  // Diff::Compare, DiffResult

#include "mainapp.h"                            // App -- Main application class
#include "node.h"                               // Node class
#include "project_handler.h"                    // ProjectHandler class
#include "utils.h"                              // PROJECT_FILE_EXTENSION constants
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector

#include "pugixml.hpp"

// Returns the file's extension in lower case *without* the leading '.'.
[[nodiscard]] static std::string LowerExtension(const wxFileName& file)
{
    std::string extension = file.GetExt().ToStdString();
    for (char& character: extension)
    {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    return extension;
}

[[nodiscard]] static bool IsProjectFile(const wxFileName& file)
{
    const std::string extension = LowerExtension(file);
    return extension == PROJECT_FILE_EXTENSION.substr(1) ||
           extension == PROJECT_LEGACY_FILE_EXTENSION.substr(1);
}

// Reads the entire file as raw text. Returns false if the file cannot be opened.
[[nodiscard]] static bool ReadFileText(const std::string& path, std::string& text)
{
    const std::ifstream stream(path, std::ios::binary);
    if (!stream)
    {
        return false;
    }

    std::ostringstream buffer;
    buffer << stream.rdbuf();
    text = buffer.str();
    return true;
}

// Removes the root data_version attribute. That attribute is derived from the
// minRequiredVer / curSupportedVer / ImportProjectVersion constants, so leaving it in place
// would make the text comparison drift whenever any of those constants change.
static void StripDataVersion(pugi::xml_document& xml_doc)
{
    pugi::xml_node root = xml_doc.document_element();
    if (root)
    {
        std::ignore = root.remove_attribute("data_version");
    }
}

[[nodiscard]] static std::string NormalizeToString(pugi::xml_document& xml_doc)
{
    StripDataVersion(xml_doc);

    std::ostringstream stream;
    xml_doc.save(stream, "  ", pugi::format_indent_attributes);
    return stream.str();
}

// Writes the differences to a log file using the same layout as verify_codegen.cpp.
static void WriteDiffLog(const wxFileName& log_file, const wxFileName& golden_file,
                         const DiffResult& diff_result)
{
    const std::string golden_path = golden_file.GetFullPath().utf8_string();

    wxue::StringVector& cmdline_log = wxGetApp().get_CmdLineLog();
    cmdline_log.clear();
    cmdline_log.emplace_back(std::format("Import differences found for {}:", golden_path));
    cmdline_log.emplace_back("");

    cmdline_log.emplace_back(std::format("File: {}", golden_path));
    cmdline_log.emplace_back(std::string(80, '-'));

    for (const auto& line_diff: diff_result.left_lines)
    {
        switch (line_diff.type)
        {
            case DiffType::deleted:
                cmdline_log.emplace_back(std::format("- {}", line_diff.text));
                break;
            case DiffType::unchanged:
                cmdline_log.emplace_back(std::format("  {}", line_diff.text));
                break;
            case DiffType::modified:
                cmdline_log.emplace_back(std::format("! {}", line_diff.text));
                break;
            default:
                break;
        }
    }

    for (const auto& line_diff: diff_result.right_lines)
    {
        if (line_diff.type == DiffType::added)
        {
            cmdline_log.emplace_back(std::format("+ {}", line_diff.text));
        }
    }

    cmdline_log.emplace_back("");
    std::ignore = cmdline_log.WriteFile(log_file.GetFullPath());
}

static void WriteErrorLog(const wxFileName& log_file, const std::string& message)
{
    wxue::StringVector& cmdline_log = wxGetApp().get_CmdLineLog();
    cmdline_log.clear();
    cmdline_log.emplace_back(message);
    std::ignore = cmdline_log.WriteFile(log_file.GetFullPath());
}

[[nodiscard]] verify_codegen::VerifyResult VerifyImport(wxCmdLineParser& parser,
                                                        bool& is_project_loaded)
{
    const bool is_save_mode = (parser.FoundSwitch("save_import") == wxCMD_SWITCH_ON);

    if (parser.GetParamCount() == 0)
    {
        // A source file to import is required.
        return verify_codegen::VERIFY_INVALID;
    }

    const wxFileName source_file(parser.GetParam(0));
    if (!source_file.FileExists())
    {
        return verify_codegen::VERIFY_FILE_NOT_FOUND;
    }

    // The golden defaults to the source path with a .wxui extension; --golden overrides it.
    wxFileName golden_file = source_file;
    wxString golden_argument;
    if (parser.Found("golden", &golden_argument) && !golden_argument.empty())
    {
        golden_file.Assign(golden_argument);
    }
    else
    {
        golden_file.SetExt("wxui");
    }

    // A pure import leaves Project.get_ProjectFile() empty, so the log path is derived from the
    // source path rather than from the (empty) project file.
    wxFileName log_file = source_file;
    log_file.SetExt("log");

    const std::string source_path = source_file.GetFullPath().utf8_string();
    if (IsProjectFile(source_file))
    {
        is_project_loaded = Project.LoadProject(source_file.GetFullPath(), false);
    }
    else
    {
        // allow_ui is false: importing must work headless (CI has no display).
        is_project_loaded = Project.ImportProject(source_path, false);
    }

    if (!is_project_loaded || Project.get_ProjectNode() == nullptr)
    {
        WriteErrorLog(log_file, std::format("Unable to import project file: {}", source_path));
        return verify_codegen::VERIFY_FAILURE;
    }

    pugi::xml_document import_doc;
    Project.get_ProjectNode()->CreateDoc(import_doc);

    const std::string golden_path = golden_file.GetFullPath().utf8_string();

    if (is_save_mode)
    {
        if (!import_doc.save_file(golden_path.c_str(), "  ", pugi::format_indent_attributes))
        {
            WriteErrorLog(log_file, std::format("Unable to write golden file: {}", golden_path));
            return verify_codegen::VERIFY_FAILURE;
        }
        return verify_codegen::VERIFY_SUCCESS;
    }

    if (!golden_file.FileExists())
    {
        return verify_codegen::VERIFY_FILE_NOT_FOUND;
    }

    std::string golden_text;
    pugi::xml_document golden_doc;
    if (golden_doc.load_file(golden_path.c_str()))
    {
        golden_text = NormalizeToString(golden_doc);
    }
    else if (!ReadFileText(golden_path, golden_text))
    {
        // The golden exists but could not be read (for example, a permissions problem).
        return verify_codegen::VERIFY_FILE_NOT_FOUND;
    }

    // If the golden could not be parsed, golden_text holds the raw bytes. Comparing that against
    // the normalized output still yields a usable diff (and exit code 1), which is more helpful
    // than failing outright on a corrupt golden.

    const std::string generated_text = NormalizeToString(import_doc);

    const DiffResult diff_result = Diff::Compare(golden_text, generated_text, 3);
    if (!diff_result.has_differences)
    {
        return verify_codegen::VERIFY_SUCCESS;
    }

    WriteDiffLog(log_file, golden_file, diff_result);
    return verify_codegen::VERIFY_FAILURE;
}
