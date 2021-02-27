/////////////////////////////////////////////////////////////////////////////
// Purpose:   Radio button and Radio box component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/event.h>     // Event classes
#include <wx/radiobox.h>  // wxRadioBox declaration
#include <wx/radiobut.h>  // wxRadioButton declaration

#include "radio_widgets.h"

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

wxObject* RadioButtonGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxRadioButton(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(txtLabel),
                                    node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                                    node->prop_as_int(txtStyle) | node->prop_as_int("window_style"));

    if (node->prop_as_bool("checked"))
        widget->SetValue(true);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool RadioButtonGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (prop->GetPropName() == "label")
    {
        wxStaticCast(widget, wxRadioButton)->SetLabel(node->prop_as_wxString(txtLabel));
        return true;
    }
    else if (prop->GetPropName() == "checked")
    {
        wxStaticCast(widget, wxRadioButton)->SetValue(prop->as_bool());
        return true;
    }

    return false;
}

std::optional<ttlib::cstr> RadioButtonGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxRadioButton(";
    code << GetParentName(node) << ", " << node->prop_as_string("id") << ", ";

    if (node->prop_as_string(txtLabel).size())
        code << GenerateQuotedString(node->prop_as_string(txtLabel));
    else
        code << "wxEmptyString";

    if (node->prop_as_string("window_name").empty())
        GeneratePosSizeFlags(node, code);
    else
    {
        // We have to generate a default validator before the window name, which GeneratePosSizeFlags doesn't do. We don't
        // actually need that validator, since GenSettings will create it, but we have to supply something before the window
        // name.

        code << ", ";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", ";
        GenStyle(node, code);
        code << ", wxDefaultValidator, " << node->prop_as_string("window_name");
        code << ");";
    }

    return code;
}

std::optional<ttlib::cstr> RadioButtonGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    // If a validator has been specified, then the variable will be initialized with the selection variable.
    if (node->prop_as_string("validator_variable").empty())
    {
        if (node->prop_as_bool("checked"))
        {
            code << node->get_node_name() << "->SetValue(true);";
        }
    }

    return code;
}

std::optional<ttlib::cstr> RadioButtonGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool RadioButtonGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/radiobut.h>", set_src, set_hdr);
    if (node->prop_as_string("validator_variable").size())
        InsertGeneratorInclude(node, "#include <wx/valgen.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  RadioBoxGenerator  //////////////////////////////////////////

wxObject* RadioBoxGenerator::Create(Node* node, wxObject* parent)
{
    auto choices = node->prop_as_wxArrayString(txtChoices);
    if (!choices.Count())
    {
        choices.Add("at least one choice required");
    }

    auto widget =
        new wxRadioBox(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(txtLabel),
                       node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"), choices,
                       node->prop_as_int("majorDimension"), node->prop_as_int(txtStyle) | node->prop_as_int("window_style"));

    if (int selection = node->prop_as_int("selection"); static_cast<size_t>(selection) < choices.Count())
    {
        widget->SetSelection(selection);
    }

    // This is the only event generated when a button within the box is clicked
    widget->Bind(wxEVT_RADIOBOX, &RadioBoxGenerator::OnRadioBox, this);

    return widget;
}

bool RadioBoxGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (prop->GetPropName() == "label")
    {
        wxStaticCast(widget, wxRadioBox)->SetLabel(node->prop_as_wxString(txtLabel));
        return true;
    }
    else if (prop->GetPropName() == "selection")
    {
        wxStaticCast(widget, wxRadioBox)->SetSelection(prop->as_int());
        return true;
    }

    return false;
}

void RadioBoxGenerator::OnRadioBox(wxCommandEvent& event)
{
    if (auto window = wxStaticCast(event.GetEventObject(), wxRadioBox); window)
        GetMockup()->SelectNode(window);
}

std::optional<ttlib::cstr> RadioBoxGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    ttlib::cstr choice_name(node->get_node_name());
    if (choice_name.is_sameprefix("m_"))
        choice_name.erase(0, 2);
    choice_name << "_choices";
    code << "\twxString " << choice_name << "[] = {";
    auto array = ConvertToArrayString(node->prop_as_string(txtChoices));
    for (auto& iter: array)
    {
        code << "\n\t\t" << GenerateQuotedString(iter) << ",";
    }
    code << "\n\t};\n\t";

    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxRadioBox(";
    code << GetParentName(node) << ", " << node->prop_as_string("id") << ", ";

    auto& label = node->prop_as_string(txtLabel);
    if (label.size())
    {
        code << GenerateQuotedString(label);
    }
    else
    {
        code << "wxEmptyString";
    }

    code << ",\n\t\t";
    GenPos(node, code);
    code << ", ";
    GenSize(node, code);

    code << ", " << array.size() << ", " << choice_name;

    bool isDimSet = false;
    if (node->prop_as_int("majorDimension") > 0)
    {
        code << ", " << node->prop_as_string("majorDimension");
        isDimSet = true;
    }

    if (node->prop_as_string("window_name").size())
    {
        code << ", ";
        if (!isDimSet)
            code << "0, ";
        GenStyle(node, code);
        code << ", wxDefaultValidator, " << node->prop_as_string("window_name") << ");";
    }
    else
    {
        if (node->prop_as_string("window_style").size() || node->prop_as_string(txtStyle) != "wxRA_SPECIFY_COLS")
        {
            code << ", ";
            if (!isDimSet)
                code << "0, ";
            GenStyle(node, code);
        }
        code << ");";
    }

    return code;
}

std::optional<ttlib::cstr> RadioBoxGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> RadioBoxGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    // If a validator has been specified, then the variable will be initialized with the selection variable.
    if (node->prop_as_string("validator_variable").empty())
    {
        auto sel = node->prop_as_int("selection");
        if (sel > 0)
        {
            code << node->get_node_name() << "->SetSelection(" << sel << ");";
        }
    }

    return code;
}

bool RadioBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/radiobox.h>", set_src, set_hdr);
    if (node->prop_as_string("validator_variable").size())
        InsertGeneratorInclude(node, "#include <wx/valgen.h>", set_src, set_hdr);

    return true;
}
