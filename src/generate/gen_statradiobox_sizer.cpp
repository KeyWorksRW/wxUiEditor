/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStaticBoxSizer with wxRadioButton generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/statbox.h>

#include "gen_common.h"       // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"    // Common XRC generating functions
#include "mainapp.h"          // App -- Main application class
#include "mockup_parent.h"    // Top-level MockUp Parent window
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_statradiobox_sizer.h"

wxObject* StaticRadioBtnBoxSizerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxStaticBoxSizer* sizer;

    // When testing, always display the checkbox, otherwise if Python is preferred, then don't
    // display the checkbox since Python doesn't support it.
    if (Project.as_string(prop_code_preference) != "Python" ||
        (Project.HasValue(prop_code_preference) && wxGetApp().isTestingMenuEnabled()))
    {
        m_radiobtn = new wxRadioButton(wxStaticCast(parent, wxWindow), wxID_ANY,
                                       node->as_wxString(prop_label));
        if (node->as_bool(prop_checked))
        {
            m_radiobtn->SetValue(true);
        }
        if (node->HasValue(prop_tooltip))
        {
            m_radiobtn->SetToolTip(node->as_wxString(prop_tooltip));
        }

        auto staticbox = new wxStaticBox(wxStaticCast(parent, wxWindow), wxID_ANY, m_radiobtn);

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

    if (node->HasValue(prop_minimum_size))
    {
        sizer->SetMinSize(node->as_wxSize(prop_minimum_size));
    }

    return sizer;
}

void StaticRadioBtnBoxSizerGenerator::AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/,
                                                    Node* node, bool /* is_preview */)
{
    if (node->as_bool(prop_hidden))
    {
        if (auto sizer = wxStaticCast(wxobject, wxSizer); sizer)
            sizer->ShowItems(getMockup()->IsShowingHidden());
    }
}

bool StaticRadioBtnBoxSizerGenerator::OnPropertyChange(wxObject* /* widget */, Node* node,
                                                       NodeProperty* prop)
{
    if (prop->isProp(prop_tooltip))
    {
        m_radiobtn->SetToolTip(node->as_wxString(prop_tooltip));
    }

    return false;
}

bool StaticRadioBtnBoxSizerGenerator::ConstructionCode(Code& code)
{
    Node* node = code.node();
    if (code.is_cpp())
    {
        code.as_string(prop_radiobtn_var_name) << " = new wxRadioButton(";
        code.ValidParentName().Comma().as_string(prop_id).Comma().QuotedString(prop_label);
        code.Comma().Pos().Comma().WxSize().Comma().Add("wxRB_SINGLE").EndFunction();

        auto cur_size = code.size();
        if (GenValidatorSettings(code); code.size() > cur_size)
        {
            code.Eol();
        }
        code.Eol();
    }
    else if (code.is_ruby())
    {
        code.VarName(code.node()->as_string(prop_radiobtn_var_name)) << " = Wx::RadioButton.new(";
        code.ValidParentName()
            .Comma()
            .as_string(prop_id)
            .Comma()
            .QuotedString(prop_label)
            .EndFunction();
        code.Eol();
    }
    else if (code.is_python())
    {
        code.Str("# wxPython currently does not support a radio button as a static box label")
            .Eol();
    }

    Code parent_name(code.node(), code.get_language());
    if (!node->get_Parent()->is_Form())
    {
        auto parent = node->get_Parent();
        while (parent)
        {
            if (parent->is_Container())
            {
                parent_name.NodeName(parent);
                break;
            }
            else if (parent->is_Gen(gen_wxStaticBoxSizer) ||
                     parent->is_Gen(gen_StaticCheckboxBoxSizer) ||
                     parent->is_Gen(gen_StaticRadioBtnBoxSizer))
            {
                // The () isn't added because Python and Ruby don't use it. C++ adds it in its
                // own section below.
                parent_name.NodeName(parent).Function("GetStaticBox");
                break;
            }
            parent = parent->get_Parent();
        }
    }
    if (parent_name.empty())
    {
        parent_name.Str(code.is_cpp() ? "this" : code.is_perl() ? "$self" : "self");
    }

    if (code.is_cpp())
    {
        if (parent_name.ends_with("GetStaticBox"))
            parent_name += "()";
        code.AddAuto().NodeName() << " = new wxStaticBoxSizer(new wxStaticBox(" << parent_name
                                  << ", wxID_ANY";
        code.Comma();
        code.as_string(prop_radiobtn_var_name).Str("), ").as_string(prop_orientation).EndFunction();
    }
    else if (code.is_ruby())
    {
        code.NodeName()
            .Assign("wxStaticBoxSizer")
            .Str("(")
            .CreateClass(false, "wxStaticBox", false);
        code.Str(parent_name).Comma().Add("wxID_ANY").Comma();
        code.VarName(code.node()->as_string(prop_radiobtn_var_name)).Str(")");
        code.Comma().Add(prop_orientation).EndFunction();
    }
    else
    {
        code.NodeName()
            .CreateClass(false, "wxStaticBoxSizer")
            .as_string(prop_orientation)
            .Comma()
            .Str(parent_name);
        if (code.HasValue(prop_label))
        {
            code.Comma().QuotedString(prop_label);
        }
        code.EndFunction();
    }

    if (code.HasValue(prop_minimum_size))
    {
        code.Eol().NodeName().Function("SetMinSize(").WxSize(prop_minimum_size).EndFunction();
    }

    return true;
}

bool StaticRadioBtnBoxSizerGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_disabled))
    {
        code.NodeName().Function("GetStaticBox()->Enable(").False().EndFunction();
    }

    if (code.HasValue(prop_tooltip) && code.is_cpp())
    {
        code.Eol(eol_if_needed).as_string(prop_radiobtn_var_name).Function("SetToolTip(");
        code.QuotedString(prop_tooltip).EndFunction();
    }

    return true;
}

bool StaticRadioBtnBoxSizerGenerator::AfterChildrenCode(Code& code)
{
    if (code.IsTrue(prop_hide_children))
    {
        code.NodeName().Function("ShowItems(").False().EndFunction();
    }

    auto parent = code.node()->get_Parent();
    if (!parent->is_Sizer() && !parent->is_Gen(gen_wxDialog) && !parent->is_Gen(gen_PanelForm) &&
        !parent->is_Gen(gen_wxPopupTransientWindow))
    {
        code.NewLine(true);
        if (parent->is_Gen(gen_wxRibbonPanel))
        {
            code.ParentName().Function("SetSizerAndFit(").NodeName().EndFunction();
        }
        else
        {
            if (get_ParentName(code.node(), code.get_language()) != "this")
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

bool StaticRadioBtnBoxSizerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                                  std::set<std::string>& set_hdr,
                                                  GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/sizer.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/statbox.h>", set_src, set_hdr);

    // The radiobtn is always a class member, so we need to force it to be added to the header set
    set_hdr.insert("#include <wx/radiobut.h>");
    if (node->HasValue(prop_validator_variable))
        set_src.insert("#include <wx/valgen.h>");
    return true;
}

// ../../wxSnapShot/src/xrc/xh_sizer.cpp
// ../../../wxWidgets/src/xrc/xh_sizer.cpp
// See Handle_wxStaticBoxSizer()

int StaticRadioBtnBoxSizerGenerator::GenXrcObject(Node* node, pugi::xml_node& object,
                                                  size_t /* xrc_flags */)
{
    pugi::xml_node item;
    auto result = BaseGenerator::xrc_sizer_item_created;

    if (node->get_Parent()->is_Sizer())
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
    if (node->HasValue(prop_minimum_size))
    {
        item.append_child("minsize").text().set(node->as_string(prop_minimum_size));
    }
    ADD_ITEM_BOOL(prop_hidden, "hideitems");

    auto checkbox = item.append_child("windowlabel");
    auto child = checkbox.append_child("object");
    child.append_attribute("class").set_value("wxRadioButton");
    child.append_attribute("name").set_value(node->as_string(prop_radiobtn_var_name));
    child.append_child("label").text().set(node->as_string(prop_label));
    if (node->as_bool(prop_checked))
        child.append_child("checked").text().set("1");

    return result;
}

void StaticRadioBtnBoxSizerGenerator::RequiredHandlers(Node* /* node */,
                                                       std::set<std::string>& handlers)
{
    handlers.emplace("wxSizerXmlHandler");
}

std::optional<wxue::string> StaticRadioBtnBoxSizerGenerator::GetWarning(Node* node, GenLang language)
{
    switch (language)
    {
        case GEN_LANG_PYTHON:
            if (!wxGetApp().isCoverageTesting())
            {
                wxue::string msg;
                if (auto form = node->get_Form(); form && form->HasValue(prop_class_name))
                {
                    msg << form->as_string(prop_class_name) << ": ";
                }
                msg << "wxPython currently does not support a radio button as a static box label";
                return msg;
            }

        case GEN_LANG_RUBY:
            return {};

        default:
            return {};
    }
}
