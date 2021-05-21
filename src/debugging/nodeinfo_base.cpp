////////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor/
//
// DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!
////////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>

#include "nodeinfo_base.h"

NodeInfoBase::NodeInfoBase(wxWindow* parent) : wxDialog()
{
    Create(parent, wxID_ANY, wxString::FromUTF8("Node Information"));

    auto parent_sizer = new wxBoxSizer(wxVERTICAL);

    auto static_box = new wxStaticBoxSizer(wxVERTICAL, this, wxString::FromUTF8("Memory Usage"));
    parent_sizer->Add(static_box, wxSizerFlags().Expand().Border(wxALL));

    m_txt_project = new wxStaticText(static_box->GetStaticBox(), wxID_ANY, wxString::FromUTF8("Project:"));
    static_box->Add(m_txt_project, wxSizerFlags().Border(wxALL));

    m_txt_selection = new wxStaticText(static_box->GetStaticBox(), wxID_ANY, wxString::FromUTF8("Selection:"));
    static_box->Add(m_txt_selection, wxSizerFlags().Border(wxALL));

    m_txt_clipboard = new wxStaticText(static_box->GetStaticBox(), wxID_ANY, wxString::FromUTF8("Clipboard:"));
    static_box->Add(m_txt_clipboard, wxSizerFlags().Border(wxALL));

    auto stdBtn = CreateStdDialogButtonSizer(wxCLOSE|wxNO_DEFAULT);
    stdBtn->GetCancelButton()->SetDefault();
    parent_sizer->Add(CreateSeparatedSizer(stdBtn), wxSizerFlags().Expand().Border(wxALL));

    SetSizerAndFit(parent_sizer);
    Centre(wxBOTH);
}
