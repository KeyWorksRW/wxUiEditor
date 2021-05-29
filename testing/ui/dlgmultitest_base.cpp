////////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor/
//
// DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!
////////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/artprov.h>
#include <wx/checkbox.h>
#include <wx/gbsizer.h>
#include <wx/panel.h>
#include <wx/sizer.h>

#include "dlgmultitest_base.h"

#include "../art/disabled_png.h"
#include "../art/focus_png.h"
#include "../art/normal_png.h"

#include <wx/mstream.h>  // Memory stream classes

// Convert a data header file into a wxImage
static wxImage GetImgFromHdr(const unsigned char* data, size_t size_data)
{
    wxMemoryInputStream strm(data, size_data);
    wxImage image;
    if (!image.FindHandler(wxBITMAP_TYPE_PNG))
        wxImage::AddHandler(new wxPNGHandler);
    image.LoadFile(strm);
    return image;
};

DlgMultiTestBase::DlgMultiTestBase(wxWindow* parent) : wxDialog()
{
    Create(parent, wxID_ANY, wxString::FromUTF8("Widgets Testing"), wxDefaultPosition, wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

    auto box_sizer_2 = new wxBoxSizer(wxVERTICAL);

    m_notebook = new wxNotebook(this, wxID_ANY);
    box_sizer_2->Add(m_notebook, wxSizerFlags(1).Expand().Border(wxALL));

    auto page_2 = new wxPanel(m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_notebook->AddPage(page_2, wxString::FromUTF8("Buttons"));

    auto box_sizer_3 = new wxBoxSizer(wxVERTICAL);

    auto grid_bag_sizer = new wxGridBagSizer();
    box_sizer_3->Add(grid_bag_sizer, wxSizerFlags().Border(wxALL));

    m_btn = new wxButton(page_2, wxID_ANY, wxString::FromUTF8("Normal"));
    m_btn->SetToolTip(wxString::FromUTF8("A normal button"));
    grid_bag_sizer->Add(m_btn, wxGBPosition(0, 0), wxGBSpan(1, 1), wxALL, 5);

    m_btn_2 = new wxButton(page_2, wxID_ANY);
    m_btn_2->SetLabelMarkup(wxString::FromUTF8("<b><span foreground='red'>Markup</span></b>"));
    m_btn_2->SetToolTip(wxString::FromUTF8("Text should be Bold and Red."));
    grid_bag_sizer->Add(m_btn_2, wxGBPosition(0, 1), wxGBSpan(1, 1), wxALL, 5);

    m_btn_bitmaps = new wxButton(page_2, wxID_ANY, wxString::FromUTF8("Bitmaps"));
    m_btn_bitmaps->SetBitmap(GetImgFromHdr(normal_png, sizeof(normal_png)));
    m_btn_bitmaps->SetBitmapDisabled(GetImgFromHdr(disabled_png, sizeof(disabled_png)));
    m_btn_bitmaps->SetBitmapCurrent(GetImgFromHdr(focus_png, sizeof(focus_png)));
    m_btn_bitmaps->SetToolTip(wxString::FromUTF8("Bitmap should change when mouse is over button, or button is disabled."));
    grid_bag_sizer->Add(m_btn_bitmaps, wxGBPosition(0, 2), wxGBSpan(1, 1), wxALL, 5);

    auto disable_bitmaps = new wxCheckBox(page_2, wxID_ANY, wxString::FromUTF8("Disable"));
    grid_bag_sizer->Add(disable_bitmaps, wxGBPosition(1, 2), wxGBSpan(1, 1), wxALL, 5);

    m_btn_4 = new wxButton(page_2, wxID_ANY, wxString::FromUTF8("Right"));
    m_btn_4->SetBitmap(GetImgFromHdr(normal_png, sizeof(normal_png)));
    m_btn_4->SetBitmapPosition(wxRIGHT);
    m_btn_4->SetToolTip(wxString::FromUTF8("Bitmap should be on the right side."));
    grid_bag_sizer->Add(m_btn_4, wxGBPosition(0, 3), wxGBSpan(1, 1), wxALL, 5);

    m_toggleBtn = new wxToggleButton(page_2, wxID_ANY, wxString::FromUTF8("Toggle"), wxDefaultPosition, wxDefaultSize,
    wxBU_EXACTFIT);
    m_toggleBtn->SetToolTip(wxString::FromUTF8("Style set to exact fit, so it should be a bit smaller than usual."));
    grid_bag_sizer->Add(m_toggleBtn, wxGBPosition(0, 4), wxGBSpan(1, 1), wxALL, 5);

    auto box_sizer_7 = new wxBoxSizer(wxHORIZONTAL);
    box_sizer_3->Add(box_sizer_7, wxSizerFlags().Border(wxALL));

    m_btn_5 = new wxCommandLinkButton(page_2, wxID_ANY, wxString::FromUTF8("Command"), wxString::FromUTF8("wxCommandLinkButton"));
    m_btn_5->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_OTHER));
    m_btn_5->SetToolTip(wxString::FromUTF8("The bitmap for this is from Art Provider."));
    box_sizer_7->Add(m_btn_5, wxSizerFlags().Border(wxALL));

    page_2->SetSizerAndFit(box_sizer_3);

    auto page_3 = new wxPanel(m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_notebook->AddPage(page_3, wxString::FromUTF8("Tab 2"));

    auto box_sizer_4 = new wxBoxSizer(wxVERTICAL);

    m_staticText_3 = new wxStaticText(page_3, wxID_ANY, wxString::FromUTF8("TODO: replace this control with something more useful..."));
    m_staticText_3->Wrap(200);
    box_sizer_4->Add(m_staticText_3, wxSizerFlags().Border(wxALL));

    page_3->SetSizerAndFit(box_sizer_4);

    auto page_4 = new wxPanel(m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_notebook->AddPage(page_4, wxString::FromUTF8("Tab 3"));

    auto box_sizer_5 = new wxBoxSizer(wxVERTICAL);

    m_staticText_4 = new wxStaticText(page_4, wxID_ANY, wxString::FromUTF8("TODO: replace this control with something more useful..."));
    m_staticText_4->Wrap(200);
    box_sizer_5->Add(m_staticText_4, wxSizerFlags().Border(wxALL));

    page_4->SetSizerAndFit(box_sizer_5);

    auto page_5 = new wxPanel(m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_notebook->AddPage(page_5, wxString::FromUTF8("Tab 4"));

    auto box_sizer_6 = new wxBoxSizer(wxVERTICAL);

    m_staticText_5 = new wxStaticText(page_5, wxID_ANY, wxString::FromUTF8("TODO: replace this control with something more useful..."));
    m_staticText_5->Wrap(200);
    box_sizer_6->Add(m_staticText_5, wxSizerFlags().Border(wxALL));

    page_5->SetSizerAndFit(box_sizer_6);

    auto page = new wxPanel(m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_notebook->AddPage(page, wxString::FromUTF8("Tab 5"));

    auto box_sizer = new wxBoxSizer(wxVERTICAL);

    m_staticText = new wxStaticText(page, wxID_ANY, wxString::FromUTF8("TODO: replace this control with something more useful..."));
    m_staticText->Wrap(200);
    box_sizer->Add(m_staticText, wxSizerFlags().Border(wxALL));

    page->SetSizerAndFit(box_sizer);

    auto stdBtn = CreateStdDialogButtonSizer(wxCLOSE|wxNO_DEFAULT);
    stdBtn->GetCancelButton()->SetDefault();
    box_sizer_2->Add(CreateSeparatedSizer(stdBtn), wxSizerFlags().Expand().Border(wxALL));

    SetSizerAndFit(box_sizer_2);
    SetSize(wxSize(600, 800));
    Centre(wxBOTH);

    // Event handlers
    disable_bitmaps->Bind(wxEVT_CHECKBOX,
        [this](wxCommandEvent& event) { m_btn_bitmaps->Enable(!event.IsChecked()); }
        );
}
