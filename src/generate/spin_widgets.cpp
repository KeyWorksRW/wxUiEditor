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

wxObject* SpinCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxSpinCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString, node->prop_as_wxPoint(prop_pos),
                                 node->prop_as_wxSize(prop_size), GetStyleInt(node), node->prop_as_int(prop_min),
                                 node->prop_as_int(prop_max), node->prop_as_int(prop_initial));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> SpinCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxSpinCtrl(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", wxEmptyString, ";
    GenPos(node, code);
    code << ", ";
    GenSize(node, code);
    code << ", ";
    GenStyle(node, code);
    code << ", " << node->prop_as_string(prop_min) << ", " << node->prop_as_string(prop_max) << ", "
         << node->prop_as_string(prop_initial);
    if (node->HasValue(prop_window_name))
    {
        code << ", " << node->prop_as_string(prop_window_name);
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

wxObject* SpinCtrlDoubleGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxSpinCtrlDouble(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_value),
                                       node->prop_as_wxPoint(prop_pos), node->prop_as_wxSize(prop_size), GetStyleInt(node),
                                       node->prop_as_double(prop_min), node->prop_as_double(prop_max),
                                       node->prop_as_double(prop_initial), node->prop_as_double(prop_inc));

    widget->SetDigits(node->prop_as_int(prop_digits));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> SpinCtrlDoubleGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxSpinCtrlDouble(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", wxEmptyString, ";

    GenPos(node, code);
    code << ", ";
    GenSize(node, code);
    code << ", ";
    GenStyle(node, code);
    code << ", " << node->prop_as_string(prop_min) << ", " << node->prop_as_string(prop_max) << ", "
         << node->prop_as_string(prop_initial) << ", " << node->prop_as_string(prop_inc);
    if (node->HasValue(prop_window_name))
    {
        code << ", " << node->prop_as_string(prop_window_name);
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
    code << node->get_node_name() << "->SetDigits(" << node->prop_as_string(prop_digits) << ");";

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

wxObject* SpinButtonGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxSpinButton(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                                   node->prop_as_wxSize(prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> SpinButtonGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxSpinButton(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);
    GeneratePosSizeFlags(node, code, false, "wxSP_VERTICAL");

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

wxObject* ScrollBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxScrollBar(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                                  node->prop_as_wxSize(prop_size), GetStyleInt(node));

    widget->SetScrollbar(node->prop_as_int(prop_position), node->prop_as_int(prop_thumbsize), node->prop_as_int(prop_range),
                         node->prop_as_int(prop_pagesize));

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
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);
    GeneratePosSizeFlags(node, code);

    code << "\n\t" << node->get_node_name() << "->SetScrollbar(" << node->prop_as_string(prop_position);
    code << ", " << node->prop_as_string(prop_thumbsize) << ", " << node->prop_as_string(prop_range);
    code << ", " << node->prop_as_string(prop_pagesize) << ");";

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
