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
        if (!dlg->GetSizer())
            dlg->SetSizer(sizer);
    }

    sizer->SetMinSize(node->prop_as_wxSize(prop_minimum_size));

    return sizer;
}

void GridSizerGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool /* is_preview */)
{
    if (node->as_bool(prop_hide_children))
    {
        if (auto sizer = wxStaticCast(wxobject, wxSizer); sizer)
            sizer->ShowItems(false);
    }
}

bool GridSizerGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    if (code.node()->prop_as_int(prop_rows) != 0)
    {
        code.Str(prop_rows).Comma();
    }
    code.Str(prop_cols).Comma().Str(prop_vgap).Comma().Str(prop_hgap).EndFunction();

    if (code.HasValue(prop_minimum_size))
    {
        code.Eol().NodeName().Function("SetMinSize(").WxSize(prop_minimum_size).EndFunction();
    }

    return true;
}

bool GridSizerGenerator::AfterChildrenCode(Code& code)
{
    if (code.IsTrue(prop_hide_children))
    {
        code.NodeName().Function("ShowItems(").AddFalse().EndFunction();
    }

    auto parent = code.node()->GetParent();
    if (!parent->IsSizer() && !parent->isGen(gen_wxDialog) && !parent->isGen(gen_PanelForm))
    {
        code.Eol(eol_if_needed);
        if (parent->isGen(gen_wxRibbonPanel))
        {
            code.ParentName().Function("SetSizerAndFit(").NodeName().EndFunction();
        }
        else
        {
            if (GetParentName(code.node()) != "this")
            {
                code.ParentName().Function("SetSizerAndFit(").NodeName().EndFunction();
            }
            else
            {
                code.FormFunction("SetSizerAndFit(").NodeName().EndFunction();
            }
        }
    }

    return true;
}

bool GridSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_sizer.cpp
// ../../../wxWidgets/src/xrc/xh_sizer.cpp
// See Handle_wxGridSizer()

int GridSizerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
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
    item.append_attribute("name").set_value(node->as_string(prop_var_name));

    ADD_ITEM_PROP(prop_rows, "rows")
    ADD_ITEM_PROP(prop_cols, "cols")
    ADD_ITEM_PROP(prop_vgap, "vgap")
    ADD_ITEM_PROP(prop_hgap, "hgap")

    if (node->HasValue(prop_minimum_size))
    {
        item.append_child("minsize").text().set(node->as_string(prop_minimum_size));
    }
    else if (node->GetParent()->IsForm() && node->GetParent()->HasValue(prop_minimum_size))
    {
        // As of wxWidgets 3.1.7, minsize can only be used for sizers, and wxSplitterWindow. That's a problem for forms which
        // often can specify their own minimum size. The workaround is to set the minimum size of the parent sizer that we
        // create for most forms.

        item.append_child("minsize").text().set(node->GetParent()->as_string(prop_minimum_size));
    }
    return result;
}

void GridSizerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSizerXmlHandler");
}
