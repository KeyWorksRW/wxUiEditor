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
    for (auto iter: g_NodeCreator.GetNodeDeclarationArray())
    {
        if (!iter)
        {
            // This will happen if there is an enumerated value but no generator for it
            continue;
        }

#if !defined(_DEBUG)
        // In a DEBUG build, we show all components, including the abstract ones -- including some that are only used for
        // importing a wxFormBuilder project and won't work in our own projects. So don't be surprised if something shows up
        // in the list that doesn't work!

        if (!iter->DeclName().is_sameprefix("wx"))
            continue;
#endif  // not defined(_DEBUG)

        if (iter->DeclName().contains(name, tt::CASE::either))
            m_listBox->AppendString(iter->DeclName().wx_str());
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
