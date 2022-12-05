//////////////////////////////////////////////////////////////////////////
// Purpose:   Custom Control generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/generic/statbmpg.h>  // wxGenericStaticBitmap header

#include "bitmaps.h"        // Contains various images handling functions
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "gen_custom_ctrl.h"

wxObject* CustomControl::CreateMockup(Node* /* node */, wxObject* parent)
{
    auto widget = new wxGenericStaticBitmap(wxStaticCast(parent, wxWindow), wxID_ANY, GetInternalImage("CustomControl"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> CustomControl::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto* ";
    code << node->get_node_name() << " = new ";
    if (node->HasValue(prop_namespace))
        code << node->prop_as_string(prop_namespace) << "::";

    ttlib::cstr parameters(node->prop_as_string(prop_parameters));
    parameters.Replace("${parent}", node->get_parent_name(), tt::REPLACE::all);

    for (auto& iter: map_MacroProps)
    {
        if (parameters.find(iter.first) != tt::npos)
        {
            if (iter.second == prop_window_style && node->prop_as_string(iter.second).empty())
                parameters.Replace(iter.first, "0");
            else
                parameters.Replace(iter.first, node->prop_as_string(iter.second));
        }
    }

    code << node->prop_as_string(prop_class_name) << parameters << ';';

    return code;
}

std::optional<ttlib::cstr> CustomControl::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;
    if (node->HasValue(prop_settings_code))
    {
        auto_indent = indent::auto_keep_whitespace;
        code << node->prop_as_string(prop_settings_code);
        code.Replace("@@", "\n", tt::REPLACE::all);
        return code;
    }
    else
    {
        return std::nullopt;
    }
}

int CustomControl::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "unknown");
    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    return result;
}

bool CustomControl::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    if (node->HasValue(prop_header))
    {
        set_src.insert(ttlib::cstr() << "#include \"" << node->prop_as_string(prop_header) << '"');
    }

    if (node->prop_as_string(prop_class_access) != "none" && node->HasValue(prop_class_name))
    {
        if (node->HasValue(prop_namespace))
        {
            set_hdr.insert(ttlib::cstr("namespace ") << node->prop_as_string(prop_namespace) << "\n{\n\t"
                                                     << "class " << node->prop_as_string(prop_class_name) << ";\n}");
        }
        else
            set_hdr.insert(ttlib::cstr() << "class " << node->prop_as_string(prop_class_name) << ';');
    }
    return true;
}
