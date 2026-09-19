/////////////////////////////////////////////////////////////////////////////
// Purpose:   Node memory usage dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [09-19-2026]

#include <format>
#include <stdexcept>
#include <tuple>  // for std::ignore

#include "mainframe.h"                   // Main window frame
#include "node.h"                        // Node class
#include "node_info.h"                   // auto-generated: node_info_base.h and node_info_base.cpp
#include "project_handler.h"             // ProjectHandler class
#include "wxue_namespace/wxue_string.h"  // wxue::string

NodeInfo::NodeInfo() {}

NodeInfo::NodeInfo(wxWindow* parent)
{
    std::ignore = Create(parent);
}

void NodeInfo::OnInit(wxInitDialogEvent& /* event */)
{
    // std::locale("") throws std::runtime_error when the environment locale is unavailable.
    std::locale display_locale = std::locale::classic();
    try
    {
        display_locale = std::locale("");
    }
    catch (const std::runtime_error&)
    {
        // Keep the classic locale as the fallback.
    }

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

    Node* cur_sel = m_node ? m_node : wxGetFrame().getSelectedNode();
    if (cur_sel)
    {
        label.clear();
        label << "Generator: gen_" << cur_sel->get_DeclName();
        m_txt_generator->SetLabel(label);
        label.clear();
        label << "Type: type_";
        decltype(GenEnum::map_GenTypes.find(cur_sel->get_GenType())) iter_type =
            GenEnum::map_GenTypes.find(cur_sel->get_GenType());
        if (iter_type != GenEnum::map_GenTypes.end())
        {
            label << iter_type->second;
        }
        else
        {
            label << "unknown";
        }
        m_txt_type->SetLabel(label);

        node_memory.size = 0;
        node_memory.children = 0;
        calc_node_memory(cur_sel, node_memory);
        label = std::format(display_locale, "Memory: {:L} ({:L} node{})", node_memory.size,
                            node_memory.children, node_memory.children == 1 ? "" : "s");
        m_txt_memory->SetLabel(label);
    }

    node_memory = {};
    if (Node* project = Project.get_ProjectNode(); project)
    {
        calc_node_memory(project, node_memory);
    }

    label = std::format(display_locale, "Project: {:L} ({:L} nodes)", node_memory.size,
                        node_memory.children);
    m_txt_project->SetLabel(label);

    Node* clipboard = wxGetFrame().getClipboard();
    if (clipboard)
    {
        node_memory.size = 0;
        node_memory.children = 0;
        calc_node_memory(clipboard, node_memory);
        label = std::format(display_locale, "Clipboard: {:L} ({:L} nodes)", node_memory.size,
                            node_memory.children);
        m_txt_clipboard->SetLabel(label);
    }

    Fit();
}
