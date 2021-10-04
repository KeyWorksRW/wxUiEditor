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

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "images_form.h"

#include "auto_freeze.h"  // AutoFreeze -- Automatically Freeze/Thaw a window
#include "bitmaps.h"      // Contains various images handling functions
#include "mainframe.h"    // MainFrame -- Main window frame
#include "node.h"         // Node class

#include "../art_headers/empty_png.h_img"
#include "../art_headers/unknown_png.h_img"

#include "../mockup/mockup_content.h"  // MockupContent -- Mockup of a form's contents
#include "../mockup/mockup_parent.h"   // MockupParent -- Top-level MockUp Parent window

//////////////////////////////////////////  ImagesGenerator  //////////////////////////////////////////

wxObject* ImagesGenerator::CreateMockup(Node* /* node */, wxObject* wxobject)
{
    auto parent = wxStaticCast(wxobject, wxWindow);

    m_image_name = new wxStaticText(parent, wxID_ANY, "Select an image to display it below.");
    m_text_info = new wxStaticText(parent, wxID_ANY, wxEmptyString);
    m_bitmap = new wxStaticBitmap(wxStaticCast(parent, wxWindow), wxID_ANY,
                                  wxBitmap(LoadHeaderImage(empty_png, sizeof(empty_png))));

    auto node = wxGetFrame().GetSelectedNode();
    if (node->isGen(gen_embedded_image))
    {
        ttlib::multiview mstr(node->prop_as_string(prop_bitmap), ';');

        if (mstr.size() > 1)
        {
            m_image_name->SetLabel(mstr[1].wx_str());
        }
        else
        {
            m_image_name->SetLabel(wxEmptyString);
        }

        auto bmp = node->prop_as_wxBitmap(prop_bitmap);
        ASSERT(bmp.IsOk());
        if (!bmp.IsOk())
        {
            m_text_info->SetLabel("Cannot locate image!");
            m_bitmap->SetBitmap(wxBitmap(LoadHeaderImage(empty_png, sizeof(empty_png))));
        }
        else
        {
            m_bitmap->SetBitmap(bmp);

            ttlib::cstr info("Dimensions: ");
            info << bmp.GetWidth() << "(w) x " << bmp.GetHeight() << "(w)  Bit depth: " << bmp.GetDepth();
            m_text_info->SetLabel(info);
        }
    }

    auto sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_image_name, wxSizerFlags(0).Border(wxALL).Expand());
    sizer->Add(m_text_info, wxSizerFlags(0).Border(wxALL).Expand());
    sizer->Add(m_bitmap, wxSizerFlags(1).Border(wxALL).Expand());

    return sizer;
}
