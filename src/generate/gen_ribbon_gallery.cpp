/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRibbonGallery generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>              // std::clamp
#include <wx/image.h>             // wxImage
#include <wx/ribbon/buttonbar.h>  // Ribbon control similar to a tool bar
#include <wx/ribbon/gallery.h>    // Ribbon control which displays a gallery of items to choose from

#include "bitmaps.h"        // Contains various images handling functions
#include "code.h"           // Code -- Helper class for generating code
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "image_gen.h"      // Functions for generating embedded images
#include "node.h"           // Node class
#include "utils.h"          // Utility functions that work with properties
#include "wx/gdicmn.h"
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector

#include "gen_ribbon_gallery.h"

// Standard Windows Ribbon gallery images range from 16x16 (96 dpi) to 64x64 (192 dpi).
// 128 provides generous headroom for very high DPI displays.
static constexpr int MAX_GALLERY_IMAGE_DIM = 128;
static constexpr int MIN_GALLERY_IMAGE_DIM = 8;

wxObject* RibbonGalleryGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto* widget = new wxRibbonGallery(wxStaticCast(parent, wxRibbonPanel), wxID_ANY,
                                       DlgPoint(node, prop_pos), DlgSize(node, prop_size), 0);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

// Called by MockupContent::CreateChildren() after the component has been created.
void RibbonGalleryGenerator::AfterCreation(wxObject* wxobject, wxWindow* /* wxparent */, Node* node,
                                           bool /* is_preview */)
{
    wxRibbonGallery* gallery = wxStaticCast(wxobject, wxRibbonGallery);
    wxSize common_size = node->as_wxSize(prop_gallery_size);

    // Sanitize: disallow negative values, and cap each dimension to a reasonable maximum.
    if (common_size != wxDefaultSize)
    {
        common_size.SetWidth(
            std::clamp(common_size.GetWidth(), MIN_GALLERY_IMAGE_DIM, MAX_GALLERY_IMAGE_DIM));
        common_size.SetHeight(
            std::clamp(common_size.GetHeight(), MIN_GALLERY_IMAGE_DIM, MAX_GALLERY_IMAGE_DIM));
    }

    auto scale_to_common = [&](const wxBitmap& bmp) -> wxBitmap
    {
        wxImage image = bmp.ConvertToImage();
        image.Rescale(common_size.GetWidth(), common_size.GetHeight(), wxIMAGE_QUALITY_HIGH);
        return wxBitmap(image);
    };

    for (const auto& child: node->get_ChildNodePtrs())
    {
        if (child->is_Gen(gen_ribbonGalleryItem))
        {
            wxBitmapBundle bundle = child->as_wxBitmapBundle(prop_bitmap);
            if (!bundle.IsOk())
            {
                bundle = GetSvgImage("unknown", common_size);
                const wxSize bitmap_size = bundle.GetBitmap(wxDefaultSize).GetSize();
            }

            // Gallery items all need to be the same size.

            const wxSize bitmap_size = bundle.GetBitmap(wxDefaultSize).GetSize();
            if (bitmap_size != common_size)
            {
                bundle = wxBitmapBundle(scale_to_common(bundle.GetBitmap(wxDefaultSize)));
            }

            gallery->Append(bundle, wxID_ANY);
        }
    }
}

bool RibbonGalleryGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName();
    code.CreateClass().ParentName().Comma().as_string(prop_id).PosSizeFlags();

    return true;
}

bool RibbonGalleryGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                         std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/ribbon/gallery.h>", set_src, set_hdr);

    return true;
}

int RibbonGalleryGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    pugi::xml_node item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "wxRibbonGallery");

    return BaseGenerator::xrc_updated;
}

/////////////////////////////////  RibbonGalleryItemGenerator ////////////////////////////////////

bool RibbonGalleryItemGenerator::ConstructionCode(Code& code)
{
    code.ParentName().Function("Append(");

    const wxue::StringVector parts(code.node()->as_view(prop_bitmap), BMP_PROP_SEPARATOR,
                                   wxue::TRIM::both);
    code.GenerateBundleParameter(parts, true);

    code.Comma().Add("wxID_ANY").EndFunction();

    return true;
}

int RibbonGalleryItemGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    pugi::xml_node item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "item");

    if (!node->HasValue(prop_bitmap))
    {
        pugi::xml_node bitmap_node = item.append_child("bitmap");
        bitmap_node.append_attribute("stock_id").set_value("wxART_QUESTION");
        bitmap_node.append_attribute("stock_client").set_value("wxART_TOOLBAR");
    }

    GenXrcBitmap(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
}
