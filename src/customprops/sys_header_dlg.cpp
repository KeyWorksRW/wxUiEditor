/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for adding system header files to a list
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [08-20-2026]

#include <wx/arrstr.h>    // wxArrayString
#include <wx/config.h>    // wxConfig
#include <wx/dir.h>       // wxDir
#include <wx/dirdlg.h>    // wxDirDialog
#include <wx/filename.h>  // wxFileName
#include <wx/msgdlg.h>    // wxMessageBox
#include <wx/tokenzr.h>   // wxTokenizer
#include <wx/utils.h>     // wxBusyCursor

#include <algorithm>  // for std::find
#include <filesystem>

#include <tuple>  // for std::ignore

#include "sys_header_dlg.h"  // includes sys_header_dlg_base.h

#include "project_handler.h"                  // Project
#include "wxue_namespace/wxue_string.h"       // wxue::string
#include "wxue_namespace/wxue_view_vector.h"  // wxue::ViewVector

// Maximum number of root directories auto-populated before $INCLUDE expansion is trimmed
static constexpr size_t MAX_DEFAULT_ROOTS = 9;

// If this constructor is used, the caller must call Create(parent)
SysHeaderDlg::SysHeaderDlg() {}

SysHeaderDlg::SysHeaderDlg(wxWindow* parent)
{
    std::ignore = Create(parent);
}

void SysHeaderDlg::Initialize(NodeProperty* prop)
{
    m_prop = prop;
}

void SysHeaderDlg::OnInit(wxInitDialogEvent& event)
{
    ASSERT_MSG(m_prop, "m_prop is nullptr -- call Initialize()!");
    if (!m_prop)
    {
        event.Skip();
        return;
    }

    // If the property already contains header filenames, pre-check them once the
    // root directory reloads the file list.
    if (m_prop->HasValue())
    {
        const wxue::ViewVector list(m_prop->value(), ';');
        for (const auto& item: list)
        {
            if (!item.empty())
            {
                m_prechecked.emplace_back(item);
            }
        }
    }

    wxConfigBase* config = wxConfig::Get();
    config->SetPath("/preferences/sys_header_dlg");
    m_FileHistory.Load(*config);
    for (size_t idx = 0; idx < m_FileHistory.GetCount(); ++idx)
    {
        m_combo_root->AppendString(m_FileHistory.GetHistoryFile(idx));
    }

    if (m_combo_root->GetCount() < MAX_DEFAULT_ROOTS)
    {
        wxString wxwin;
        if (wxGetEnv("WXWIN", &wxwin) && !wxwin.empty())
        {
            if (wxwin.Last() != wxFILE_SEP_PATH)
            {
                wxwin += wxFILE_SEP_PATH;
            }
            wxFileName wxwin_path(wxwin);
            if (wxwin_path.IsOk() && wxwin_path.DirExists())
            {
                // check to see if the last directory is "include" and if not, append it
                if (!wxwin_path.GetDirs().empty() && wxwin_path.GetDirs().Last() != "include")
                {
                    wxwin_path.AppendDir("include");
                }
                m_combo_root->AppendString(wxwin_path.GetFullPath());
            }
        }
    }

    if (m_combo_root->GetCount() < MAX_DEFAULT_ROOTS)
    {
        m_combo_root->AppendString(Project.get_ProjectPath());

        // Add all the directories in the $INCLUDE environment variable
        wxString include_path;
        wxGetEnv("INCLUDE", &include_path);
        wxStringTokenizer include_paths(include_path, wxASCII_STR(",;"), wxTOKEN_STRTOK);
        while (include_paths.HasMoreTokens())
        {
            m_combo_root->AppendString(include_paths.GetNextToken());
        }
    }
    m_combo_root->SetSelection(0);
    wxCommandEvent dummy;
    OnRootSelected(dummy);
}

void SysHeaderDlg::OnRootSelected(wxCommandEvent& /* event unused */)
{
    m_check_list_files->Clear();

    const wxue::string root_path = m_combo_root->GetStringSelection().utf8_string();
    if (root_path.empty() || !root_path.dir_exists())
    {
        return;
    }

    try
    {
        // Fill wxCheckListBox with filenames containing .h, .hh, .hpp, or .hxx
        const wxBusyCursor busy_cursor;
        for (const auto& entry: std::filesystem::recursive_directory_iterator(
                 root_path.c_str(), std::filesystem::directory_options::skip_permission_denied))
        {
            if (entry.is_regular_file())
            {
                // Convert the native wide path to UTF-8 (wxString::utf8_string does the
                // conversion on all platforms; std::filesystem::u8string() would yield
                // char8_t which does not implicitly convert to wxue::string).
                const wxString path_wx(entry.path().wstring());
                wxue::string file = path_wx.utf8_string();
                if (file.has_extension(".h") || file.has_extension(".hh") ||
                    file.has_extension(".hpp") || file.has_extension(".hxx"))
                {
                    file.make_relative(root_path);
                    m_check_list_files->Append(file);
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        // In Release version, we simply stop adding filenames and return
        MSG_ERROR(e.what());
    }

    // Re-check files that were pre-selected from the property value. The list is
    // freshly populated above, so match by name (indices may differ between roots).
    if (!m_prechecked.empty())
    {
        for (size_t idx = 0; idx < m_check_list_files->GetCount(); ++idx)
        {
            const wxString item = m_check_list_files->GetString(idx);
            if (std::find(m_prechecked.begin(), m_prechecked.end(), item.utf8_string()) !=
                m_prechecked.end())
            {
                m_check_list_files->Check(idx);
            }
        }
    }

    // Keep the OK button enabled only when at least one header is checked
    SetButtonsEnableState();
}

// Called to add a path to the list of root directories
void SysHeaderDlg::SetButtonsEnableState(bool set_ok_btn)
{
    const bool has_checked = (m_check_list_files->GetCheckedItems(m_checked_indices) > 0);
    if (set_ok_btn)
    {
        FindWindow(GetAffirmativeId())->Enable(has_checked);
    }
}

// Called to add a path to the list of root directories
void SysHeaderDlg::OnDirectory(wxCommandEvent& /* event unused */)
{
    wxDirDialog dir_dialog(this, "Choose directory", wxEmptyString,
                           wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    dir_dialog.SetPath(Project.get_ProjectPath());
    if (dir_dialog.ShowModal() != wxID_OK)
    {
        return;
    }

    m_FileHistory.AddFileToHistory(dir_dialog.GetPath());
    wxConfigBase* config = wxConfig::Get();
    config->SetPath("/preferences/sys_header_dlg");
    m_FileHistory.Save(*config);

    m_combo_root->AppendString(dir_dialog.GetPath());
    m_combo_root->SetSelection(m_combo_root->GetCount() - 1);
    wxCommandEvent dummy;
    OnRootSelected(dummy);
}

void SysHeaderDlg::OnOK(wxCommandEvent& event)
{
    // TransferDataFromWindow will set the checked files in m_file_indexes
    if (!Validate() || !TransferDataFromWindow())
    {
        return;
    }

    // Assume that the user has specified the root directory in their build system, passing
    // that path to the compiler via /Ipath -- if there is a folder specified other than '.'
    // then prefix the filename with that folder.

    const wxue::string root_path = m_combo_root->GetStringSelection().utf8_string();
    m_value.clear();
    for (size_t idx = 0; idx < m_file_indexes.GetCount(); ++idx)
    {
        if (idx > 0)
        {
            m_value << ";";
        }
#if defined(_WIN32)
        wxue::string file = m_check_list_files->GetString(m_file_indexes[idx]).utf8_string();
        file.backslashestoforward();
        m_value << file;
#else
        m_value << m_check_list_files->GetString(m_file_indexes[idx]).utf8_string();
#endif
    }

    if (m_value.empty())
    {
        wxMessageBox("No header files were checked.", "Add System Header(s)",
                     wxOK | wxICON_INFORMATION, this);
        return;
    }

    EndModal(wxID_OK);
    event.Skip();  // This must be called for wxPersistenceManager to work
}
