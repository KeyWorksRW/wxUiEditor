/////////////////////////////////////////////////////////////////////////////
// Purpose:   Panel component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/bookctrl.h>  // wxBookCtrlBase: common base class for wxList/Tree/Notebook
#include <wx/collpane.h>  // wxCollapsiblePane
#include <wx/event.h>     // Event classes
#include <wx/panel.h>     // Base header for wxPanel

#include "../mainframe.h"  // MainFrame -- Main window frame
#include "gen_common.h"    // GeneratorLibrary -- Generator classes
#include "node.h"          // Node class

#include "panel_widgets.h"

wxObject* BookPageGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxPanel(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(txt_pos), node->prop_as_wxSize(txt_size),
                    node->prop_as_int(txt_style) | node->prop_as_int(txt_window_style));
    auto book = wxDynamicCast(parent, wxBookCtrlBase);
    if (book)
    {
        auto cur_selection = book->GetSelection();
        book->AddPage(widget, node->prop_as_string(txt_label));
        if (node->prop_as_bool("select"))
        {
            book->SetSelection(book->GetPageCount() - 1);
        }
        else if (cur_selection >= 0)
        {
            book->SetSelection(cur_selection);
        }

        // TODO: [KeyWorks - 11-22-2020] If the page has a bitmap and the parent set bitmap size, then we need to add the
        // image to the parent's image list
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool BookPageGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/panel.h>", set_src, set_hdr);
    return true;
}

std::optional<ttlib::cstr> BookPageGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxPanel(";
    code << GetParentName(node) << ", " << node->prop_as_string("id");

    GeneratePosSizeFlags(node, code);

    code << '\n';
    code << GetParentName(node) << "->AddPage(" << node->get_node_name() << ", ";
    if (node->HasValue(txt_label))
        code << GenerateQuotedString(node->prop_as_string(txt_label));
    else
        code << "wxEmptyString";

    // Default is false, so only add parameter if it is true.
    if (node->prop_as_bool("select"))
        code << ", true";
    code << ");";

    return code;
}

//////////////////////////////////////////  PanelGenerator  //////////////////////////////////////////

wxObject* PanelGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxPanel(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(txt_pos), node->prop_as_wxSize(txt_size),
                    node->prop_as_int(txt_style) | node->prop_as_int("window_style"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool PanelGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/panel.h>", set_src, set_hdr);
    return true;
}

std::optional<ttlib::cstr> PanelGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxPanel(";
    code << GetParentName(node) << ", " << node->prop_as_string("id");

    GeneratePosSizeFlags(node, code);

    return code;
}

//////////////////////////////////////////  CollapsiblePaneGenerator  //////////////////////////////////////////

wxObject* CollapsiblePaneGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxCollapsiblePane(wxStaticCast(parent, wxWindow), wxID_ANY, node->GetPropertyAsString(txt_label),
                                        node->prop_as_wxPoint(txt_pos), node->prop_as_wxSize(txt_size),
                                        node->prop_as_int(txt_style) | node->prop_as_int("window_style"));

    if (GetMockup()->IsShowingHidden())
        widget->Collapse(false);
    else
        widget->Collapse(node->prop_as_bool("collapsed"));

    widget->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &CollapsiblePaneGenerator::OnCollapse, this);

    return widget;
}

void CollapsiblePaneGenerator::OnCollapse(wxCollapsiblePaneEvent& event)
{
    if (auto wxobject = event.GetEventObject(); wxobject)
    {
        auto node = wxGetFrame().GetMockup()->GetNode(wxobject);

        if (wxGetFrame().GetSelectedNode() != node)
        {
            wxGetFrame().GetMockup()->SelectNode(wxobject);
        }
    }

    event.Skip();
}

std::optional<ttlib::cstr> CollapsiblePaneGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxCollapsiblePane(";
    code << GetParentName(node) << ", " << node->prop_as_string("id") << ", ";

    auto& label = node->prop_as_string(txt_label);
    if (label.size())
    {
        code << GenerateQuotedString(label);
    }
    else
    {
        code << "wxEmptyString";
    }

    GeneratePosSizeFlags(node, code, true, "wxCP_DEFAULT_STYLE", "wxCP_DEFAULT_STYLE");

    return code;
}

std::optional<ttlib::cstr> CollapsiblePaneGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_bool("collapsed"))
    {
        if (code.size())
            code << "\n";

        code << node->get_node_name() << "->Collapse();";
    }

    return code;
}

std::optional<ttlib::cstr> CollapsiblePaneGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool CollapsiblePaneGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/collpane.h>", set_src, set_hdr);
    return true;
}
