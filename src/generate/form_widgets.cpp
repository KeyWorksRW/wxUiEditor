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

std::optional<ttlib::cstr> DialogFormGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    // This is the code to add to the source file
    code << node->prop_as_string(prop_class_name) << "::" << node->prop_as_string(prop_class_name);
    code << "(wxWindow* parent, wxWindowID id, const wxString& title,";
    code << "\n\t\tconst wxPoint& pos, const wxSize& size, long style";
    if (node->prop_as_string(prop_window_name).size())
        code << ", const wxString& name";
    code << ") :";
    code << "\n\twxDialog(parent, id, title, pos, size, style";
    if (node->prop_as_string(prop_window_name).size())
        code << ", name";
    code << ")\n{";

    return code;
}

std::optional<ttlib::cstr> DialogFormGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> DialogFormGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    if (cmd == code_after_children)
    {
        ttlib::cstr code;

        Node* dlg;
        if (node->IsForm())
        {
            dlg = node;
            ASSERT_MSG(dlg->GetChildCount(), "Trying to generate code for a dialog with no children.")
            if (!dlg->GetChildCount())
                return {};  // empty dialog, so nothing to do
            ASSERT_MSG(dlg->GetChild(0)->IsSizer(), "Expected first child of a dialog to be a sizer.");
            if (dlg->GetChild(0)->IsSizer())
                node = dlg->GetChild(0);
        }
        else
        {
            dlg = node->FindParentForm();
        }

        auto min_size = dlg->prop_as_wxSize(prop_minimum_size);
        auto max_size = dlg->prop_as_wxSize(prop_maximum_size);
        auto size = dlg->prop_as_wxSize(prop_size);

        if (min_size == wxDefaultSize && max_size == wxDefaultSize)
        {
            code << "\tSetSizerAndFit(" << node->get_node_name() << ");";
        }
        else
        {
            code << "\tSetSizer(" << node->get_node_name() << ");";
            if (min_size != wxDefaultSize)
            {
                code << "\n\tSetMinSize(wxSize(" << min_size.GetWidth() << ", " << min_size.GetHeight() << "));";
            }
            if (max_size != wxDefaultSize)
            {
                code << "\n\tSetMaxSize(wxSize(" << max_size.GetWidth() << ", " << max_size.GetHeight() << "));";
            }
            code << "\n\tFit();";
        }

        if (size != wxDefaultSize)
        {
            code << "\n\tSetSize(wxSize(" << size.GetWidth() << ", " << size.GetHeight() << "));";
        }

        auto& center = dlg->prop_as_string(prop_center);
        if (center.size() && !center.is_sameas("no"))
        {
            code << "\n\tCentre(" << center << ");";
        }

        return code;
    }
    else
    {
        return GenFormCode(cmd, node);
    }
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

std::optional<ttlib::cstr> FrameFormGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    // This is the code to add to the source file
    code << node->prop_as_string(prop_class_name) << "::" << node->prop_as_string(prop_class_name);
    code << "(wxWindow* parent, wxWindowID id, const wxString& title,";
    code << "\n\t\tconst wxPoint& pos, const wxSize& size, long style";
    if (node->prop_as_string(prop_window_name).size())
        code << ", const wxString& name";
    code << ") :";
    code << "\n\twxFrame(parent, id, title, pos, size, style";
    if (node->prop_as_string(prop_window_name).size())
        code << ", name";
    code << ")\n{";

    return code;
}

std::optional<ttlib::cstr> FrameFormGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    return GenFormCode(cmd, node);
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
    // if (node->prop_as_bool("aui_managed"))
    // InsertGeneratorInclude(node, "#include <wx/aui/aui.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  PopupWinGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> PopupWinGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    // This is the code to add to the source file
    code << node->prop_as_string(prop_class_name) << "::" << node->prop_as_string(prop_class_name);
    code << "(wxWindow* parent, int border) : wxPopupTransientWindow(parent, border)\n{";

    return code;
}

std::optional<ttlib::cstr> PopupWinGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    ttlib::cstr code;

    if (cmd == code_header)
    {
        code << node->get_node_name() << "(wxWindow* parent, int border_flag = " << node->prop_as_string(prop_border);
        code << ");";
        return code;
    }
    else if (cmd == code_base_class)
        return GenFormCode(cmd, node);

    return {};
}

std::optional<ttlib::cstr> PopupWinGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> PopupWinGenerator::GenSettings(Node* /* node */, size_t& /* auto_indent */)
{
    return {};
}

bool PopupWinGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/popupwin.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  PanelFormGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> PanelFormGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    // This is the code to add to the source file
    code << node->prop_as_string(prop_class_name) << "::" << node->prop_as_string(prop_class_name);
    code << "(wxWindow* parent, wxWindowID id,";
    code << "\n\t\tconst wxPoint& pos, const wxSize& size, long style";
    if (node->prop_as_string(prop_window_name).size())
        code << ", const wxString& name";
    code << ") :";
    code << "\n\twxPanel(parent, id, pos, size, style";
    if (node->prop_as_string(prop_window_name).size())
        code << ", name";
    code << ")\n{";

    return code;
}

std::optional<ttlib::cstr> PanelFormGenerator::GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node)
{
    return GenFormCode(cmd, node);
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
    // if (node->prop_as_bool("aui_managed"))
    // InsertGeneratorInclude(node, "#include <wx/aui/aui.h>", set_src, set_hdr);

    return true;
}
