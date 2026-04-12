/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRibbonGallery generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

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
    wxSize common_size = wxDefaultSize;

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
            // REVIEW: [Randalphwa - 04-07-2026] We've got a big problem if the very first gallery
            // bitmap in invalid -- we are then going to set *all* the gallery items to that default
            // bitmap's size which is clearly wrong...

            wxBitmap bitmap = child->as_wxBitmap(prop_bitmap);
            if (!bitmap.IsOk())
            {
                bitmap = GetInternalImage("default");
                const wxSize bitmap_size = bitmap.GetSize();
                if (bitmap_size != common_size)
                {
                    bitmap = scale_to_common(bitmap);
                }
            }

            // Gallery items all need to be the same size.

            const wxSize bitmap_size = bitmap.GetSize();
            if (common_size == wxDefaultSize && bitmap.IsOk())
            {
                common_size = bitmap_size;
            }
            else if (bitmap_size != common_size)
            {
                bitmap = scale_to_common(bitmap);
            }

            gallery->Append(bitmap, wxID_ANY);
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
