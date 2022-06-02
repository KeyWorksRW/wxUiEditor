/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRadioBox generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/radiobox.h>  // wxRadioBox declaration

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_radio_box.h"

wxObject* RadioBoxGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto choices = node->prop_as_wxArrayString(prop_contents);
    if (!choices.Count())
    {
        choices.Add("at least one choice required");
    }

    auto widget = new wxRadioBox(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_label),
                                 DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), choices,
                                 node->prop_as_int(prop_majorDimension), GetStyleInt(node, "rb_"));

    if (int selection = node->prop_as_int(prop_selection); (to_size_t) selection < choices.Count())
    {
        widget->SetSelection(selection);
    }

    // This is the only event generated when a button within the box is clicked
    widget->Bind(wxEVT_RADIOBOX, &RadioBoxGenerator::OnRadioBox, this);

    return widget;
}

bool RadioBoxGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (prop->isProp(prop_label))
    {
        wxStaticCast(widget, wxRadioBox)->SetLabel(node->prop_as_wxString(prop_label));
        return true;
    }
    else if (prop->isProp(prop_selection))
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
    if (choice_name.starts_with("m_"))
        choice_name.erase(0, 2);
    choice_name << "_choices";
    code << "\twxString " << choice_name << "[] = {";
    auto array = ConvertToArrayString(node->prop_as_string(prop_contents));
    for (auto& iter: array)
    {
        code << "\n\t\t" << GenerateQuotedString(iter) << ",";
    }
    code << "\n\t};\n\t";

    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    auto& label = node->prop_as_string(prop_label);
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
    if (node->prop_as_int(prop_majorDimension) > 0)
    {
        code << ", " << node->prop_as_string(prop_majorDimension);
        isDimSet = true;
    }

    if (node->prop_as_string(prop_window_name).size())
    {
        code << ", ";
        if (!isDimSet)
            code << "0, ";
        GenStyle(node, code, "rb_");
        code << ", wxDefaultValidator, " << node->prop_as_string(prop_window_name) << ");";
    }
    else
    {
        if (node->prop_as_string(prop_window_style).size() || node->prop_as_string(prop_style) != "columns")
        {
            code << ", ";
            if (!isDimSet)
                code << "0, ";
            GenStyle(node, code, "rb_");
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
    if (node->prop_as_string(prop_validator_variable).empty())
    {
        auto sel = node->prop_as_int(prop_selection);
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
    if (node->prop_as_string(prop_validator_variable).size())
        InsertGeneratorInclude(node, "#include <wx/valgen.h>", set_src, set_hdr);

    return true;
}
