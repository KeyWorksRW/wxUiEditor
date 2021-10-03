/////////////////////////////////////////////////////////////////////////////
// Purpose:   Embedded images generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/panel.h>     // Base header for wxPanel
#include <wx/sizer.h>     // provide wxSizer class for layout
#include <wx/statbmp.h>   // wxStaticBitmap class interface
#include <wx/stattext.h>  // wxStaticText base header

#include "images_form.h"

#include "bitmaps.h"    // Contains various images handling functions
#include "mainframe.h"  // MainFrame -- Main window frame
#include "node.h"       // Node class

#include "../art_headers/empty_png.h_img"
#include "art_headers/unknown_png.h_img"

//////////////////////////////////////////  ImagesGenerator  //////////////////////////////////////////

wxObject* ImagesGenerator::CreateMockup(Node* /* node */, wxObject* parent)
{
    auto panel = new wxPanel(wxStaticCast(parent, wxWindow));
    auto sizer = new wxBoxSizer(wxVERTICAL);
    auto label = new wxStaticText(panel, wxID_ANY, "Select an image to display it below.");
    sizer->Add(label, wxSizerFlags().Border(wxALL));
    m_text_info = new wxStaticText(panel, wxID_ANY, wxEmptyString);
    sizer->Add(m_text_info, wxSizerFlags().Border(wxALL).Expand());

    m_bitmap = new wxStaticBitmap(wxStaticCast(parent, wxWindow), wxID_ANY,
                                  wxBitmap(LoadHeaderImage(empty_png, sizeof(empty_png))));
    sizer->Add(m_bitmap, wxSizerFlags().Border(wxALL));

    panel->SetSizerAndFit(sizer);

    return panel;
}

void ImagesGenerator::OnImageSelected()
{
    auto node = wxGetFrame().GetSelectedNode();

    if (!node)
    {
        m_text_info->SetLabel(wxEmptyString);
        m_bitmap->SetBitmap(wxBitmap(LoadHeaderImage(empty_png, sizeof(empty_png))));
        return;
    }

    auto bmp = node->prop_as_wxBitmap(prop_bitmap);
    ASSERT(bmp.IsOk());
    if (!bmp.IsOk())
    {
        m_text_info->SetLabel("Cannot locate image!");
        m_bitmap->SetBitmap(wxBitmap(LoadHeaderImage(unknown_png, sizeof(unknown_png))));
        return;
    }

    ttlib::cstr info("Dimensions: ");
    info << bmp.GetWidth() << "(W) x " << bmp.GetHeight() << "(H)  Bit depth: " << bmp.GetDepth();
    m_text_info->SetLabel(info);
}
