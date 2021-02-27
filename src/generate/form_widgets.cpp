/////////////////////////////////////////////////////////////////////////////
// Purpose:   Form component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// wxFrame, wxPanel, and wxDialog all inherit from wxWindow and therefore the code and settings used for all three is
// identical. This module contains two functions: GenFormCode and GenFormSettings which are called by all three form classes.

#include "pch.h"

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "form_widgets.h"

//////////////////////////////////////////  DialogFormGenerator  //////////////////////////////////////////

wxObject* DialogFormGenerator::Create(Node* /*node*/, wxObject* parent)
{
    // Note that this is NOT a wxDialog -- this function is used to Mockup the dialog, and for that we use a panel
    // inside of a window.
    return new wxPanel(wxStaticCast(parent, wxWindow), wxID_ANY);
}

std::optional<ttlib::cstr> DialogFormGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    // This is the code to add to the source file
    code << node->prop_as_string(txtClassName) << "::" << node->prop_as_string(txtClassName);
    code << "(wxWindow* parent, wxWindowID id, const wxString& title,";
    code << "\n\t\tconst wxPoint& pos, const wxSize& size, long style";
    if (node->prop_as_string("window_name").size())
        code << ", const wxString& name";
    code << ") :";
    code << "\n\twxDialog(parent, id, title, pos, size, style";
    if (node->prop_as_string("window_name").size())
        code << ", name";
    code << ")\n{";

    return code;
}

std::optional<ttlib::cstr> DialogFormGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> DialogFormGenerator::GenCode(const std::string& cmd, Node* node)
{
    return GenFormCode(cmd, node, "wxDialog");
}

std::optional<ttlib::cstr> DialogFormGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    return GenFormSettings(node);
}

bool DialogFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/dialog.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  FrameFormGenerator  //////////////////////////////////////////

wxObject* FrameFormGenerator::Create(Node* /* node */, wxObject* parent)
{
    // Note that this is NOT a wxFrame -- this function is used to Mockup the dialog, and for that we use a panel
    // inside of a window.
    auto panel = new wxPanel(wxStaticCast(parent, wxWindow), wxID_ANY);
    panel->SetBackgroundColour(wxColour(50, 50, 50));
    return panel;
}

std::optional<ttlib::cstr> FrameFormGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    // This is the code to add to the source file
    code << node->prop_as_string(txtClassName) << "::" << node->prop_as_string(txtClassName);
    code << "(wxWindow* parent, wxWindowID id, const wxString& title,";
    code << "\n\t\tconst wxPoint& pos, const wxSize& size, long style";
    if (node->prop_as_string("window_name").size())
        code << ", const wxString& name";
    code << ") :";
    code << "\n\twxFrame(parent, id, title, pos, size, style";
    if (node->prop_as_string("window_name").size())
        code << ", name";
    code << ")\n{";

    return code;
}

std::optional<ttlib::cstr> FrameFormGenerator::GenCode(const std::string& cmd, Node* node)
{
    return GenFormCode(cmd, node, "wxFrame");
}

std::optional<ttlib::cstr> FrameFormGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> FrameFormGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    return GenFormSettings(node);
}

bool FrameFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/frame.h>", set_src, set_hdr);
    if (node->prop_as_bool("aui_managed"))
        InsertGeneratorInclude(node, "#include <wx/aui/aui.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  PanelFormGenerator  //////////////////////////////////////////

wxObject* PanelFormGenerator::Create(Node* /* node */, wxObject* parent)
{
    return new wxPanel(wxStaticCast(parent, wxWindow), wxID_ANY);
}

std::optional<ttlib::cstr> PanelFormGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    // This is the code to add to the source file
    code << node->prop_as_string(txtClassName) << "::" << node->prop_as_string(txtClassName);
    code << "(wxWindow* parent, wxWindowID id,";
    code << "\n\t\tconst wxPoint& pos, const wxSize& size, long style";
    if (node->prop_as_string("window_name").size())
        code << ", const wxString& name";
    code << ") :";
    code << "\n\twxPanel(parent, id, pos, size, style";
    if (node->prop_as_string("window_name").size())
        code << ", name";
    code << ")\n{";

    return code;
}

std::optional<ttlib::cstr> PanelFormGenerator::GenCode(const std::string& cmd, Node* node)
{
    return GenFormCode(cmd, node, "wxPanel");
}

std::optional<ttlib::cstr> PanelFormGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> PanelFormGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    return GenFormSettings(node);
}

bool PanelFormGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/panel.h>", set_src, set_hdr);
    if (node->prop_as_bool("aui_managed"))
        InsertGeneratorInclude(node, "#include <wx/aui/aui.h>", set_src, set_hdr);

    return true;
}
