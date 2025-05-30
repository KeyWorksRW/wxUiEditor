///////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor - see https://github.com/KeyWorksRW/wxUiEditor/
//
// Do not edit any code above the "End of generated code" comment block.
// Any changes before that block will be lost if it is re-generated!
///////////////////////////////////////////////////////////////////////////////

// clang-format off

#include <wx/button.h>

#include "../custom_ctrls/colour_rect_ctrl.h"
#include "../custom_ctrls/kw_color_picker.h"

#include "colourprop_base.h"

bool ColourPropBase::Create(wxWindow* parent, wxWindowID id, const wxString& title,
    const wxPoint& pos, const wxSize& size, long style, const wxString &name)
{
    // Scaling of pos and size are handled after the dialog
    // has been created and controls added.
    if (!wxDialog::Create(parent, id, title, pos, size, style, name))
    {
        return false;
    }

    auto* dlg_sizer = new wxBoxSizer(wxVERTICAL);

    auto* box_sizer_3 = new wxBoxSizer(wxVERTICAL);

    auto* box_sizer = new wxBoxSizer(wxHORIZONTAL);

    m_colour_rect = new wxue_ctrl::ColourRectCtrl(this);
    m_colour_rect->SetMaxSize(FromDIP(wxSize(64, 80)));
    box_sizer->Add(m_colour_rect, wxSizerFlags().Border(wxALL));

    m_static_sample_text = new wxStaticText(this, wxID_ANY, "Sample Text");
    box_sizer->Add(m_static_sample_text, wxSizerFlags().Center().Border(wxALL));

    box_sizer_3->Add(box_sizer, wxSizerFlags().Center().Border(wxALL));

    dlg_sizer->Add(box_sizer_3, wxSizerFlags().Expand().Border(wxALL));

    dlg_sizer->AddSpacer(5 + wxSizerFlags::GetDefaultBorder());

    auto* box_sizer_2 = new wxBoxSizer(wxVERTICAL);

    m_radio_default = new wxRadioButton(this, wxID_ANY, "Let wxWidgets choose the colour");
    m_radio_default->SetValue(true);
    box_sizer_2->Add(m_radio_default, wxSizerFlags().DoubleBorder(wxALL));

    dlg_sizer->Add(box_sizer_2, wxSizerFlags().Expand().Border(wxALL));

    dlg_sizer->AddSpacer(5 + wxSizerFlags::GetDefaultBorder());

    m_radio_custom = new wxRadioButton(this, wxID_ANY, "Custom Colour", wxDefaultPosition, wxDefaultSize, wxRB_SINGLE);
    m_staticbox_custom = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, m_radio_custom), wxVERTICAL);
    m_staticbox_custom->GetStaticBox()->Enable(false);

    m_colourPicker = new kwColourPickerCtrl(m_staticbox_custom->GetStaticBox(), wxID_ANY, *wxBLACK, wxDefaultPosition,
        wxDefaultSize, wxCLRP_USE_TEXTCTRL|wxCLRP_SHOW_LABEL|wxWANTS_CHARS);
    m_colourPicker->Enable(false);
    m_staticbox_custom->Add(m_colourPicker, wxSizerFlags().Border(wxALL));

    dlg_sizer->Add(m_staticbox_custom, wxSizerFlags().Expand().Border(wxALL));

    dlg_sizer->AddSpacer(5 + wxSizerFlags::GetDefaultBorder());

    dlg_sizer->AddSpacer(5 + wxSizerFlags::GetDefaultBorder());

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
    Bind(wxEVT_BUTTON, &ColourPropBase::OnOK, this, wxID_OK);
    m_colourPicker->Bind(wxEVT_COLOURPICKER_CHANGED, &ColourPropBase::OnColourChanged, this);
    Bind(wxEVT_INIT_DIALOG, &ColourPropBase::OnInit, this);
    m_radio_custom->Bind(wxEVT_RADIOBUTTON, &ColourPropBase::OnRadioCustomColour, this);
    m_radio_default->Bind(wxEVT_RADIOBUTTON, &ColourPropBase::OnSetDefault, this);

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
