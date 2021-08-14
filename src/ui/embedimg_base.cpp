////////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor/
//
// DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!
////////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/collpane.h>
#include <wx/colour.h>
#include <wx/panel.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbox.h>

#include "embedimg_base.h"

bool EmbedImageBase::Create(wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint&pos, const wxSize& size, long style, const wxString &name)
{
    if (!wxDialog::Create(parent, id, title, pos, size, style, name))
        return false;

    auto parent_sizer = new wxBoxSizer(wxVERTICAL);

    auto box_sizer = new wxBoxSizer(wxHORIZONTAL);
    parent_sizer->Add(box_sizer, wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT|wxTOP, wxSizerFlags::GetDefaultBorder()));

    auto collapsiblePane = new wxCollapsiblePane(this, wxID_ANY, wxString::FromUTF8("Dialog Description"));
    collapsiblePane->Collapse();
    box_sizer->Add(collapsiblePane, wxSizerFlags(1).Expand().Border(wxLEFT|wxRIGHT|wxTOP, wxSizerFlags::GetDefaultBorder()));

    auto box_sizer2 = new wxBoxSizer(wxHORIZONTAL);

    m_staticDescription = new wxStaticText(collapsiblePane->GetPane(), wxID_ANY, wxString::FromUTF8("This dialog can be used to convert an image into a file that can be #included into a source file. The original image can be any file format that wxWidgets supports.\n\nThe header output type is an array containing the image data in whatever format you choose. While the disk file size might be larger than an XPM file, the size in your executable will typically be quite a bit smaller."));
    m_staticDescription->Wrap(400);
    box_sizer2->Add(m_staticDescription, wxSizerFlags().Border(wxALL));

    collapsiblePane->GetPane()->SetSizerAndFit(box_sizer2);

    auto flex_grid_sizer = new wxFlexGridSizer(2, 0, 0);
    {
        flex_grid_sizer->AddGrowableCol(1);
    }
    parent_sizer->Add(flex_grid_sizer, wxSizerFlags().Expand().Border(wxALL));

    m_staticOriginal = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("&Source:"));
    flex_grid_sizer->Add(m_staticOriginal, wxSizerFlags().Center().Border(wxALL));

    m_fileOriginal = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, wxFileSelectorPromptStr, 
    wxString::FromUTF8("Select file(s)\", \"All files|*.*|PNG|*.png|XPM|*.xpm|Tiff|*.tif;*.tiff|Bitmaps|*.bmp|Icon|*.ico||"), wxDefaultPosition, wxSize(300, -1),
    wxFLP_USE_TEXTCTRL|wxFLP_OPEN|wxFLP_FILE_MUST_EXIST);
    flex_grid_sizer->Add(m_fileOriginal, wxSizerFlags().Expand().Border(wxALL));

    m_staticHeader = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("O&utput:"));
    flex_grid_sizer->Add(m_staticHeader, wxSizerFlags().Center().Border(wxALL));

    m_fileOutput = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, wxFileSelectorPromptStr, 
    wxString::FromUTF8("Header files|*.h_img||"), wxDefaultPosition, wxDefaultSize,
    wxFLP_SAVE|wxFLP_USE_TEXTCTRL);
    flex_grid_sizer->Add(m_fileOutput, wxSizerFlags().Expand().Border(wxALL));

    auto static_box = new wxStaticBoxSizer(wxVERTICAL, this, wxString::FromUTF8("Output Type"));
    parent_sizer->Add(static_box, wxSizerFlags().Border(wxALL));

    m_choicebook = new wxChoicebook(static_box->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize,
    wxCHB_LEFT);
    static_box->Add(m_choicebook, wxSizerFlags().Border(wxALL));

    auto header_page = new wxPanel(m_choicebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_choicebook->AddPage(header_page, wxString::FromUTF8("Header"));
    header_page->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

    auto parent_sizer_2 = new wxBoxSizer(wxVERTICAL);

    auto hdr_static_box = new wxStaticBoxSizer(wxVERTICAL, header_page, wxString::FromUTF8("Settings"));
    parent_sizer_2->Add(hdr_static_box, wxSizerFlags().Border(wxALL));

    auto box_sizer_2 = new wxBoxSizer(wxVERTICAL);
    hdr_static_box->Add(box_sizer_2, wxSizerFlags().Expand().Border(wxALL));

    m_check_make_png = new wxCheckBox(hdr_static_box->GetStaticBox(), wxID_ANY, wxString::FromUTF8("Convert to &PNG"));
    m_check_make_png->SetValue(true);
    m_check_make_png->SetToolTip(wxString::FromUTF8("If checked, image will be converted to PNG before being saved."));
    box_sizer_2->Add(m_check_make_png, wxSizerFlags().Border(wxALL));

    m_check_c17 = new wxCheckBox(hdr_static_box->GetStaticBox(), wxID_ANY, wxString::FromUTF8("C++1&7 &encoding"));
    m_check_c17->SetToolTip(wxString::FromUTF8("If checked, this will prefix the array with \"inline constexpr\" instead of \"static\"."));
    box_sizer_2->Add(m_check_c17, wxSizerFlags().Border(wxALL));

    m_ForceHdrMask = new wxCheckBox(hdr_static_box->GetStaticBox(), wxID_ANY, wxString::FromUTF8("&Force Mask"));
    m_ForceHdrMask->SetToolTip(wxString::FromUTF8("Check this to override any mask specified in the original image file."));
    box_sizer_2->Add(m_ForceHdrMask, wxSizerFlags().Border(wxALL));

    auto box_sizer_3 = new wxBoxSizer(wxHORIZONTAL);
    box_sizer_2->Add(box_sizer_3, wxSizerFlags().Border(wxALL));

    box_sizer_3->AddSpacer(10);

    m_comboHdrMask = new wxComboBox(hdr_static_box->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(150, -1), 0, nullptr, wxCB_READONLY);
    box_sizer_3->Add(m_comboHdrMask, wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxBOTTOM, wxSizerFlags::GetDefaultBorder()));

    auto box_sizer_5 = new wxBoxSizer(wxHORIZONTAL);
    box_sizer_2->Add(box_sizer_5, wxSizerFlags().Border(wxLEFT|wxRIGHT, wxSizerFlags::GetDefaultBorder()));

    box_sizer_5->AddSpacer(10);

    m_staticHdrRGB = new wxStaticText(hdr_static_box->GetStaticBox(), wxID_ANY, wxString::FromUTF8("0 0 0"));
    box_sizer_5->Add(m_staticHdrRGB, wxSizerFlags().Border(wxLEFT|wxRIGHT, wxSizerFlags::GetDefaultBorder()));

    header_page->SetSizerAndFit(parent_sizer_2);

    auto xpm_page = new wxPanel(m_choicebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_choicebook->AddPage(xpm_page, wxString::FromUTF8("XPM"));
    xpm_page->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

    auto parent_sizer_3 = new wxBoxSizer(wxVERTICAL);

    auto mask_static_box = new wxStaticBoxSizer(wxVERTICAL, xpm_page, wxString::FromUTF8("Settings"));
    parent_sizer_3->Add(mask_static_box, wxSizerFlags().Border(wxALL));

    auto box_sizer7 = new wxBoxSizer(wxVERTICAL);
    mask_static_box->Add(box_sizer7, wxSizerFlags().Expand().Border(wxALL));

    m_ConvertAlphaChannel = new wxCheckBox(mask_static_box->GetStaticBox(), wxID_ANY, wxString::FromUTF8("&Alpha Channel to Mask"));
    m_ConvertAlphaChannel->SetValue(true);
    m_ConvertAlphaChannel->SetToolTip(wxString::FromUTF8("Check this to replace any alpha channel with a mask."));
    box_sizer7->Add(m_ConvertAlphaChannel, wxSizerFlags().Expand().Border(wxALL));

    m_ForceXpmMask = new wxCheckBox(mask_static_box->GetStaticBox(), wxID_ANY, wxString::FromUTF8("&Force Mask"));
    m_ForceXpmMask->SetToolTip(wxString::FromUTF8("Check this to override any mask specified in the original image file."));
    box_sizer7->Add(m_ForceXpmMask, wxSizerFlags().Border(wxALL));

    auto box_sizer_4 = new wxBoxSizer(wxHORIZONTAL);
    box_sizer7->Add(box_sizer_4, wxSizerFlags().Border(wxALL));

    box_sizer_4->AddSpacer(10);

    m_comboXpmMask = new wxComboBox(mask_static_box->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(150, -1), 0, nullptr, wxCB_READONLY);
    box_sizer_4->Add(m_comboXpmMask, wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxBOTTOM, wxSizerFlags::GetDefaultBorder()));

    auto box_sizer_6 = new wxBoxSizer(wxHORIZONTAL);
    box_sizer7->Add(box_sizer_6, wxSizerFlags().Border(wxLEFT|wxRIGHT, wxSizerFlags::GetDefaultBorder()));

    box_sizer_6->AddSpacer(10);

    m_staticXpmRGB = new wxStaticText(mask_static_box->GetStaticBox(), wxID_ANY, wxString::FromUTF8("0 0 0"));
    box_sizer_6->Add(m_staticXpmRGB, wxSizerFlags().Border(wxLEFT|wxRIGHT, wxSizerFlags::GetDefaultBorder()));

    xpm_page->SetSizerAndFit(parent_sizer_3);

    auto box_sizer6 = new wxBoxSizer(wxHORIZONTAL);
    parent_sizer->Add(box_sizer6, wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT|wxTOP, wxSizerFlags::GetDefaultBorder()));

    m_staticDimensions = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("16 x 16"));
    m_staticDimensions->Hide();
    box_sizer6->Add(m_staticDimensions, wxSizerFlags(1).Center().Border(wxLEFT|wxRIGHT|wxTOP, wxSizerFlags::GetDefaultBorder()));

    auto grid_sizer2 = new wxGridSizer(2);
    parent_sizer->Add(grid_sizer2, wxSizerFlags(1).Expand().Border(wxALL));

    m_staticOriginal = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("Source"));
    m_staticOriginal->Hide();
    grid_sizer2->Add(m_staticOriginal, wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP, wxSizerFlags::GetDefaultBorder()));

    m_staticOutput = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("Current"));
    m_staticOutput->Hide();
    grid_sizer2->Add(m_staticOutput, wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP, wxSizerFlags::GetDefaultBorder()));

    m_bmpOriginal = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap);
    m_bmpOriginal->Hide();
    grid_sizer2->Add(m_bmpOriginal, wxSizerFlags().Center().Border(wxALL));

    m_bmpOutput = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap);
    m_bmpOutput->Hide();
    grid_sizer2->Add(m_bmpOutput, wxSizerFlags().Center().Border(wxALL));

    auto flex_grid_sizer2 = new wxFlexGridSizer(2, 1, 0, 0);
    parent_sizer->Add(flex_grid_sizer2, wxSizerFlags().Expand().Border(wxALL));

    m_staticSave = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("save"));
    m_staticSave->Hide();
    flex_grid_sizer2->Add(m_staticSave, wxSizerFlags().Expand().Border(wxALL));

    m_staticSize = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("size"));
    m_staticSize->Hide();
    flex_grid_sizer2->Add(m_staticSize, wxSizerFlags().Expand().Border(wxALL));

    auto grid_sizer = new wxGridSizer(2);
    parent_sizer->Add(grid_sizer, wxSizerFlags().Expand().Border(wxALL));

    m_btnConvert = new wxButton(this, wxID_ANY, wxString::FromUTF8("Convert"));
    grid_sizer->Add(m_btnConvert, wxSizerFlags().Border(wxALL));

    m_btnClose = new wxButton(this, wxID_OK, wxString::FromUTF8("Close"));
    grid_sizer->Add(m_btnClose, wxSizerFlags().Right().Border(wxALL));

    SetSizerAndFit(parent_sizer);
    Centre(wxBOTH);

    // Event handlers
    m_fileOriginal->Bind(wxEVT_FILEPICKER_CHANGED, &EmbedImageBase::OnInputChange, this);
    m_fileOutput->Bind(wxEVT_FILEPICKER_CHANGED, &EmbedImageBase::OnOutputChange, this);
    m_choicebook->Bind(wxEVT_CHOICEBOOK_PAGE_CHANGED, &EmbedImageBase::OnPageChanged, this);
    m_check_make_png->Bind(wxEVT_CHECKBOX, &EmbedImageBase::OnCheckPngConversion, this);
    m_check_c17->Bind(wxEVT_CHECKBOX, &EmbedImageBase::OnC17Encoding, this);
    m_ForceHdrMask->Bind(wxEVT_CHECKBOX, &EmbedImageBase::OnForceHdrMask, this);
    m_comboHdrMask->Bind(wxEVT_COMBOBOX, &EmbedImageBase::OnComboHdrMask, this);
    m_ConvertAlphaChannel->Bind(wxEVT_CHECKBOX, &EmbedImageBase::OnConvertAlpha, this);
    m_ForceXpmMask->Bind(wxEVT_CHECKBOX, &EmbedImageBase::OnForceXpmMask, this);
    m_comboXpmMask->Bind(wxEVT_COMBOBOX, &EmbedImageBase::OnComboXpmMask, this);
    m_btnConvert->Bind(wxEVT_BUTTON, &EmbedImageBase::OnConvert, this);

    return true;
}
