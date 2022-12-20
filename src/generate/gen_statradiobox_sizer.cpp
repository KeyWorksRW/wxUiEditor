/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStaticBoxSizer with wxRadioButton generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/statbox.h>

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "node.h"           // Node class
#include "project_class.h"  // Project class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_statradiobox_sizer.h"

wxObject* StaticRadioBtnBoxSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxStaticBoxSizer* sizer;

    if (GetProject()->as_string(prop_code_preference) != "Python")
    {
        m_radiobtn = new wxRadioButton(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_label));
        if (node->prop_as_bool(prop_checked))
            m_radiobtn->SetValue(true);
        if (node->HasValue(prop_tooltip))
            m_radiobtn->SetToolTip(node->prop_as_wxString(prop_tooltip));

        auto staticbox = new wxStaticBox(wxStaticCast(parent, wxWindow), wxID_ANY, m_radiobtn);

        sizer = new wxStaticBoxSizer(staticbox, node->prop_as_int(prop_orientation));
    }
    else
    {
        sizer = new wxStaticBoxSizer(node->prop_as_int(prop_orientation), wxStaticCast(parent, wxWindow),
                                     node->prop_as_wxString(prop_label));
    }

    if (auto dlg = wxDynamicCast(parent, wxDialog); dlg)
    {
        if (!dlg->GetSizer())
            dlg->SetSizer(sizer);
    }

    if (node->HasValue(prop_minimum_size))
        sizer->SetMinSize(node->as_wxSize(prop_minimum_size));

    return sizer;
}

void StaticRadioBtnBoxSizerGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node,
                                                    bool /* is_preview */)
{
    if (node->as_bool(prop_hidden))
    {
        if (auto sizer = wxStaticCast(wxobject, wxSizer); sizer)
            sizer->ShowItems(false);
    }
}

bool StaticRadioBtnBoxSizerGenerator::OnPropertyChange(wxObject* /* widget */, Node* node, NodeProperty* prop)
{
    if (prop->isProp(prop_tooltip))
    {
        m_radiobtn->SetToolTip(node->prop_as_wxString(prop_tooltip));
    }

    return false;
}

std::optional<ttlib::sview> StaticRadioBtnBoxSizerGenerator::CommonConstruction(Code& code)
{
    Node* node = code.node();
    if (code.is_cpp())
    {
        code.as_string(prop_radiobtn_var_name) << " = new wxRadioButton(";
        code.GetParentName().Comma().as_string(prop_id).Comma().QuotedString(prop_label).EndFunction();

        if (auto result = GenValidatorSettings(node); result)
        {
            code.Str(result.value());
        }
        code.Eol();
    }
    else
    {
        code.Str("# wxPython currently does not support a radio button as a static box label").Eol();
    }

    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";

    ttlib::cstr parent_name(code.is_cpp() ? "this" : "self");
    if (!node->GetParent()->IsForm())
    {
        auto parent = node->GetParent();
        while (parent)
        {
            if (parent->IsContainer())
            {
                parent_name = parent->get_node_name();
                break;
            }
            else if (parent->isGen(gen_wxStaticBoxSizer) || parent->isGen(gen_StaticCheckboxBoxSizer) ||
                     parent->isGen(gen_StaticRadioBtnBoxSizer))
            {
                parent_name = parent->get_node_name();
                if (code.is_cpp())
                    parent_name << "->GetStaticBox()";
                else
                    parent_name << ".GetStaticBox()";
                break;
            }
            parent = parent->GetParent();
        }
    }
    if (code.is_cpp())
    {
        code.NodeName() << " = new wxStaticBoxSizer(new wxStaticBox(" << parent_name << ", wxID_ANY";
        code.Comma();
        if (wxGetProject().value(prop_wxWidgets_version) == "3.1")
        {
            code.Eol().Str("#if wxCHECK_VERSION(3, 1, 1)").Eol().Tab();
            code.as_string(prop_radiobtn_var_name) << "),";
            code.Eol().Str("#else").Eol().Tab().Str("wxEmptyString),");
            code.Eol().Str("#endif").Eol();
            code.as_string(prop_orientation).EndFunction();
        }
        else
        {
            code.as_string(prop_radiobtn_var_name).Str("), ").as_string(prop_orientation).EndFunction();
        }
    }
    else
    {
        code.NodeName().CreateClass(false, "wxStaticBoxSizer").as_string(prop_orientation).Comma().Str(parent_name);
        auto& label = node->prop_as_string(prop_label);
        if (label.size())
        {
            code.Comma().QuotedString(label);
        }
        code.EndFunction();
    }

    if (code.HasValue(prop_minimum_size))
    {
        code.Eol().NodeName().Function("SetMinSize(").WxSize(prop_minimum_size).EndFunction();
    }

    return code.m_code;
}

std::optional<ttlib::sview> StaticRadioBtnBoxSizerGenerator::CommonSettings(Code& code)
{
    if (code.IsTrue(prop_disabled))
    {
        if (code.is_cpp())
            code.NodeName().Function("GetStaticBox()->Enable(false);");
        else
            code.NodeName().Function("GetStaticBox().Enable(False)");
    }

    if (code.HasValue(prop_tooltip) && code.is_cpp())
    {
        code.NewLine(true).as_string(prop_radiobtn_var_name).Function("SetToolTip(");
        code.QuotedString(prop_tooltip).EndFunction();
    }

    return code.m_code;
}

std::optional<ttlib::sview> StaticRadioBtnBoxSizerGenerator::CommonAfterChildren(Code& code)
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

bool StaticRadioBtnBoxSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/statbox.h>", set_src, set_hdr);

    // The radiobtn is always a class member, so we need to force it to be added to the header set
    set_hdr.insert("#include <wx/radiobut.h>");
    return true;
}

// ../../wxSnapShot/src/xrc/xh_sizer.cpp
// ../../../wxWidgets/src/xrc/xh_sizer.cpp
// See Handle_wxStaticBoxSizer()

int StaticRadioBtnBoxSizerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
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

    item.append_attribute("class").set_value("wxStaticBoxSizer");
    item.append_attribute("name").set_value(node->prop_as_string(prop_var_name));
    item.append_child("orient").text().set(node->prop_as_string(prop_orientation));
    if (node->HasValue(prop_minimum_size))
    {
        item.append_child("minsize").text().set(node->prop_as_string(prop_minimum_size));
    }
    ADD_ITEM_BOOL(prop_hidden, "hideitems");

    auto checkbox = item.append_child("windowlabel");
    auto child = checkbox.append_child("object");
    child.append_attribute("class").set_value("wxRadioButton");
    child.append_child("label").text().set(node->prop_as_string(prop_label));
    if (node->prop_as_bool(prop_checked))
        child.append_child("checked").text().set("1");

    return result;
}

void StaticRadioBtnBoxSizerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSizerXmlHandler");
}
