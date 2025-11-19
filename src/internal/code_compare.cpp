/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code Generation Comparison
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/dir.h>  // wxDir is a class for enumerating the files in a directory

#include "code_compare.h"

#include "gen_common.h"       // Common code generation functions
#include "gen_results.h"      // Code generation file writing functions
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class

#include "pugixml.hpp"

// clang-format on

CodeCompare::~CodeCompare()
{
    wxArrayString files;

    // Some project files will be placed in a subdirectory which will be our current cwd.
    // However, the actual generated files can be pretty much anywhere. In the following, we
    // check to see if the parent directory is named "src" and if so, we change to the parent
    // directory. This allows us to find the generated files no matter where they are located,
    // or at least as long as they were generated under the src/ directory.
    tt_cwd cwd(tt_cwd::restore);
    cwd.remove_filename();
    if (cwd.size() && (cwd.back() == '\\' || cwd.back() == '/'))
    {
        cwd.pop_back();
    }
    if (cwd.filename() == "src")
    {
        cwd.ChangeDir("..");
    }

    wxDir::GetAllFiles(".", &files, "~wxue_**.*");

    for (auto& iter: files)
    {
        // ~wxue_.WinMerge will often be added to this list, but deleted before we start
        // processing, so check first
        if (wxFileExists(iter))
        {
            wxRemoveFile(iter);
        }
    }

    if (Project.HasValue(prop_base_directory))
    {
        wxDir::GetAllFiles(Project.as_string(prop_base_directory).make_wxString(), &files,
                           "~wxue_**.*");

        for (auto& iter: files)
        {
            // ~wxue_.WinMerge will often be added to this list, but deleted before we start
            // processing, so check first
            if (wxFileExists(iter))
            {
                wxRemoveFile(iter);
            }
        }
    }
}

void CodeCompare::OnInit(wxInitDialogEvent& /* event */)
{
    GenLang language = Project.get_CodePreference(wxGetFrame().getSelectedNode());
    wxCommandEvent dummy;
    switch (language)
    {
        case GEN_LANG_PYTHON:
            m_radio_python->SetValue(true);
            OnPython(dummy);
            break;
        case GEN_LANG_RUBY:
            m_radio_ruby->SetValue(true);
            OnRuby(dummy);
            break;
        case GEN_LANG_CPLUSPLUS:
        default:
            m_radio_cplusplus->SetValue(true);
            OnCPlusPlus(dummy);
            break;
    }
}

void CodeCompare::OnRadioButton(GenLang language)
{
    GenResults results;

    m_class_list.clear();
    m_list_changes->Clear();
    m_btn->Enable(false);

    bool result = false;
    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
            result = GenerateLanguageFiles(results, &m_class_list, GEN_LANG_CPLUSPLUS);
            break;
        case GEN_LANG_PERL:
            result = GenerateLanguageFiles(results, &m_class_list, GEN_LANG_PERL);
            break;
        case GEN_LANG_PYTHON:
            result = GenerateLanguageFiles(results, &m_class_list, GEN_LANG_PYTHON);
            break;
        case GEN_LANG_RUBY:
            result = GenerateLanguageFiles(results, &m_class_list, GEN_LANG_RUBY);
            break;
        case GEN_LANG_XRC:
            result = GenerateLanguageFiles(results, &m_class_list, GEN_LANG_XRC);
            break;

        default:
            FAIL_MSG(tt_string() << "Unknown language: " << language);
            break;
    }

    if (result)
    {
        for (auto& iter: m_class_list)
        {
            m_list_changes->AppendString(wxString::FromUTF8(iter));
        }
        m_btn->Enable();
    }
}

void CodeCompare::OnCPlusPlus(wxCommandEvent& /* event */)
{
    OnRadioButton(GEN_LANG_CPLUSPLUS);
}

void CodeCompare::OnPython(wxCommandEvent& /* event */)
{
    OnRadioButton(GEN_LANG_PYTHON);
}

void CodeCompare::OnRuby(wxCommandEvent& /* event unused */)
{
    OnRadioButton(GEN_LANG_RUBY);
}

void CodeCompare::OnPerl(wxCommandEvent& /* event */)
{
    OnRadioButton(GEN_LANG_PERL);
}

void CodeCompare::OnXRC(wxCommandEvent& /* event */)
{
    OnRadioButton(GEN_LANG_XRC);
}

// clang-format off

#if defined(_WIN32)

#include <windows.h>

// clang-format on

// Converts all text to UTF16 before calling ShellExecuteW(...)
HINSTANCE winShellRun(std::string_view filename, std::string_view args, std::string_view directory,
                      INT nShow = SW_SHOWNORMAL, HWND hwndParent = NULL);

HINSTANCE winShellRun(std::string_view filename, std::string_view args, std::string_view dir,
                      INT nShow, HWND hwndParent)
{
    std::wstring name16;
    tt::utf8to16(filename, name16);
    std::wstring args16;
    tt::utf8to16(args, args16);
    std::wstring dir16;
    tt::utf8to16(dir, dir16);

    return ShellExecuteW(hwndParent, NULL, name16.c_str(), args16.c_str(), dir16.c_str(), nShow);
}

void CodeCompare::OnWinMerge(wxCommandEvent& /* event */)
{
    pugi::xml_document doc;
    auto root = doc.append_child("project");

    GenLang language = GEN_LANG_CPLUSPLUS;
    if (m_radio_python->GetValue())
    {
        language = GEN_LANG_PYTHON;
    }
    else if (m_radio_ruby->GetValue())
    {
        language = GEN_LANG_RUBY;
    }

    GenerateTmpFiles(m_class_list, root, language);

    doc.save_file("~wxue_.WinMerge");

    // WinMergeU.exe typically is not in the system PATH, and as such, wxExecute won't be able to
    // find it. ShellExecute will find the registered location for the program.

    tt_cwd cwd;

    // /e -- terminate with escape
    // /u -- don't add files to MRU
    winShellRun("WinMergeU.exe", "/e /u ~wxue_.WinMerge", cwd.c_str());
}

#else

void CodeCompare::OnWinMerge(wxCommandEvent& /* event */)
{
    wxMessageBox("WinMerge is only supported on Windows.", "WinMerge Not Found",
                 wxOK | wxICON_INFORMATION);
}

#endif  // _WIN32
