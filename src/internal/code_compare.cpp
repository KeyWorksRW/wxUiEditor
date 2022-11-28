/////////////////////////////////////////////////////////////////////////////
// Purpose:   Compare code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#if !defined(INTERNAL_TESTING)
    #error "INTERNAL_TESTING must be defined if you include this moduel!"
#endif

#include <wx/dir.h>  // wxDir is a class for enumerating the files in a directory

#include "ttcwd_wx.h"  // cwd -- Class for storing and optionally restoring the current directory

#include "internal/code_compare_base.h"

#include "gen_base.h"       // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "generate_dlg.h"   // GenerateDlg -- Dialog for choosing and generating specific language file(s)
#include "node.h"           // Node class
#include "project_class.h"  // Project class

#include "pugixml.hpp"

// clang-format on

CodeCompare::~CodeCompare()
{
    wxDir dir;
    wxArrayString files;

    dir.GetAllFiles(".", &files, "~wxue_**.*");

    for (auto& iter: files)
    {
        wxRemoveFile(iter);
    }

    auto project = GetProject();
    if (project->HasValue(prop_base_directory))
    {
        dir.GetAllFiles(project->prop_as_wxString(prop_base_directory), &files, "~wxue_**.*");

        for (auto& iter: files)
        {
            // ~wxue_.WinMerge will often be added to this list, but deleted before we start processing, so check first
            if (wxFileExists(iter))
            {
                wxRemoveFile(iter);
            }
        }
    }
}

void CodeCompare::OnInit(wxInitDialogEvent& /* event */)
{
    GenResults results;

    if (GenerateCodeFiles(results, &m_class_list))
    {
        for (auto& iter: m_class_list)
        {
            m_list_changes->AppendString(iter.wx_str());
        }
        m_btn->Enable();
    }
}

void CodeCompare::OnCPlusPlus(wxCommandEvent& /* event */)
{
    GenResults results;

    m_class_list.clear();
    m_list_changes->Clear();

    if (GenerateCodeFiles(results, &m_class_list))
    {
        for (auto& iter: m_class_list)
        {
            m_list_changes->AppendString(iter.wx_str());
        }
        m_btn->Enable();
    }
}

void CodeCompare::OnLua(wxCommandEvent& /* event */)
{
    GenResults results;

    m_class_list.clear();
    m_list_changes->Clear();

    if (GenerateLuaFiles(results, &m_class_list))
    {
        for (auto& iter: m_class_list)
        {
            m_list_changes->AppendString(iter.wx_str());
        }
        m_btn->Enable();
    }
}

void CodeCompare::OnPython(wxCommandEvent& /* event */)
{
    GenResults results;

    m_class_list.clear();
    m_list_changes->Clear();

    if (GeneratePythonFiles(results, &m_class_list))
    {
        for (auto& iter: m_class_list)
        {
            m_list_changes->AppendString(iter.wx_str());
        }
        m_btn->Enable();
    }
}

// clang-format off

#if defined(_WIN32)

#include <windows.h>

// clang-format on

// Converts all text to UTF16 before calling ShellExecuteW(...)
HINSTANCE winShellRun(std::string_view filename, std::string_view args, std::string_view directory,
                      INT nShow = SW_SHOWNORMAL, HWND hwndParent = NULL);

HINSTANCE winShellRun(std::string_view filename, std::string_view args, std::string_view dir, INT nShow, HWND hwndParent)
{
    std::wstring name16;
    ttlib::utf8to16(filename, name16);
    std::wstring args16;
    ttlib::utf8to16(args, args16);
    std::wstring dir16;
    ttlib::utf8to16(dir, dir16);

    return ShellExecuteW(hwndParent, NULL, name16.c_str(), args16.c_str(), dir16.c_str(), nShow);
}

void CodeCompare::OnWinMerge(wxCommandEvent& /* event */)
{
    pugi::xml_document doc;
    auto root = doc.append_child("project");

    int language = GEN_LANG_CPLUSPLUS;
    if (m_radio_python->GetValue())
        language = GEN_LANG_PYTHON;
    else if (m_radio_Lua->GetValue())
        language = GEN_LANG_LUA;

    GenerateTmpFiles(m_class_list, root, language);

    doc.save_file("~wxue_.WinMerge");

    // WinMergeU.exe typically is not in the system PATH, and as such, wxExecute won't be able to find it. ShellExecute will
    // find the registered location for the program.

    ttlib::cwd cwd;

    // /e -- terminate with escape
    // /u -- don't add files to MRU
    winShellRun("WinMergeU.exe", "/e /u ~wxue_.WinMerge", cwd.sub_cstr().c_str());
}

#endif  // _WIN32
