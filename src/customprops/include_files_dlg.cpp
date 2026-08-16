/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for editing a list of include files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [08-16-2026]

#include <tuple>  // for std::ignore

#include <wx/filedlg.h>  // wxFileDialog

#include "include_files_dlg.h"  // auto-generated: include_files_dlg_base.h and include_files_dlg_base.cpp

#include "project_handler.h"                    // ProjectHandler class
#include "sys_header_dlg.h"                     // SysHeaderDlg class
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector
#include "wxue_namespace/wxue_view_vector.h"    // wxue::ViewVector

// If this constructor is used, the caller must call Create(parent)
IncludeFilesDialog::IncludeFilesDialog() {}

IncludeFilesDialog::IncludeFilesDialog(wxWindow* parent)
{
    std::ignore = Create(parent);
}

void IncludeFilesDialog::Initialize(NodeProperty* prop)
{
    m_prop = prop;
    if (m_prop->isProp(prop_relative_require_list))
    {
        m_language = GenLang::ruby;
    }
    else if (m_prop->isProp(prop_python_import_list))
    {
        m_language = GenLang::python;
    }
    else
    {
        m_language = GenLang::cplusplus;
    }
}

void IncludeFilesDialog::SetButtonsEnableState(bool set_ok_btn)
{
    const int selection = m_listbox->GetSelection();
    m_btn_remove->Enable(selection != wxNOT_FOUND);
    m_btn_move_up->Enable(selection > 0);
    m_btn_move_down->Enable(selection != wxNOT_FOUND &&
                            static_cast<unsigned int>(selection) < m_listbox->GetCount() - 1);
    m_btn_sort->Enable(m_listbox->GetCount() > 1);
    if (set_ok_btn)
    {
        FindWindow(GetAffirmativeId())->Enable(m_listbox->GetCount() > 0);
    }
}

void IncludeFilesDialog::OnInit([[maybe_unused]] wxInitDialogEvent& event)
{
    ASSERT_MSG(m_prop, "m_prop is nullptr -- call Initialize()!");
    if (!m_prop)
    {
        event.Skip();
        return;
    }

    if (m_prop->isProp(prop_relative_require_list))
    {
        m_staticText->SetLabel("These files will be loaded using relative require statements.");
    }
    else if (m_prop->isProp(prop_python_import_list))
    {
        m_staticText->SetLabel("These files will be loading using import statements.");
    }

    if (m_prop->isProp(prop_system_hdr_includes) || m_prop->isProp(prop_system_src_includes))
    {
        SetTitle("System Header Files");
    }

    if (m_prop->HasValue())
    {
        wxue::ViewVector list;
        list.SetString(std::string_view { m_prop->value() }, ';');
        for (auto& iter: list)
        {
            if (!iter.empty())
            {
                m_listbox->Append(iter.wx());
            }
        }
        if (m_listbox->GetCount())
        {
            m_listbox->SetSelection(0);
        }
    }

    SetButtonsEnableState();
}

void IncludeFilesDialog::OnAdd([[maybe_unused]] wxCommandEvent& event)
{
    if (m_prop->isProp(prop_system_src_includes) || m_prop->isProp(prop_system_hdr_includes))
    {
        SysHeaderDlg dialog(this);
        dialog.Initialize(m_prop);
        if (dialog.ShowModal() == wxID_OK)
        {
            const wxue::StringVector files(dialog.GetResults(), ';');
            for (auto& iter: files)
            {
                const wxString item = iter.wx();
                if (m_listbox->FindString(item) == wxNOT_FOUND)
                {
                    m_listbox->Append(item);
                }
            }
            SetButtonsEnableState();
        }
        return;
    }

    wxue::string path;
    wxue::string cur_file;
    if (m_prop->isProp(prop_local_hdr_includes) || m_prop->isProp(prop_local_src_includes) ||
        m_prop->isProp(prop_project_src_includes) || m_prop->isProp(prop_relative_require_list) ||
        m_prop->isProp(prop_python_import_list))
    {
        // prop_project_src_includes lives on the project node, which is neither a Form nor a
        // Folder -- GetOutputPath() would trip its assert. Project-level paths are relative to
        // the project directory.
        if (m_prop->isProp(prop_project_src_includes))
        {
            path = Project.get_ProjectPath();
        }
        else
        {
            Node* form = m_prop->getNode();
            auto [output_path, has_base_file] = Project.GetOutputPath(form, m_language);
            path = std::move(output_path);
            if (has_base_file)
            {
                cur_file = path.filename();

                // We only got the node's filename in case it includes a path. We don't want the
                // filename portion as part of the path.
                path.remove_filename();
            }
        }

        if (path.empty())
        {
            path = Project.get_ProjectPath();
        }
    }

    wxue::string title;
    wxue::string filter;
    if (m_prop->isProp(prop_python_import_list))
    {
        title = "Import Python File";
        filter = "Python Files|*.py";
    }
    else if (m_prop->isProp(prop_relative_require_list))
    {
        title = "Require Ruby File";
        filter = "Ruby Files|*.rb;*.rbw";
    }
    else
    {
        title = "Include Header File";
        filter = "Header Files|*.;*.h;*.hh;*.hpp;*.hxx";
    }

    if (!path.empty() && path.back() == '/')
    {
        path.pop_back();
    }

    const wxue::SaveCwd saved_cwd(wxue::restore_cwd);
    wxFileDialog dialog(this, title.wx(), path.wx(), wxEmptyString, filter.wx(),
                        wxFD_OPEN | wxFD_CHANGE_DIR);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxue::string filename = dialog.GetPath().utf8_string();
        if (filename.empty())
        {
            return;
        }
        filename.make_relative(path);
        filename.backslashestoforward();
        if (filename == cur_file)
        {
            wxMessageBox("You cannot add the current file to the list.", title.wx(), wxOK, this);
            return;
        }
        if (m_listbox->FindString(filename.wx()) == wxNOT_FOUND)
        {
            m_listbox->Append(filename.wx());
        }
        SetButtonsEnableState();
    }
}

void IncludeFilesDialog::OnItemSelected([[maybe_unused]] wxCommandEvent& event)
{
    SetButtonsEnableState();
}

void IncludeFilesDialog::OnMoveUp([[maybe_unused]] wxCommandEvent& event)
{
    // Move the currently selected item up one position in the listbox
    const int selection = m_listbox->GetSelection();
    if (selection == wxNOT_FOUND || selection == 0)
    {
        return;
    }
    const wxString stringTmp = m_listbox->GetString(selection - 1);
    m_listbox->SetString(selection - 1, m_listbox->GetString(selection));
    m_listbox->SetString(selection, stringTmp);
    m_listbox->SetSelection(selection - 1);
    SetButtonsEnableState();
}

void IncludeFilesDialog::OnMoveDown([[maybe_unused]] wxCommandEvent& event)
{
    // Move the currently selected item down one position in the listbox
    const int selection = m_listbox->GetSelection();
    if (selection == wxNOT_FOUND ||
        static_cast<unsigned int>(selection) == m_listbox->GetCount() - 1)
    {
        return;
    }
    const wxString stringTmp = m_listbox->GetString(selection + 1);
    m_listbox->SetString(selection + 1, m_listbox->GetString(selection));
    m_listbox->SetString(selection, stringTmp);
    m_listbox->SetSelection(selection + 1);
    SetButtonsEnableState();
}

void IncludeFilesDialog::OnRemove([[maybe_unused]] wxCommandEvent& event)
{
    if (const int selection = m_listbox->GetSelection(); selection != wxNOT_FOUND)
    {
        m_listbox->Delete(selection);
        if (static_cast<unsigned int>(selection) < m_listbox->GetCount())
        {
            m_listbox->SetSelection(selection);
        }
        // Don't disable the OK button, but do disable the other buttons
        SetButtonsEnableState(false);
    }
}

void IncludeFilesDialog::OnSort([[maybe_unused]] wxCommandEvent& event)
{
    if (m_listbox->GetCount() < 2)
    {
        return;
    }

    std::vector<std::string> items;
    items.reserve(m_listbox->GetCount());
    for (unsigned int i = 0; i < m_listbox->GetCount(); ++i)
    {
        items.push_back(m_listbox->GetString(i).utf8_string());
    }
    std::sort(items.begin(), items.end(),
              [](const std::string& lhs, const std::string& rhs)
              {
                  return (wxue::string_view(lhs).comparei(rhs) < 0);
              });
    m_listbox->Clear();
    for (auto& item: items)
    {
        m_listbox->Append(wxString::FromUTF8(item));
    }
    m_listbox->SetSelection(0);
    SetButtonsEnableState();
}

void IncludeFilesDialog::OnOK(wxCommandEvent& event)
{
    if (m_listbox->GetCount() > 0)
    {
        m_value.clear();
        for (unsigned int i = 0; i < m_listbox->GetCount(); ++i)
        {
            if (!m_value.empty())
            {
                m_value += ";";
            }
            m_value += m_listbox->GetString(i);
        }
    }

    event.Skip();
}
