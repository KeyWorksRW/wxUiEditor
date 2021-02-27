/////////////////////////////////////////////////////////////////////////////
// Purpose:   Stores messages
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/filedlg.h>           // wxFileDialog base header
#include <wx/persist/toplevel.h>  // persistence support for wxTLW

#include <tttextfile.h>  // textfile -- Classes for reading and writing line-oriented files

#include "msgframe.h"

#include "mainapp.h"    // App -- Main application class
#include "mainframe.h"  // MainFrame -- Main window frame
#include "uifuncs.h"    // Miscellaneous functions for displaying UI

MsgLogger* g_pMsgLogger { nullptr };

wxBEGIN_EVENT_TABLE(CMsgFrame, wxFrame)
    EVT_MENU(wxID_SAVE, CMsgFrame::OnSave)
    EVT_MENU(wxID_CLEAR, CMsgFrame::OnClear)

    EVT_CLOSE(CMsgFrame::OnClose)
wxEND_EVENT_TABLE()

CMsgFrame::CMsgFrame(ttlib::cstrVector* pMsgs, bool* pDestroyed) : wxFrame(nullptr, wxID_ANY, "wxUiEditor Messages")
{
    m_pMsgs = pMsgs;
    m_pDestroyed = pDestroyed;

    m_pTextCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                 wxTE_MULTILINE | wxHSCROLL | wxTE_READONLY | wxTE_RICH);

    auto pMenu = new wxMenu;
    pMenu->Append(wxID_SAVE, "Save &As...", "Save contents to a file");
    pMenu->AppendSeparator();
    pMenu->Append(wxID_CLEAR, "C&lear", "Clear the contents");
    pMenu->Append(wxID_CLOSE, "&Hide", "Hide this window");

    auto pMenuBar = new wxMenuBar;
    pMenuBar->Append(pMenu, "&File");
    SetMenuBar(pMenuBar);

    CreateStatusBar();

    for (auto& iter: *m_pMsgs)
    {
        m_pTextCtrl->AppendText(iter.wx_str());
    }

    wxPersistentRegisterAndRestore(this, "MsgWindow");
}

void CMsgFrame::OnSave(wxCommandEvent& WXUNUSED(event))
{
    auto filename = wxSaveFileSelector("Save messages", "txt", wxEmptyString, this);
    if (filename.empty())
        return;

    ttlib::textfile file;

    auto totalLines = m_pTextCtrl->GetNumberOfLines();
    for (int curLine = 0; curLine < totalLines; ++curLine)
    {
        file.addEmptyLine().utf(m_pTextCtrl->GetLineText(curLine).wx_str());
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

void CMsgFrame::OnClear(wxCommandEvent& WXUNUSED(event))
{
    m_pTextCtrl->Clear();
    m_pMsgs->clear();
}

void CMsgFrame::AddWarningMsg(ttlib::cview msg)
{
    if (wxGetApp().GetPrefs().flags & App::PREFS_MSG_WARNING)
    {
        m_pTextCtrl->SetDefaultStyle(wxTextAttr(*wxBLUE));
        m_pTextCtrl->AppendText("Warning: ");
        m_pTextCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
        m_pTextCtrl->AppendText(msg.wx_str());
    }
}

void CMsgFrame::AddErrorMsg(ttlib::cview msg)
{
    // Note that we always display error messages

    m_pTextCtrl->SetDefaultStyle(wxTextAttr(*wxRED));
    m_pTextCtrl->AppendText("Error: ");
    m_pTextCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
    m_pTextCtrl->AppendText(msg.wx_str());
}

void CMsgFrame::OnClose(wxCloseEvent& event)
{
    *m_pDestroyed = true;  // So that our host will know we've been destroyed

    event.Skip();
}

void MsgLogger::ShowLogger()
{
    if (m_bDestroyed)
    {
        m_msgFrame = new CMsgFrame(&m_Msgs, &m_bDestroyed);
        m_bDestroyed = false;
    }

    m_msgFrame->Show();
    if (wxGetApp().GetMainFrame())
        wxGetApp().GetMainFrame()->SetFocus();
}

void MsgLogger::CloseLogger()
{
    if (!m_bDestroyed)
        m_msgFrame->Close(true);
}

void MsgLogger::AddInfoMsg(ttlib::cview msg)
{
    if (wxGetApp().isMainFrameClosing())
        return;

    if (wxGetApp().GetPrefs().flags & App::PREFS_MSG_INFO)
    {
        auto& str = m_Msgs.emplace_back(msg);
        str << '\n';

        if ((wxGetApp().GetPrefs().flags & App::PREFS_MSG_WINDOW) && !m_isFirstShown)
        {
            m_isFirstShown = true;
            ShowLogger();
        }

        else if (!m_bDestroyed)
            m_msgFrame->AddInfoMsg(str);
    }

    auto frame = wxGetApp().GetMainFrame();
    if (frame && frame->IsShown())
        frame->SetRightStatusField(msg);
}

void MsgLogger::AddEventMsg(ttlib::cview msg)
{
    if (wxGetApp().isMainFrameClosing())
        return;

    if (wxGetApp().GetPrefs().flags & App::PREFS_MSG_EVENT)
    {
        auto& str = m_Msgs.emplace_back(msg);
        str << '\n';

        if ((wxGetApp().GetPrefs().flags & App::PREFS_MSG_WINDOW) && !m_isFirstShown)
        {
            m_isFirstShown = true;
            ShowLogger();
        }

        else if (!m_bDestroyed)
            m_msgFrame->AddEventMsg(str);
    }

    auto frame = wxGetApp().GetMainFrame();
    if (frame && frame->IsShown())
        frame->SetRightStatusField(msg);
}

void MsgLogger::AddWarningMsg(ttlib::cview msg)
{
    if (wxGetApp().isMainFrameClosing())
        return;

    if (wxGetApp().GetPrefs().flags & App::PREFS_MSG_WARNING)
    {
        auto& str = m_Msgs.emplace_back("Warning: ");
        str << msg << '\n';

        if ((wxGetApp().GetPrefs().flags & App::PREFS_MSG_WINDOW) && !m_isFirstShown)
        {
            m_isFirstShown = true;
            ShowLogger();
        }

        else if (!m_bDestroyed)
            m_msgFrame->AddWarningMsg(str.view_stepover());
    }

    auto frame = wxGetApp().GetMainFrame();
    if (frame && frame->IsShown())
        frame->SetRightStatusField(msg);
}

void MsgLogger::AddErrorMsg(ttlib::cview msg)
{
    if (wxGetApp().isMainFrameClosing())
        return;

    auto& str = m_Msgs.emplace_back("Error: ");
    str << msg << '\n';

    if ((wxGetApp().GetPrefs().flags & App::PREFS_MSG_WINDOW) && !m_isFirstShown)
    {
        m_isFirstShown = true;
        ShowLogger();
    }

    else if (!m_bDestroyed)
        m_msgFrame->AddErrorMsg(str.view_stepover());

    auto frame = wxGetApp().GetMainFrame();
    if (frame && frame->IsShown())
        frame->SetRightStatusField(msg);
}
