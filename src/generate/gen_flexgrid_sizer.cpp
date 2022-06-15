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

std::optional<ttlib::cstr> FlexGridSizerGenerator::GenConstruction(Node* node)
{
    // The leading tab is in case we indent in a brace block later on
    ttlib::cstr code("\t");
    if (node->IsLocal())
        code << "auto ";

    code << node->get_node_name() << " = new wxFlexGridSizer(";
    auto rows = node->prop_as_int(prop_rows);
    auto cols = node->prop_as_int(prop_cols);
    auto vgap = node->prop_as_int(prop_vgap);
    auto hgap = node->prop_as_int(prop_hgap);

    // If rows is empty, only columns are supplied and wxFlexGridSizer will deduece the number of rows to use
    if (rows != 0)
    {
        code << rows << ", ";
    }
    code << cols << ", " << vgap << ", " << hgap << ");";

    // If growable settings are used, there can be a lot of lines of code generated. To make it a bit clearer, we put it in
    // braces
    bool isExpanded = false;

    if (auto& growable = node->prop_as_string(prop_growablecols); growable.size())
    {
        ttlib::multiview values(growable, ',');
        for (auto& iter: values)
        {
            auto val = iter.atoi();
            if (val <= cols)
            {
                if (!isExpanded)
                {
                    code << "\n\t{";
                    isExpanded = true;
                }
                int proportion = 0;
                if (auto pos = iter.find(':'); ttlib::is_found(pos))
                {
                    proportion = ttlib::atoi(ttlib::find_nonspace(iter.data() + pos + 1));
                }
                code << "\n\t    " << node->get_node_name() << "->AddGrowableCol(" << val;
                if (proportion > 0)
                    code << ", " << proportion;
                code << ");";
            }
        }
    }

    if (auto& growable = node->prop_as_string(prop_growablerows); growable.size())
    {
        ttlib::multiview values(growable, ',');
        for (auto& iter: values)
        {
            auto val = iter.atoi();
            if (val <= rows)
            {
                if (!isExpanded)
                {
                    code << "\n\t{";
                    isExpanded = true;
                }
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
    }

    auto& direction = node->prop_as_string(prop_flexible_direction);
    if (direction.empty() || direction.is_sameas("wxBOTH"))
    {
        if (isExpanded)
            code << "\n\t}";
        return code;
    }

    code << (isExpanded ? "\n\t    " : "\n\t") << node->get_node_name() << "->SetFlexibleDirection(" << direction << ");";

    auto& non_flex_growth = node->prop_as_string(prop_non_flexible_grow_mode);
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

std::optional<ttlib::cstr> FlexGridSizerGenerator::GenAfterChildren(Node* node)
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

int FlexGridSizerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool /* add_comments */)
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
        // As of wxWidgets 3.1.7, minsize can only be used for sizers, and wxSplitterWindow. That's a problem for forms which
        // often can specify their own minimum size. The workaround is to set the minimum size of the parent sizer that we
        // create for most forms.

        item.append_child("minsize").text().set(node->GetParent()->prop_as_string(prop_minimum_size));
    }
    return result;
}

void FlexGridSizerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSizerXmlHandler");
}
