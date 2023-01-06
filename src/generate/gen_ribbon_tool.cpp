/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRibbonToolBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/ribbon/buttonbar.h>  // Ribbon control similar to a tool bar
#include <wx/ribbon/toolbar.h>    // Ribbon-style tool bar

#include "bitmaps.h"     // Contains various images handling functions
#include "code.h"        // Code -- Helper class for generating code
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_ribbon_tool.h"

wxObject* RibbonToolBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRibbonToolBar((wxRibbonPanel*) parent, wxID_ANY, DlgPoint(parent, node, prop_pos),
                                      DlgSize(parent, node, prop_size));
    if (node->prop_as_int(prop_min_rows) != 1 || node->prop_as_string(prop_max_rows) != "-1")
    {
        auto min_rows = node->prop_as_int(prop_min_rows);
        auto max_rows = node->prop_as_int(prop_max_rows);
        if (max_rows < min_rows)
            max_rows = min_rows;
        widget->SetRows(min_rows, max_rows);
    }

    return widget;
}

void RibbonToolBarGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool /* is_preview */)
{
    auto btn_bar = wxDynamicCast(wxobject, wxRibbonToolBar);

    for (const auto& child: node->GetChildNodePtrs())
    {
        if (child->isGen(gen_ribbonSeparator))
        {
            btn_bar->AddSeparator();
        }
        else
        {
            auto bmp = child->prop_as_wxBitmap(prop_bitmap);
            if (!bmp.IsOk())
                bmp = GetInternalImage("default");
            // REVIEW: This is still a bitmap rather then a bundle as of the 3.1.6 release
            btn_bar->AddTool(wxID_ANY, bmp, child->prop_as_wxString(prop_help),
                             (wxRibbonButtonKind) child->prop_as_int(prop_kind));
        }
    }
    btn_bar->Realize();
}

std::optional<ttlib::cstr> RibbonToolBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto* ";
    code << node->get_node_name() << " = new wxRibbonToolBar(";
    code << node->get_parent_name() << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code, false);

    return code;
}

std::optional<ttlib::cstr> RibbonToolBarGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    auto min_rows = node->prop_as_int(prop_min_rows);
    auto max_rows = node->prop_as_int(prop_max_rows);
    if (min_rows != 1 || max_rows != -1)
    {
        if (max_rows < min_rows)
            max_rows = min_rows;
        code << node->get_node_name() << "->SetRows(" << ttlib::cstr().Format("%d, %d", min_rows, max_rows) << ");";
    }
    else
    {
        return {};
    }

    return code;
}

bool RibbonToolBarGenerator::GetIncludes(Node* /* node */, std::set<std::string>& /* set_src */,
                                         std::set<std::string>& set_hdr)
{
    // Normally we'd use the access property to determin if the header should be in the source or header file. However,
    // the two events used by this component are also in this header file and the tools themselves are fairly useless
    // without processing the events, so we just add the header file to the header generated file.

    set_hdr.insert("#include <wx/ribbon/toolbar.h>");

    return true;
}

int RibbonToolBarGenerator::GenXrcObject(Node* /* node */, pugi::xml_node& /* object */, size_t /* xrc_flags */)
{
    return BaseGenerator::xrc_not_supported;
}

//////////////////////////////////////////  RibbonToolGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> RibbonToolGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    code << node->get_parent_name() << "->AddTool(";
    if (node->prop_as_string(prop_id).size())
        code << node->prop_as_string(prop_id);
    else
        code << "wxID_ANY";
    code << ", ";

    if (node->prop_as_string(prop_bitmap).size())
        code << GenerateBitmapCode(node->prop_as_string(prop_bitmap));
    else
        code << "wxNullBitmap";

    code << ", ";
    auto& help = node->prop_as_string(prop_help);
    if (help.size())
        code << GenerateQuotedString(help);
    else
        code << "wxEmptyString";

    code << ", " << node->prop_as_string(prop_kind) << ");";

    return code;
}

int RibbonToolGenerator::GenXrcObject(Node* /* node */, pugi::xml_node& /* object */, size_t /* xrc_flags */)
{
    return BaseGenerator::xrc_not_supported;
}
