/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFontPickerCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/fontpicker.h>

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_font_picker.h"

wxObject* FontPickerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxFontPickerCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_font(prop_initial_font),
                             DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->HasValue(prop_max_point_size))
    {
        widget->SetMaxPointSize(node->prop_as_int(prop_max_point_size));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> FontPickerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";
    if (node->prop_as_string(prop_initial_font).size())
    {
        auto fontprop = node->prop_as_font_prop(prop_initial_font);
        wxFont font = fontprop.GetFont();

        code << "\n\t\t\twxFont(";

        if (fontprop.GetPointSize() <= 0)
            code << "wxNORMAL_FONT->GetPointSize()";
        else
            code << fontprop.GetPointSize();

        code << ", " << ConvertFontFamilyToString(fontprop.GetFamily()) << ", " << font.GetStyleString().wx_str();
        code << ", " << font.GetWeightString().wx_str() << ", " << (fontprop.IsUnderlined() ? "true" : "false") << ", ";
        if (fontprop.GetFaceName().size())
            code << '\"' << fontprop.GetFaceName().wx_str() << '\"';
        else
            code << "wxEmptyString";

        code << ")";
        code.insert(0, 4, ' ');
    }
    else
    {
        code << "wxNullFont";
    }

    GeneratePosSizeFlags(node, code, true, "wxFNTP_DEFAULT_STYLE");

    return code;
}

std::optional<ttlib::cstr> FontPickerGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_string(prop_min_point_size) != "0")
    {
        if (code.size())
            code << "\n\t";
        code << node->get_node_name() << "->SetMinPointSize(" << node->prop_as_string(prop_min_point_size) << ");";
    }

    if (node->prop_as_string(prop_max_point_size) != "100")
    {
        if (code.size())
            code << "\n\t";
        code << node->get_node_name() << "->SetMaxPointSize(" << node->prop_as_string(prop_max_point_size) << ");";
    }

    return code;
}

std::optional<ttlib::cstr> FontPickerGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool FontPickerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/fontpicker.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/font.h>", set_src, set_hdr);
    return true;
}
