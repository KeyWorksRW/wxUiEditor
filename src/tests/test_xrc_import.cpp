/////////////////////////////////////////////////////////////////////////////
// Purpose:   XRC Import tests
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// XRC exporting can be verified simply by running the XRC Preview dialog. What this test
// does is create an XML document containing the XRC for the selected node, then imports that
// document letting the standard MSG_INFO messages to let us know if there were problems.

#include "../import/import_wxsmith.h"  // Import a wxSmith file
#include "gen_xrc.h"                   // Generate XRC file
#include "mainframe.h"                 // MainFrame -- Main window frame
#include "node.h"                      // Node class
#include "project_class.h"             // Project class
#include "undo_cmds.h"                 // InsertNodeAction -- Undoable command classes derived from UndoAction

#include "pugixml.hpp"

static void CountNodes(Node* node, size_t& nodes)
{
    ++nodes;
    for (auto& iter: node->GetChildNodePtrs())
    {
        CountNodes(iter.get(), nodes);
    }
}

void MainFrame::OnTestXrcImport(wxCommandEvent& /* event */)
{
    if (!m_selected_node)
    {
        wxMessageBox("You need to select a form first.", "Compare");
        return;
    }

    auto form_node = m_selected_node.get();
    if (!form_node->IsForm())
    {
        if (!form_node->isGen(gen_Project))
        {
            form_node = form_node->get_form();
        }
    }

    pugi::xml_document doc;
    {
        // Place this in a block so that the string is destroyed before we process the XML
        // document (to save allocated memory).
        auto doc_str = GenerateXrcStr(form_node, xrc::no_flags);
        auto result = doc.load_string(doc_str.c_str());
        if (!result)
        {
            wxMessageBox("Error parsing XRC document: " + ttlib::cstr(result.description()), "XRC Import Test");
            return;
        }
    }

    auto root = doc.first_child();
    if (!ttlib::is_sameas(root.name(), "resource", tt::CASE::either))
    {
        wxMessageBox("Invalid XRC -- no resource object", "Import XRC Test");
        return;
    }

    MSG_INFO("--- Importing XRC document ---");

    WxSmith doc_import;

    size_t original_node_count = 0;
    CountNodes(form_node, original_node_count);
    size_t new_node_count = 0;
    // If this is an actual form rather than the project, then there will only be one child
    // object, which is the form.
    for (auto& iter: root.children())
    {
        auto new_node = doc_import.CreateXrcNode(iter, nullptr);
        if (new_node)
        {
            CountNodes(new_node.get(), new_node_count);
        }
    }

    if (original_node_count != new_node_count)
    {
        // This isn't always a problem, but it's an indication that something might be wrong.
        MSG_INFO(ttlib::cstr("Original node count: ") << original_node_count);
        MSG_INFO(ttlib::cstr("New node count: ") << new_node_count);
    }
    else
    {
        MSG_INFO("Node counts match");
    }
}

void MainFrame::OnTestXrcDuplicate(wxCommandEvent& /* event */)
{
    if (!m_selected_node)
    {
        wxMessageBox("You need to select a form first.", "Compare");
        return;
    }

    if (m_selected_node.get() == GetProject())
    {
        wxMessageBox("You cannot duplicate the entire project, only forms.", "Test XRC Duplicate");
        return;
    }

    auto form_node = m_selected_node.get();
    if (!form_node->IsForm())
    {
        form_node = form_node->get_form();
    }

    pugi::xml_document doc;
    {
        // Place this in a block so that the string is destroyed before we process the XML
        // document (to save allocated memory).
        auto doc_str = GenerateXrcStr(form_node, xrc::no_flags);
        auto result = doc.load_string(doc_str.c_str());
        if (!result)
        {
            wxMessageBox("Error parsing XRC document: " + ttlib::cstr(result.description()), "XRC Import Test");
            return;
        }
    }

    auto root = doc.first_child();
    if (!ttlib::is_sameas(root.name(), "resource", tt::CASE::either))
    {
        wxMessageBox("Invalid XRC -- no resource object", "Import XRC Test");
        return;
    }

    WxSmith doc_import;

    auto first_child = root.first_child();
    auto new_node = doc_import.CreateXrcNode(first_child, nullptr);
    if (new_node)
    {
        ttlib::cstr undo_str("duplicate ");
        undo_str << new_node->DeclName();
        auto pos = GetProject()->FindInsertionPos(form_node);
        PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), GetProject(), undo_str, pos));
        FireCreatedEvent(new_node);
        SelectNode(new_node, evt_flags::queue_event);
    }
    else
    {
        MSG_ERROR("Failed to create node");
    }
}
