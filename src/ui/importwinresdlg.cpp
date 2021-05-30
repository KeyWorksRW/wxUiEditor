/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for Importing a Windows resource file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/dir.h>

#include "tttextfile.h"  // textfile -- Classes for reading and writing line-oriented files

#include "importwinresdlg.h"  // auto-generated: importwinres_base.h and importwinres_base.cpp

#include "mainapp.h"  // App -- App class
#include "uifuncs.h"  // Miscellaneous functions for displaying UI

ImportWinResDlg::ImportWinResDlg(wxWindow* parent, ttString filename) : ImportWinResBase(parent)
{
    if (filename.size())
        m_rcFilename.utf(filename.wx_str());
}

void ImportWinResDlg::OnInit(wxInitDialogEvent& WXUNUSED(event))
{
    if (m_rcFilename.empty())
    {
        wxDir dir(wxGetCwd());
        wxString filename;
        if (dir.GetFirst(&filename, "*.rc"))
        {
            m_fileResource->SetPath(filename);
            ReadRcFile();
        }
    }
    else
    {
        m_fileResource->SetPath(m_rcFilename.wx_str());
        ReadRcFile();
    }
}

void ImportWinResDlg::ReadRcFile()
{
    m_rcFilename.utf(m_fileResource->GetPath().wx_str());
    ttlib::textfile rc_file;
    if (!rc_file.ReadFile(m_rcFilename))
    {
        appMsgBox(_tt(strIdCantReadFile) + m_rcFilename);
        return;
    }

    for (auto& iter: rc_file)
    {
        if (iter.contains(" DIALOG"))
        {
            auto pos_end = iter.find(' ');
            auto name = iter.substr(0, pos_end);
            if (ttlib::is_alpha(name[0]))
            {
                auto sel = m_checkListResUI->Append(name);
                m_checkListResUI->Check(sel);
            }
        }
    }
}

void ImportWinResDlg::OnResourceFile(wxFileDirPickerEvent& WXUNUSED(event))
{
    ReadRcFile();
}

void ImportWinResDlg::OnSelectAll(wxCommandEvent& WXUNUSED(event))
{
    auto count = m_checkListResUI->GetCount();
    for (size_t pos = 0; pos < count; ++pos)
    {
        m_checkListResUI->Check(pos);
    }
}

void ImportWinResDlg::OnClearAll(wxCommandEvent& WXUNUSED(event))
{
    auto count = m_checkListResUI->GetCount();
    for (size_t pos = 0; pos < count; ++pos)
    {
        m_checkListResUI->Check(pos, false);
    }
}

void ImportWinResDlg::OnOk(wxCommandEvent& event)
{
    auto count = m_checkListResUI->GetCount();
    for (size_t pos = 0; pos < count; ++pos)
    {
        if (m_checkListResUI->IsChecked(pos))
        {
            auto& name = m_dialogs.emplace_back();
            name << m_checkListResUI->GetString(pos).wx_str();
        }
    }

    m_rcFilename.utf(m_fileResource->GetTextCtrlValue().wx_str());
    event.Skip();
}
