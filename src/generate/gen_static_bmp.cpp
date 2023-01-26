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
        new wxGenericStaticBitmap(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxBitmapBundle(prop_bitmap),
                                  DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));
#if defined(_DEBUG)
    // auto default_size = node->prop_as_wxBitmapBundle(prop_bitmap).GetDefaultSize();
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
        if (code.HasValue(prop_bitmap))
        {
            bool is_list_created = PythonBitmapList(code, prop_bitmap);
            code.NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id).Comma();

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
            code.NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id).Comma();
            code.Add("wxNullBitmap");
            code.PosSizeFlags();
        }
    }

    return true;
}

// Check for pos, size, flags, window_flags, and window name, and generate code if needed
// starting with a comma, e.g. -- ", wxPoint(x, y), wxSize(x, y), styles, name);"
//
// If the only style specified is def_style, then it will not be added.
static void GeneratePosSizeFlags(Node* node, tt_string& code, bool uses_def_validator = false,
                                 tt_string_view def_style = tt_empty_cstr);

void StaticBitmapGenerator::GenCppConstruction(Code& gen_code)
{
    Node* node = gen_code.node();
    tt_string& code = gen_code.GetCode();
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

        gen_code.ValidParentName().Comma().as_string(prop_id).Comma();

        if (!is_vector_code)
        {
            tt_string bundle_code;
            if (Project.value(prop_wxWidgets_version) != "3.1")
            {
                GenerateBundleCode(description, bundle_code);
                code << bundle_code;
            }
            else
            {
                if (Project.value(prop_wxWidgets_version) == "3.1")
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
                gen_code.PosSizeFlags();
                code << "\n#endif";
                return;
            }
        }
        else
        {
            if (Project.value(prop_wxWidgets_version) != "3.1")
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

        gen_code.ValidParentName().Comma().as_string(prop_id).Comma();

        code << "wxNullBitmap";
    }

    gen_code.PosSizeFlags();
}

bool StaticBitmapGenerator::SettingsCode(Code& code)
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
    return true;
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

static void GeneratePosSizeFlags(Node* node, tt_string& code, bool uses_def_validator, tt_string_view def_style)
{
    if (node->HasValue(prop_window_name))
    {
        // Window name is always the last parameter, so if it is specified, everything has to be generated.
        if (code.size() < 80)
            code << ", ";
        else
            code << ",\n\t";

        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", ";
        GenStyle(node, code);
        if (uses_def_validator)
            code << ", wxDefaultValidator";
        code << ", " << node->prop_as_string(prop_window_name) << ");";
        return;
    }

    tt_string all_styles;
    GenStyle(node, all_styles);
    if (all_styles.is_sameas("0") || all_styles.is_sameas(def_style))
        all_styles.clear();

    bool isPosSet { false };
    auto pos = node->prop_as_wxPoint(prop_pos);
    if (pos.x != -1 || pos.y != -1)
    {
        if (node->prop_as_string(prop_pos).contains("d", tt::CASE::either))
        {
            code << ", ConvertDialogToPixels(wxPoint(" << pos.x << ", " << pos.y << "))";
        }
        else
        {
            code << ", wxPoint(" << pos.x << ", " << pos.y << ")";
        }

        isPosSet = true;
    }

    bool isSizeSet { false };
    if (node->as_wxSize(prop_size) != wxDefaultSize)
    {
        if (!isPosSet)
        {
            code << ", wxDefaultPosition";
            isPosSet = true;
        }
        code << ", " << GenerateWxSize(node, prop_size);

        isSizeSet = true;
    }

    if (node->HasValue(prop_window_style) && !node->prop_as_string(prop_window_style).is_sameas("wxTAB_TRAVERSAL"))
    {
        if (!isPosSet)
            code << ", wxDefaultPosition";
        if (!isSizeSet)
            code << ", wxDefaultSize";

        code << ", " << all_styles << ");";
        return;
    }

    if (all_styles.size())
    {
        if (!isPosSet)
            code << ", wxDefaultPosition";
        if (!isSizeSet)
            code << ", wxDefaultSize";

        if (code.size() < 100)
            code << ", ";
        else
        {
            code << ",\n\t";
        }

        code << all_styles << ");";
        return;
    }

    code << ");";
}
