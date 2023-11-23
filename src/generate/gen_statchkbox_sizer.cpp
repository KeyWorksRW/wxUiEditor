/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStaticBoxSizer with wxCheckBox generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/statbox.h>

#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_statchkbox_sizer.h"

wxObject* StaticCheckboxBoxSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxStaticBoxSizer* sizer;

    // When testing, always display the checkbox, otherwise if Python is preferred, then don't
    // display the checkbox since Python doesn't support it.
#if defined(INTERNAL_TESTING)
    if (Project.hasValue(prop_code_preference))
#else
    if (Project.as_string(prop_code_preference) != "Python")
#endif
    {
        long style_value = 0;
        if (node->as_string(prop_style).contains("wxALIGN_RIGHT"))
            style_value |= wxALIGN_RIGHT;

        m_checkbox = new wxCheckBox(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_label),
                                    wxDefaultPosition, wxDefaultSize, style_value);
        if (node->as_bool(prop_checked))
            m_checkbox->SetValue(true);

        if (node->hasValue(prop_tooltip))
            m_checkbox->SetToolTip(node->as_wxString(prop_tooltip));

        auto staticbox = new wxStaticBox(wxStaticCast(parent, wxWindow), wxID_ANY, m_checkbox);

        sizer = new wxStaticBoxSizer(staticbox, node->as_int(prop_orientation));
    }
    else
    {
        sizer = new wxStaticBoxSizer(node->as_int(prop_orientation), wxStaticCast(parent, wxWindow),
                                     node->as_wxString(prop_label));
    }

    if (auto dlg = wxDynamicCast(parent, wxDialog); dlg)
    {
        if (!dlg->GetSizer())
            dlg->SetSizer(sizer);
    }

    if (node->hasValue(prop_minimum_size))
        sizer->SetMinSize(node->as_wxSize(prop_minimum_size));

    return sizer;
}

void StaticCheckboxBoxSizerGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* node,
                                                    bool /* is_preview */)
{
    if (node->as_bool(prop_hidden))
    {
        if (auto sizer = wxStaticCast(wxobject, wxSizer); sizer)
            sizer->ShowItems(false);
    }
}

bool StaticCheckboxBoxSizerGenerator::OnPropertyChange(wxObject* /* widget */, Node* node, NodeProperty* prop)
{
    if (prop->isProp(prop_tooltip))
    {
        m_checkbox->SetToolTip(node->as_wxString(prop_tooltip));
    }

    return false;
}

bool StaticCheckboxBoxSizerGenerator::ConstructionCode(Code& code)
{
    Node* node = code.node();
    if (code.is_cpp())
    {
        code.as_string(prop_checkbox_var_name) << " = new wxCheckBox(";
        code.ValidParentName().Comma().as_string(prop_id).Comma().QuotedString(prop_label).EndFunction();

        auto cur_size = code.size();
        if (GenValidatorSettings(code); code.size() > cur_size)
        {
            code.Eol();
        }
        code.Eol();
    }
    else if (code.is_ruby())
    {
        code.VarName(code.node()->as_string(prop_checkbox_var_name)) << " = Wx::CheckBox.new(";
        code.ValidParentName().Comma().as_string(prop_id).Comma().QuotedString(prop_label).EndFunction();
        code.Eol();
    }
    else if (code.is_python())
    {
        code.Str("# wxPython currently does not support a checkbox as a static box label").Eol();
    }

    code.AddAuto();

    Code parent_name(code.node(), code.get_language());
    if (!node->getParent()->isForm())
    {
        auto parent = node->getParent();
        while (parent)
        {
            if (parent->isContainer())
            {
                parent_name.NodeName(parent);
                break;
            }
            else if (parent->isGen(gen_wxStaticBoxSizer) || parent->isGen(gen_StaticCheckboxBoxSizer) ||
                     parent->isGen(gen_StaticRadioBtnBoxSizer))
            {
                // The () isn't added because Python and Ruby don't use it. C++ adds it in its
                // own section below.
                parent_name.NodeName(parent).Function("GetStaticBox");
                break;
            }
            parent = parent->getParent();
        }
    }
    if (parent_name.empty())
    {
        parent_name.Str(code.is_cpp() ? "this" : "self");
    }

    if (code.is_cpp())
    {
        if (parent_name.ends_with("GetStaticBox"))
            parent_name += "()";
        code.NodeName() << " = new wxStaticBoxSizer(new wxStaticBox(" << parent_name << ", wxID_ANY";
        code.Comma();
        if (Project.as_string(prop_wxWidgets_version) == "3.1")
        {
            code.Eol().Str("#if wxCHECK_VERSION(3, 1, 1)").Eol().Tab();
            code.as_string(prop_checkbox_var_name) << "),";
            code.Eol().Str("#else").Eol().Tab().Str("wxEmptyString),");
            code.Eol().Str("#endif").Eol();
            code.as_string(prop_orientation).EndFunction();
        }
        else
        {
            code.as_string(prop_checkbox_var_name).Str("), ").as_string(prop_orientation).EndFunction();
        }
    }
    else if (code.is_ruby())
    {
        code.NodeName().Assign("wxStaticBoxSizer").Str("(").CreateClass(false, "wxStaticBox", false);
        code.Str(parent_name)
            .Comma()
            .Add("wxID_ANY")
            .Comma()
            .VarName(code.node()->as_string(prop_checkbox_var_name))
            .Str(")");
        code.Comma().Add(prop_orientation).EndFunction();
    }
    else
    {
        code.NodeName().CreateClass(false, "wxStaticBoxSizer").as_string(prop_orientation).Comma().Str(parent_name);
        if (code.hasValue(prop_label))
        {
            code.Comma().QuotedString(prop_label);
        }
        code.EndFunction();
    }

    if (code.hasValue(prop_minimum_size))
    {
        code.Eol().NodeName().Function("SetMinSize(").WxSize(prop_minimum_size).EndFunction();
    }

    return true;
}

bool StaticCheckboxBoxSizerGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_disabled))
    {
        code.Eol(eol_if_needed).NodeName().Function("GetStaticBox()->Enable(").False().EndFunction();
    }

    if (code.hasValue(prop_tooltip) && code.is_cpp())
    {
        code.Eol(eol_if_needed).as_string(prop_checkbox_var_name).Function("SetToolTip(");
        code.QuotedString(prop_tooltip).EndFunction();
    }

    return true;
}

bool StaticCheckboxBoxSizerGenerator::AfterChildrenCode(Code& code)
{
    if (code.IsTrue(prop_hide_children))
    {
        code.NodeName().Function("ShowItems(").False().EndFunction();
    }

    auto parent = code.node()->getParent();
    if (!parent->isSizer() && !parent->isGen(gen_wxDialog) && !parent->isGen(gen_PanelForm))
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
                code.ParentName().Add(".");
                code.Function("SetSizerAndFit(");
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

bool StaticCheckboxBoxSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                                  int /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/statbox.h>", set_src, set_hdr);

    // The checkbox is always a class member, so we need to force it to be added to the header set
    set_hdr.insert("#include <wx/checkbox.h>");
    if (node->hasValue(prop_validator_variable))
        set_src.insert("#include <wx/valgen.h>");
    return true;
}

// ../../wxSnapShot/src/xrc/xh_sizer.cpp
// ../../../wxWidgets/src/xrc/xh_sizer.cpp
// See Handle_wxStaticBoxSizer()

int StaticCheckboxBoxSizerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
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
    item.append_child("orient").text().set(node->as_string(prop_orientation));
    if (node->hasValue(prop_minimum_size))
    {
        item.append_child("minsize").text().set(node->as_string(prop_minimum_size));
    }
    ADD_ITEM_BOOL(prop_hidden, "hideitems");

    auto checkbox = item.append_child("windowlabel");
    auto child = checkbox.append_child("object");
    child.append_attribute("class").set_value("wxCheckBox");
    child.append_child("label").text().set(node->as_string(prop_label));
    if (node->as_bool(prop_checked))
        child.append_child("checked").text().set("1");

    return result;
}

void StaticCheckboxBoxSizerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSizerXmlHandler");
}

std::optional<tt_string> StaticCheckboxBoxSizerGenerator::GetWarning(Node* node, int language)
{
    switch (language)
    {
        case GEN_LANG_PYTHON:
            {
                tt_string msg;
                if (auto form = node->getForm(); form && form->hasValue(prop_class_name))
                {
                    msg << form->as_string(prop_class_name) << ": ";
                }
                msg << "wxPython currently does not support a checkbox as a static box label";
                return msg;
            }

        case GEN_LANG_RUBY:
            return {};

        default:
            return {};
    }
}
