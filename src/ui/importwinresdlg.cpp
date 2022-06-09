/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for Importing a Windows resource file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/dir.h>

#include "tttextfile.h"  // textfile -- Classes for reading and writing line-oriented files

#include "importwinres_base.h"  // auto-generated: importwinres_base.h and importwinres_base.cpp

#include "mainapp.h"  // App -- App class

void ImportWinRes::OnInit(wxInitDialogEvent& WXUNUSED(event))
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

void ImportWinRes::ReadRcFile()
{
    m_rcFilename.utf(m_fileResource->GetPath().wx_str());
    ttlib::textfile rc_file;
    if (!rc_file.ReadFile(m_rcFilename))
    {
        wxMessageBox(wxString("Unable to read the file ") << m_fileResource->GetPath());
        return;
    }

    for (auto& iter: rc_file)
    {
        if (iter.empty() || !ttlib::is_alpha(iter[0]))
            continue;

        auto type = iter.view_stepover();

        // If there is a DESIGNINFO section, there may be a DIALOG specified for APSTUDIO to used -- however that dialog may
        // not actually exist. So instead, we look for a trailing space which should indicate the statement is followed by
        // dimensions.

        if (type.starts_with("DIALOG ") || type.starts_with("DIALOGEX ") || type.starts_with("MENU"))
        {
            auto pos_end = iter.find(' ');
            auto name = iter.substr(0, pos_end);
            if (ttlib::is_alnum(name[0]) || name[0] == '"')
            {
                auto sel = m_checkListResUI->Append(name);
                m_checkListResUI->Check(sel);
            }
        }
    }
}

void ImportWinRes::OnResourceFile(wxFileDirPickerEvent& WXUNUSED(event))
{
    ReadRcFile();
}

void ImportWinRes::OnSelectAll(wxCommandEvent& WXUNUSED(event))
{
    auto count = m_checkListResUI->GetCount();
    for (unsigned int pos = 0; pos < count; ++pos)
    {
        m_checkListResUI->Check(pos);
    }
}

void ImportWinRes::OnClearAll(wxCommandEvent& WXUNUSED(event))
{
    auto count = m_checkListResUI->GetCount();
    for (unsigned int pos = 0; pos < count; ++pos)
    {
        m_checkListResUI->Check(pos, false);
    }
}

void ImportWinRes::OnOk(wxCommandEvent& event)
{
    auto count = m_checkListResUI->GetCount();
    for (unsigned int pos = 0; pos < count; ++pos)
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
