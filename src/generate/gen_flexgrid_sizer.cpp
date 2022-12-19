/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFlexGridSizer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/sizer.h>

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_flexgrid_sizer.h"

wxObject* FlexGridSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxFlexGridSizer* sizer = new wxFlexGridSizer(node->prop_as_int(prop_rows), node->prop_as_int(prop_cols),
                                                 node->prop_as_int(prop_vgap), node->prop_as_int(prop_hgap));
    if (auto dlg = wxDynamicCast(parent, wxDialog); dlg)
    {
        if (!dlg->GetSizer())
            dlg->SetSizer(sizer);
    }

    if (auto& growable = node->prop_as_string(prop_growablecols); growable.size())
    {
        auto num_cols = node->prop_as_int(prop_cols);
        ttlib::multiview values(growable, ',');
        for (auto& iter: values)
        {
            auto value = iter.atoi();
            if (value <= num_cols)
            {
                int proportion = 0;
                if (auto pos = iter.find(':'); ttlib::is_found(pos))
                {
                    proportion = ttlib::atoi(ttlib::find_nonspace(iter.data() + pos + 1));
                }
                sizer->AddGrowableCol(value, proportion);
            }
        }
    }
    if (auto& growable = node->prop_as_string(prop_growablerows); growable.size())
    {
        auto num_rows = node->prop_as_int(prop_rows);
        ttlib::multiview values(growable, ',');
        for (auto& iter: values)
        {
            auto value = iter.atoi();
            if (value <= num_rows)
            {
                int proportion = 0;
                if (auto pos = iter.find(':'); ttlib::is_found(pos))
                {
                    proportion = ttlib::atoi(ttlib::find_nonspace(iter.data() + pos + 1));
                }
                sizer->AddGrowableRow(value, proportion);
            }
        }
    }

    sizer->SetMinSize(node->prop_as_wxSize(prop_minimum_size));
    sizer->SetFlexibleDirection(node->prop_as_int(prop_flexible_direction));
    sizer->SetNonFlexibleGrowMode((wxFlexSizerGrowMode) node->prop_as_int(prop_non_flexible_grow_mode));

    return sizer;
}

std::optional<ttlib::sview> FlexGridSizerGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();

    Node* node = code.node();

    // If rows is empty, only columns are supplied and wxFlexGridSizer will deduece the
    // number of rows to use
    if (node->prop_as_int(prop_rows) != 0)
    {
        code.as_string(prop_rows).Comma();
    }
    code.as_string(prop_cols).Comma().as_string(prop_vgap).Comma().as_string(prop_hgap).EndFunction();

    // If growable settings are used, there can be a lot of lines of code generated. To make
    // it a bit clearer in C++, we put it in braces.
    bool is_within_braces = false;

    auto lambda = [&](GenEnum::PropName prop_name)
    {
        if (auto& growable = node->prop_as_string(prop_name); growable.size())
        {
            ttlib::multiview values(growable, ',');
            auto rows = node->prop_as_int(prop_rows);
            auto cols = node->prop_as_int(prop_cols);
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
                    if (auto pos = iter.find(':'); ttlib::is_found(pos))
                    {
                        proportion = ttlib::atoi(ttlib::find_nonspace(iter.data() + pos + 1));
                    }
                    if (!ttlib::is_whitespace(code.m_code.back()))
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

    auto& direction = node->prop_as_string(prop_flexible_direction);
    if (direction.empty() || direction.is_sameas("wxBOTH"))
    {
        if (is_within_braces)
            code.CloseBrace();
        return code.m_code;
    }

    code.NodeName().Function("SetFlexibleDirection").Add(direction).EndFunction();

    auto& non_flex_growth = node->prop_as_string(prop_non_flexible_grow_mode);
    if (non_flex_growth.empty() || non_flex_growth.is_sameas("wxFLEX_GROWMODE_SPECIFIED"))
    {
        if (is_within_braces)
            code.CloseBrace();
        return code.m_code;
    }
    code.NodeName().Function("SetNonFlexibleGrowMode").Add(non_flex_growth).EndFunction();
    if (is_within_braces)
        code.CloseBrace();

    return code.m_code;
}

std::optional<ttlib::sview> FlexGridSizerGenerator::CommonAfterChildren(Code& code)
{
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

void FlexGridSizerGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool /* is_preview */)
{
    if (node->as_bool(prop_hide_children))
    {
        if (auto sizer = wxStaticCast(wxobject, wxSizer); sizer)
            sizer->ShowItems(false);
    }
}

bool FlexGridSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
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

    item.append_attribute("class").set_value("wxFlexGridSizer");
    item.append_attribute("name").set_value(node->prop_as_string(prop_var_name));

    ADD_ITEM_PROP(prop_rows, "rows")
    ADD_ITEM_PROP(prop_cols, "cols")
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
        // As of wxWidgets 3.1.7, minsize can only be used for sizers, and wxSplitterWindow. That's a problem for forms
        // which often can specify their own minimum size. The workaround is to set the minimum size of the parent sizer
        // that we create for most forms.

        item.append_child("minsize").text().set(node->GetParent()->prop_as_string(prop_minimum_size));
    }
    return result;
}

void FlexGridSizerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSizerXmlHandler");
}
