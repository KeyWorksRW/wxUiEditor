//////////////////////////////////////////////////////////////////////////
// Purpose:   wxStaticBitmap generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
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
    auto widget = new wxGenericStaticBitmap(
        wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxBitmapBundle(prop_bitmap),
        DlgPoint(node, prop_pos), DlgSize(node, prop_size), GetStyleInt(node));

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
        if (code.HasValue(prop_bitmap))
        {
            bool use_generic_version = (code.node()->as_string(prop_scale_mode) != "None");
            if (code.is_perl())
            {
                bool is_list_created = PerlBitmapList(code, prop_bitmap);
                if (code.is_local_var())
                {
                    code << "my ";
                }
                code.NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id).Comma();
                if (is_list_created)
                {
                    code += "Wx::BitmapBundle::FromBitmaps($bitmaps)";
                }
                else
                {
                    code.Bundle(prop_bitmap);
                }
            }
            else if (code.is_python())
            {
                bool is_list_created = PythonBitmapList(code, prop_bitmap);
                if (!use_generic_version)
                    code.NodeName()
                        .CreateClass()
                        .ValidParentName()
                        .Comma()
                        .as_string(prop_id)
                        .Comma();
                else
                    code.NodeName()
                        .CreateClass("GenericStaticBitmap")
                        .ValidParentName()
                        .Comma()
                        .as_string(prop_id)
                        .Comma();

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
                if (!use_generic_version)
                    code.NodeName()
                        .CreateClass()
                        .ValidParentName()
                        .Comma()
                        .as_string(prop_id)
                        .Comma();
                else
                {
                    code.NodeName()
                        .CreateClass("GenericStaticBitmap")
                        .ValidParentName()
                        .Comma()
                        .as_string(prop_id)
                        .Comma();
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
    Node* node = code.node();  // for convenience
    auto class_override_type = GetClassOverrideType(node);
    if (class_override_type == ClassOverrideType::None &&
        node->as_string(prop_scale_mode) != "None")
    {
        // If we are using a scale mode, we must use the wxGenericStaticBitmap.
        class_override_type = ClassOverrideType::Generic;
    }

    if (node->HasValue(prop_bitmap))
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

        if (node->is_Local())
            code << "auto* ";

        switch (class_override_type)
        {
            case ClassOverrideType::Generic:
                code.NodeName() << " = new wxGenericStaticBitmap(";
                break;
            case ClassOverrideType::Subclass:
                code.NodeName() << " = new " << node->as_string(prop_subclass) << "(";
                if (node->HasValue(prop_subclass_params))
                {
                    code += node->as_string(prop_subclass_params);
                    code.RightTrim();
                    if (code.back() != ',')
                        code.Comma();
                    else
                        code += ' ';
                }
                break;
            case ClassOverrideType::None:
                code.NodeName() << " = new wxStaticBitmap(";
                break;
        }

        code.ValidParentName().Comma().as_string(prop_id).Comma();

        if (!is_vector_code)
        {
            code += bundle_code;
        }
        else  // bundle_code contains a vector
        {
            code += "wxBitmapBundle::FromBitmaps(bitmaps)";
        }
    }
    else  // no bitmap
    {
        if (node->is_Local())
            code << "auto* ";

        switch (class_override_type)
        {
            case ClassOverrideType::Generic:
                code.NodeName() << " = new wxGenericStaticBitmap(";
                break;
            case ClassOverrideType::Subclass:
                code.NodeName() << " = new " << node->as_string(prop_subclass) << "(";
                if (node->HasValue(prop_subclass_params))
                {
                    code += node->as_string(prop_subclass_params);
                    code.RightTrim();
                    if (code.back() != ',')
                        code.Comma();
                    else
                        code += ' ';
                }
                break;
            case ClassOverrideType::None:
                code.NodeName() << " = new wxStaticBitmap(";
                break;
        }

        code.ValidParentName().Comma().as_string(prop_id).Comma();
        code += "wxNullBitmap";
    }

    code.PosSizeFlags();
}

bool StaticBitmapGenerator::SettingsCode(Code& code)
{
    if (code.node()->as_string(prop_scale_mode) != "None")
    {
        // C++ and wxRuby3 use wxStaticBitmap::ScaleMode, wxPython uses
        // wxGenericStaticBitmap::ScaleMode
        if (!code.is_python())
            code.NodeName().Function("SetScaleMode(").Add("wxStaticBitmap");
        else
            code.NodeName().Function("SetScaleMode(").Add("wxGenericStaticBitmap");
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
                mode = "::ScaleMode::Scale_Fill";
            }
            else if (mode == "AspectFit")
            {
                mode = "::ScaleMode::Scale_AspectFit";
            }
            else if (mode == "AspectFill")
            {
                mode = "::ScaleMode::Scale_AspectFill";
            }
            code.Str(mode).EndFunction();
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

bool StaticBitmapGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                        std::set<std::string>& set_hdr, GenLang /* language */)
{
    if (node->as_string(prop_scale_mode) != "None" ||
        node->as_string(prop_subclass).starts_with("wxGeneric"))
        InsertGeneratorInclude(node, "#include <wx/generic/statbmpg.h>", set_src, set_hdr);
    else
        InsertGeneratorInclude(node, "#include <wx/statbmp.h>", set_src, set_hdr);

    // Add wxBitmapBundle header -- the BaseCodeGenerator class will see it and replace it with a
    // conditional include if needed.
    set_src.insert("#include <wx/bmpbndl.h>");
    return true;
}

int StaticBitmapGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                   BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxStaticBitmap");
    GenXrcBitmap(node, item, xrc_flags);

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->HasValue(prop_scale_mode) && node->as_string(prop_scale_mode) != "None")
        {
            item.append_child(pugi::node_comment)
                .set_value(" scale mode cannot be be set in the XRC file. ");
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
