/////////////////////////////////////////////////////////////////////////////
// Purpose:   Stores messages
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>            // wxConfig base header
#include <wx/filedlg.h>           // wxFileDialog base header
#include <wx/persist/toplevel.h>  // persistence support for wxTLW

#include <format>

#include "msgframe.h"  // auto-generated: msgframe_base.h and msgframe_base.cpp

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "gen_xrc.h"          // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "mainapp.h"          // App -- Main application class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "preferences.h"      // Set/Get wxUiEditor preferences
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Miscellaneous utility functions

#include "internal/node_info.h"  // NodeInfo -- Node memory usage dialog

struct NodeMemory
{
    size_t size { 0 };
    size_t children { 0 };
};

static void CalcNodeMemory(Node* node, NodeMemory& node_memory)
{
    node_memory.size += node->get_NodeSize();
    ++node_memory.children;

    for (auto& iter: node->get_ChildNodePtrs())
    {
        CalcNodeMemory(iter.get(), node_memory);
    }
}

MsgFrame::MsgFrame(std::vector<tt_string>* pMsgs, bool* pDestroyed, wxWindow* parent) :
    MsgFrameBase(parent), m_pMsgs(pMsgs), m_pDestroyed(pDestroyed)
{
    // These will adjust for both dark mode and high contrast mode if needed
    auto fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
    auto bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);

    m_textCtrl->SetBackgroundColour(bg);
    m_textCtrl->SetForegroundColour(fg);

    FontProperty font_prop(UserPrefs.get_CodeDisplayFont().ToStdView());
    m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font_prop.GetFont());
    m_textCtrl->SetFont(font_prop.GetFont());

    wxTextAttr textAttr(fg, bg);
    textAttr.SetFlags(wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    textAttr.SetBackgroundColour(bg);

    wxColor clrError(UserPrefs.is_HighContrast() ? "#FF0000" : *wxRED);
    wxColor clrWarning(UserPrefs.is_HighContrast() ? "#569CD6" : *wxBLUE);
    wxColor clrInfo(UserPrefs.is_HighContrast() ? "#1cc462" : *wxCYAN);

    for (auto& iter: *m_pMsgs)
    {
        if (iter.starts_with("Error:"))
        {
            textAttr.SetTextColour(clrError);
            m_textCtrl->SetDefaultStyle(textAttr);
            m_textCtrl->AppendText("Error: ");
            textAttr.SetTextColour(fg);
            m_textCtrl->SetDefaultStyle(textAttr);
            m_textCtrl->AppendText(iter.view_stepover().make_wxString());
        }
        else if (iter.starts_with("wxError:"))
        {
            textAttr.SetTextColour(clrError);
            m_textCtrl->SetDefaultStyle(textAttr);
            m_textCtrl->AppendText("wxError: ");
            textAttr.SetTextColour(fg);
            m_textCtrl->SetDefaultStyle(textAttr);
            m_textCtrl->AppendText(iter.view_stepover().make_wxString());
        }
        else if (iter.starts_with("Warning:"))
        {
            textAttr.SetTextColour(clrWarning);
            m_textCtrl->SetDefaultStyle(textAttr);
            m_textCtrl->AppendText("Warning: ");
            textAttr.SetTextColour(fg);
            m_textCtrl->SetDefaultStyle(textAttr);
            m_textCtrl->AppendText(iter.view_stepover().make_wxString());
        }
        else if (iter.starts_with("wxWarning:"))
        {
            textAttr.SetTextColour(clrWarning);
            m_textCtrl->SetDefaultStyle(textAttr);
            m_textCtrl->AppendText("wxWarning: ");
            textAttr.SetTextColour(fg);
            m_textCtrl->SetDefaultStyle(textAttr);
            m_textCtrl->AppendText(iter.view_stepover().make_wxString());
        }
        else if (iter.starts_with("wxInfo:"))
        {
            textAttr.SetTextColour(clrInfo);
            m_textCtrl->SetDefaultStyle(textAttr);
            m_textCtrl->AppendText("wxInfo: ");
            textAttr.SetTextColour(fg);
            m_textCtrl->SetDefaultStyle(textAttr);
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
    SetStcColors(m_scintilla, GEN_LANG_XRC, false, true);

    wxPersistentRegisterAndRestore(this, "MsgWindow");
}

void MsgFrame::AddWarningMsg(tt_string_view msg)
{
    if (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WARNING)
    {
        auto bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);
        auto fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
        wxTextAttr textAttr(fg, bg);
        textAttr.SetFlags(wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
        textAttr.SetBackgroundColour(bg);
        textAttr.SetTextColour(UserPrefs.is_HighContrast() ? "#569CD6" : *wxBLUE);
        m_textCtrl->SetDefaultStyle(textAttr);
        m_textCtrl->AppendText("Warning: ");

        textAttr.SetTextColour(fg);
        m_textCtrl->SetDefaultStyle(textAttr);
        m_textCtrl->AppendText(msg.make_wxString());
    }
}

void MsgFrame::Add_wxWarningMsg(tt_string_view msg)
{
    if (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WARNING)
    {
        auto bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);
        auto fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
        wxTextAttr textAttr(fg, bg);
        textAttr.SetFlags(wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
        textAttr.SetBackgroundColour(bg);
        textAttr.SetTextColour(UserPrefs.is_HighContrast() ? "#569CD6" : *wxBLUE);
        m_textCtrl->SetDefaultStyle(textAttr);
        m_textCtrl->AppendText("wxWarning: ");

        textAttr.SetTextColour(fg);
        m_textCtrl->SetDefaultStyle(textAttr);
        m_textCtrl->AppendText(msg.make_wxString());
    }
}

void MsgFrame::Add_wxInfoMsg(tt_string_view msg)
{
    if (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_INFO)
    {
        auto bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);
        auto fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
        wxTextAttr textAttr(fg, bg);
        textAttr.SetFlags(wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
        textAttr.SetBackgroundColour(bg);

        textAttr.SetTextColour(UserPrefs.is_HighContrast() ? "#1cc462" : *wxCYAN);
        m_textCtrl->SetDefaultStyle(textAttr);
        m_textCtrl->AppendText("wxInfo: ");

        textAttr.SetTextColour(fg);
        m_textCtrl->SetDefaultStyle(textAttr);
        m_textCtrl->AppendText(msg.make_wxString());
    }
}

void MsgFrame::AddErrorMsg(tt_string_view msg)
{
    // Note that we always display error messages

    auto bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);
    auto fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
    wxTextAttr textAttr(fg, bg);
    textAttr.SetFlags(wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    textAttr.SetBackgroundColour(bg);
    textAttr.SetTextColour(UserPrefs.is_HighContrast() ? "#FF0000" : *wxRED);
    m_textCtrl->SetDefaultStyle(textAttr);
    m_textCtrl->AppendText("Error: ");

    textAttr.SetTextColour(fg);
    m_textCtrl->SetDefaultStyle(textAttr);
    m_textCtrl->AppendText(msg.make_wxString());
}

void MsgFrame::Add_wxErrorMsg(tt_string_view msg)
{
    // Note that we always display error messages

    auto bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);
    auto fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
    wxTextAttr textAttr(fg, bg);
    textAttr.SetFlags(wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    textAttr.SetBackgroundColour(bg);
    textAttr.SetTextColour(UserPrefs.is_HighContrast() ? "#FF0000" : *wxRED);
    m_textCtrl->SetDefaultStyle(textAttr);
    m_textCtrl->AppendText("wxError: ");

    textAttr.SetTextColour(fg);
    m_textCtrl->SetDefaultStyle(textAttr);
    m_textCtrl->AppendText(msg.make_wxString());
}

void MsgFrame::OnClose(wxCloseEvent& event)
{
    *m_pDestroyed = true;  // So that our host will know we've been destroyed

    event.Skip();
}

void MsgFrame::OnSaveAs(wxCommandEvent& /* event unused */)
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

void MsgFrame::Clear()
{
    m_textCtrl->Clear();
    m_pMsgs->clear();
}

void MsgFrame::OnClear(wxCommandEvent& /* event unused */)
{
    Clear();
}

void MsgFrame::OnHide(wxCommandEvent& /* event unused */)
{
    Hide();
}

void MsgFrame::OnWarnings(wxCommandEvent& /* event unused */)
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

void MsgFrame::OnEvents(wxCommandEvent& /* event unused */)
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

void MsgFrame::OnInfo(wxCommandEvent& /* event unused */)
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

void MsgFrame::OnPageChanged(wxAuiNotebookEvent& /* event unused */)
{
    m_isXrcPage = (m_aui_notebook->GetCurrentPage() == m_page_xrc);
    m_isNodeInfoPage = (m_aui_notebook->GetCurrentPage() == m_page_node);
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
        label << "Generator: gen_" << cur_sel->get_DeclName();
        m_txt_generator->SetLabel(label);
        label.clear();
        label << "Type: " << GenEnum::map_GenTypes.at(cur_sel->get_GenType());
        m_txt_type->SetLabel(label);

        node_memory.size = 0;
        node_memory.children = 0;
        CalcNodeMemory(cur_sel, node_memory);
        label = std::format(std::locale(""), "Memory: {:L} ({:L} node{})", node_memory.size,
                            node_memory.children, node_memory.children == 1 ? "" : "s");
        m_txt_memory->SetLabel(label);

        if (auto generator = cur_sel->get_Generator(); generator)
        {
            auto gen_label = generator->GetHelpText(cur_sel);
            if (gen_label.empty())
            {
                gen_label << "wxWidgets";
            }
            m_hyperlink->SetLabel(gen_label.make_wxString());
            wxString url("https://docs.wxwidgets.org/latest/");
            auto file = generator->GetHelpURL(cur_sel);
            if (file.size())
            {
                url << "class" << file.make_wxString();
            }
            m_hyperlink->SetURL(url);
        }

        CalcNodeMemory(Project.get_ProjectNode(), node_memory);

        label = std::format(std::locale(""), "Project: {:L} ({:L} nodes)", node_memory.size,
                            node_memory.children);
        m_txt_project->SetLabel(label);

        auto clipboard = wxGetFrame().getClipboard();
        if (clipboard)
        {
            node_memory.size = 0;
            node_memory.children = 0;
            CalcNodeMemory(clipboard, node_memory);
            label = std::format(std::locale(""), "Clipboard: {:L} ({:L} nodes)", node_memory.size,
                                node_memory.children);
            m_txt_clipboard->SetLabel(label);
        }
    }
}

void MsgFrame::OnParent(wxCommandEvent& /* event unused */)
{
    auto cur_sel = wxGetFrame().getSelectedNode();
    if (cur_sel)
    {
        auto parent = cur_sel->get_Parent();
        if (!parent)
        {
            wxMessageBox("Current node doesn't have a parent!");
        }
        else
        {
            NodeInfo dlg(this);
            dlg.SetNode(parent);
            dlg.ShowModal();
        }
    }
}
