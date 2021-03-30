/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to lookup and insert a widget
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "insertdialog.h"  // auto-generated: insertdialog_base.h and insertdialog_base.cpp

#include "node_creator.h"  // NodeCreator -- Class used to create nodes

InsertDialog::InsertDialog(wxWindow* parent) : InsertDialogBase(parent) {}

void InsertDialog::OnInit(wxInitDialogEvent& WXUNUSED(event))
{
    m_stdBtn->GetAffirmativeButton()->Disable();
}

void InsertDialog::OnNameText(wxCommandEvent& WXUNUSED(event))
{
    ttlib::cstr name = m_text_name->GetValue().utf8_str().data();
    m_listBox->Clear();
    auto node_map = g_NodeCreator.GetNodeDeclarationMap();
    for (auto& iter: node_map)
    {
        if (iter.second->GetClassName().contains(name, tt::CASE::either))
            m_listBox->AppendString(iter.second->GetClassName().wx_str());
    }

    if (m_listBox->GetCount() > 0)
    {
        m_listBox->Select(0);
        m_stdBtn->GetAffirmativeButton()->Enable();
    }
    else
    {
        m_stdBtn->GetAffirmativeButton()->Disable();
    }
}

void InsertDialog::OnListBoxDblClick(wxCommandEvent& WXUNUSED(event))
{
    wxCommandEvent event(wxEVT_BUTTON, m_stdBtn->GetAffirmativeButton()->GetId());
    ProcessEvent(event);
}

void InsertDialog::OnOK(wxCommandEvent& event)
{
    m_widget << m_listBox->GetStringSelection().wx_str();
    event.Skip();
}
