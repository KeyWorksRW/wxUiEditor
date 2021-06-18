/////////////////////////////////////////////////////////////////////////////
// Purpose:   Stores messages
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/config.h>            // wxConfig base header
#include <wx/filedlg.h>           // wxFileDialog base header
#include <wx/persist/toplevel.h>  // persistence support for wxTLW

#include "tttextfile.h"  // textfile -- Classes for reading and writing line-oriented files

#include "msgframe.h"  // auto-generated: ../ui/msgframe_base.h and ../ui/msgframe_base.cpp

#include "mainapp.h"    // App -- Main application class
#include "mainframe.h"  // MainFrame -- Main window frame
#include "uifuncs.h"    // Miscellaneous functions for displaying UI

MsgFrame::MsgFrame(ttlib::cstrVector* pMsgs, bool* pDestroyed, wxWindow* parent) :
    MsgFrameBase(parent), m_pMsgs(pMsgs), m_pDestroyed(pDestroyed)
{
    for (auto& iter: *m_pMsgs)
    {
        if (iter.is_sameprefix("Error:"))
        {
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxRED));
            m_textCtrl->AppendText("Error: ");
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
            m_textCtrl->AppendText(iter.view_stepover().wx_str());
        }
        if (iter.is_sameprefix("wxError:"))
        {
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxRED));
            m_textCtrl->AppendText("wxError: ");
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
            m_textCtrl->AppendText(iter.view_stepover().wx_str());
        }
        else if (iter.is_sameprefix("Warning:"))
        {
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLUE));
            m_textCtrl->AppendText("Warning: ");
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
            m_textCtrl->AppendText(iter.view_stepover().wx_str());
        }
        else if (iter.is_sameprefix("wxWarning:"))
        {
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLUE));
            m_textCtrl->AppendText("wxWarning: ");
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
            m_textCtrl->AppendText(iter.view_stepover().wx_str());
        }
        else if (iter.is_sameprefix("wxInfo:"))
        {
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxCYAN));
            m_textCtrl->AppendText("wxInfo: ");
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
            m_textCtrl->AppendText(iter.view_stepover().wx_str());
        }
        else
        {
            m_textCtrl->AppendText(iter.wx_str());
        }
    }

    auto& prefs = wxGetApp().GetPrefs();

    if ((prefs.flags & App::PREFS_MSG_WARNING))
        m_menu_item_warnings->Check(true);
    if ((prefs.flags & App::PREFS_MSG_EVENT))
        m_menu_item_events->Check(true);
    if ((prefs.flags & App::PREFS_MSG_INFO))
        m_menu_item_info->Check(true);

    wxPersistentRegisterAndRestore(this, "MsgWindow");
}

void MsgFrame::AddWarningMsg(ttlib::cview msg)
{
    if (wxGetApp().GetPrefs().flags & App::PREFS_MSG_WARNING)
    {
        m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLUE));
        m_textCtrl->AppendText("Warning: ");
        m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
        m_textCtrl->AppendText(msg.wx_str());
    }
}

void MsgFrame::Add_wxWarningMsg(ttlib::cview msg)
{
    if (wxGetApp().GetPrefs().flags & App::PREFS_MSG_WARNING)
    {
        m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLUE));
        m_textCtrl->AppendText("wxWarning: ");
        m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
        m_textCtrl->AppendText(msg.wx_str());
    }
}

void MsgFrame::Add_wxInfoMsg(ttlib::cview msg)
{
    if (wxGetApp().GetPrefs().flags & App::PREFS_MSG_INFO)
    {
        m_textCtrl->SetDefaultStyle(wxTextAttr(*wxCYAN));
        m_textCtrl->AppendText("wxInfo: ");
        m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
        m_textCtrl->AppendText(msg.wx_str());
    }
}

void MsgFrame::AddErrorMsg(ttlib::cview msg)
{
    // Note that we always display error messages

    m_textCtrl->SetDefaultStyle(wxTextAttr(*wxRED));
    m_textCtrl->AppendText("Error: ");
    m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
    m_textCtrl->AppendText(msg.wx_str());
}

void MsgFrame::Add_wxErrorMsg(ttlib::cview msg)
{
    // Note that we always display error messages

    m_textCtrl->SetDefaultStyle(wxTextAttr(*wxRED));
    m_textCtrl->AppendText("wxError: ");
    m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
    m_textCtrl->AppendText(msg.wx_str());
}

void MsgFrame::OnClose(wxCloseEvent& event)
{
    *m_pDestroyed = true;  // So that our host will know we've been destroyed

    event.Skip();
}

void MsgFrame::OnSaveAs(wxCommandEvent& WXUNUSED(event))
{
    auto filename = wxSaveFileSelector("Save messages", "txt", wxEmptyString, this);
    if (filename.empty())
        return;

    ttlib::textfile file;

    auto totalLines = m_textCtrl->GetNumberOfLines();
    for (int curLine = 0; curLine < totalLines; ++curLine)
    {
        file.addEmptyLine().utf(m_textCtrl->GetLineText(curLine).wx_str());
    }

    if (auto result = file.WriteFile(ttlib::cstr().utf(filename.wx_str())); !result)
    {
        appMsgBox(_ttc(strIdCantWrite) << filename.wx_str(), "Save messages");
    }
    else
    {
        wxLogStatus(this, "Contents saved to: %s", filename.c_str());
    }
}

void MsgFrame::OnClear(wxCommandEvent& WXUNUSED(event))
{
    m_textCtrl->Clear();
    m_pMsgs->clear();
}

void MsgFrame::OnHide(wxCommandEvent& WXUNUSED(event))
{
    Hide();
}

void MsgFrame::OnWarnings(wxCommandEvent& WXUNUSED(event))
{
    auto& prefs = wxGetApp().GetPrefs();

    if ((prefs.flags & App::PREFS_MSG_WARNING))
    {
        prefs.flags &= ~App::PREFS_MSG_WARNING;
        m_menu_item_warnings->Check(false);
    }
    else
    {
        prefs.flags |= App::PREFS_MSG_WARNING;
        m_menu_item_warnings->Check(true);
    }

    auto config = wxConfig::Get();
    config->SetPath("/preferences");
    config->Write("flags", prefs.flags);
    config->SetPath("/");
}

void MsgFrame::OnEvents(wxCommandEvent& WXUNUSED(event))
{
    auto& prefs = wxGetApp().GetPrefs();

    if ((prefs.flags & App::PREFS_MSG_EVENT))
    {
        prefs.flags &= ~App::PREFS_MSG_EVENT;
        m_menu_item_events->Check(false);
    }
    else
    {
        prefs.flags |= App::PREFS_MSG_EVENT;
        m_menu_item_events->Check(true);
    }

    auto config = wxConfig::Get();
    config->SetPath("/preferences");
    config->Write("flags", prefs.flags);
    config->SetPath("/");
}

void MsgFrame::OnInfo(wxCommandEvent& WXUNUSED(event))
{
    auto& prefs = wxGetApp().GetPrefs();

    if ((prefs.flags & App::PREFS_MSG_INFO))
    {
        prefs.flags &= ~App::PREFS_MSG_INFO;
        m_menu_item_info->Check(false);
    }
    else
    {
        prefs.flags |= App::PREFS_MSG_INFO;
        m_menu_item_info->Check(true);
    }

    auto config = wxConfig::Get();
    config->SetPath("/preferences");
    config->Write("flags", prefs.flags);
    config->SetPath("/");
}
