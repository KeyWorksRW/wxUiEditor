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
