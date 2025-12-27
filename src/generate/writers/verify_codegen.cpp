//////////////////////////////////////////////////////////////////////////
// Purpose:   Verify that code generation did not change
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/**
 * @file verify_codegen.cpp
 * @brief Verify that code generation did not change
 *
 * This code is called from the mainapp command line to verify that code generation for one or more
 * languages has not changed. This should be called after refactoring, design changes, or any other
 * changes that should *NOT* affect code generation.
 *
 * @author Ralph Walden
 * @date 2025-10-19
 */

#include <array>
#include <filesystem>
#include <format>

#include <wx/dir.h>
#include <wx/filename.h>

#include "verify_codegen.h"

#include "../../internal/compare/code_compare.h"  // CodeCompare class
#include "mainapp.h"                              // App -- Main application class
#include "project_handler.h"                      // ProjectHandler class
#include "utils.h"                                // Utility functions that work with properties

namespace
{
    [[nodiscard]] auto ParseLanguageSwitch(wxCmdLineParser& parser) -> size_t
    {
        constexpr auto switches = std::to_array<std::pair<std::string_view, size_t>>({
            { "verify_cpp", GEN_LANG_CPLUSPLUS },
            { "verify_perl", GEN_LANG_PERL },
            { "verify_python", GEN_LANG_PYTHON },
            { "verify_ruby", GEN_LANG_RUBY },
            { "verify_all", GEN_LANG_CPLUSPLUS | GEN_LANG_PERL | GEN_LANG_PYTHON | GEN_LANG_RUBY },
        });

        for (const auto& [switch_name, lang]: switches)
        {
            if (parser.FoundSwitch(wxString(switch_name)) == wxCMD_SWITCH_ON)
            {
                return lang;
            }
        }
        return GEN_LANG_NONE;
    }

    [[nodiscard]] auto FindProjectFile(wxString& filename) -> verify_codegen::VerifyResult
    {
        wxDir dir;
        dir.Open("./");
        if (!dir.GetFirst(&filename, "*" + std::string(PROJECT_FILE_EXTENSION), wxDIR_FILES))
        {
            wxMessageBox("No project file found in current directory. Filenane is required if "
                         "switch is used.",
                         "Command-line Switch Error", wxOK | wxICON_ERROR);
            return verify_codegen::VERIFY_INVALID;
        }
        return verify_codegen::VERIFY_SUCCESS;
    }

    [[nodiscard]] auto LoadProjectFile(wxFileName& project_file, size_t generate_type,
                                       bool& is_project_loaded) -> verify_codegen::VerifyResult
    {
        if (!project_file.FileExists())
        {
            if (generate_type != GEN_LANG_NONE)
            {
                wxMessageBox("Unable to find project file: " +
                                 project_file.GetFullPath().utf8_string(),
                             "Verify");
                return verify_codegen::VERIFY_FILE_NOT_FOUND;
            }
            return verify_codegen::VERIFY_SUCCESS;
        }

        wxGetApp().set_Generating(true);
        if (project_file.GetExt().IsSameAs("wxui", false) ||
            project_file.GetExt().IsSameAs("wxue", false))
        {
            is_project_loaded =
                Project.LoadProject(project_file.GetFullPath(), generate_type == GEN_LANG_NONE);
        }
        else
        {
            is_project_loaded = Project.ImportProject(project_file.GetFullPath().ToStdString(),
                                                      generate_type == GEN_LANG_NONE);
        }

        if (generate_type != GEN_LANG_NONE && !is_project_loaded)
        {
            wxMessageBox("Unable to load project file: " + project_file.GetFullPath().utf8_string(),
                         "Verify");
            return verify_codegen::VERIFY_FAILURE;
        }

        return verify_codegen::VERIFY_SUCCESS;
    }

    [[nodiscard]] auto VerifyLanguageGeneration(GenLang language, size_t generate_type)
        -> verify_codegen::VerifyResult
    {
        if (!(generate_type & language))
        {
            return verify_codegen::VERIFY_SUCCESS;
        }

        // Use CodeCompare to collect diffs between generated code and disk files
        auto diffs = CodeCompare::CollectFileDiffsForLanguage(language);

        if (diffs.empty())
        {
            return verify_codegen::VERIFY_SUCCESS;
        }

        // Write differences to log
        auto& log = wxGetApp().get_CmdLineLog();
        log.clear();
        log.emplace_back(
            std::format("Code generation differences found for {}:", GenLangToString(language)));
        log.emplace_back("");

        for (const auto& diff: diffs)
        {
            log.emplace_back(std::format("File: {}", diff.filename));
            log.emplace_back(std::string(80, '-'));

            // Write diff details from left side (original/disk)
            for (const auto& line_diff: diff.diff_result.left_lines)
            {
                switch (line_diff.type)
                {
                    case DiffType::deleted:
                        log.emplace_back(std::format("- {}", line_diff.text));
                        break;
                    case DiffType::unchanged:
                        log.emplace_back(std::format("  {}", line_diff.text));
                        break;
                    case DiffType::modified:
                        log.emplace_back(std::format("! {}", line_diff.text));
                        break;
                    default:
                        break;
                }
            }

            // Write diff details from right side (new/generated)
            for (const auto& line_diff: diff.diff_result.right_lines)
            {
                if (line_diff.type == DiffType::added)
                {
                    log.emplace_back(std::format("+ {}", line_diff.text));
                }
            }

            log.emplace_back("");
        }

        // Write log file based on project filename
        std::filesystem::path log_file(Project.get_ProjectFile().ToStdString());
        log_file.replace_extension(".log");
        log.WriteFile(log_file.string());

#if defined(MESSAGE_BOX)
        std::string msg =
            std::format("Code generation altered {} file(s) for {}. See {} for details.",
                        diffs.size(), GenLangToString(language), log_file.filename().string());
        wxMessageBox(msg, "Verify");
#endif

        return verify_codegen::VERIFY_FAILURE;
    }
}  // namespace

[[nodiscard]] auto VerifyCodeGen(wxCmdLineParser& parser, bool& is_project_loaded)
    -> verify_codegen::VerifyResult
{
    if ((parser.GetParamCount() || parser.GetArguments().size()) &&
        parser.FoundSwitch("verbose") == wxCMD_SWITCH_ON)
    {
        wxGetApp().set_VerboseCodeGen(true);
    }

    wxString filename;
    if (parser.GetParamCount())
    {
        filename = parser.GetParam(0);
    }

    size_t generate_type = ParseLanguageSwitch(parser);
    if (generate_type == GEN_LANG_NONE)
    {
        wxMessageBox("Unknown Language", "Verify");
        return verify_codegen::VERIFY_INVALID;
    }

    // If no project filename was given on the command line, call FindProjectFile() to look
    // for a .wxui file in the current directory.
    if (generate_type != GEN_LANG_NONE && filename.empty())
    {
        auto result = FindProjectFile(filename);
        if (result != verify_codegen::VERIFY_SUCCESS)
        {
            return result;
        }
    }

    wxFileName project_file = filename;
    auto result = LoadProjectFile(project_file, generate_type, is_project_loaded);
    if (result != verify_codegen::VERIFY_SUCCESS)
    {
        return result;
    }

    constexpr auto languages = std::to_array<GenLang>({
        GEN_LANG_CPLUSPLUS,
        GEN_LANG_PERL,
        GEN_LANG_PYTHON,
        GEN_LANG_RUBY,
    });

    // Testing menu is disabled here so that GenerateLanguageFiles() does not start/end a timer.
    DisableTestingMenuScope scope;

    for (auto lang: languages)
    {
        result = VerifyLanguageGeneration(lang, generate_type);
        if (result != verify_codegen::VERIFY_SUCCESS)
        {
            return result;
        }
    }

    return verify_codegen::VERIFY_SUCCESS;
}

// Testing menu is automatically restored in the DisableTestingMenuScope destructor
