///////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor - see https://github.com/KeyWorksRW/wxUiEditor/
//
// Do not edit any code above the "End of generated code" comment block.
// Any changes before that block will be lost if it is re-generated!
///////////////////////////////////////////////////////////////////////////////

// clang-format off

#include <wx/sizer.h>

#include "import_winres_dlg.h"

bool ImportWinRes::Create(wxWindow* parent, wxWindowID id, const wxString& title,
        const wxPoint& pos, const wxSize& size, long style, const wxString &name)
{
    if (!wxDialog::Create(parent, id, title, pos, size, style, name))
        return false;

    auto* parent_sizer = new wxBoxSizer(wxVERTICAL);

    auto* box_sizer4 = new wxBoxSizer(wxVERTICAL);

    m_staticResFile = new wxStaticText(this, wxID_ANY, "&Resource File:");
    box_sizer4->Add(m_staticResFile, wxSizerFlags().Border(wxLEFT|wxRIGHT|wxTOP, wxSizerFlags::GetDefaultBorder()));

    m_fileResource = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, wxFileSelectorPromptStr,
        "Resource Files|*.rc;*.dlg||", wxDefaultPosition, wxDefaultSize,
        wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL);
    m_fileResource->SetMinSize(wxSize(240, -1));
    box_sizer4->Add(m_fileResource, wxSizerFlags().Expand().Border(wxALL));

    parent_sizer->Add(box_sizer4, wxSizerFlags(1).Expand().Border(wxALL));

    auto* box_sizer5 = new wxBoxSizer(wxVERTICAL);

    m_staticText = new wxStaticText(this, wxID_ANY, "&Dialogs to Import");
    box_sizer5->Add(m_staticText, wxSizerFlags().Border(wxLEFT|wxRIGHT|wxTOP, wxSizerFlags::GetDefaultBorder()));

    m_checkListResUI = new wxCheckListBox(this, wxID_ANY);
    m_checkListResUI->SetMinSize(wxSize(-1, 160));
    m_checkListResUI->SetToolTip("Uncheck any resources you don\'t want converted.");
    box_sizer5->Add(m_checkListResUI, wxSizerFlags().Expand().Border(wxALL));

    auto* box_sizer = new wxBoxSizer(wxHORIZONTAL);

    m_btnSelectAll = new wxButton(this, wxID_ANY, "Select &All");
    box_sizer->Add(m_btnSelectAll, wxSizerFlags().Border(wxALL));

    m_btnClearAll = new wxButton(this, wxID_ANY, "&Clear All");
    box_sizer->Add(m_btnClearAll, wxSizerFlags().Border(wxALL));

    box_sizer5->Add(box_sizer, wxSizerFlags().Expand().Border(wxALL));

    parent_sizer->Add(box_sizer5, wxSizerFlags().Expand().Border(wxALL));

    auto* stdBtn = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
    parent_sizer->Add(CreateSeparatedSizer(stdBtn), wxSizerFlags().Expand().Border(wxALL));

    SetSizerAndFit(parent_sizer);
    Centre(wxBOTH);

    // Event handlers
    Bind(wxEVT_INIT_DIALOG, &ImportWinRes::OnInit, this);
    m_fileResource->Bind(wxEVT_FILEPICKER_CHANGED, &ImportWinRes::OnResourceFile, this);
    m_btnSelectAll->Bind(wxEVT_BUTTON, &ImportWinRes::OnSelectAll, this);
    m_btnClearAll->Bind(wxEVT_BUTTON, &ImportWinRes::OnClearAll, this);
    Bind(wxEVT_BUTTON, &ImportWinRes::OnOk, this, wxID_OK);

    return true;
}

// ************* End of generated code ***********
// DO NOT EDIT THIS COMMENT BLOCK!
//
// Code below this comment block will be preserved
// if the code for this class is re-generated.
//
// clang-format on
// ***********************************************

/////////////////// Non-generated Copyright/License Info ////////////////////
// Purpose:   Dialog for Importing a Windows resource file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/dir.h>

#include <tttextfile_wx.h>  // textfile -- Classes for reading and writing line-oriented files

#include "mainframe.h"  // MainFrame -- Main window frame

void MainFrame::OnImportWindowsResource(wxCommandEvent&)
{
    ImportWinRes dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxGetApp().AppendWinRes(dlg.GetRcFilename(), dlg.GetDlgNames());
    }
}

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
