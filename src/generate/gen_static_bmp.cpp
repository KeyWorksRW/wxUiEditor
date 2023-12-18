//////////////////////////////////////////////////////////////////////////
// Purpose:   wxStaticBitmap generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/generic/statbmpg.h>  // wxGenericStaticBitmap header
#include <wx/statbmp.h>           // wxStaticBitmap class interface

#include "code.h"             // Code -- Helper class for generating code
#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"    // Common XRC generating functions
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "pugixml.hpp"        // xml read/write/create/process
#include "utils.h"            // Utility functions that work with properties

#include "gen_static_bmp.h"

wxObject* StaticBitmapGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxGenericStaticBitmap(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxBitmapBundle(prop_bitmap),
                                  DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));
#if defined(_DEBUG)
    // auto default_size = node->as_wxBitmapBundle(prop_bitmap).GetDefaultSize();
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

bool StaticBitmapGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        GenCppConstruction(code);
    }
    else
    {
        if (code.hasValue(prop_bitmap))
        {
            if (code.is_python())
            {
                bool is_list_created = PythonBitmapList(code, prop_bitmap);
                code.NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id).Comma();

                if (is_list_created)
                {
                    code += "wx.BitmapBundle.FromBitmaps(bitmaps)";
                }
                else
                {
                    code.Bundle(prop_bitmap);
                }
            }
            else if (code.is_ruby())
            {
                bool use_generic_version = (code.node()->as_string(prop_scale_mode) != "None");
                if (!use_generic_version)
                    code.NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id).Comma();
                else
                {
                    code.NodeName().CreateClass("GenericStaticBitmap").ValidParentName().Comma().as_string(prop_id).Comma();
                }

                code.Bundle(prop_bitmap);
            }
            code.PosSizeFlags();
        }
        else
        {
            code.NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id).Comma();
            code.Add("wxNullBitmap");
            code.PosSizeFlags();
        }
    }

    return true;
}

void StaticBitmapGenerator::GenCppConstruction(Code& code)
{
    Node* node = code.node();
    if (node->hasValue(prop_bitmap))
    {
        auto& description = node->as_string(prop_bitmap);
        tt_string bundle_code;
        bool is_vector_code = GenerateBundleCode(description, bundle_code);
        code.UpdateBreakAt();

        if (is_vector_code)
        {
            if (bundle_code.starts_with("{\n\t"))
            {
                bundle_code.erase(0, 3);
                code.OpenBrace();
                code += bundle_code;
            }
            code.Tab();
        }

        if (node->isLocal())
            code << "auto* ";

        bool use_generic_version = (node->as_string(prop_scale_mode) != "None");
        if (use_generic_version)
            code.NodeName() << " = new wxGenericStaticBitmap(";
        else
            code.NodeName() << " = new wxStaticBitmap(";

        code.ValidParentName().Comma().as_string(prop_id).Comma();

        if (!is_vector_code)
        {
            if (!Project.is_wxWidgets31())
            {
                code += bundle_code;
            }
            else
            {
                code.GetCode().insert(0, "\t");
                code += "\n#if wxCHECK_VERSION(3, 1, 6)\n\t\t";
                code += bundle_code;
                code.UpdateBreakAt();
                code.PosSizeFlags();

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
                code.UpdateBreakAt();
                code.PosSizeFlags();
                code << "\n#endif";
                return;
            }
        }
        else  // bundle_code contains a vector
        {
            if (!Project.is_wxWidgets31())
            {
                code += "wxBitmapBundle::FromBitmaps(bitmaps)";
            }
            else
            {
                code += "\n#if wxCHECK_VERSION(3, 1, 6)\n\t\t";
                code += "wxBitmapBundle::FromBitmaps(bitmaps)";
                code.Eol().Tab();
                code.PosSizeFlags();

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
                code.PosSizeFlags();

                code << "\n#endif";
                return;
            }
        }
    }
    else  // no bitmap
    {
        if (node->isLocal())
            code << "auto* ";

        bool use_generic_version = (node->as_string(prop_scale_mode) != "None");
        if (use_generic_version)
            code.NodeName() += " = new wxGenericStaticBitmap(";
        else
            code.NodeName() += " = new wxStaticBitmap(";

        code.ValidParentName().Comma().as_string(prop_id).Comma();
        code += "wxNullBitmap";
    }

    code.PosSizeFlags();
}

bool StaticBitmapGenerator::SettingsCode(Code& code)
{
    if (code.node()->as_string(prop_scale_mode) != "None")
    {
        code.NodeName().Function("SetScaleMode(").Add("wxStaticBitmap");
        if (code.is_cpp())
        {
            code += "::Scale_";
            code.as_string(prop_scale_mode);
        }
        else if (code.is_ruby())
        {
            tt_string mode = code.node()->as_string(prop_scale_mode);
            tt_string comment("  # ");
            if (mode == "Fill")
            {
                mode = "::ScaleMode.new(1)";
                comment += "Fill";
            }
            else if (mode == "AspectFit")
            {
                mode = "::ScaleMode.new(2)";
                comment += "AspectFit";
            }
            else if (mode == "AspectFill")
            {
                mode = "::ScaleMode.new(3)";
                comment += "AspectFill";
            }
            code.Str(mode).EndFunction() += comment;
            return true;
        }
        else
        {
            code += ".Scale_";
            code.as_string(prop_scale_mode);
        }
        code.EndFunction();
    }
    return true;
}

bool StaticBitmapGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                        int /* language */)
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
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxStaticBitmap");
    GenXrcBitmap(node, item, xrc_flags);

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->hasValue(prop_scale_mode) && node->as_string(prop_scale_mode) != "None")
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
