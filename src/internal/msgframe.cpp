/////////////////////////////////////////////////////////////////////////////
// Purpose:   Stores messages
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>            // wxConfig base header
#include <wx/filedlg.h>           // wxFileDialog base header
#include <wx/persist/toplevel.h>  // persistence support for wxTLW

#include "msgframe.h"  // auto-generated: msgframe_base.h and msgframe_base.cpp

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "gen_xrc.h"          // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "mainapp.h"          // App -- Main application class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "preferences.h"      // Set/Get wxUiEditor preferences
#include "project_handler.h"  // ProjectHandler class

#include "pugixml.hpp"

#if defined(INTERNAL_TESTING)
    #include "internal/node_info.h"  // NodeInfo -- Node memory usage dialog
#endif

struct NodeMemory
{
    size_t size { 0 };
    size_t children { 0 };
};

static void CalcNodeMemory(Node* node, NodeMemory& node_memory)
{
    node_memory.size += node->getNodeSize();
    ++node_memory.children;

    for (auto& iter: node->getChildNodePtrs())
    {
        CalcNodeMemory(iter.get(), node_memory);
    }
}

#ifndef SCI_SETKEYWORDS
    #define SCI_SETKEYWORDS 4005
    #define SCI_GETTEXT_MSG 2182
#endif

extern const char* g_xrc_keywords;

MsgFrame::MsgFrame(std::vector<tt_string>* pMsgs, bool* pDestroyed, wxWindow* parent) :
    MsgFrameBase(parent), m_pMsgs(pMsgs), m_pDestroyed(pDestroyed)
{
    for (auto& iter: *m_pMsgs)
    {
        if (iter.starts_with("Error:"))
        {
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxRED));
            m_textCtrl->AppendText("Error: ");
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
            m_textCtrl->AppendText(iter.view_stepover().make_wxString());
        }
        if (iter.starts_with("wxError:"))
        {
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxRED));
            m_textCtrl->AppendText("wxError: ");
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
            m_textCtrl->AppendText(iter.view_stepover().make_wxString());
        }
        else if (iter.starts_with("Warning:"))
        {
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLUE));
            m_textCtrl->AppendText("Warning: ");
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
            m_textCtrl->AppendText(iter.view_stepover().make_wxString());
        }
        else if (iter.starts_with("wxWarning:"))
        {
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLUE));
            m_textCtrl->AppendText("wxWarning: ");
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
            m_textCtrl->AppendText(iter.view_stepover().make_wxString());
        }
        else if (iter.starts_with("wxInfo:"))
        {
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxCYAN));
            m_textCtrl->AppendText("wxInfo: ");
            m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
            m_textCtrl->AppendText(iter.view_stepover().make_wxString());
        }
        else
        {
            m_textCtrl->AppendText(iter.make_wxString());
        }
    }

    if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WARNING))
        m_menu_item_warnings->Check(true);
    if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_EVENT))
        m_menu_item_events->Check(true);
    if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_INFO))
        m_menu_item_info->Check(true);

    // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
    m_scintilla->SendMsg(SCI_SETKEYWORDS, 0, (wxIntPtr) g_xrc_keywords);

    m_scintilla->StyleSetBold(wxSTC_H_TAG, true);
    m_scintilla->StyleSetForeground(wxSTC_H_ATTRIBUTE, wxColour("#E91AFF"));
    m_scintilla->StyleSetForeground(wxSTC_H_TAG, *wxBLUE);
    m_scintilla->StyleSetForeground(wxSTC_H_COMMENT, wxColour(0, 128, 0));
    m_scintilla->StyleSetForeground(wxSTC_H_NUMBER, *wxRED);
    m_scintilla->StyleSetForeground(wxSTC_H_ENTITY, *wxRED);
    m_scintilla->StyleSetForeground(wxSTC_H_DOUBLESTRING, wxColour(0, 128, 0));
    m_scintilla->StyleSetForeground(wxSTC_H_SINGLESTRING, wxColour(0, 128, 0));

    wxPersistentRegisterAndRestore(this, "MsgWindow");
}

void MsgFrame::AddWarningMsg(tt_string_view msg)
{
    if (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WARNING)
    {
        m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLUE));
        m_textCtrl->AppendText("Warning: ");
        m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
        m_textCtrl->AppendText(msg.make_wxString());
    }
}

void MsgFrame::Add_wxWarningMsg(tt_string_view msg)
{
    if (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WARNING)
    {
        m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLUE));
        m_textCtrl->AppendText("wxWarning: ");
        m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
        m_textCtrl->AppendText(msg.make_wxString());
    }
}

void MsgFrame::Add_wxInfoMsg(tt_string_view msg)
{
    if (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_INFO)
    {
        m_textCtrl->SetDefaultStyle(wxTextAttr(*wxCYAN));
        m_textCtrl->AppendText("wxInfo: ");
        m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
        m_textCtrl->AppendText(msg.make_wxString());
    }
}

void MsgFrame::AddErrorMsg(tt_string_view msg)
{
    // Note that we always display error messages

    m_textCtrl->SetDefaultStyle(wxTextAttr(*wxRED));
    m_textCtrl->AppendText("Error: ");
    m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
    m_textCtrl->AppendText(msg.make_wxString());
}

void MsgFrame::Add_wxErrorMsg(tt_string_view msg)
{
    // Note that we always display error messages

    m_textCtrl->SetDefaultStyle(wxTextAttr(*wxRED));
    m_textCtrl->AppendText("wxError: ");
    m_textCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));
    m_textCtrl->AppendText(msg.make_wxString());
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

    tt_string_vector file;

    auto totalLines = m_textCtrl->GetNumberOfLines();
    for (int curLine = 0; curLine < totalLines; ++curLine)
    {
        file.addEmptyLine() << m_textCtrl->GetLineText(curLine).utf8_string();
    }

    if (auto result = file.WriteFile(filename.utf8_string()); !result)
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
    if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WARNING))
    {
        UserPrefs.SetDebugFlags(UserPrefs.GetDebugFlags() & ~Prefs::PREFS_MSG_WARNING);
        m_menu_item_warnings->Check(false);
    }
    else
    {
        UserPrefs.SetDebugFlags(UserPrefs.GetDebugFlags() | Prefs::PREFS_MSG_WARNING);
        m_menu_item_warnings->Check(true);
    }

    UserPrefs.WriteConfig();
}

void MsgFrame::OnEvents(wxCommandEvent& WXUNUSED(event))
{
    if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_EVENT))
    {
        UserPrefs.SetDebugFlags(UserPrefs.GetDebugFlags() & ~Prefs::PREFS_MSG_EVENT);
        m_menu_item_events->Check(false);
    }
    else
    {
        UserPrefs.SetDebugFlags(UserPrefs.GetDebugFlags() | Prefs::PREFS_MSG_EVENT);
        m_menu_item_events->Check(true);
    }

    UserPrefs.WriteConfig();
}

void MsgFrame::OnInfo(wxCommandEvent& WXUNUSED(event))
{
    if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_INFO))
    {
        UserPrefs.SetDebugFlags(UserPrefs.GetDebugFlags() & ~Prefs::PREFS_MSG_INFO);
        m_menu_item_info->Check(false);
    }
    else
    {
        UserPrefs.SetDebugFlags(UserPrefs.GetDebugFlags() | Prefs::PREFS_MSG_INFO);
        m_menu_item_info->Check(true);
    }

    UserPrefs.WriteConfig();
}

void MsgFrame::OnPageChanged(wxBookCtrlEvent& WXUNUSED(event))
{
    m_isXrcPage = (m_notebook->GetCurrentPage() == m_page_xrc);
    m_isNodeInfoPage = (m_notebook->GetCurrentPage() == m_page_node);
    if (m_isNodeInfoPage || m_isXrcPage)
    {
        UpdateNodeInfo();
    }
}

void MsgFrame::OnNodeSelected()
{
    if (m_isNodeInfoPage || m_isXrcPage)
    {
        UpdateNodeInfo();
    }
}

void MsgFrame::UpdateNodeInfo()
{
    tt_string label;
    NodeMemory node_memory;

    auto cur_sel = wxGetFrame().getSelectedNode();
    if (cur_sel)
    {
        if (m_isXrcPage)
        {
            auto doc_str = GenerateXrcStr(cur_sel, xrc::add_comments | xrc::use_xrc_dir);

            m_scintilla->SetReadOnly(false);
            m_scintilla->ClearAll();
            m_scintilla->AddTextRaw(doc_str.c_str(), (to_int) doc_str.size());
            m_scintilla->SetReadOnly(true);
            return;
        }

        label.clear();
        label << "Generator: gen_" << cur_sel->declName();
        m_txt_generator->SetLabel(label);
        label.clear();
        label << "Type: " << GenEnum::map_GenTypes.at(cur_sel->getGenType());
        m_txt_type->SetLabel(label);

        node_memory.size = 0;
        node_memory.children = 0;
        CalcNodeMemory(cur_sel, node_memory);
        label.clear();
        label.Format("Memory: %kzu (%kzu node%s)", node_memory.size, node_memory.children,
                     node_memory.children == 1 ? "" : "s");
        m_txt_memory->SetLabel(label);

        if (auto generator = cur_sel->getGenerator(); generator)
        {
            auto gen_label = generator->GetHelpText(cur_sel);
            if (gen_label.empty())
            {
                gen_label << "wxWidgets";
            }
            m_hyperlink->SetLabel(gen_label.make_wxString());
            wxString url("https://docs.wxwidgets.org/trunk/");
            auto file = generator->GetHelpURL(cur_sel);
            if (file.size())
            {
                url << "class" << file.make_wxString();
            }
            m_hyperlink->SetURL(url);
        }

        CalcNodeMemory(Project.getProjectNode(), node_memory);

        label.Format("Project: %kzu (%kzu nodes)", node_memory.size, node_memory.children);
        m_txt_project->SetLabel(label);

        auto clipboard = wxGetFrame().getClipboard();
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
}

void MsgFrame::OnParent(wxCommandEvent& WXUNUSED(event))
{
    auto cur_sel = wxGetFrame().getSelectedNode();
    if (cur_sel)
    {
        auto parent = cur_sel->getParent();
        if (!parent)
        {
            wxMessageBox("Current node doesn't have a parent!");
        }
        else
        {
#if defined(INTERNAL_TESTING)
            NodeInfo dlg(this);
            dlg.SetNode(parent);
            dlg.ShowModal();
#endif
        }
    }
}
