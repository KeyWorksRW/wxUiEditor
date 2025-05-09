///////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor - see https://github.com/KeyWorksRW/wxUiEditor/
//
// Do not edit any code above the "End of generated code" comment block.
// Any changes before that block will be lost if it is re-generated!
///////////////////////////////////////////////////////////////////////////////

// clang-format off

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/valtext.h>

#include "editstringdialog_base.h"

bool EditStringDialogBase::Create(wxWindow* parent, wxWindowID id, const wxString& title,
    const wxPoint& pos, const wxSize& size, long style, const wxString &name)
{
    if (!wxDialog::Create(parent, id, title, pos, size, style, name))
    {
        return false;
    }

    auto* parent_sizer = new wxBoxSizer(wxVERTICAL);

    m_static_hdr_text = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_static_hdr_text->Hide();
    parent_sizer->Add(m_static_hdr_text, wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT|wxTOP, 15));

    m_textCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    m_textCtrl->SetValidator(wxTextValidator(wxFILTER_NONE, &m_value));
    m_textCtrl->SetMinSize(FromDIP(wxSize(500, -1)));
    parent_sizer->Add(m_textCtrl, wxSizerFlags().Expand().TripleBorder(wxALL));

    parent_sizer->AddStretchSpacer(1);

    auto* stdBtn_2 = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
    parent_sizer->Add(CreateSeparatedSizer(stdBtn_2), wxSizerFlags().Expand().Border(wxALL));

    if (pos != wxDefaultPosition)
    {
        SetPosition(FromDIP(pos));
    }
    if (size == wxDefaultSize)
    {
        SetSizerAndFit(parent_sizer);
    }
    else
    {
        SetSizer(parent_sizer);
        if (size.x == wxDefaultCoord || size.y == wxDefaultCoord)
        {
            Fit();
        }
        SetSize(FromDIP(size));
        Layout();
    }
    m_textCtrl->SetFocus();

    Centre(wxBOTH);

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
