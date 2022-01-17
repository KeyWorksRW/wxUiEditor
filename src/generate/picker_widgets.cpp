/////////////////////////////////////////////////////////////////////////////
// Purpose:   Picker component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/clrpicker.h>   // wxColourPickerCtrl base header
#include <wx/datectrl.h>    // implements wxDatePickerCtrl
#include <wx/event.h>       // Event classes
#include <wx/filepicker.h>  // wxFilePickerCtrl, wxDirPickerCtrl base header
#include <wx/fontpicker.h>
#include <wx/timectrl.h>  // Declaration of wxTimePickerCtrl class.

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "picker_widgets.h"

//////////////////////////////////////////  DatePickerCtrlGenerator  //////////////////////////////////////////

wxObject* DatePickerCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxDatePickerCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, wxDefaultDateTime, DlgPoint(parent, node, prop_pos),
                             DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> DatePickerCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", wxDefaultDateTime";
    GeneratePosSizeFlags(node, code, true, "wxDP_DEFAULT|wxDP_SHOWCENTURY");

    return code;
}

std::optional<ttlib::cstr> DatePickerCtrlGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool DatePickerCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/datectrl.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/dateevt.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  TimePickerCtrlGenerator  //////////////////////////////////////////

wxObject* TimePickerCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxTimePickerCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, wxDefaultDateTime, DlgPoint(parent, node, prop_pos),
                             DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> TimePickerCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", wxDefaultDateTime";
    GeneratePosSizeFlags(node, code, true, "wxTP_DEFAULT");

    return code;
}

std::optional<ttlib::cstr> TimePickerCtrlGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool TimePickerCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/timectrl.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/dateevt.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  FilePickerGenerator  //////////////////////////////////////////

wxObject* FilePickerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxFilePickerCtrl(
        wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_initial_path),
        node->prop_as_string(prop_message).size() ? node->prop_as_wxString(prop_message) : wxFileSelectorPromptStr,
        node->prop_as_string(prop_wildcard).size() ? node->prop_as_wxString(prop_wildcard) :
                                                     wxFileSelectorDefaultWildcardStr,
        DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> FilePickerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code("\t");
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";
    {
        auto& path = node->prop_as_string(prop_initial_path);
        if (path.size())
        {
            code << GenerateQuotedString(path);
        }
        else
        {
            code << "wxEmptyString";
        }
    }

    code << ", ";
    {
        auto& msg = node->prop_as_string(prop_message);
        if (msg.size())
        {
            code << "\n\t\t" << GenerateQuotedString(msg);
        }
        else
        {
            code << "wxFileSelectorPromptStr";
        }
    }

    code << ", ";
    {
        auto& msg = node->prop_as_string(prop_wildcard);
        if (msg.size())
        {
            code << "\n\t\t" << GenerateQuotedString(msg);
        }
        else
        {
            code << "wxFileSelectorDefaultWildcardStr";
        }
    }

    // Note that wxFLP_DEFAULT_STYLE cannot be specified by the user. We use this to force writing 0 if the user doesn't
    // select any options.
    GeneratePosSizeFlags(node, code, true, "wxFLP_DEFAULT_STYLE");

    return code;
}

std::optional<ttlib::cstr> FilePickerGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_bool(prop_focus))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetFocus()";
    }

    if (code.size())
        return code;
    else

        return {};
}
std::optional<ttlib::cstr> FilePickerGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool FilePickerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/filepicker.h>", set_src, set_hdr);
    return true;
}

std::optional<ttlib::cstr> FilePickerGenerator::GetPropertyDescription(NodeProperty* prop)
{
    if (prop->isProp(prop_message))
    {
        return (
            ttlib::cstr() << "Title bar text for the file picker dialog. If not specified, \"Select a file\" will be used.");
    }
    else
    {
        return {};
    }
}

//////////////////////////////////////////  DirPickerGenerator  //////////////////////////////////////////

wxObject* DirPickerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxDirPickerCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_initial_path),
                                      node->prop_as_string(prop_message).size() ? node->prop_as_wxString(prop_message) :
                                                                                  wxDirSelectorPromptStr,
                                      DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> DirPickerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code("\t");
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";
    {
        auto& path = node->prop_as_string(prop_initial_path);
        if (path.size())
        {
            code << GenerateQuotedString(path);
        }
        else
        {
            code << "wxEmptyString";
        }
    }

    code << ", ";
    {
        auto& msg = node->prop_as_string(prop_message);
        if (msg.size())
        {
            code << "\n\t\t" << GenerateQuotedString(msg);
        }
        else
        {
            code << " wxDirSelectorPromptStr";
        }
    }

    GeneratePosSizeFlags(node, code, true, "wxDIRP_DEFAULT_STYLE");

    return code;
}

std::optional<ttlib::cstr> DirPickerGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_bool(prop_focus))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetFocus()";
    }

    if (code.size())
        return code;
    else

        return {};
}
std::optional<ttlib::cstr> DirPickerGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool DirPickerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/filepicker.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  ColourPickerGenerator  //////////////////////////////////////////

wxObject* ColourPickerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxColourPickerCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxColour(prop_colour),
                               DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> ColourPickerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";
    if (node->prop_as_string(prop_colour).size())
        code << node->prop_as_string(prop_colour);
    else
        code << "*wxBLACK";
    GeneratePosSizeFlags(node, code, true, "wxCLRP_DEFAULT_STYLE");

    return code;
}

std::optional<ttlib::cstr> ColourPickerGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool ColourPickerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/clrpicker.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  FontPickerGenerator  //////////////////////////////////////////

wxObject* FontPickerGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxFontPickerCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_font(prop_initial_font),
                             DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->HasValue(prop_max_point_size))
    {
        widget->SetMaxPointSize(node->prop_as_int(prop_max_point_size));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> FontPickerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";
    if (node->prop_as_string(prop_initial_font).size())
    {
        auto fontprop = node->prop_as_font_prop(prop_initial_font);
        wxFont font = fontprop.GetFont();

        code << "\n\t\t\twxFont(";

        if (fontprop.GetPointSize() <= 0)
            code << "wxNORMAL_FONT->GetPointSize()";
        else
            code << fontprop.GetPointSize();

        code << ", " << ConvertFontFamilyToString(fontprop.GetFamily()) << ", " << font.GetStyleString().wx_str();
        code << ", " << font.GetWeightString().wx_str() << ", " << (fontprop.IsUnderlined() ? "true" : "false") << ", ";
        if (fontprop.GetFaceName().size())
            code << '\"' << fontprop.GetFaceName().wx_str() << '\"';
        else
            code << "wxEmptyString";

        code << ")";
        code.insert(0, 4, ' ');
    }
    else
    {
        code << "wxNullFont";
    }

    GeneratePosSizeFlags(node, code, true, "wxFNTP_DEFAULT_STYLE");

    return code;
}

std::optional<ttlib::cstr> FontPickerGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_string(prop_min_point_size) != "0")
    {
        if (code.size())
            code << "\n\t";
        code << node->get_node_name() << "->SetMinPointSize(" << node->prop_as_string(prop_min_point_size) << ");";
    }

    if (node->prop_as_string(prop_max_point_size) != "100")
    {
        if (code.size())
            code << "\n\t";
        code << node->get_node_name() << "->SetMaxPointSize(" << node->prop_as_string(prop_max_point_size) << ");";
    }

    return code;
}

std::optional<ttlib::cstr> FontPickerGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool FontPickerGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/fontpicker.h>", set_src, set_hdr);
    InsertGeneratorInclude(node, "#include <wx/font.h>", set_src, set_hdr);
    return true;
}
