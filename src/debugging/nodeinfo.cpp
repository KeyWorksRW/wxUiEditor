/////////////////////////////////////////////////////////////////////////////
// Purpose:   Node memory usage dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "nodeinfo.h"  // auto-generated: nodeinfo_base.h and nodeinfo_base.cpp

#include "mainapp.h"     // App -- Main application class
#include "mainframe.h"   // Main window frame
#include "node.h"        // Node class
#include "undo_stack.h"  // UndoAction -- Maintain a undo and redo stack

struct NodeMemory
{
    size_t size { 0 };
    size_t children { 0 };
};

void CalcNodeMemory(Node* node, NodeMemory& node_memory)
{
    node_memory.size += node->GetNodeSize();
    ++node_memory.children;

    for (auto& iter: node->GetChildNodePtrs())
    {
        CalcNodeMemory(iter.get(), node_memory);
    }
}

NodeInfo::NodeInfo(wxWindow* parent) : NodeInfoBase(parent)
{
    auto project = wxGetApp().GetProject();
    NodeMemory node_memory;
    CalcNodeMemory(project, node_memory);

    ttlib::cstr label;
    label.Format("Project: %kzu (%kzu nodes)", node_memory.size, node_memory.children);
    m_txt_project->SetLabel(label);

    auto cur_sel = wxGetFrame().GetSelectedNode();
    if (cur_sel)
    {
        node_memory.size = 0;
        node_memory.children = 0;
        CalcNodeMemory(cur_sel, node_memory);
        label.clear();
        label.Format("Selection: %kzu (%kzu nodes)", node_memory.size, node_memory.children);
        auto name = cur_sel->get_node_name();
        if (name.size())
            label.Replace("Selection", name);
        m_txt_selection->SetLabel(label);
    }

    auto clipboard = wxGetFrame().GetClipboard();
    if (clipboard)
    {
        node_memory.size = 0;
        node_memory.children = 0;
        CalcNodeMemory(clipboard, node_memory);
        label.clear();
        label.Format("Clipboard: %kzu (%kzu nodes)", node_memory.size, node_memory.children);
        m_txt_clipboard->SetLabel(label);
    }

    Fit();
}
