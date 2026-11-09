/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to lookup and insert a widget
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [08-22-2026]

#include "insert_widget.h"  // InsertWidget -- derived class

#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator -- Class used to create nodes
#include "project_handler.h"  // ProjectHandler class

// Returns true if the node can accept at least one child node type.
static bool CanHaveChildren(Node* node)
{
    for (auto* decl: NodeCreation.get_NodeDeclarationArray())
    {
        if (decl && node->is_ChildAllowed(decl))
        {
            return true;
        }
    }
    return false;
}

InsertWidget::InsertWidget(wxWindow* parent)
{
    std::ignore = Create(parent);
}

void MainFrame::OnInsertWidget(wxCommandEvent& /* event unused */)
{
    InsertWidget dialog(this);
    if (dialog.ShowModal() == wxID_OK)
    {
        if (auto result = rmap_GenNames.find(dialog.GetWidget()); result != rmap_GenNames.end())
        {
            const InsertWidget::InsertMode mode = dialog.GetInsertMode();
            if (mode != InsertWidget::InsertMode::Child)
            {
                Node* selected = wxGetFrame().getSelectedNode();
                if (selected && !selected->is_Form() && selected->get_Parent())
                {
                    Node* parent = selected->get_Parent();
                    int pos = static_cast<int>(parent->get_ChildPosition(selected));
                    if (mode == InsertWidget::InsertMode::Below)
                    {
                        ++pos;
                    }
                    CreateToolNode(result->second, pos);
                    return;
                }
                // No usable sibling position -- fall back to inserting as a child.
            }
            CreateToolNode(result->second);
            return;
        }
        FAIL_MSG(wxue::string() << "No property enum type exists for " << dialog.GetWidget()
                                << "! This should be impossible...")
    }
}

InsertWidget::InsertMode InsertWidget::GetInsertMode() const
{
    if (!m_radio_insert)
    {
        return InsertMode::Child;
    }
    switch (m_radio_insert->GetSelection())
    {
        case 0:
            return InsertMode::Above;
        case 1:
            return InsertMode::Below;
        default:
            return InsertMode::Child;
    }
}

void InsertWidget::OnInsertMode(wxCommandEvent& /* event unused */)
{
    RebuildList();
}

void InsertWidget::OnInit(wxInitDialogEvent& /* event unused */)
{
    Node* selected = wxGetFrame().getSelectedNode();
    if (!selected)
    {
        // If no node is selected, then if we limit the list to children, there will
        // be nothing in the list. Instead, uncheck the limit, and disable it.
        m_limit_to_children->SetValue(false);
        m_limit_to_children->Disable();
    }
    else
    {
        // A sibling can only be created when the selection is not a form and its parent
        // accepts at least one child type.
        Node* parent = selected->get_Parent();
        const bool can_insert_sibling = parent && !selected->is_Form() && CanHaveChildren(parent);
        const bool can_insert_child = CanHaveChildren(selected);
        if (can_insert_child && !can_insert_sibling)
        {
            // Inserting as a child is the only mode that can work, so select it and
            // disable the two sibling modes.
            m_radio_insert->SetSelection(2);
            m_radio_insert->Enable(0, false);
            m_radio_insert->Enable(1, false);
        }
        else if (!can_insert_child && can_insert_sibling)
        {
            m_radio_insert->Enable(2, false);
        }
    }

    m_stdBtn->GetAffirmativeButton()->Disable();
    m_text_name->SetFocus();
    RebuildList();
}

void InsertWidget::OnChangeLimit(wxCommandEvent& /* event unused */)
{
    RebuildList();
}

void InsertWidget::OnNameText(wxCommandEvent& /* event unused */)
{
    RebuildList();
}

void InsertWidget::RebuildList()
{
    // The auto-generated base class binds a lambda to wxEVT_LISTBOX that unconditionally
    // enables the OK button on every list event, including events emitted during
    // Clear()/Select(). Keep this function the single writer of the button state:
    // disable first, then apply the definitive state below.
    m_stdBtn->GetAffirmativeButton()->Disable();

    const wxue::string name = m_text_name->GetValue().utf8_string();
    m_listbox->Clear();
    Node* node = wxGetFrame().getSelectedNode();
    if (!node)
    {
        node = Project.get_ProjectNode();
    }
    // When inserting a sibling, the new node's parent will determine what is allowed, so
    // retarget to the parent before the allowable-children checks below.
    if (m_radio_insert && m_radio_insert->GetSelection() != 2 && !node->is_Form() &&
        node->get_Parent())
    {
        node = node->get_Parent();
    }
    // type_widgets cannot have children, so change the "selected" node to the parent
    if (node->is_Type(type_widget) && node->get_Parent())
    {
        node = node->get_Parent();
    }

    for (auto* iter: NodeCreation.get_NodeDeclarationArray())
    {
        if (!iter)
        {
            // This will happen if there is an enumerated value but no generator for it
            continue;
        }

        if (m_limit_to_children->GetValue())
        {
            if (!node->is_ChildAllowed(iter))
            {
                continue;
            }
        }
        else if (!NodeCreation.is_ValidCreateParent(iter->get_GenName(), node))
        {
            continue;
        }

        // Only one Data and Images form are allowed per project
        if (iter->get_GenName() == gen_Data && Project.get_DataForm())
        {
            continue;
        }
        if (iter->get_GenName() == gen_Images && Project.get_ImagesForm())
        {
            continue;
        }

        if (name.empty() ||
            wxue::string_view(iter->get_DeclName()).contains(name, wxue::CASE::either))
        {
            m_listbox->AppendString(wxString(iter->get_DeclName()));
        }
    }

    if (m_listbox->GetCount() > 0)
    {
        m_listbox->Select(0);
        m_stdBtn->GetAffirmativeButton()->Enable();
    }
    else
    {
        m_stdBtn->GetAffirmativeButton()->Disable();
    }
}

void InsertWidget::OnListBoxDblClick(wxCommandEvent& /* event unused */)
{
    if (!m_stdBtn->GetAffirmativeButton()->IsEnabled())
    {
        return;
    }
    wxCommandEvent event(wxEVT_BUTTON, m_stdBtn->GetAffirmativeButton()->GetId());
    ProcessEvent(event);
}

void InsertWidget::OnOK(wxCommandEvent& event)
{
    m_widget = m_listbox->GetStringSelection().utf8_string();
    event.Skip();
}

void InsertWidget::OnKeyDown(wxKeyEvent& event)
{
    const int keycode = event.GetKeyCode();
    if (keycode == WXK_DOWN || keycode == WXK_NUMPAD_DOWN)
    {
        // Treat a missing selection as before-the-first: Down selects the first item,
        // which preserves the old (accidental) -1 + 1 == 0 behavior done explicitly.
        const int current = m_listbox->GetSelection();
        const int next = (current == wxNOT_FOUND) ? 0 : current + 1;
        if (next < static_cast<int>(m_listbox->GetCount()))
        {
            m_listbox->SetSelection(next);
        }
    }
    else if (keycode == WXK_UP || keycode == WXK_NUMPAD_UP)
    {
        // No selection: Up stays put instead of computing -2 (which fails the >= 0 test).
        const int current = m_listbox->GetSelection();
        if (current != wxNOT_FOUND && current > 0)
        {
            m_listbox->SetSelection(current - 1);
        }
    }
    else
    {
        event.Skip();
    }
}
