/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTreeCtrl component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/event.h>     // Event classes
#include <wx/sizer.h>     // provide wxSizer class for layout
#include <wx/treectrl.h>  // wxTreeCtrl base header
#include <wx/treelist.h>  // wxTreeListCtrl class declaration.

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "tree_widgets.h"

//////////////////////////////////////////  TreeCtrlGenerator  //////////////////////////////////////////

wxObject* TreeCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxTreeCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                 DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> TreeCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);
    GeneratePosSizeFlags(node, code, true, "wxTR_DEFAULT_STYLE");

    code.Replace(", wxID_ANY);", ");");

    return code;
}

std::optional<ttlib::cstr> TreeCtrlGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool TreeCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/treectrl.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  TreeListCtrlGenerator  //////////////////////////////////////////

wxObject* TreeListCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxTreeListCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                     DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

void TreeListCtrlGenerator::AfterCreation(wxObject* wxobject, wxWindow* /* wxparent */)
{
    auto widget = wxStaticCast(wxobject, wxTreeListCtrl);
    auto node = GetMockup()->GetNode(wxobject);

    for (const auto& iter: node->GetChildNodePtrs())
    {
        widget->AppendColumn(iter->prop_as_wxString(prop_label), iter->prop_as_int(prop_width),
                             static_cast<wxAlignment>(iter->prop_as_int(prop_alignment)), iter->prop_as_int(prop_flags));
    }
}

std::optional<ttlib::cstr> TreeListCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxTreeListCtrl(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);
    GeneratePosSizeFlags(node, code, true, "wxTL_DEFAULT_STYLE");

    return code;
}

std::optional<ttlib::cstr> TreeListCtrlGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool TreeListCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/treelist.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  TreeListCtrlColumnGenerator  //////////////////////////////////////////

std::optional<ttlib::cstr> TreeListCtrlColumnGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    code << node->get_parent_name() << "->AppendColumn(" << GenerateQuotedString(node->prop_as_string(prop_label)) << ", ";
    if (node->prop_as_int(prop_width) == -2)
    {
        code << "wxCOL_WIDTH_AUTOSIZE";
    }
    else
    {
        code << node->prop_as_int(prop_width);
    }

    if (node->prop_as_string(prop_alignment) != "wxALIGN_LEFT" || node->prop_as_string(prop_flags) != "wxCOL_RESIZABLE")
    {
        code << ", " << node->prop_as_string(prop_alignment) << ", ";
        if (node->prop_as_string(prop_flags).size())
        {
            code << node->prop_as_string(prop_flags);
        }
        else
        {
            code << "0";
        }
    }

    code << ")";

    return code;
}
