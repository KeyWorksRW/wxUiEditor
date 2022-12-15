//////////////////////////////////////////////////////////////////////////
// Purpose:   wxStaticBitmap generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/generic/statbmpg.h>  // wxGenericStaticBitmap header
#include <wx/statbmp.h>           // wxStaticBitmap class interface

#include "code.h"           // Code -- Helper class for generating code
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_static_bmp.h"

wxObject* StaticBitmapGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxGenericStaticBitmap(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxBitmapBundle(prop_bitmap),
                                  DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));
#if defined(_DEBUG)
    auto default_size = node->prop_as_wxBitmapBundle(prop_bitmap).GetDefaultSize();
#endif  // _DEBUG

    if (auto value = node->as_string(prop_scale_mode); value != "None")
    {
        if (value == "Fill")
            widget->SetScaleMode(wxStaticBitmap::Scale_Fill);
        else if (value == "AspectFit")
            widget->SetScaleMode(wxStaticBitmap::Scale_AspectFit);
        else if (value == "AspectFill")
            widget->SetScaleMode(wxStaticBitmap::Scale_AspectFill);
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::sview> StaticBitmapGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp())
    {
        GenCppConstruction(code);
    }
    else
    {
        if (code.HasValue(prop_bitmap))
        {
            bool is_list_created = PythonBitmapList(code, prop_bitmap);
            code.NodeName().CreateClass().GetParentName().Comma().as_string(prop_id).Comma();

            if (is_list_created)
            {
                code += "wx.BitmapBundle.FromBitmaps(bitmaps)";
            }
            else
            {
                PythonBundleCode(code, prop_bitmap);
            }
            code.PosSizeFlags();
        }
        else
        {
            code.NodeName().CreateClass().GetParentName().Comma().as_string(prop_id).Comma();
            code.Add("wxNullBitmap");
            code.PosSizeFlags();
        }
    }

    return code.m_code;
}

void StaticBitmapGenerator::GenCppConstruction(Code& gen_code)
{
    Node* node = gen_code.node();
    ttlib::cstr& code = gen_code.m_code;
    if (node->HasValue(prop_bitmap))
    {
        auto& description = node->as_string(prop_bitmap);
        bool is_vector_code = GenerateVectorCode(description, code);
        gen_code.UpdateBreakAt();

        if (is_vector_code)
        {
            gen_code.Tab();
        }

        if (node->IsLocal())
            code << "auto* ";

        bool use_generic_version = (node->as_string(prop_scale_mode) != "None");
        if (use_generic_version)
            gen_code.NodeName() << " = new wxGenericStaticBitmap(";
        else
            gen_code.NodeName() << " = new wxStaticBitmap(";

        gen_code.GetParentName().Comma().as_string(prop_id).Comma();

        if (!is_vector_code)
        {
            ttlib::cstr bundle_code;
            if (wxGetProject().value(prop_wxWidgets_version) != "3.1")
            {
                GenerateBundleCode(description, bundle_code);
                code << bundle_code;
            }
            else
            {
                if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
                {
                    code.insert(0, "\t");
                    code += "\n#if wxCHECK_VERSION(3, 1, 6)\n\t\t";
                    GenerateBundleCode(description, bundle_code);
                    code << bundle_code;
                    GeneratePosSizeFlags(node, code);
                }

                code += "\n#else\n\t";
                if (use_generic_version)
                {
                    // wxGenericStaticBitmap expects a wxBitmap, so it's fine to pass it a wxImage
                    code << GenerateBitmapCode(description);
                }
                else
                {
                    // wxStaticBitmap requires a wxGDIImage for the bitmap, and that won't accept a wxImage.
                    code << "wxBitmap(" << GenerateBitmapCode(description) << ")";
                }
                // GeneratePosSizeFlags(node, code);
                gen_code.PosSizeFlags();
                code << "\n#endif";
                return;
            }
        }
        else
        {
            if (wxGetProject().value(prop_wxWidgets_version) != "3.1")
            {
                code += "wxBitmapBundle::FromBitmaps(bitmaps)";
            }
            else
            {
                code += "\n#if wxCHECK_VERSION(3, 1, 6)\n\t\t";
                code += "wxBitmapBundle::FromBitmaps(bitmaps)";
                GeneratePosSizeFlags(node, code);

                code += "\n#else\n\t\t";
                if (use_generic_version)
                {
                    // wxGenericStaticBitmap expects a wxBitmap, so it's fine to pass it a wxImage
                    code << GenerateBitmapCode(description);
                }
                else
                {
                    // wxStaticBitmap requires a wxGDIImage for the bitmap, and that won't accept a wxImage.
                    code << "wxBitmap(" << GenerateBitmapCode(description) << ")";
                }
                // GeneratePosSizeFlags(node, code);
                gen_code.PosSizeFlags();

                code << "\n#endif";
                return;
            }
        }
    }
    else
    {
        if (node->IsLocal())
            code << "auto* ";

        bool use_generic_version = (node->as_string(prop_scale_mode) != "None");
        if (use_generic_version)
            gen_code.NodeName() << " = new wxGenericStaticBitmap(";
        else
            gen_code.NodeName() << " = new wxStaticBitmap(";

        gen_code.GetParentName().Comma().as_string(prop_id).Comma();

        code << "wxNullBitmap";
    }

    // GeneratePosSizeFlags(node, code);
    gen_code.PosSizeFlags();
}

std::optional<ttlib::sview> StaticBitmapGenerator::CommonSettings(Code& code)
{
    if (code.node()->as_string(prop_scale_mode) != "None")
    {
        code.NodeName().Function("SetScaleMode(").Add("wxStaticBitmap");
        if (code.is_cpp())
        {
            code += "::Scale_";
        }
        else
        {
            code += ".Scale_";
        }
        code.as_string(prop_scale_mode).EndFunction();
    }
    return code.m_code;
}

bool StaticBitmapGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    if (node->as_string(prop_scale_mode) != "None")
        InsertGeneratorInclude(node, "#include <wx/generic/statbmpg.h>", set_src, set_hdr);
    else
        InsertGeneratorInclude(node, "#include <wx/statbmp.h>", set_src, set_hdr);

    // Add wxBitmapBundle header -- the BaseCodeGenerator class will see it and replace it with a conditional include if
    // needed.
    set_src.insert("#include <wx/bmpbndl.h>");
    return true;
}

int StaticBitmapGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxStaticBitmap");
    GenXrcBitmap(node, item, xrc_flags);

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->HasValue(prop_scale_mode) && node->as_string(prop_scale_mode) != "None")
        {
            item.append_child(pugi::node_comment).set_value(" scale mode cannot be be set in the XRC file. ");
        }

        GenXrcComments(node, item);
    }

    return result;
}

void StaticBitmapGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxStaticBitmapXmlHandler");
    handlers.emplace("wxBitmapXmlHandler");
}
