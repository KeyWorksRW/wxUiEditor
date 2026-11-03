/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid utilities
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-30-2026]

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "gen_enums.h"  // Enumerations for generators
#include "node.h"       // Node class
#include "pugixml.hpp"  // xml read/write/create/process

#include "utils_prop_grid.h"

void AfterCreationAddItems(wxPropertyGridInterface* grid_iface, Node* node)
{
    for (const auto& child: node->get_ChildNodePtrs())
    {
        if (child->is_Gen(gen_propGridItem))
        {
            if (child->as_string(prop_type) == "Category")
            {
                grid_iface->Append(new wxPropertyCategory(child->as_wxString(prop_label),
                                                          child->as_wxString(prop_label)));
            }
            else
            {
                wxPGProperty* prop = wxDynamicCast(
                    wxCreateDynamicObject("wx" + (child->as_string(prop_type)) + "Property"),
                    wxPGProperty);
                if (prop)
                {
                    prop->SetLabel(child->as_wxString(prop_label));
                    prop->SetName(child->as_wxString(prop_label));
                    grid_iface->Append(prop);

                    if (child->HasValue(prop_help))
                    {
                        grid_iface->SetPropertyHelpString(prop, child->as_wxString(prop_help));
                    }
                }
            }
        }
        else if (child->is_Gen(gen_propGridCategory))
        {
            grid_iface->Append(new wxPropertyCategory(child->as_wxString(prop_label),
                                                      child->as_wxString(prop_label)));
            AfterCreationAddItems(grid_iface, child.get());
        }
    }
}

std::string GetXrcPropertyClass(Node* node)
{
    if (node->is_Gen(gen_propGridCategory))
    {
        return "wxPropertyCategory";
    }

    if (node->is_Gen(gen_propGridItem) && node->as_string(prop_type) == "Category")
    {
        return "wxPropertyCategory";
    }

    std::string class_name = "wx";
    class_name += node->as_string(prop_type);
    class_name += "Property";
    return class_name;
}

void GenXrcPropertyGridItem(Node* node, pugi::xml_node& parent)
{
    pugi::xml_node item = parent.append_child("property");

    const std::string class_name = GetXrcPropertyClass(node);
    item.append_attribute("class").set_value(class_name.c_str());

    item.append_child("label").text().set(node->as_string(prop_label));
    item.append_child("name").text().set(node->as_string(prop_var_name));

    if (node->HasProp(prop_help) && node->HasValue(prop_help))
    {
        item.append_child("tip").text().set(node->as_string(prop_help));
    }

    GenXrcPropertyGridChildren(node, item);
}

void GenXrcPropertyGridChildren(Node* node, pugi::xml_node& parent)
{
    for (const auto& child: node->get_ChildNodePtrs())
    {
        if (child->is_Gen(gen_propGridItem) || child->is_Gen(gen_propGridCategory))
        {
            GenXrcPropertyGridItem(child.get(), parent);
        }
    }
}

std::string GetXrcPropertyGridStyle(Node* node, std::string_view default_style)
{
    const wxue::string& style = node->as_string(prop_style);

    std::string result;
    size_t start = 0;
    while (start < style.size())
    {
        size_t end = style.find('|', start);
        if (end == wxue::string::npos)
        {
            end = style.size();
        }

        const std::string_view token = std::string_view(style).substr(start, end - start);
        if (!token.empty() && token != default_style)
        {
            if (!result.empty())
            {
                result += '|';
            }
            result += token;
        }

        start = end + 1;
    }

    return result;
}

// clang-format off
static const std::initializer_list<const char*> advanced_items = {

    "Colour",
    "Cursor",
    "Date",
    "Font",
    "ImageFile",
    "MultiChoice",
    "SystemColour",

};
// clang-format on

bool CheckAdvancePropertyInclude(Node* node)
{
    for (const auto& child: node->get_ChildNodePtrs())
    {
        if (child->is_Gen(gen_propGridItem))
        {
            const wxue::string& value = child->as_string(prop_type);
            for (const auto& iter: advanced_items)
            {
                if (value == iter)
                {
                    return true;
                }
            }
        }
        else if (child->is_Gen(gen_propGridPage))
        {
            if (CheckAdvancePropertyInclude(child.get()))
            {
                return true;
            }
        }
    }
    return false;
}
