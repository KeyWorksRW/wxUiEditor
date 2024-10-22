/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFlexGridSizer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/sizer.h>

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_flexgrid_sizer.h"

wxObject* FlexGridSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxFlexGridSizer* sizer = new wxFlexGridSizer(node->as_int(prop_rows), node->as_int(prop_cols), node->as_int(prop_vgap),
                                                 node->as_int(prop_hgap));
    if (auto dlg = wxDynamicCast(parent, wxDialog); dlg)
    {
        if (!dlg->GetSizer())
            dlg->SetSizer(sizer);
    }

    auto lambda = [&](GenEnum::PropName prop_name)
    {
        if (auto& growable = node->as_string(prop_name); growable.size())
        {
            tt_view_vector values(growable, ',');
            auto rows = node->as_int(prop_rows);
            auto cols = node->as_int(prop_cols);
            int row_or_col = (prop_name == prop_growablerows) ? rows : cols;
            for (auto& iter: values)
            {
                auto value = iter.atoi();
                if (value <= row_or_col)
                {
                    int proportion = 0;
                    if (auto pos = iter.find(':'); tt::is_found(pos))
                    {
                        proportion = tt::atoi(tt::find_nonspace(iter.data() + pos + 1));
                    }
                    if (prop_name == prop_growablerows)
                        sizer->AddGrowableRow(value, proportion);
                    else
                        sizer->AddGrowableCol(value, proportion);
                }
            }
        }
    };
    lambda(prop_growablecols);
    lambda(prop_growablerows);

    sizer->SetMinSize(node->as_wxSize(prop_minimum_size));
    sizer->SetFlexibleDirection(node->as_int(prop_flexible_direction));
    sizer->SetNonFlexibleGrowMode((wxFlexSizerGrowMode) node->as_int(prop_non_flexible_grow_mode));

    return sizer;
}

bool FlexGridSizerGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();

    Node* node = code.node();

    // If rows is empty, only columns are supplied and wxFlexGridSizer will deduece the
    // number of rows to use
    if (node->as_int(prop_rows) != 0)
    {
        code.as_string(prop_rows).Comma();
    }
    code.as_string(prop_cols).Comma().as_string(prop_vgap).Comma().as_string(prop_hgap).EndFunction();

    // If growable settings are used, there can be a lot of lines of code generated. To make
    // it a bit clearer in C++, we put it in braces.
    bool is_within_braces = false;

    auto lambda = [&](GenEnum::PropName prop_name)
    {
        if (auto& growable = node->as_string(prop_name); growable.size())
        {
            tt_view_vector values(growable, ',');
            auto rows = node->as_int(prop_rows);
            auto cols = node->as_int(prop_cols);
            int row_or_col = (prop_name == prop_growablerows) ? rows : cols;
            for (auto& iter: values)
            {
                auto val = iter.atoi();
                if (val <= row_or_col)
                {
                    if (!is_within_braces)
                    {
                        code.OpenBrace();
                        is_within_braces = true;
                    }
                    int proportion = 0;
                    if (auto pos = iter.find(':'); tt::is_found(pos))
                    {
                        proportion = tt::atoi(tt::find_nonspace(iter.data() + pos + 1));
                    }
                    if (!tt::is_whitespace(code.GetCode().back()))
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
        }
    };
    lambda(prop_growablecols);
    lambda(prop_growablerows);

    auto& direction = node->as_string(prop_flexible_direction);
    if (direction.empty() || direction.is_sameas("wxBOTH"))
    {
        if (is_within_braces)
            code.CloseBrace();
        return true;
    }

    code.Eol(eol_if_empty).NodeName().Function("SetFlexibleDirection(").Add(direction).EndFunction();

    auto& non_flex_growth = node->as_string(prop_non_flexible_grow_mode);
    if (non_flex_growth.empty() || non_flex_growth.is_sameas("wxFLEX_GROWMODE_SPECIFIED"))
    {
        if (is_within_braces)
            code.CloseBrace();
        return true;
    }
    code.NodeName().Function("SetNonFlexibleGrowMode").Add(non_flex_growth).EndFunction();
    if (is_within_braces)
        code.CloseBrace();

    return true;
}

void FlexGridSizerGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool /* is_preview */)
{
    if (node->as_bool(prop_hide_children))
    {
        if (auto sizer = wxStaticCast(wxobject, wxSizer); sizer)
            sizer->ShowItems(getMockup()->IsShowingHidden());
    }
}

bool FlexGridSizerGenerator::AfterChildrenCode(Code& code)
{
    if (code.IsTrue(prop_hide_children))
    {
        code.NodeName().Function("ShowItems(").False().EndFunction();
    }

    auto parent = code.node()->getParent();
    if (!parent->isSizer() && !parent->isGen(gen_wxDialog) && !parent->isGen(gen_PanelForm) &&
        !parent->isGen(gen_wxPopupTransientWindow))
    {
        code.Eol(eol_if_needed);
        if (parent->isGen(gen_wxRibbonPanel))
        {
            code.ParentName().Function("SetSizerAndFit(").NodeName().EndFunction();
        }
        else
        {
            if (GetParentName(code.node(), code.get_language()) != "this")
            {
                code.ValidParentName().Function("SetSizerAndFit(");
            }
            else
            {
                if (parent->as_wxSize(prop_size) == wxDefaultSize)
                    code.FormFunction("SetSizerAndFit(");
                else  // Don't call Fit() if size has been specified
                    code.FormFunction("SetSizer(");
            }
            code.NodeName().EndFunction();
        }
    }

    return true;
}

bool FlexGridSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                         GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_sizer.cpp
// ../../../wxWidgets/src/xrc/xh_sizer.cpp
// See Handle_wxFlexGridSizer()

int FlexGridSizerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    pugi::xml_node item;
    auto result = BaseGenerator::xrc_sizer_item_created;

    if (node->getParent()->isSizer())
    {
        GenXrcSizerItem(node, object);
        item = object.append_child("object");
    }
    else
    {
        item = object;
        result = BaseGenerator::xrc_updated;
    }

    item.append_attribute("class").set_value("wxFlexGridSizer");
    item.append_attribute("name").set_value(node->as_string(prop_var_name));

    ADD_ITEM_PROP(prop_rows, "rows")
    ADD_ITEM_PROP(prop_cols, "cols")
    ADD_ITEM_PROP(prop_vgap, "vgap")
    ADD_ITEM_PROP(prop_hgap, "hgap")
    ADD_ITEM_PROP(prop_growablerows, "growablerows")
    ADD_ITEM_PROP(prop_growablecols, "growablecols")
    ADD_ITEM_PROP(prop_flexible_direction, "flexibledirection")
    ADD_ITEM_PROP(prop_non_flexible_grow_mode, "nonflexiblegrowmode")

    ADD_ITEM_BOOL(prop_hide_children, "hideitems");

    if (node->hasValue(prop_minimum_size))
    {
        item.append_child("minsize").text().set(node->as_string(prop_minimum_size));
    }
    else if (node->getParent()->isForm() && node->getParent()->hasValue(prop_minimum_size))
    {
        // As of wxWidgets 3.1.7, minsize can only be used for sizers, and wxSplitterWindow. That's a problem for forms
        // which often can specify their own minimum size. The workaround is to set the minimum size of the parent sizer
        // that we create for most forms.

        item.append_child("minsize").text().set(node->getParent()->as_string(prop_minimum_size));
    }
    return result;
}

void FlexGridSizerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSizerXmlHandler");
}
