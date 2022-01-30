/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to lookup and insert a widget
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "insertwidget_base.h"  // auto-generated: insertwidget_base.h and insertwidget_base.cpp

#include "mainframe.h"     // MainFrame -- Main window frame
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator -- Class used to create nodes

void InsertWidget::OnInit(wxInitDialogEvent& WXUNUSED(event))
{
    m_stdBtn->GetAffirmativeButton()->Disable();
    m_text_name->SetFocus();
    wxCommandEvent dummy;
    OnNameText(dummy);
}

void InsertWidget::OnNameText(wxCommandEvent& WXUNUSED(event))
{
    ttlib::cstr name = m_text_name->GetValue().utf8_str().data();
    m_listbox->Clear();
    auto node = wxGetFrame().GetSelectedNode();

    for (auto iter: g_NodeCreator.GetNodeDeclarationArray())
    {
        if (!iter)
        {
            // This will happen if there is an enumerated value but no generator for it
            continue;
        }

        if (!node || !node->IsChildAllowed(iter))
        {
            continue;
        }

        if (name.empty() || iter->DeclName().contains(name, tt::CASE::either))
        {
            m_listbox->AppendString(iter->DeclName().wx_str());
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

void InsertWidget::OnListBoxDblClick(wxCommandEvent& WXUNUSED(event))
{
    wxCommandEvent event(wxEVT_BUTTON, m_stdBtn->GetAffirmativeButton()->GetId());
    ProcessEvent(event);
}

void InsertWidget::OnOK(wxCommandEvent& event)
{
    m_widget << m_listbox->GetStringSelection().wx_str();
    event.Skip();
}

void InsertWidget::OnKeyDown(wxKeyEvent& event)
{
    auto keycode = event.GetKeyCode();
    if (keycode == WXK_DOWN || keycode == WXK_NUMPAD_DOWN)
    {
        auto sel = m_listbox->GetSelection() + 1;
        if (sel < static_cast<int>(m_listbox->GetCount()))
            m_listbox->SetSelection(sel);
    }
    else if (keycode == WXK_UP || keycode == WXK_NUMPAD_UP)
    {
        auto sel = m_listbox->GetSelection() - 1;
        if (sel >= 0)
            m_listbox->SetSelection(sel);
    }
    else
    {
        event.Skip();
    }
}
