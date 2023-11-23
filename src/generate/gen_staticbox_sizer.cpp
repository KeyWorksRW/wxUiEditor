/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStaticBoxSizer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/sizer.h>
#include <wx/statbox.h>

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_staticbox_sizer.h"

wxObject* StaticBoxSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto sizer =
        new wxStaticBoxSizer(node->as_int(prop_orientation), wxStaticCast(parent, wxWindow), node->as_wxString(prop_label));
    if (auto dlg = wxDynamicCast(parent, wxDialog); dlg)
    {
        if (!dlg->GetSizer())
            dlg->SetSizer(sizer);
    }

    auto min_size = node->as_wxSize(prop_minimum_size);
    if (min_size.x != -1 || min_size.y != -1)
        sizer->SetMinSize(min_size);

    if (node->as_bool(prop_hidden) && !getMockup()->IsShowingHidden())
        sizer->GetStaticBox()->Hide();

    return sizer;
}

void StaticBoxSizerGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node, bool /* is_preview */)
{
    if (node->as_bool(prop_hide_children))
    {
        if (auto sizer = wxStaticCast(wxobject, wxSizer); sizer)
            sizer->ShowItems(false);
    }
}

bool StaticBoxSizerGenerator::ConstructionCode(Code& code)
{
    Node* node = code.node();

    tt_string parent_name(code.is_cpp() ? "this" : "self");
    if (!node->getParent()->isForm())
    {
        auto parent = node->getParent();
        while (parent)
        {
            if (parent->isContainer())
            {
                parent_name = parent->getNodeName();
                break;
            }
            else if (parent->isGen(gen_wxStaticBoxSizer) || parent->isGen(gen_StaticCheckboxBoxSizer) ||
                     parent->isGen(gen_StaticRadioBtnBoxSizer))
            {
                parent_name = parent->getNodeName();
                if (code.is_cpp())
                    parent_name << "->GetStaticBox()";
                else if (code.is_python())
                    parent_name << ".GetStaticBox()";
                else if (code.is_ruby())
                    parent_name << ".get_static_box";
                break;
            }
            parent = parent->getParent();
        }
    }
    code.AddAuto().NodeName().CreateClass().Add(prop_orientation).Comma().Str(parent_name);

    if (auto& label = node->as_string(prop_label); label.size())
    {
        code.Comma().QuotedString(label);
    }
    code.EndFunction();

    if (code.hasValue(prop_minimum_size))
    {
        code.Eol().NodeName().Function("SetMinSize(").WxSize(prop_minimum_size).EndFunction();
    }

    return true;
}

bool StaticBoxSizerGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_disabled))
    {
        code.NodeName().Function("GetStaticBox()->Enable(").False().EndFunction();
    }

    return true;
}

bool StaticBoxSizerGenerator::AfterChildrenCode(Code& code)
{
    if (code.IsTrue(prop_hide_children))
    {
        code.NodeName().Function("ShowItems(").False().EndFunction();
    }

    auto parent = code.node()->getParent();
    if (!parent->isSizer() && !parent->isGen(gen_wxDialog) && !parent->isGen(gen_PanelForm))
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

bool StaticBoxSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                          int /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/statbox.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_sizer.cpp
// ../../../wxWidgets/src/xrc/xh_sizer.cpp
// See Handle_wxStaticBoxSizer()

int StaticBoxSizerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
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

    item.append_attribute("class").set_value("wxStaticBoxSizer");
    item.append_attribute("name").set_value(node->as_string(prop_var_name));
    // item.append_child("orient").text().set(node->as_string(prop_orientation));
    // if (node->hasValue(prop_minimum_size))
    // {
    // item.append_child("minsize").text().set(node->as_string(prop_minimum_size));
    // }

    ADD_ITEM_PROP(prop_orientation, "orient")
    ADD_ITEM_PROP(prop_minimum_size, "minsize")
    ADD_ITEM_PROP(prop_label, "label")
    ADD_ITEM_BOOL(prop_hide_children, "hideitems");

    return result;
}

void StaticBoxSizerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSizerXmlHandler");
}
