/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxGridSizer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/sizer.h>

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_grid_sizer.h"

wxObject* GridSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto sizer = new wxGridSizer(node->prop_as_int(prop_rows), node->prop_as_int(prop_cols), node->prop_as_int(prop_vgap),
                                 node->prop_as_int(prop_hgap));

    if (auto dlg = wxDynamicCast(parent, wxDialog); dlg)
    {
        dlg->SetSizer(sizer);
    }

    sizer->SetMinSize(node->prop_as_wxSize(prop_minimum_size));

    return sizer;
}

std::optional<ttlib::cstr> GridSizerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxGridSizer(";
    auto rows = node->prop_as_int(prop_rows);
    auto cols = node->prop_as_int(prop_cols);
    auto vgap = node->prop_as_int(prop_vgap);
    auto hgap = node->prop_as_int(prop_hgap);

    if (rows != 0)
    {
        code << rows << ", ";
    }
    code << cols;

    if (vgap != 0 || hgap != 0)
    {
        code << ", " << vgap << ", " << hgap;
    }
    code << ");";

    auto min_size = node->prop_as_wxSize(prop_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code << "\n\t" << node->get_node_name() << "->SetMinSize(" << min_size.GetX() << ", " << min_size.GetY() << ");";
    }

    return code;
}

void GridSizerGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool /* is_preview */)
{
    if (node->as_bool(prop_hide_children))
    {
        if (auto sizer = wxStaticCast(wxobject, wxSizer); sizer)
            sizer->ShowItems(false);
    }
}

std::optional<ttlib::cstr> GridSizerGenerator::GenAfterChildren(Node* node)
{
    ttlib::cstr code;
    if (node->as_bool(prop_hide_children))
    {
        code << "\t" << node->get_node_name() << "->ShowItems(false);";
    }

    auto parent = node->GetParent();
    if (!parent->IsSizer() && !parent->isGen(gen_wxDialog) && !parent->isGen(gen_PanelForm))
    {
        if (code.size())
            code << '\n';
        code << "\n\t";

        // The parent node is not a sizer -- which is expected if this is the parent sizer underneath a form or
        // wxPanel.

        if (parent->isGen(gen_wxRibbonPanel))
        {
            code << parent->get_node_name() << "->SetSizerAndFit(" << node->get_node_name() << ");";
        }
        else
        {
            if (GetParentName(node) != "this")
                code << GetParentName(node) << "->";
            code << "SetSizerAndFit(" << node->get_node_name() << ");";
        }
    }

    if (code.size())
        return code;
    else
        return {};
}

bool GridSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_sizer.cpp
// ../../../wxWidgets/src/xrc/xh_sizer.cpp
// See Handle_wxGridSizer()

int GridSizerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool /* add_comments */)
{
    pugi::xml_node item;
    auto result = BaseGenerator::xrc_sizer_item_created;

    if (node->GetParent()->IsSizer())
    {
        GenXrcSizerItem(node, object);
        item = object.append_child("object");
    }
    else
    {
        item = object;
        result = BaseGenerator::xrc_updated;
    }

    item.append_attribute("class").set_value("wxGridSizer");
    item.append_attribute("name").set_value(node->prop_as_string(prop_var_name));

    ADD_ITEM_PROP(prop_rows, "rows")
    ADD_ITEM_PROP(prop_cols, "cols")
    ADD_ITEM_PROP(prop_vgap, "vgap")
    ADD_ITEM_PROP(prop_hgap, "hgap")

    if (node->HasValue(prop_minimum_size))
    {
        item.append_child("minsize").text().set(node->prop_as_string(prop_minimum_size));
    }
    else if (node->GetParent()->IsForm() && node->GetParent()->HasValue(prop_minimum_size))
    {
        // As of wxWidgets 3.1.7, minsize can only be used for sizers, and wxSplitterWindow. That's a problem for forms which
        // often can specify their own minimum size. The workaround is to set the minimum size of the parent sizer that we
        // create for most forms.

        item.append_child("minsize").text().set(node->GetParent()->prop_as_string(prop_minimum_size));
    }
    return result;
}

void GridSizerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSizerXmlHandler");
}
