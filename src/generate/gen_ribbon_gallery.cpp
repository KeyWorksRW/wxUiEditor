/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRibbonGallery generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-11-2026]

#include <algorithm>              // std::clamp
#include <wx/image.h>             // wxImage
#include <wx/ribbon/buttonbar.h>  // Ribbon control similar to a tool bar
#include <wx/ribbon/gallery.h>    // Ribbon control which displays a gallery of items to choose from

#include "bitmaps.h"        // Contains various images handling functions
#include "code.h"           // Code -- Helper class for generating code
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "image_gen.h"      // Functions for generating embedded images
#include "image_handler.h"  // ImageHandler class
#include "mainframe.h"
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties
#include "version.h"          // Version numbers and other constants
#include "write_code.h"       // Write code to Scintilla or file
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
    if (common_size == wxDefaultSize)
    {
        if (node->get_ChildNodePtrs().empty())
        {
            // No gallery items, so use a default size
            return;
        }
        // Determine the common size based on the first gallery item image
        for (const auto& child: node->get_ChildNodePtrs())
        {
            if (child->is_Gen(gen_ribbonGalleryItem))
            {
                const wxBitmapBundle bundle = child->as_wxBitmapBundle(prop_bitmap);
                if (!bundle.IsOk())
                {
                    common_size = wxSize(MIN_GALLERY_IMAGE_DIM, MIN_GALLERY_IMAGE_DIM);
                }
                else
                {
                    common_size = bundle.GetBitmap(wxDefaultSize).GetSize();
                    if (common_size == wxDefaultSize)
                    {
                        common_size = wxSize(MIN_GALLERY_IMAGE_DIM, MIN_GALLERY_IMAGE_DIM);
                    }
                }
                break;
            }
        }
    }

    // Sanitize: disallow negative values, and cap each dimension to a reasonable maximum.
    common_size.SetWidth(
        std::clamp(common_size.GetWidth(), MIN_GALLERY_IMAGE_DIM, MAX_GALLERY_IMAGE_DIM));
    common_size.SetHeight(
        std::clamp(common_size.GetHeight(), MIN_GALLERY_IMAGE_DIM, MAX_GALLERY_IMAGE_DIM));

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

void RibbonGalleryGenerator::GenerateGalleryItems(Node* gallery_node, WriteCode* source,
                                                  GenLang language)
{
    const wxSize gallery_size = gallery_node->as_wxSize(prop_gallery_size);
    const bool check_size = (gallery_size != wxDefaultSize);

    // Open brace scope (C++ only)
    if (language == GenLang::cplusplus)
    {
        source->writeLine("{");
        source->Indent();
    }

    if (check_size)
    {
        // Declare gallery_size and bmp variables used by all gallery items
        Code decl_code(gallery_node, language);
        if (decl_code.is_cpp())
        {
            decl_code << "wxSize gallery_size(";
            decl_code.itoa(gallery_size.x).Comma().itoa(gallery_size.y);
            decl_code << ");";
            decl_code.Eol() << "wxBitmap bmp;";
        }
        else if (decl_code.is_python())
        {
            decl_code << "gallery_size = ";
            decl_code.Add("wxSize(").itoa(gallery_size.x).Comma().itoa(gallery_size.y) << ")";
        }
        else if (decl_code.is_ruby())
        {
            decl_code << "gallery_size = ";
            decl_code.Class("Wx::Size.new(").itoa(gallery_size.x).Comma().itoa(gallery_size.y)
                << ")";
        }
        source->writeLine(decl_code);
    }
    else
    {
        // Scan for size inconsistency across gallery items before generating code.
        wxSize first_size = wxDefaultSize;
        bool found_first = false;
        bool sizes_consistent = true;
        for (const auto& child: gallery_node->get_ChildNodePtrs())
        {
            if (!child->is_Gen(gen_ribbonGalleryItem))
            {
                continue;
            }
            const wxBitmapBundle bundle = child->as_wxBitmapBundle(prop_bitmap);
            if (!bundle.IsOk())
            {
                continue;
            }
            const wxSize child_size = bundle.GetBitmap(wxDefaultSize).GetSize();
            if (!found_first)
            {
                first_size = child_size;
                found_first = true;
            }
            else if (child_size != first_size)
            {
                sizes_consistent = false;
                break;
            }
        }

        if (!sizes_consistent)
        {
            if (language == GenLang::cplusplus)
            {
                if (auto* frame = wxGetMainFrame(); frame)
                {
                    frame->ShowInfoBarMsg("Not all images for wxRibbonGallery are the same size!");
                }
            }
            Code warning_code(gallery_node, language);
            warning_code.AddComment(
                "WARNING: not all images for wxRibbonGallery are the same size!", true);

            source->writeLine(warning_code);
        }
    }

    for (const auto& child: gallery_node->get_ChildNodePtrs())
    {
        if (!child->is_Gen(gen_ribbonGalleryItem))
        {
            continue;
        }

        if (!check_size)
        {
            // No gallery size specified, use standard construction code
            Code child_code(child.get(), language);
            if (child->get_Generator()->ConstructionCode(child_code))
            {
                source->writeLine(child_code);
            }
            continue;
        }

        const wxue::StringVector parts(child->as_view(prop_bitmap), BMP_PROP_SEPARATOR,
                                       wxue::TRIM::both);

        if (parts.empty())
        {
            continue;
        }

        Code item_code(child.get(), language);

        if (item_code.is_cpp())
        {
            const wxue::string& img_type = parts[IndexType];
            const ImageBundle* bundle = nullptr;
            if (img_type.starts_with("Embed"))
            {
                bundle = ProjectImages.GetPropertyImageBundle(&parts);
            }

            if (bundle && bundle->lst_filenames.size() > 1)
            {
                // Case A: Multi-image Embed bundle — generate a lambda that
                // scales every bitmap individually before building the bundle.
                item_code.ParentName().Function("Append(");
                item_code << "[&]()";
                item_code.OpenBrace();
                item_code << "wxVector<wxBitmap> bitmaps;";

                for (const wxue::string& filename: bundle->lst_filenames)
                {
                    EmbeddedImage* embed = ProjectImages.GetEmbeddedImage(filename);
                    if (!embed)
                    {
                        continue;
                    }

                    const wxue::string name = "wxue_img::" + embed->base_image().array_name;
                    item_code.Eol()
                        << "bmp = wxBitmap(wxueImage(" << name << ", sizeof(" << name << ")));";
                    item_code.Eol() << "if (bmp.IsOk() && bmp.GetSize() != gallery_size)";
                    item_code.OpenBrace();
                    item_code << "wxImage img(bmp.ConvertToImage());";
                    item_code.Eol()
                        << "img.Rescale(gallery_size.GetWidth(), gallery_size.GetHeight(), "
                           "wxIMAGE_QUALITY_HIGH);";
                    item_code.Eol() << "bmp = wxBitmap(img);";
                    item_code.CloseBrace();
                    item_code.Eol() << "bitmaps.push_back(bmp);";
                }
                item_code.Eol() << "return wxBitmapBundle::FromBitmaps(bitmaps);";
                item_code.CloseBrace() << "()";
                item_code.Comma().Add("wxID_ANY").EndFunction();
            }
            else if (img_type.starts_with("SVG"))
            {
                // Case B: SVG — renders at the requested size; no rescale needed.
                item_code << "bmp = ";
                item_code.GenerateBundleParameter(parts, false);
                item_code << ".GetBitmap(gallery_size);";
                item_code.Eol();
                item_code.ParentName()
                    .Function("Append(")
                    .Str("bmp")
                    .Comma()
                    .Add("wxID_ANY")
                    .EndFunction();
            }
            else
            {
                // Case C/D/E: Single Embed, Art, or XPM — create bitmap, rescale if needed, append.
                item_code << "bmp = wxBitmap(";
                item_code.GenerateBundleParameter(parts, true);
                item_code << ");";

                item_code.Eol() << "if (bmp.IsOk() && bmp.GetSize() != gallery_size)";
                item_code.OpenBrace();
                item_code << "wxImage img(bmp.ConvertToImage());";
                item_code.Eol() << "img.Rescale(gallery_size.GetWidth(), gallery_size.GetHeight(), "
                                   "wxIMAGE_QUALITY_HIGH);";
                item_code.Eol() << "bmp = wxBitmap(img);";
                item_code.CloseBrace();

                item_code.Eol();
                item_code.ParentName()
                    .Function("Append(")
                    .Str("bmp")
                    .Comma()
                    .Add("wxID_ANY")
                    .EndFunction();
            }
        }
        else if (item_code.is_python())
        {
            item_code << "bmp = ";
            item_code.GenerateBundleParameter(parts, true);

            item_code.Eol() << "if bmp.IsOk() and bmp.GetSize() != gallery_size:";
            item_code.Eol().Tab() << "img = bmp.ConvertToImage()";
            item_code.Eol().Tab()
                << "img.Rescale(gallery_size.GetWidth(), gallery_size.GetHeight(), "
                   "wx.IMAGE_QUALITY_HIGH)";
            item_code.Eol().Tab() << "bmp = wx.Bitmap(img)";

            item_code.Eol();
            item_code.ParentName()
                .Function("Append(")
                .Str("bmp")
                .Comma()
                .Add("wxID_ANY")
                .EndFunction();
        }
        else if (item_code.is_ruby())
        {
            item_code << "bmp = ";
            item_code.GenerateBundleParameter(parts, true);

            item_code.Eol() << "if bmp.ok? && bmp.size != gallery_size";
            item_code.Eol().Tab() << "img = bmp.convert_to_image";
            item_code.Eol().Tab() << "img.rescale(gallery_size.width, gallery_size.height, "
                                     "Wx::IMAGE_QUALITY_HIGH)";
            item_code.Eol().Tab() << "bmp = Wx::Bitmap.new(img)";
            item_code.Eol() << "end";

            item_code.Eol();
            item_code.ParentName()
                .Function("Append(")
                .Str("bmp")
                .Comma()
                .Add("wxID_ANY")
                .EndFunction();
        }

        source->writeLine(item_code);
    }

    // Close brace scope (C++ only)
    if (language == GenLang::cplusplus)
    {
        source->Unindent();
        source->writeLine("}");
    }
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

    // REVIEW: [Randalphwa - 04-17-2026] Technically, this is in 3.3.3, but we don't currently
    // support that level of granularity for wxWidgets versions, so we'll just use 3.3.0 as the
    // cutoff for bundle support in C++.
    if (code.is_cpp() && Project.get_LangVersion(GenLang::cplusplus) >= CPP_WIDGETS_VERSION_3_3_0)
    {
        code.GenerateBundleParameter(parts, false);
    }
    else
    {
        code.GenerateBundleParameter(parts, true);
    }

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
