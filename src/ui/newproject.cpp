/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to create a new project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/dir.h>      // wxDir is a class for enumerating the files in a directory
#include <wx/dirdlg.h>   // wxDirDialog base class
#include <wx/filedlg.h>  // wxFileDialog base header

#include "newproject.h"  // auto-generated: newproject_base.h and newproject_base.cpp
#include "uifuncs.h"     // Miscellaneous functions for displaying UI

#include "../pugixml/pugixml.hpp"

NewProjectDlg::NewProjectDlg(wxWindow* parent) : NewProjectBase(parent) {}

void NewProjectDlg::OnInitDialog(wxInitDialogEvent& WXUNUSED(event))
{
    if (m_checkBoxEmptyProject->IsChecked())
    {
        m_import_staticbox->GetStaticBox()->Enable(false);
    }
    else
    {
        m_import_staticbox->GetStaticBox()->Enable();
    }
}
void NewProjectDlg::OnOK(wxCommandEvent& event)
{
    if (!m_checkBoxEmptyProject->IsChecked())
    {
        for (size_t pos = 0; pos < m_checkListProjects->GetCount(); ++pos)
        {
            if (m_checkListProjects->IsChecked(pos))
            {
                m_lstProjects.emplace_back(m_checkListProjects->GetString(pos));
            }
        }
    }

    event.Skip();
}

void NewProjectDlg::OnEmptyProject(wxCommandEvent& WXUNUSED(event))
{
    if (m_checkBoxEmptyProject->IsChecked())
    {
        m_import_staticbox->GetStaticBox()->Enable(false);
    }
    else
    {
        m_import_staticbox->GetStaticBox()->Enable();
    }
}

void NewProjectDlg::OnDirectory(wxCommandEvent& WXUNUSED(event))
{
    wxDirDialog dlg(this, "Choose directory", wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dlg.ShowModal() != wxID_OK)
        return;

    ttlib::ChangeDir(dlg.GetPath().utf8_str().data());

    wxDir dir;
    wxArrayString files;

    m_checkListProjects->Clear();

    if (m_radioBtnFormBuilder->GetValue())
        dir.GetAllFiles(".", &files, "*.fbp");
    else if (m_radioBtnSmith->GetValue())
        dir.GetAllFiles(".", &files, "*.wxs");
    else if (m_radioBtnGlade->GetValue())
        dir.GetAllFiles(".", &files, "*.wxg");
    else if (m_radioBtnXrc->GetValue())
        dir.GetAllFiles(".", &files, "*.xrc");
    else if (m_radioBtnWinRes->GetValue())
    {
        dir.GetAllFiles(".", &files, "*.rc");
        dir.GetAllFiles(".", &files, "*.dlg");
    }

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void NewProjectDlg::OnFormBuilder(wxCommandEvent& WXUNUSED(event))
{
    m_checkListProjects->Clear();

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.fbp");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void NewProjectDlg::OnWindowsResource(wxCommandEvent& WXUNUSED(event))
{
    m_checkListProjects->Clear();

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.rc");
    dir.GetAllFiles(".", &files, "*.dlg");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void NewProjectDlg::OnWxSmith(wxCommandEvent& WXUNUSED(event))
{
    m_checkListProjects->Clear();

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.wxs");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void NewProjectDlg::OnXRC(wxCommandEvent& WXUNUSED(event))
{
    m_checkListProjects->Clear();

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.xrc");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void NewProjectDlg::OnWxGlade(wxCommandEvent& WXUNUSED(event))
{
    m_checkListProjects->Clear();

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.wxg");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void NewProjectDlg::OnSelectAll(wxCommandEvent& WXUNUSED(event))
{
    for (size_t pos = 0; pos < m_checkListProjects->GetCount(); ++pos)
    {
        m_checkListProjects->Check(pos, true);
    }
}

void NewProjectDlg::OnSelectNone(wxCommandEvent& WXUNUSED(event))
{
    for (size_t pos = 0; pos < m_checkListProjects->GetCount(); ++pos)
    {
        m_checkListProjects->Check(pos, false);
    }
}
