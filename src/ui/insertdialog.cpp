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
    m_text_name->SetFocus();
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

#if !defined(_DEBUG)
    // There are some generators that we allow in a Release build even though they don't have a 'wx' prefix

    // The various else clauses are to prevent adding the same name more than once -- e.g., static or sizer appends the same
    // string, so the else {} prevents adding them twice.

    if (name.contains("box", tt::CASE::either))
    {
        m_listBox->AppendString("VerticalBoxSizer");
        m_listBox->AppendString("Check3State");
    }
    else if (name.contains("static", tt::CASE::either))
    {
        m_listBox->AppendString("StaticCheckboxBoxSizer");
        m_listBox->AppendString("StaticRadioBtnBoxSizer");
    }
    else if (name.contains("sizer", tt::CASE::either))
    {
        m_listBox->AppendString("StaticCheckboxBoxSizer");
        m_listBox->AppendString("StaticRadioBtnBoxSizer");
    }
    else if (name.contains("check", tt::CASE::either))
    {
        m_listBox->AppendString("Check3State");
    }
    else if (name.contains("custom", tt::CASE::either))
    {
        m_listBox->AppendString("CustomControl");
    }
    else if (name.contains("book", tt::CASE::either))
    {
        m_listBox->AppendString("BookPage");
    }
    else if (name.contains("page", tt::CASE::either))
    {
        m_listBox->AppendString("BookPage");
    }
#endif  // not defined(_DEBUG)

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
