/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxGridBagSizer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/gbsizer.h>

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_gridbag_sizer.h"

wxObject* GridBagSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto sizer = new wxGridBagSizer(node->prop_as_int(prop_vgap), node->prop_as_int(prop_hgap));
    if (auto dlg = wxDynamicCast(parent, wxDialog); dlg)
    {
        if (!dlg->GetSizer())
            dlg->SetSizer(sizer);
    }

    if (auto& growable = node->prop_as_string(prop_growablecols); growable.size())
    {
        ttlib::multiview values(growable, ',');
        for (auto& iter: values)
        {
            int proportion = 0;
            if (auto pos = iter.find(':'); ttlib::is_found(pos))
            {
                proportion = ttlib::atoi(ttlib::find_nonspace(iter.data() + pos + 1));
            }
            sizer->AddGrowableCol(iter.atoi(), proportion);
        }
    }
    if (auto& growable = node->prop_as_string(prop_growablerows); growable.size())
    {
        ttlib::multiview values(growable, ',');
        for (auto& iter: values)
        {
            int proportion = 0;
            if (auto pos = iter.find(':'); ttlib::is_found(pos))
            {
                proportion = ttlib::atoi(ttlib::find_nonspace(iter.data() + pos + 1));
            }
            sizer->AddGrowableRow(iter.atoi(), proportion);
        }
    }

    sizer->SetMinSize(node->prop_as_wxSize(prop_minimum_size));
    sizer->SetFlexibleDirection(node->prop_as_int(prop_flexible_direction));
    sizer->SetNonFlexibleGrowMode((wxFlexSizerGrowMode) node->prop_as_int(prop_non_flexible_grow_mode));

    if (node->HasValue(prop_empty_cell_size))
    {
        sizer->SetEmptyCellSize(node->prop_as_wxSize(prop_empty_cell_size));
    }

    return sizer;
}

void GridBagSizerGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool is_preview)
{
    if (node->as_bool(prop_hide_children))
    {
        if (auto sizer = wxStaticCast(wxobject, wxSizer); sizer)
            sizer->ShowItems(false);
    }

    // For storing objects whose postion needs to be determined
    std::vector<std::pair<wxObject*, wxGBSizerItem*>> newNodes;
    wxGBPosition lastPosition(0, 0);

    auto sizer = wxStaticCast(wxobject, wxGridBagSizer);
    if (!sizer)
    {
        FAIL_MSG("This should be a wxGridBagSizer!");
        return;
    }

    auto count = node->GetChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        const wxObject* child;
        if (!is_preview)
            child = GetMockup()->GetChild(wxobject, i);
        else
            child = node->GetChild(i)->GetMockupObject();

        if (!child)
            continue;  // spacer's don't have objects

        // Get the location of the item
        wxGBSpan span(node->prop_as_int(prop_rowspan), node->prop_as_int(prop_colspan));

        int column = node->prop_as_int(prop_column);
        if (column < 0)
        {
            // Needs to be auto positioned after the other children are added

            if (auto item = GetGBSizerItem(node, lastPosition, span, const_cast<wxObject*>(child)); item)
            {
                newNodes.push_back(std::pair<wxObject*, wxGBSizerItem*>(const_cast<wxObject*>(child), item));
            }
            continue;
        }

        wxGBPosition position(node->prop_as_int(prop_row), column);

        if (sizer->CheckForIntersection(position, span))
        {
            // REVIEW: [KeyWorks - 10-22-2020] Not creating it in the Mockup isn't very helpful to the user since
            // they won't be able to see why the item hasn't been created.
            continue;
        }

        lastPosition = position;

        if (auto item = GetGBSizerItem(node, position, span, const_cast<wxObject*>(child)); item)
        {
            sizer->Add(item);
        }
    }

    for (auto& iter: newNodes)
    {
        wxGBPosition position = iter.second->GetPos();
        wxGBSpan span = iter.second->GetSpan();
        int column = position.GetCol();
        while (sizer->CheckForIntersection(position, span))
        {
            column++;
            position.SetCol(column);
        }
        iter.second->SetPos(position);
        sizer->Add(iter.second);
    }
}

std::optional<ttlib::cstr> GridBagSizerGenerator::GenConstruction(Node* node)
{
    // The leading tab is in case we indent in a brace block later on
    ttlib::cstr code("\t");
    if (node->IsLocal())
        code << "auto ";

    code << node->get_node_name() << " = new wxGridBagSizer(";

    auto vgap = node->prop_as_int(prop_vgap);
    auto hgap = node->prop_as_int(prop_hgap);
    if (vgap != 0 || hgap != 0)
    {
        code << vgap << ", " << hgap;
    }
    code << ");";

    // If growable settings are used, there can be a lot of lines of code generated. To make it a bit clearer, we put it
    // in braces
    bool isExpanded = false;

    if (auto& growable = node->prop_as_string(prop_growablecols); growable.size())
    {
        ttlib::multistr values(growable, ',');
        for (auto& iter: values)
        {
            if (!isExpanded)
            {
                code << "\n\t{";
                isExpanded = true;
            }
            auto val = iter.atoi();
            int proportion = 0;
            if (auto pos = iter.find(':'); ttlib::is_found(pos))
            {
                proportion = ttlib::atoi(ttlib::find_nonspace(iter.c_str() + pos + 1));
            }
            code << "\n\t    " << node->get_node_name() << "->AddGrowableCol(" << val;
            if (proportion > 0)
                code << ", " << proportion;
            code << ");";
        }
    }

    if (auto& growable = node->prop_as_string(prop_growablerows); growable.size())
    {
        ttlib::multiview values(growable, ',');
        for (auto& iter: values)
        {
            if (!isExpanded)
            {
                code << "\n\t{";
                isExpanded = true;
            }
            auto val = iter.atoi();
            int proportion = 0;
            if (auto pos = iter.find(':'); ttlib::is_found(pos))
            {
                proportion = ttlib::atoi(ttlib::find_nonspace(iter.data() + pos + 1));
            }
            code << "\n\t    " << node->get_node_name() << "->AddGrowableRow(" << val;
            if (proportion > 0)
                code << ", " << proportion;
            code << ");";
        }
    }

    if (node->HasValue(prop_empty_cell_size))
    {
        code << (isExpanded ? "\n\t    " : "\n\t") << node->get_node_name() << "->SetEmptyCellSize(";
        code << GenerateWxSize(node, prop_empty_cell_size) << ");";
    }

    auto& direction = node->prop_as_string(prop_flexible_direction);
    if (direction.empty() || direction.is_sameas("wxBOTH"))
    {
        if (isExpanded)
            code << "\n\t}";
        return code;
    }

    code << (isExpanded ? "\n\t    " : "\n\t") << node->get_node_name() << "->SetFlexibleDirection(" << direction << ");";

    auto non_flex_growth = node->prop_as_string(prop_non_flexible_grow_mode);
    if (non_flex_growth.empty() || non_flex_growth.is_sameas("wxFLEX_GROWMODE_SPECIFIED"))
    {
        if (isExpanded)
            code << "\n\t}";
        return code;
    }
    code << (isExpanded ? "\n\t    " : "\n\t") << node->get_node_name() << "->SetNonFlexibleGrowMode(" << non_flex_growth
         << ");";

    if (isExpanded)
        code << "\n\t}";
    return code;
}

std::optional<ttlib::cstr> GridBagSizerGenerator::GenAfterChildren(Node* node)
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

bool GridBagSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/gbsizer.h>", set_src, set_hdr);
    return true;
}

wxGBSizerItem* GridBagSizerGenerator::GetGBSizerItem(Node* sizeritem, const wxGBPosition& position, const wxGBSpan& span,
                                                     wxObject* child)
{
    auto sizer_flags = sizeritem->GetSizerFlags();

    if (sizeritem->isGen(gen_spacer))
    {
        return new wxGBSizerItem(sizeritem->prop_as_int(prop_width), sizeritem->prop_as_int(prop_height), position, span,
                                 sizer_flags.GetFlags(), sizer_flags.GetBorderInPixels());
    }

    // Add the child (window or sizer) to the sizer
    auto windowChild = wxDynamicCast(child, wxWindow);
    auto sizerChild = wxDynamicCast(child, wxSizer);

    if (windowChild)
    {
        return new wxGBSizerItem(windowChild, position, span, sizer_flags.GetFlags(), sizer_flags.GetBorderInPixels());
    }
    else if (sizerChild)
    {
        return new wxGBSizerItem(sizerChild, position, span, sizer_flags.GetFlags(), sizer_flags.GetBorderInPixels());
    }
    else
    {
        FAIL_MSG("The GBSizerItem component's child is not a wxWindow or a wxSizer or a Spacer - this should not be "
                 "possible!");
        return nullptr;
    }
}

// ../../wxSnapShot/src/xrc/xh_sizer.cpp
// ../../../wxWidgets/src/xrc/xh_sizer.cpp
// See Handle_wxGridBagSizer()

int GridBagSizerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
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

    item.append_attribute("class").set_value("wxGridBagSizer");
    item.append_attribute("name").set_value(node->prop_as_string(prop_var_name));

    ADD_ITEM_PROP(prop_vgap, "vgap")
    ADD_ITEM_PROP(prop_hgap, "hgap")
    ADD_ITEM_PROP(prop_growablerows, "growablerows")
    ADD_ITEM_PROP(prop_growablecols, "growablecols")
    ADD_ITEM_PROP(prop_flexible_direction, "flexibledirection")
    ADD_ITEM_PROP(prop_non_flexible_grow_mode, "nonflexiblegrowmode")
    ADD_ITEM_BOOL(prop_hide_children, "hideitems");

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

    if (node->HasValue(prop_empty_cell_size))
    {
        item.append_child("empty_cellsize").text().set(node->value(prop_empty_cell_size));
    }

    return result;
}

void GridBagSizerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSizerXmlHandler");
}
