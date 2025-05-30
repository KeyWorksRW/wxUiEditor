///////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor - see https://github.com/KeyWorksRW/wxUiEditor/
//
// Do not edit any code above the "End of generated code" comment block.
// Any changes before that block will be lost if it is re-generated!
///////////////////////////////////////////////////////////////////////////////

// clang-format off

#include <wx/button.h>
#include <wx/valgen.h>
#include <wx/valnum.h>
#include <wx/valtext.h>

#include "edit_custom_mockup_base.h"

bool EditCustomMockupBase::Create(wxWindow* parent, wxWindowID id, const wxString& title,
    const wxPoint& pos, const wxSize& size, long style, const wxString &name)
{
    // Scaling of pos and size are handled after the dialog
    // has been created and controls added.
    if (!wxDialog::Create(parent, id, title, pos, size, style, name))
    {
        return false;
    }

    auto* dlg_sizer = new wxBoxSizer(wxVERTICAL);

    auto* static_text3 = new wxStaticText(this, wxID_ANY, "Mockup widget");
    dlg_sizer->Add(static_text3, wxSizerFlags().Border(wxALL));

    m_widget_types = new wxChoice(this, wxID_ANY);
    m_widget_types->Append("wxBitmap");
    m_widget_types->Append("wxStaticText");
    m_widget_type = "wxBitmap";  // set validator variable
    m_widget_types->SetValidator(wxGenericValidator(&m_widget_type));
    dlg_sizer->Add(m_widget_types, wxSizerFlags().Expand().Border(wxALL));

    m_static_box = new wxStaticBoxSizer(wxVERTICAL, this, "wxStaticText options");
    m_static_box->GetStaticBox()->Show(false);

    auto* box_sizer = new wxBoxSizer(wxHORIZONTAL);

    m_static_text3 = new wxStaticText(m_static_box->GetStaticBox(), wxID_ANY, "Text:");
    box_sizer->Add(m_static_text3, wxSizerFlags().Center().Border(wxALL));

    m_text_static = new wxTextCtrl(m_static_box->GetStaticBox(), wxID_ANY, wxEmptyString);
    box_sizer->Add(m_text_static, wxSizerFlags().Border(wxALL));

    m_static_box->Add(box_sizer, wxSizerFlags().Expand().Border(wxALL));

    auto* box_sizer2 = new wxBoxSizer(wxHORIZONTAL);

    m_check_centered = new wxCheckBox(m_static_box->GetStaticBox(), wxID_ANY, "Centered");
    box_sizer2->Add(m_check_centered, wxSizerFlags().Border(wxALL));

    m_static_box->Add(box_sizer2, wxSizerFlags().Border(wxALL));

    dlg_sizer->Add(m_static_box, wxSizerFlags().Expand().Border(wxALL));

    auto* flex_grid_sizer = new wxFlexGridSizer(2, 0, 0);

    m_static_text = new wxStaticText(this, wxID_ANY, "width:");
    flex_grid_sizer->Add(m_static_text, wxSizerFlags().CenterVertical().Border(wxALL));

    auto* width = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    width->SetValidator(wxIntegerValidator<int>(&m_width));
    wxStaticCast(width->GetValidator(), wxIntegerValidator<int>)->SetMin(-1);
    wxStaticCast(width->GetValidator(), wxIntegerValidator<int>)->SetMax(9999);
    flex_grid_sizer->Add(width, wxSizerFlags().Border(wxALL));

    m_static_text2 = new wxStaticText(this, wxID_ANY, "height:");
    flex_grid_sizer->Add(m_static_text2, wxSizerFlags().CenterVertical().Border(wxALL));

    auto* text_ctrl2 = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    text_ctrl2->SetValidator(wxTextValidator(wxFILTER_NUMERIC, &m_height));
    flex_grid_sizer->Add(text_ctrl2, wxSizerFlags().Border(wxALL));

    dlg_sizer->Add(flex_grid_sizer, wxSizerFlags().Border(wxALL));

    auto* stdBtn = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
    dlg_sizer->Add(CreateSeparatedSizer(stdBtn), wxSizerFlags().Expand().Border(wxALL));

    if (pos != wxDefaultPosition)
    {
        // Now that the dialog is created, set the scaled position
        SetPosition(FromDIP(pos));
    }
    if (size == wxDefaultSize)
    {
        // If default size let the sizer set the dialog's size
        // so that it is large enough to fit it's child controls.
        SetSizerAndFit(dlg_sizer);
    }
    else
    {
        SetSizer(dlg_sizer);
        if (size.x == wxDefaultCoord || size.y == wxDefaultCoord)
        {
            // Use the sizer to calculate the missing dimension
            Fit();
        }
        SetSize(FromDIP(size));
        Layout();
    }
    Centre(wxBOTH);

    // Event handlers
    Bind(wxEVT_BUTTON, &EditCustomMockupBase::OnOK, this, wxID_OK);
    m_widget_types->Bind(wxEVT_CHOICE, &EditCustomMockupBase::OnSelect, this);
    Bind(wxEVT_INIT_DIALOG, &EditCustomMockupBase::OnInit, this);

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
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

void EditCustomMockupBase::OnInit(wxInitDialogEvent& event)
{
    tt_string_vector parts(m_result.ToStdString(), ';');
    if (parts.size() == 3)
    {
        m_widget_type = parts[0];
        m_width = parts[1].atoi();
        m_height = parts[2];

        if (m_widget_type.starts_with("wxStaticText"))
        {
            if (auto pos = m_widget_type.find('('); pos != tt::npos)
            {
                tt_string_vector options(m_widget_type.Mid(pos + 1).utf8_string(), ",");
                m_text_static->SetValue(options[0]);
                if (options.size() > 1)
                    m_check_centered->SetValue(options[1].contains("1"));
            }

            m_widget_types->SetStringSelection("wxStaticText");
            m_static_box->GetStaticBox()->Show(true);
            Fit();
        }
    }

    event.Skip();
}

void EditCustomMockupBase::OnSelect(wxCommandEvent& WXUNUSED(event))
{
    auto widget_type = m_widget_types->GetStringSelection();
    if (widget_type.StartsWith("wxStaticText"))
    {
        m_static_box->GetStaticBox()->Show(true);
        Fit();
    }
    else
    {
        m_static_box->GetStaticBox()->Show(false);
        Fit();
    }
}

void EditCustomMockupBase::OnOK(wxCommandEvent& event)
{
    if (!Validate() || !TransferDataFromWindow())
        return;

    m_result.clear();
    m_result << m_widget_type;
    if (m_widget_type.StartsWith("wxStaticText"))
    {
        m_result << "(" << m_text_static->GetValue();
        m_result << (m_check_centered->GetValue() ? ", 1" : ", 0");
        m_result << ")";
    }

    // wxBitmap is the default, and currently we don't allow any parameters for it

    m_result << ";" << m_width << ";" << m_height;
    event.Skip();
}
