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

    auto lambda = [&](GenEnum::PropName prop_name)
    {
        if (auto& growable = node->prop_as_string(prop_name); growable.size())
        {
            ttlib::multiview values(growable, ',');
            for (auto& iter: values)
            {
                int proportion = 0;
                if (auto pos = iter.find(':'); ttlib::is_found(pos))
                {
                    proportion = ttlib::atoi(ttlib::find_nonspace(iter.data() + pos + 1));
                }
                if (prop_name == prop_growablecols)
                    sizer->AddGrowableCol(iter.atoi(), proportion);
                else
                    sizer->AddGrowableRow(iter.atoi(), proportion);
            }
        }
    };
    lambda(prop_growablecols);
    lambda(prop_growablerows);
}

std::optional<ttlib::sview> GridBagSizerGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();

    if (code.HasValue(prop_vgap) || code.HasValue(prop_hgap))
        code.as_string(prop_vgap).Comma().as_string(prop_hgap);
    code.EndFunction();

    Node* node = code.node();
    if (code.HasValue(prop_empty_cell_size))
    {
        code.NodeName().Function("SetEmptyCellSize(").WxSize(prop_empty_cell_size).EndFunction();
    }

    auto& direction = node->prop_as_string(prop_flexible_direction);
    if (direction.empty() || direction.is_sameas("wxBOTH"))
    {
        return code.m_code;
    }

    code.NodeName().Function("SetFlexibleDirection").Add(direction).EndFunction();

    auto& non_flex_growth = node->prop_as_string(prop_non_flexible_grow_mode);
    if (non_flex_growth.empty() || non_flex_growth.is_sameas("wxFLEX_GROWMODE_SPECIFIED"))
    {
        return code.m_code;
    }
    code.NodeName().Function("SetNonFlexibleGrowMode").Add(non_flex_growth).EndFunction();

    return code.m_code;
}

std::optional<ttlib::sview> GridBagSizerGenerator::CommonAfterChildren(Code& code)
{
    Node* node = code.node();

    // If growable settings are used, there can be a lot of lines of code generated. To make
    // it a bit clearer in C++, we put it in braces.
    bool is_within_braces = false;

    auto lambda = [&](GenEnum::PropName prop_name)
    {
        if (auto& growable = node->prop_as_string(prop_name); growable.size())
        {
            ttlib::multiview values(growable, ',');
            for (auto& iter: values)
            {
                auto val = iter.atoi();
                if (!is_within_braces)
                {
                    code.OpenBrace();
                    is_within_braces = true;
                }
                int proportion = 0;
                if (auto pos = iter.find(':'); ttlib::is_found(pos))
                {
                    proportion = ttlib::atoi(ttlib::find_nonspace(iter.data() + pos + 1));
                }
                if (!code.size() || !ttlib::is_whitespace(code.m_code.back()))
                    code.Eol();

                // Note that iter may start with a space, so using itoa() ensures that we
                // don't add any extra space.
                if (prop_name == prop_growablerows)
                    code.NodeName().Function("AddGrowableRow(").itoa(val);
                else
                    code.NodeName().Function("AddGrowableCol(").itoa(val);
                if (proportion > 0)
                    code.Comma().itoa(proportion);
                code.EndFunction();
            }
        }
    };
    lambda(prop_growablecols);
    lambda(prop_growablerows);

    if (code.IsTrue(prop_hide_children))
    {
        code.NodeName().Function("ShowItems(").Str(code.is_cpp() ? "false" : "False").EndFunction();
    }

    auto parent = code.node()->GetParent();
    if (!parent->IsSizer() && !parent->isGen(gen_wxDialog) && !parent->isGen(gen_PanelForm))
    {
        code.NewLine(true);
        if (parent->isGen(gen_wxRibbonPanel))
        {
            code.ParentName().Function("SetSizerAndFit(").NodeName().EndFunction();
        }
        else
        {
            if (GetParentName(code.node()) != "this")
            {
                code.ParentName().Add(".");
                code.Function("SetSizerAndFit(").NodeName().EndFunction();
            }
            else
            {
                code.FormFunction("SetSizerAndFit(").NodeName().EndFunction();
            }
        }
    }

    return code.m_code;
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
