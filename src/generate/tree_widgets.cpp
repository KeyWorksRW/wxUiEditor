/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTreeCtrl component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/event.h>     // Event classes
#include <wx/treectrl.h>  // wxTreeCtrl base header
#include <wx/treelist.h>  // wxTreeListCtrl class declaration.

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "tree_widgets.h"

//////////////////////////////////////////  TreeCtrlGenerator  //////////////////////////////////////////

wxObject* TreeCtrlGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxTreeCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"), node->prop_as_wxSize("size"),
                       node->prop_as_int(txtStyle) | node->prop_as_int("window_style"));

#if 0
// REVIEW: [KeyWorks - 12-13-2020] This is the original code.

        // dummy nodes
        wxTreeItemId root = tc->AddRoot("root node");
        wxTreeItemId node1 = tc->AppendItem(root, "node1");
        wxTreeItemId node2 = tc->AppendItem(root, "node2");
        wxTreeItemId node3 = tc->AppendItem(node2, "node3");
        if ((style & wxTR_HIDE_ROOT) == 0)
        {
            tc->Expand(root);
        }
        tc->Expand(node1);
        tc->Expand(node2);
        tc->Expand(node3);
#endif

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> TreeCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxTreeCtrl(";
    code << GetParentName(node) << ", " << node->prop_as_string("id");
    GeneratePosSizeFlags(node, code, true, "wxTR_DEFAULT_STYLE", "wxTR_DEFAULT_STYLE");

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

//////////////////////////////////////////  TreeListViewGenerator  //////////////////////////////////////////

wxObject* TreeListViewGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxTreeListCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint("pos"),
                           node->prop_as_wxSize("size"), node->prop_as_int(txtStyle) | node->prop_as_int("window_style"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> TreeListViewGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxTreeListCtrl(";
    code << GetParentName(node) << ", " << node->prop_as_string("id");
    GeneratePosSizeFlags(node, code, true, "wxTL_DEFAULT_STYLE", "wxTL_DEFAULT_STYLE");

    return code;
}

std::optional<ttlib::cstr> TreeListViewGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool TreeListViewGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/treelist.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  TreeListCtrlColumnGenerator  //////////////////////////////////////////

void TreeListCtrlColumnGenerator::AfterCreation(wxObject* wxobject, wxWindow* wxparent)
{
    auto node = GetMockup()->GetNode(wxobject);
    auto treeList = wxDynamicCast(wxparent, wxTreeListCtrl);
    ASSERT(treeList);
    ASSERT(node);

    treeList->AppendColumn(node->GetPropertyAsString(txtVarName), node->prop_as_int(txtWidth),
                           static_cast<wxAlignment>(node->prop_as_int(txtAlignment)), node->GetSizerFlags().GetFlags());
}

std::optional<ttlib::cstr> TreeListCtrlColumnGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    code << node->get_parent_name() << "->AppendColumn(" << GenerateQuotedString(node->get_node_name()) << ", ";
    code << node->prop_as_string(txtWidth) << ", " << node->prop_as_string(txtAlignment) << ", "
         << node->prop_as_string("flag");
    code << ")";

    return code;
}
