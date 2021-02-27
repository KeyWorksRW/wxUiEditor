/////////////////////////////////////////////////////////////////////////////
// Purpose:   Calendar, GenericDir, Search Ctrl component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/calctrl.h>   // date-picker control
#include <wx/dirctrl.h>   // Directory control base header
#include <wx/event.h>     // Event classes
#include <wx/srchctrl.h>  // wxSearchCtrlBase class

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "ctrl_widgets.h"

//////////////////////////////////////////  CalendarCtrlGenerator  //////////////////////////////////////////

wxObject* CalendarCtrlGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxCalendarCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, wxDefaultDateTime, node->prop_as_wxPoint("pos"),
                           node->prop_as_wxSize("size"), node->prop_as_int(txtStyle) | node->prop_as_int("window_style"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> CalendarCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxCalendarCtrl(";
    code << GetParentName(node) << ", " << node->prop_as_string("id") << ", wxDefaultDateTime";
    GeneratePosSizeFlags(node, code, false, "wxCAL_SHOW_HOLIDAYS", "wxCAL_SHOW_HOLIDAYS");

    code.Replace(", wxDefaultDateTime);", ");");

    return code;
}

std::optional<ttlib::cstr> CalendarCtrlGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool CalendarCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/calctrl.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  GenericDirCtrlGenerator  //////////////////////////////////////////

wxObject* GenericDirCtrlGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxGenericDirCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->GetPropertyAsString("defaultfolder"),
                                       node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                                       node->prop_as_int(txtStyle) | node->prop_as_int("window_style"),
                                       node->GetPropertyAsString("filter"), node->prop_as_int("defaultfilter"));

    widget->ShowHidden(node->prop_as_bool("show_hidden"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> GenericDirCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    code << '\t';
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxGenericDirCtrl(";
    code << GetParentName(node) << ", " << node->prop_as_string("id") << ", ";
    if (node->HasValue("defaultfolder"))
        code << GenerateQuotedString(node->prop_as_string("defaultfolder"));
    else
        code << "wxDirDialogDefaultFolderStr";

    if (!node->HasValue("filter") && node->prop_as_int("defaultfilter") == 0 && !node->HasValue("window_name"))
    {
        GeneratePosSizeFlags(node, code, false, "wxDIRCTRL_DEFAULT_STYLE", "wxDIRCTRL_DEFAULT_STYLE");
    }
    else
    {
        code << ",\n\t\t\t";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", ";
        GenStyle(node, code);
        code << ", " << GenerateQuotedString(node->prop_as_string("filter")) << ", "
             << node->prop_as_string("defaultfilter");
        if (node->HasValue("window_name"))
        {
            code << ", " << node->prop_as_string("window_name");
        }
        code << ");";
    }

    if (node->prop_as_bool("show_hidden"))
    {
        code << "\n\t" << node->get_node_name() << "->ShowHidden(true);";
    }

    return code;
}

std::optional<ttlib::cstr> GenericDirCtrlGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool GenericDirCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/dirctrl.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  SearchCtrlGenerator  //////////////////////////////////////////

wxObject* SearchCtrlGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxSearchCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->GetPropertyAsString(txtValue),
                                   node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                                   node->prop_as_int(txtStyle) | node->prop_as_int("window_style"));

    if (node->HasValue("search_button"))
    {
        widget->ShowSearchButton(node->prop_as_bool("search_button"));
    }

    if (node->HasValue("cancel_button"))
    {
        widget->ShowCancelButton(node->prop_as_bool("cancel_button"));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> SearchCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxSearchCtrl(";
    code << GetParentName(node) << ", " << node->prop_as_string("id") << ", ";

    if (node->HasValue(txtValue))
        code << GenerateQuotedString(node->prop_as_string(txtValue));
    else
        code << "wxEmptyString";

    GeneratePosSizeFlags(node, code, true);

    code.Replace(", wxEmptyString);", ");");

    return code;
}

std::optional<ttlib::cstr> SearchCtrlGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_bool("search_button"))
    {
        if (code.size())
            code << "\n\t";
        code << node->get_node_name() << "->ShowSearchButton(true);";
    }

    if (node->prop_as_bool("cancel_button"))
    {
        if (code.size())
            code << "\n\t";
        code << node->get_node_name() << "->ShowCancelButton(true);";
    }

    return code;
}

std::optional<ttlib::cstr> SearchCtrlGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool SearchCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/srchctrl.h>", set_src, set_hdr);
    return true;
}
