/////////////////////////////////////////////////////////////////////////////
// Purpose:   Spin and ScrollBar component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/event.h>     // Event classes
#include <wx/scrolbar.h>  // wxScrollBar base header
#include <wx/spinctrl.h>  // wxSpinCtrlBase class

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "spin_widgets.h"

//////////////////////////////////////////  SpinCtrlGenerator  //////////////////////////////////////////

wxObject* SpinCtrlGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxSpinCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString, node->prop_as_wxPoint("pos"),
                       node->prop_as_wxSize("size"), node->prop_as_int(txtStyle) | node->prop_as_int("window_style"),
                       node->prop_as_int("min"), node->prop_as_int("max"), node->prop_as_int("initial"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> SpinCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxSpinCtrl(";
    code << GetParentName(node) << ", " << node->prop_as_string("id") << ", wxEmptyString, ";
    GenPos(node, code);
    code << ", ";
    GenSize(node, code);
    code << ", ";
    GenStyle(node, code);
    code << ", " << node->prop_as_string("min") << ", " << node->prop_as_string("max") << ", "
         << node->prop_as_string("initial");
    if (node->HasValue("window_name"))
    {
        code << ", " << node->prop_as_string("window_name");
    }

    code << ");";

    code.Replace(", wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0);", ");");

    if (code.contains("wxEmptyString"))
    {
        code.Replace("wxEmptyString, ", "wxEmptyString,\n\t\t\t");
        code.insert(0, 1, '\t');
    }

    return code;
}

std::optional<ttlib::cstr> SpinCtrlGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool SpinCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/spinctrl.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  SpinCtrlDoubleGenerator  //////////////////////////////////////////

wxObject* SpinCtrlDoubleGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxSpinCtrlDouble(wxStaticCast(parent, wxWindow), wxID_ANY, node->GetPropertyAsString(txtValue),
                                       node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                                       node->prop_as_int(txtStyle) | node->prop_as_int("window_style"),
                                       node->GetPropertyAsFloat("min"), node->GetPropertyAsFloat("max"),
                                       node->GetPropertyAsFloat("initial"), node->GetPropertyAsFloat("inc"));

    widget->SetDigits(node->prop_as_int("digits"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> SpinCtrlDoubleGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxSpinCtrlDouble(";
    code << GetParentName(node) << ", " << node->prop_as_string("id") << ", wxEmptyString, ";

    GenPos(node, code);
    code << ", ";
    GenSize(node, code);
    code << ", ";
    GenStyle(node, code);
    code << ", " << node->prop_as_string("min") << ", " << node->prop_as_string("max") << ", "
         << node->prop_as_string("initial") << ", " << node->prop_as_string("inc");
    if (node->HasValue("window_name"))
    {
        code << ", " << node->prop_as_string("window_name");
    }

    code << ");";

    code.Replace(", wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0, 1);", ");");

    if (code.contains("wxEmptyString"))
    {
        code.Replace("wxEmptyString, ", "wxEmptyString,\n\t\t\t");
        code.insert(0, 1, '\t');
    }

    return code;
}

std::optional<ttlib::cstr> SpinCtrlDoubleGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    // REVIEW: [KeyWorks - 12-09-2020] What is the default behaviour if this isn't set?
    code << node->get_node_name() << "->SetDigits(" << node->prop_as_string("digits") << ");";

    return code;
}

std::optional<ttlib::cstr> SpinCtrlDoubleGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool SpinCtrlDoubleGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/spinctrl.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  SpinButtonGenerator  //////////////////////////////////////////

wxObject* SpinButtonGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxSpinButton(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"),
                         node->prop_as_wxSize("size"), node->prop_as_int(txtStyle) | node->prop_as_int("window_style"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> SpinButtonGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxSpinButton(";
    code << GetParentName(node) << ", " << node->prop_as_string("id");
    GeneratePosSizeFlags(node, code, false, "wxSP_VERTICAL", "wxSP_VERTICAL");

    code.Replace(", wxID_ANY);", ");");

    return code;
}

std::optional<ttlib::cstr> SpinButtonGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool SpinButtonGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/spinbutt.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  ScrollBarGenerator  //////////////////////////////////////////

wxObject* ScrollBarGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxScrollBar(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                        node->prop_as_int(txtStyle) | node->prop_as_int("window_style"));

    widget->SetScrollbar(node->prop_as_int("position"), node->prop_as_int("thumbsize"), node->prop_as_int("range"),
                         node->prop_as_int("pagesize"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> ScrollBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    code << '\t';  // lead with tab since we add a second line
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxScrollBar(";
    code << GetParentName(node) << ", " << node->prop_as_string("id");
    GeneratePosSizeFlags(node, code);

    code << "\n\t" << node->get_node_name() << "->SetScrollbar(" << node->prop_as_string("position");
    code << ", " << node->prop_as_string("thumbsize") << ", " << node->prop_as_string("range");
    code << ", " << node->prop_as_string("pagesize") << ");";

    return code;
}

std::optional<ttlib::cstr> ScrollBarGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool ScrollBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/scrolbar.h>", set_src, set_hdr);
    return true;
}
