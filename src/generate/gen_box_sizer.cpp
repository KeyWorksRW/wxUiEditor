/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxBoxSizer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/sizer.h>

#include "gen_common.h"       // Common component functions
#include "gen_lang_common.h"  // Common mulit-language functions
#include "node.h"             // Node class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_box_sizer.h"

wxObject* BoxSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto sizer = new wxBoxSizer(node->prop_as_int(prop_orientation));
    sizer->SetMinSize(node->prop_as_wxSize(prop_minimum_size));
    if (auto dlg = wxDynamicCast(parent, wxDialog); dlg)
    {
        if (!dlg->GetSizer())
            dlg->SetSizer(sizer);
    }
    return sizer;
}

std::optional<ttlib::sview> BoxSizerGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";

    code.NodeName().CreateClass().as_string(prop_orientation).EndFunction();

    auto min_size = code.m_node->prop_as_wxSize(prop_minimum_size);
    if (min_size.GetX() != -1 || min_size.GetY() != -1)
    {
        code.Eol().Tab().NodeName().Function("SetMinSize(") << min_size.GetX() << ", " << min_size.GetY();
        code.EndFunction();
    }

    return code.m_code;
}

void BoxSizerGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool /* is_preview */)
{
    if (node->as_bool(prop_hide_children))
    {
        if (auto sizer = wxStaticCast(wxobject, wxSizer); sizer)
            sizer->ShowItems(false);
    }
}

std::optional<ttlib::sview> BoxSizerGenerator::CommonAfterChildren(Code& code)
{
    if (code.m_node->as_bool(prop_hide_children))
    {
        code.Tab().NodeName().Function("ShowItems(false").EndFunction();
    }
    auto parent = code.m_node->GetParent();
    if (!parent->IsSizer() && !parent->isGen(gen_wxDialog) && !parent->isGen(gen_PanelForm))
    {
        if (code.size())
            code.Eol();
        code.Eol().Tab();

        // The parent node is not a sizer -- which is expected if this is the parent sizer underneath a form or
        // wxPanel.

        if (parent->isGen(gen_wxRibbonPanel))
        {
            code.Add(parent->get_node_name()).Function("SetSizerAndFit(").NodeName().EndFunction();
        }
        else
        {
            if (GetParentName(code.m_node) != "this")
                code.Add(GetParentName(code.m_node)).Function("");
            code.Add("SetSizerAndFit(").NodeName().EndFunction();
        }
    }

    if (code.size())
        return code.m_code;
    else
        return {};
}

bool BoxSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_sizer.cpp
// ../../../wxWidgets/src/xrc/xh_sizer.cpp

int BoxSizerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
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

    item.append_attribute("class").set_value("wxBoxSizer");
    item.append_attribute("name").set_value(node->prop_as_string(prop_var_name));
    item.append_child("orient").text().set(node->prop_as_string(prop_orientation));

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

void BoxSizerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSizerXmlHandler");
}
