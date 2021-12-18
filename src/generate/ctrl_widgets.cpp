/////////////////////////////////////////////////////////////////////////////
// Purpose:   Calendar, GenericDir, Search Ctrl component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/calctrl.h>   // date-picker control
#include <wx/dirctrl.h>   // Directory control base header
#include <wx/event.h>     // Event classes
#include <wx/filectrl.h>  // Header for wxFileCtrlBase and other common functions used by
#include <wx/srchctrl.h>  // wxSearchCtrlBase class

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "ctrl_widgets.h"

//////////////////////////////////////////  CalendarCtrlGenerator  //////////////////////////////////////////

wxObject* CalendarCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxCalendarCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, wxDefaultDateTime,
                                     DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> CalendarCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", wxDefaultDateTime";
    GeneratePosSizeFlags(node, code, false, "wxCAL_SHOW_HOLIDAYS");

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

//////////////////////////////////////////  FileCtrlGenerator  //////////////////////////////////////////

wxObject* FileCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    wxString wild;
    if (node->HasValue(prop_wildcard))
        wild = node->prop_as_wxString(prop_wildcard);
    else
        wild = wxFileSelectorDefaultWildcardStr;

    auto widget = new wxFileCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_initial_folder),
                                 node->prop_as_wxString(prop_initial_filename), wild, GetStyleInt(node),
                                 DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size));

    if (!(node->prop_as_int(prop_style) & wxFC_NOSHOWHIDDEN))
        widget->ShowHidden(node->prop_as_bool(prop_show_hidden));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> FileCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    code << "\t";
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    if (node->HasValue(prop_initial_folder))
        code << GenerateQuotedString(node->prop_as_string(prop_initial_folder));
    else
        code << "wxEmptyString";
    code << ", ";

    if (node->HasValue(prop_initial_filename))
        code << GenerateQuotedString(node->prop_as_string(prop_initial_filename));
    else
        code << "wxEmptyString";
    code << ", ";

    if (node->HasValue(prop_wildcard))
        code << GenerateQuotedString(node->prop_as_string(prop_wildcard));
    else
        code << "wxFileSelectorDefaultWildcardStr";
    code << ", ";

    GenStyle(node, code);

    code << ",\n\t\t\t";
    GenPos(node, code);
    code << ", ";
    GenSize(node, code);
    code << ", ";
    if (node->HasValue(prop_window_name))
    {
        code << ", " << node->prop_as_string(prop_window_name);
    }
    code << ");";

    if (node->prop_as_bool(prop_show_hidden))
    {
        code << "\n\t" << node->get_node_name() << "->ShowHidden(true);";
    }

    return code;
}

std::optional<ttlib::cstr> FileCtrlGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool FileCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/filectrl.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  GenericDirCtrlGenerator  //////////////////////////////////////////

wxObject* GenericDirCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxGenericDirCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_defaultfolder),
                                       DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node),
                                       node->prop_as_wxString(prop_filter), node->prop_as_int(prop_defaultfilter));

    widget->ShowHidden(node->prop_as_bool(prop_show_hidden));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> GenericDirCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    code << '\t';
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";
    if (node->HasValue(prop_defaultfolder))
        code << GenerateQuotedString(node->prop_as_string(prop_defaultfolder));
    else
        code << "wxDirDialogDefaultFolderStr";

    if (!node->HasValue(prop_filter) && node->prop_as_int(prop_defaultfilter) == 0 && !node->HasValue(prop_window_name))
    {
        GeneratePosSizeFlags(node, code, false, "wxDIRCTRL_DEFAULT_STYLE");
    }
    else
    {
        code << ",\n\t\t\t";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", ";
        GenStyle(node, code);
        code << ", " << GenerateQuotedString(node->prop_as_string(prop_filter)) << ", "
             << node->prop_as_string(prop_defaultfilter);
        if (node->HasValue(prop_window_name))
        {
            code << ", " << node->prop_as_string(prop_window_name);
        }
        code << ");";
    }

    if (node->prop_as_bool(prop_show_hidden))
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

wxObject* SearchCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxSearchCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_value),
                                   DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->HasValue(prop_hint))
        widget->SetHint(node->prop_as_wxString(prop_hint));

    if (node->HasValue(prop_search_button))
    {
        widget->ShowSearchButton(node->prop_as_bool(prop_search_button));
    }

    if (node->HasValue(prop_cancel_button))
    {
        widget->ShowCancelButton(node->prop_as_bool(prop_cancel_button));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> SearchCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    if (node->HasValue(prop_value))
        code << GenerateQuotedString(node->prop_as_string(prop_value));
    else
        code << "wxEmptyString";

    GeneratePosSizeFlags(node, code, true);

    code.Replace(", wxEmptyString);", ");");

    return code;
}

std::optional<ttlib::cstr> SearchCtrlGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->HasValue(prop_hint))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetHint(" << GenerateQuotedString(node->prop_as_string(prop_hint)) << ");";
    }

    if (node->prop_as_bool(prop_search_button))
    {
        if (code.size())
            code << "\n\t";
        code << node->get_node_name() << "->ShowSearchButton(true);";
    }

    if (node->prop_as_bool(prop_cancel_button))
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
