/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to import one or more projects
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/dir.h>      // wxDir is a class for enumerating the files in a directory
#include <wx/dirdlg.h>   // wxDirDialog base class
#include <wx/filedlg.h>  // wxFileDialog base header

#include "import_dlg.h"  // auto-generated: import_base.h and import_base.cpp
#include "uifuncs.h"     // Miscellaneous functions for displaying UI

#include "../pugixml/pugixml.hpp"

ImportDlg::ImportDlg(wxWindow* parent) : ImportBase(parent) {}

void ImportDlg::OnInitDialog(wxInitDialogEvent& WXUNUSED(event))
{
    m_stdBtn->GetAffirmativeButton()->Disable();
    m_radio_wxFormBuilder->SetFocus();

    ttString cwd;
    cwd.assignCwd();
    m_static_cwd->SetLabel(cwd);

    wxDir dir;
    wxArrayString files;

    m_checkListProjects->Clear();

    if (m_radio_wxFormBuilder->GetValue())
        dir.GetAllFiles(".", &files, "*.fbp");
    else if (m_radio_wxSmith->GetValue())
        dir.GetAllFiles(".", &files, "*.wxs");
    else if (m_radio_wxGlade->GetValue())
        dir.GetAllFiles(".", &files, "*.wxg");
    else if (m_radio_XRC->GetValue())
        dir.GetAllFiles(".", &files, "*.xrc");
    else if (m_radio_WindowsResource->GetValue())
    {
        dir.GetAllFiles(".", &files, "*.rc");
        dir.GetAllFiles(".", &files, "*.dlg");
    }

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnCheckFiles(wxCommandEvent& WXUNUSED(event))
{
    m_stdBtn->GetAffirmativeButton()->Disable();

    for (unsigned int pos = 0; pos < m_checkListProjects->GetCount(); ++pos)
    {
        if (m_checkListProjects->IsChecked(pos))
        {
            m_stdBtn->GetAffirmativeButton()->Enable();
            return;
        }
    }
}

void ImportDlg::OnOK(wxCommandEvent& event)
{
    for (unsigned int pos = 0; pos < m_checkListProjects->GetCount(); ++pos)
    {
        if (m_checkListProjects->IsChecked(pos))
        {
            m_lstProjects.emplace_back(m_checkListProjects->GetString(pos));
        }
    }

    event.Skip();
}

void ImportDlg::OnDirectory(wxCommandEvent& WXUNUSED(event))
{
    wxDirDialog dlg(this, "Choose directory", wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dlg.ShowModal() != wxID_OK)
        return;

    ttlib::ChangeDir(dlg.GetPath().utf8_str().data());

    ttString cwd;
    cwd.assignCwd();
    m_static_cwd->SetLabel(cwd);

    wxDir dir;
    wxArrayString files;

    m_checkListProjects->Clear();

    if (m_radio_wxFormBuilder->GetValue())
        dir.GetAllFiles(".", &files, "*.fbp");
    else if (m_radio_wxSmith->GetValue())
        dir.GetAllFiles(".", &files, "*.wxs");
    else if (m_radio_wxGlade->GetValue())
        dir.GetAllFiles(".", &files, "*.wxg");
    else if (m_radio_XRC->GetValue())
        dir.GetAllFiles(".", &files, "*.xrc");
    else if (m_radio_WindowsResource->GetValue())
    {
        dir.GetAllFiles(".", &files, "*.rc");
        dir.GetAllFiles(".", &files, "*.dlg");
    }

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnFormBuilder(wxCommandEvent& WXUNUSED(event))
{
    m_checkListProjects->Clear();

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.fbp");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnWindowsResource(wxCommandEvent& WXUNUSED(event))
{
    m_checkListProjects->Clear();

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.rc");
    dir.GetAllFiles(".", &files, "*.dlg");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnWxSmith(wxCommandEvent& WXUNUSED(event))
{
    m_checkListProjects->Clear();

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.wxs");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnXRC(wxCommandEvent& WXUNUSED(event))
{
    m_checkListProjects->Clear();

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.xrc");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnWxGlade(wxCommandEvent& WXUNUSED(event))
{
    m_checkListProjects->Clear();

    wxDir dir;
    wxArrayString files;
    dir.GetAllFiles(".", &files, "*.wxg");

    if (files.size())
        m_checkListProjects->InsertItems(files, 0);
}

void ImportDlg::OnSelectAll(wxCommandEvent& WXUNUSED(event))
{
    for (unsigned int pos = 0; pos < m_checkListProjects->GetCount(); ++pos)
    {
        m_checkListProjects->Check(pos, true);
    }
    m_stdBtn->GetAffirmativeButton()->Enable();
}

void ImportDlg::OnSelectNone(wxCommandEvent& WXUNUSED(event))
{
    for (unsigned int pos = 0; pos < m_checkListProjects->GetCount(); ++pos)
    {
        m_checkListProjects->Check(pos, false);
    }
    m_stdBtn->GetAffirmativeButton()->Disable();
}
