/////////////////////////////////////////////////////////////////////////////
// Purpose:   Undoable command classes derived from UndoAction
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "undo_cmds.h"

#include "mainframe.h"     // MainFrame -- Main window frame
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator -- Class used to create nodes
#include "prop_names.h"    // Property names

///////////////////////////////// InsertNodeAction ////////////////////////////////////

InsertNodeAction::InsertNodeAction(Node* node, Node* parent, const ttlib::cstr& undo_str, int pos) :
    UndoAction(undo_str.c_str()), m_pos(pos)
{
    m_old_selected = wxGetFrame().GetSelectedNodePtr();
    m_node = node->GetSharedPtr();
    m_parent = parent->GetSharedPtr();
}

void InsertNodeAction::Change()
{
    m_node->SetParent(m_parent);

    ASSERT_MSG(!m_parent->isGen(gen_wxGridBagSizer),
               "Only use AppendGridBagAction or InsertGridBagAction to add items to a wxGridBagSizer!");

#if 0
    if (m_parent->isGen(gen_wxGridBagSizer) && m_parent->GetChildCount() > 0)
    {
        // This is a child of a wxGridBagSizer, so if m_pos is -1, then add as a new row. If m_pos >= 0, then add as a column

        auto row = -1;
        if (m_pos < 0)
        {
            for (size_t pos = 0; pos < m_parent->GetChildCount(); ++pos)
            {
                auto child_row = m_parent->GetChild(pos)->prop_as_int(prop_row);
                if (child_row > row)
                    row = child_row;
            }

            m_parent->AddChild(m_node);
            m_node->get_prop_ptr(prop_row)->set_value(row + 1);
        }
        else
        {
            if (m_pos > 0)
            {
                // This assumes the children are in row order, which is not necessarily the case
                row = m_parent->GetChild(m_pos - 1)->prop_as_int(prop_row);
            }
            auto col = -1;
            for (size_t pos = 0; pos < m_parent->GetChildCount(); ++pos)
            {
                if (m_parent->GetChild(pos)->prop_as_int(prop_row) == row)
                {
                    auto child_col = m_parent->GetChild(pos)->prop_as_int(prop_column);
                    if (child_col > col)
                    {
                        auto col_span = m_parent->GetChild(pos)->prop_as_int(prop_colspan);
                        col = child_col + (col_span - 1);
                    }
                }
            }
            if (row == -1)
                ++row;
            m_node->get_prop_ptr(prop_row)->set_value(row);
            m_node->get_prop_ptr(prop_column)->set_value(col + 1);

            m_parent->AddChild(m_node);
            m_parent->ChangeChildPosition(m_node, m_pos);
        }
    }
    else
    {
        if (m_pos == -1 && m_parent->IsSizer() && m_parent->GetChildCount() > 0 &&
            m_parent->GetChildPtr(m_parent->GetChildCount() - 1)->isGen(gen_wxStdDialogButtonSizer))
        {
            m_parent->AddChild(m_node);
            m_parent->ChangeChildPosition(m_node, m_parent->GetChildCount() - 2);
        }
        else
        {
            m_parent->AddChild(m_node);
            if (m_pos >= 0)
                m_parent->ChangeChildPosition(m_node, m_pos);
        }
    }
#else
    if (m_pos == -1 && m_parent->IsSizer() && m_parent->GetChildCount() > 0 &&
        m_parent->GetChildPtr(m_parent->GetChildCount() - 1)->isGen(gen_wxStdDialogButtonSizer))
    {
        m_parent->AddChild(m_node);
        m_parent->ChangeChildPosition(m_node, m_parent->GetChildCount() - 2);
    }
    else
    {
        m_parent->AddChild(m_node);
        if (m_pos >= 0)
            m_parent->ChangeChildPosition(m_node, m_pos);
    }
#endif
    wxGetFrame().SelectNode(m_node.get(), false, false);
}

void InsertNodeAction::Revert()
{
    m_parent->RemoveChild(m_node);
    m_node->SetParent(NodeSharedPtr());
    wxGetFrame().SelectNode(m_old_selected.get());
}

///////////////////////////////// RemoveNodeAction ////////////////////////////////////

RemoveNodeAction::RemoveNodeAction(Node* node, const ttlib::cstr& undo_str, bool AddToClipboard) :
    UndoAction(undo_str.c_str())
{
    m_AddToClipboard = AddToClipboard;
    m_node = node->GetSharedPtr();
    m_parent = node->GetParentPtr();
    m_old_pos = m_parent->GetChildPosition(node);
    m_old_selected = wxGetFrame().GetSelectedNodePtr();
}

void RemoveNodeAction::Change()
{
    if (m_AddToClipboard)
        wxGetFrame().CopyNode(m_node.get());

    m_parent->RemoveChild(m_node);
    m_node->SetParent(NodeSharedPtr());

    if (m_parent->GetChildCount())
    {
        auto pos = (m_old_pos < m_parent->GetChildCount() ? m_old_pos : m_parent->GetChildCount() - 1);
        wxGetFrame().SelectNode(m_parent->GetChild(pos));
    }
    else
    {
        wxGetFrame().SelectNode(m_parent.get());
    }
}

void RemoveNodeAction::Revert()
{
    m_parent->AddChild(m_node);
    m_node->SetParent(m_parent);
    m_parent->ChangeChildPosition(m_node, m_old_pos);

    wxGetFrame().SelectNode(m_old_selected.get(), true, false);
}

///////////////////////////////// ModifyPropertyAction ////////////////////////////////////

ModifyPropertyAction::ModifyPropertyAction(NodeProperty* prop, ttlib::cview value) : m_property(prop)
{
    SetUndoString(ttlib::cstr() << "change " << prop->DeclName());

    m_change_value << value;
    m_revert_value = prop->as_string();
}

ModifyPropertyAction::ModifyPropertyAction(NodeProperty* prop, int value) : m_property(prop)
{
    SetUndoString(ttlib::cstr() << "change " << prop->DeclName());

    m_change_value << value;
    m_revert_value = prop->as_string();
}

void ModifyPropertyAction::Change()
{
    m_property->set_value(m_change_value);
}

void ModifyPropertyAction::Revert()
{
    m_property->set_value(m_revert_value);
}

///////////////////////////////// ModifyEventAction ////////////////////////////////////

ModifyEventAction::ModifyEventAction(NodeEvent* event, ttlib::cview value) : m_event(event)
{
    SetUndoString(ttlib::cstr() << "change " << event->get_name() << " handler");

    m_change_value << value;
    m_revert_value = event->get_value();
}

void ModifyEventAction::Change()
{
    m_event->set_value(m_change_value);
}

void ModifyEventAction::Revert()
{
    m_event->set_value(m_revert_value);
}

///////////////////////////////// ChangePositionAction ////////////////////////////////////

ChangePositionAction::ChangePositionAction(Node* node, size_t position)
{
    SetUndoString(ttlib::cstr() << "change " << node->DeclName() << " position");

    m_node = node->GetSharedPtr();
    m_parent = node->GetParentPtr();

    m_change_pos = position;
    m_revert_pos = m_parent->GetChildPosition(node);
}

void ChangePositionAction::Change()
{
    m_parent->ChangeChildPosition(m_node, m_change_pos);
}

void ChangePositionAction::Revert()
{
    m_parent->ChangeChildPosition(m_node, m_revert_pos);
}

///////////////////////////////// ChangeParentAction ////////////////////////////////////

ChangeParentAction::ChangeParentAction(Node* node, Node* parent)
{
    SetUndoString(ttlib::cstr() << "change " << node->DeclName() << " parent");

    m_node = node->GetSharedPtr();
    m_change_parent = parent->GetSharedPtr();
    m_revert_parent = node->GetParentPtr();

    m_revert_position = m_revert_parent->GetChildPosition(node);
    m_revert_row = node->prop_as_int(prop_row);
    m_revert_col = node->prop_as_int(prop_column);
}

void ChangeParentAction::Change()
{
    if (m_change_parent->AddChild(m_node))
    {
        m_revert_parent->RemoveChild(m_node);
        m_node->SetParent(m_change_parent);

        // TODO: [KeyWorks - 11-18-2020] If we got moved into a gridbag sizer, then things are a bit complicated since row
        // and column aren't going to be right. We need to make some intelligent guess and change the node's property
        // accordingly
    }
}

void ChangeParentAction::Revert()
{
    m_change_parent->RemoveChild(m_node);

    m_node->SetParent(m_revert_parent);
    m_revert_parent->AddChild(m_node);
    m_revert_parent->ChangeChildPosition(m_node, m_revert_position);
    if (auto prop = m_node->get_prop_ptr(prop_row); prop)
        prop->set_value(m_revert_row);
    if (auto prop = m_node->get_prop_ptr(prop_column); prop)
        prop->set_value(m_revert_col);
}

///////////////////////////////// MultiAction ////////////////////////////////////

void MultiAction::Add(UndoActionPtr command)
{
    m_cmds.emplace_back(command);
}

void MultiAction::Change()
{
    for (auto& cmd: m_cmds)
    {
        cmd->Change();
    }
}

void MultiAction::Revert()
{
    for (auto cmd = m_cmds.rbegin(); cmd != m_cmds.rend(); ++cmd)
    {
        cmd->get()->Revert();
    }
}

///////////////////////////////// AppendGridBagAction ////////////////////////////////////

AppendGridBagAction::AppendGridBagAction(Node* node, Node* parent, const ttlib::cstr& undo_str, int pos) :
    UndoAction(undo_str.c_str()), m_pos(pos)
{
    m_old_selected = wxGetFrame().GetSelectedNodePtr();
    m_node = node->GetSharedPtr();
    m_parent = parent->GetSharedPtr();
}

void AppendGridBagAction::Change()
{
    m_node->SetParent(m_parent);
    if (m_pos == -1 && m_parent->GetChildCount() > 0 &&
        m_parent->GetChildPtr(m_parent->GetChildCount() - 1)->isGen(gen_wxStdDialogButtonSizer))
    {
        m_parent->AddChild(m_node);
        m_parent->ChangeChildPosition(m_node, m_parent->GetChildCount() - 2);
    }
    else
    {
        m_parent->AddChild(m_node);
        if (m_pos >= 0)
            m_parent->ChangeChildPosition(m_node, m_pos);
    }

    wxGetFrame().SelectNode(m_node.get(), false, false);
}

void AppendGridBagAction::Revert()
{
    m_parent->RemoveChild(m_node);
    m_node->SetParent(NodeSharedPtr());
    wxGetFrame().SelectNode(m_old_selected.get());
}

///////////////////////////////// GridBagAction ////////////////////////////////////

GridBagAction::GridBagAction(Node* cur_gbsizer, const ttlib::cstr& undo_str) : UndoAction(undo_str.c_str())
{
    m_cur_gbsizer = cur_gbsizer->GetSharedPtr();
    m_old_gbsizer = g_NodeCreator.MakeCopy(cur_gbsizer);
    auto selected = wxGetFrame().GetSelectedNodePtr();
    if (selected->isGen(gen_wxGridBagSizer))
    {
        m_idx_old_selected = 0;
    }
    else
    {
        for (size_t idx = 0; idx < cur_gbsizer->GetChildCount(); ++idx)
        {
            if (selected == cur_gbsizer->GetChildPtr(idx))
            {
                m_idx_old_selected = idx;
                break;
            }
        }
    }
}

void GridBagAction::Change()
{
    // m_old_cur_gbsizer doesn't get set until Update() is called, which should be after the first time that Change() is
    // called.

    if (m_old_cur_gbsizer)
    {
        m_cur_gbsizer->RemoveAllChildren();
        for (size_t idx = 0; idx < m_old_cur_gbsizer->GetChildCount(); ++idx)
        {
            m_cur_gbsizer->Adopt(g_NodeCreator.MakeCopy(m_old_cur_gbsizer->GetChild(idx)));
        }
        wxGetFrame().SelectNode(m_cur_gbsizer->GetChild(m_idx_cur_selected));
    }
}

void GridBagAction::Revert()
{
    m_cur_gbsizer->RemoveAllChildren();
    for (size_t idx = 0; idx < m_old_gbsizer->GetChildCount(); ++idx)
    {
        m_cur_gbsizer->Adopt(g_NodeCreator.MakeCopy(m_old_gbsizer->GetChild(idx)));
    }

    wxGetFrame().SelectNode(m_cur_gbsizer->GetChild(m_idx_old_selected));
}

void GridBagAction::Update(Node* cur_gbsizer, Node* selected)
{
    m_old_cur_gbsizer = g_NodeCreator.MakeCopy(cur_gbsizer);

    for (size_t idx = 0; idx < cur_gbsizer->GetChildCount(); ++idx)
    {
        if (selected == cur_gbsizer->GetChild(idx))
        {
            m_idx_cur_selected = idx;
            break;
        }
    }
}
