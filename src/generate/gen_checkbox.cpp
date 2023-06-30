/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxCheckBox generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/checkbox.h>  // wxCheckBox class interface

#include "code.h"        // Code -- Helper class for generating code
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_checkbox.h"

wxObject* CheckBoxGenerator::CreateMockup(Node* node, wxObject* parent)
{
    long style_value = 0;
    if (node->as_string(prop_style).contains("wxALIGN_RIGHT"))
        style_value |= wxALIGN_RIGHT;

    auto widget =
        new wxCheckBox(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_label),
                       DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), style_value | GetStyleInt(node));

    if (node->as_bool(prop_checked))
        widget->SetValue(true);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool CheckBoxGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (prop->isProp(prop_label))
    {
        wxStaticCast(widget, wxCheckBox)->SetLabel(node->as_wxString(prop_label));
        return true;
    }
    else if (prop->isProp(prop_checked))
    {
        wxStaticCast(widget, wxCheckBox)->SetValue(prop->as_bool());
        return true;
    }

    return false;
}

bool CheckBoxGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma().QuotedString(prop_label);
    code.PosSizeFlags(true);

    return true;
}

bool CheckBoxGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_checked))
    {
        code.NodeName().Function("SetValue(").AddTrue().EndFunction();
    }
    return true;
}

bool CheckBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/checkbox.h>", set_src, set_hdr);
    if (node->as_string(prop_validator_variable).size())
        InsertGeneratorInclude(node, "#include <wx/valgen.h>", set_src, set_hdr);
    return true;
}

int CheckBoxGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxCheckBox");

    ADD_ITEM_PROP(prop_label, "label")
    ADD_ITEM_BOOL(prop_checked, "checked")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void CheckBoxGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxCheckBoxXmlHandler");
}

//////////////////////////////////////////  Check3StateGenerator  //////////////////////////////////////////

wxObject* Check3StateGenerator::CreateMockup(Node* node, wxObject* parent)
{
    long style_value = wxCHK_3STATE | GetStyleInt(node);

    auto widget = new wxCheckBox(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_label),
                                 DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), style_value);

    auto& state = node->as_string(prop_initial_state);
    if (state == "wxCHK_UNCHECKED")
        widget->Set3StateValue(wxCHK_UNCHECKED);
    else if (state == "wxCHK_CHECKED")
        widget->Set3StateValue(wxCHK_CHECKED);
    else
        widget->Set3StateValue(wxCHK_UNDETERMINED);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool Check3StateGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (prop->isProp(prop_label))
    {
        wxStaticCast(widget, wxCheckBox)->SetLabel(node->as_wxString(prop_label));
        return true;
    }
    else if (prop->isProp(prop_initial_state))
    {
        auto& state = prop->as_string();
        if (state == "wxCHK_UNCHECKED")
            wxStaticCast(widget, wxCheckBox)->Set3StateValue(wxCHK_UNCHECKED);
        else if (state == "wxCHK_CHECKED")
            wxStaticCast(widget, wxCheckBox)->Set3StateValue(wxCHK_CHECKED);
        else
            wxStaticCast(widget, wxCheckBox)->Set3StateValue(wxCHK_UNDETERMINED);
        return true;
    }

    return false;
}

bool Check3StateGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass(false, "wxCheckBox");
    code.ValidParentName().Comma().as_string(prop_id).Comma().QuotedString(prop_label);
    code.PosSizeForceStyle("wxCHK_3STATE");

    return true;
}

bool Check3StateGenerator::SettingsCode(Code& code)
{
    auto& state = code.node()->as_string(prop_initial_state);
    if (state == "wxCHK_CHECKED")
    {
        code.NodeName().Function("Set3StateValue(").Add("wxCHK_CHECKED").EndFunction();
    }
    else if (state == "wxCHK_UNDETERMINED")
    {
        code.NodeName().Function("Set3StateValue(").Add("wxCHK_UNDETERMINED").EndFunction();
    }

    return true;
}

bool Check3StateGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/checkbox.h>", set_src, set_hdr);
    return true;
}

int Check3StateGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxCheckBox");

    ADD_ITEM_PROP(prop_label, "label")

    if (node->as_string(prop_initial_state) == "wxCHK_CHECKED")
    {
        item.append_child("checked").text().set("1");
    }
    else if (node->as_string(prop_initial_state) == "wxCHK_UNDETERMINED")
    {
        item.append_child("checked").text().set("2");
    }

    tt_string styles(node->as_string(prop_style));
    if (styles.size())
    {
        styles << '|';
    }
    styles << "wxCHK_3STATE";
    GenXrcPreStylePosSize(node, item, styles);

    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void Check3StateGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxCheckBoxXmlHandler");
}
