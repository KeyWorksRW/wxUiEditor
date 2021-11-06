/////////////////////////////////////////////////////////////////////////////
// Purpose:   Stores messages
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>            // wxConfig base header
#include <wx/filedlg.h>           // wxFileDialog base header
#include <wx/persist/toplevel.h>  // persistence support for wxTLW

#include "tttextfile.h"  // textfile -- Classes for reading and writing line-oriented files

#include "msgframe.h"  // auto-generated: msgframe_base.h and msgframe_base.cpp

#include "mainapp.h"    // App -- Main application class
#include "mainframe.h"  // MainFrame -- Main window frame
#include "node.h"       // Node class
#include "nodeinfo.h"   // NodeInfo -- Node memory usage dialog

struct NodeMemory
{
    size_t size { 0 };
    size_t children { 0 };
};

void CalcNodeMemory(Node* node, NodeMemory& node_memory);  // Defined in nodeinfo.cpp

MsgFrame::MsgFrame(std::vector<ttlib::cstr>* pMsgs, bool* pDestroyed, wxWindow* parent) :
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
        wxMessageBox(wxString("Cannot create or write to the file ") << filename, "Save messages");
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

void MsgFrame::OnPageChanged(wxBookCtrlEvent& WXUNUSED(event))
{
    m_isNodeInfoPage = (m_notebook->GetCurrentPage() == m_page_node);
    if (m_isNodeInfoPage)
    {
        UpdateNodeInfo();
    }
}

void MsgFrame::OnNodeSelected()
{
    if (m_isNodeInfoPage)
    {
        UpdateNodeInfo();
    }
}

void MsgFrame::UpdateNodeInfo()
{
    ttlib::cstr label;
    NodeMemory node_memory;

    auto cur_sel = wxGetFrame().GetSelectedNode();
    if (cur_sel)
    {
        label.clear();
        label << "Generator: gen_" << cur_sel->DeclName();
        m_txt_generator->SetLabel(label);
        label.clear();
        label << "Type: " << GenEnum::map_GenTypes.at(cur_sel->gen_type());
        m_txt_type->SetLabel(label);

        node_memory.size = 0;
        node_memory.children = 0;
        CalcNodeMemory(cur_sel, node_memory);
        label.clear();
        label.Format("Memory: %kzu (%kzu node%s)", node_memory.size, node_memory.children,
                     node_memory.children == 1 ? "" : "s");
        m_txt_memory->SetLabel(label);
    }

    auto project = wxGetApp().GetProject();
    CalcNodeMemory(project, node_memory);

    label.Format("Project: %kzu (%kzu nodes)", node_memory.size, node_memory.children);
    m_txt_project->SetLabel(label);

    auto clipboard = wxGetFrame().GetClipboard();
    if (clipboard)
    {
        node_memory.size = 0;
        node_memory.children = 0;
        CalcNodeMemory(clipboard, node_memory);
        label.clear();
        label.Format("Clipboard: %kzu (%kzu nodes)", node_memory.size, node_memory.children);
        m_txt_clipboard->SetLabel(label);
    }
}

void MsgFrame::OnParent(wxCommandEvent& WXUNUSED(event))
{
    auto cur_sel = wxGetFrame().GetSelectedNode();
    if (cur_sel)
    {
        auto parent = cur_sel->GetParent();
        if (!parent)
        {
            wxMessageBox("Current node doesn't have a parent!");
        }
        else
        {
            NodeInfo dlg(this, parent);
            dlg.ShowModal();
        }
    }
}
