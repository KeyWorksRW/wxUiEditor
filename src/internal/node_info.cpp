/////////////////////////////////////////////////////////////////////////////
// Purpose:   Node memory usage dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <format>

#include "mainframe.h"                   // Main window frame
#include "node.h"                        // Node class
#include "node_info.h"                   // auto-generated: node_info_base.h and node_info_base.cpp
#include "project_handler.h"             // ProjectHandler class
#include "wxue_namespace/wxue_string.h"  // wxue::string

NodeInfo::NodeInfo() {}

NodeInfo::NodeInfo(wxWindow* parent)
{
    Create(parent);
}

void NodeInfo::OnInit(wxInitDialogEvent& /* event */)
{
    struct NodeMemory
    {
        size_t size { 0 };
        size_t children { 0 };
    };

    auto calc_node_memory = [](this auto&& self, Node* node, NodeMemory& node_memory) -> void
    {
        node_memory.size += node->get_NodeSize();
        ++node_memory.children;

        for (auto& iter: node->get_ChildNodePtrs())
        {
            self(iter.get(), node_memory);
        }
    };

    wxue::string label;
    NodeMemory node_memory;

    auto* cur_sel = m_node ? m_node : wxGetFrame().getSelectedNode();
    if (cur_sel)
    {
        label.clear();
        label << "Generator: gen_" << cur_sel->get_DeclName();
        m_txt_generator->SetLabel(label);
        label.clear();
        label << "Type: type_" << GenEnum::map_GenTypes.at(cur_sel->get_GenType());
        m_txt_type->SetLabel(label);

        node_memory.size = 0;
        node_memory.children = 0;
        calc_node_memory(cur_sel, node_memory);
        label = std::format(std::locale(""), "Memory: {:L} ({:L} node{})", node_memory.size,
                            node_memory.children, node_memory.children == 1 ? "" : "s");
        m_txt_memory->SetLabel(label);
    }

    calc_node_memory(Project.get_ProjectNode(), node_memory);

    label = std::format(std::locale(""), "Project: {:L} ({:L} nodes)", node_memory.size,
                        node_memory.children);
    m_txt_project->SetLabel(label);

    auto* clipboard = wxGetFrame().getClipboard();
    if (clipboard)
    {
        node_memory.size = 0;
        node_memory.children = 0;
        calc_node_memory(clipboard, node_memory);
        label = std::format(std::locale(""), "Clipboard: {:L} ({:L} nodes)", node_memory.size,
                            node_memory.children);
        m_txt_clipboard->SetLabel(label);
    }

    Fit();
}
