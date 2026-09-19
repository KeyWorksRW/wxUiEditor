/////////////////////////////////////////////////////////////////////////////
// Purpose:   Stores messages
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [09-19-2026]

#include <wx/config.h>            // wxConfig base header
#include <wx/debug.h>             // wxASSERT
#include <wx/filedlg.h>           // wxFileDialog base header
#include <wx/persist/toplevel.h>  // persistence support for wxTLW

#include <format>
#include <limits>
#include <locale>
#include <string>
#include <string_view>
#include <unordered_map>

#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector class

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

// Assertions get their own color so that they stand out from warnings and errors.
static wxColor AssertionColour()
{
    return UserPrefs.is_HighContrast() ? wxColor("#E1A0FF") : wxColor("#A020F0");
}

// RAII guard that restores the read-only state of the XRC preview control on scope exit, including
// when AddTextRaw() throws (e.g. std::bad_alloc) and unwinds out of the event handler.
class ScintillaReadOnlyGuard
{
public:
    explicit ScintillaReadOnlyGuard(wxStyledTextCtrl* scintilla) : m_scintilla(scintilla)
    {
        m_scintilla->SetReadOnly(false);
    }

    ~ScintillaReadOnlyGuard() { m_scintilla->SetReadOnly(true); }

    ScintillaReadOnlyGuard(const ScintillaReadOnlyGuard&) = delete;
    ScintillaReadOnlyGuard(ScintillaReadOnlyGuard&&) = delete;
    ScintillaReadOnlyGuard& operator=(const ScintillaReadOnlyGuard&) = delete;
    ScintillaReadOnlyGuard& operator=(ScintillaReadOnlyGuard&&) = delete;

private:
    wxStyledTextCtrl* m_scintilla;
};

// Shared by the six Add*Msg() writers. The prefix is drawn in `color`, the message itself in the
// window's text colour. is_enabled is false when the Prefs::PREFS_MSG_* gate for that message type
// is cleared.
static void AppendPrefixMsg(wxTextCtrl* text_ctrl, std::string_view prefix, const wxColour& color,
                            bool is_enabled, std::string_view msg)
{
    if (!is_enabled)
    {
        return;
    }

    const wxColor clr_bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);
    const wxColor clr_fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
    wxTextAttr text_attr(clr_fg, clr_bg);
    text_attr.SetFlags(wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    text_attr.SetBackgroundColour(clr_bg);

    text_attr.SetTextColour(color);
    text_ctrl->SetDefaultStyle(text_attr);
    text_ctrl->AppendText(wxString::FromUTF8(prefix.data(), prefix.size()));

    text_attr.SetTextColour(clr_fg);
    text_ctrl->SetDefaultStyle(text_attr);
    text_ctrl->AppendText(wxString::FromUTF8(msg.data(), msg.size()));
}

MsgFrame::MsgFrame(std::vector<wxString>* pMsgs, bool* pDestroyed, wxWindow* parent) :
    MsgFrameBase(parent),
    m_pMsgs(pMsgs),
    m_pDestroyed(pDestroyed)
{
    // Ownership contract: the host owns both pointers and must keep them alive for the entire
    // lifetime of this frame. m_pMsgs is re-read on every render, and the destructor writes
    // through m_pDestroyed. Neither pointer may be null.
    wxASSERT(pMsgs);
    wxASSERT(pDestroyed);

    // These will adjust for both dark mode and high contrast mode if needed
    const wxColor clr_fg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOWTEXT);
    const wxColor clr_bg = UserPrefs.GetColour(wxSYS_COLOUR_WINDOW);

    m_textCtrl->SetBackgroundColour(clr_bg);
    m_textCtrl->SetForegroundColour(clr_fg);

    // Bind the font string to a named local: ToStdView() returns a view into it, so the temporary
    // must outlive the FontProperty construction.
    const wxue::string code_font = UserPrefs.get_CodeDisplayFont();
    const FontProperty font_prop(code_font.ToStdView());
    m_scintilla->StyleSetFont(wxSTC_STYLE_DEFAULT, font_prop.GetFont());
    m_textCtrl->SetFont(font_prop.GetFont());

    wxTextAttr textAttr(clr_fg, clr_bg);
    textAttr.SetFlags(wxTEXT_ATTR_TEXT_COLOUR | wxTEXT_ATTR_BACKGROUND_COLOUR);
    textAttr.SetBackgroundColour(clr_bg);

    const wxColor clrError(UserPrefs.is_HighContrast() ? "#FF0000" : *wxRED);
    const wxColor clrWarning(UserPrefs.is_HighContrast() ? "#569CD6" : *wxBLUE);
    const wxColor clrInfo(UserPrefs.is_HighContrast() ? "#1cc462" : *wxCYAN);
    const wxColor clrAssertion = AssertionColour();

    const std::unordered_map<std::string_view, wxColor> prefix_colors = {
        { "Error:", clrError },       { "wxError:", clrError }, { "Warning:", clrWarning },
        { "wxWarning:", clrWarning }, { "wxInfo:", clrInfo },   { "Assertion:", clrAssertion }
    };

    auto append_message =
        [&](std::string_view prefix, const wxColor& color, std::string_view remaining)
    {
        textAttr.SetTextColour(color);
        m_textCtrl->SetDefaultStyle(textAttr);
        m_textCtrl->AppendText(wxString::FromUTF8(prefix.data(), prefix.size()));
        textAttr.SetTextColour(clr_fg);
        m_textCtrl->SetDefaultStyle(textAttr);
        m_textCtrl->AppendText(wxString::FromUTF8(remaining.data(), remaining.size()));
    };

    // Render by index, re-reading size() on every pass, and copy each entry before rendering it.
    //
    // Rendering runs wxWidgets code, and an assertion raised from it is logged -- which appends
    // to *m_pMsgs. That vector is never shrunk, only appended to, so indices stay valid while a
    // reallocation invalidates the cached end() of a range-for and any reference into the vector.
    // Indexing also means the newly appended entries get rendered here instead of being missed.
    for (size_t index = 0; index < m_pMsgs->size(); ++index)
    {
        const wxString msg_text = (*m_pMsgs)[index];
        bool handled = false;
        for (const auto& [prefix, color]: prefix_colors)
        {
            if (msg_text.starts_with(wxString(prefix)))
            {
                // Skip exactly the prefix itself. stepover() would also swallow the whitespace
                // that separates the prefix from the message, and the keys in prefix_colors have
                // no trailing space (unlike the strings the AddXxxMsg() functions write), so the
                // separating space has to come from msg_text.
                // Consume the view in the same statement: get_View() may point into a temporary
                // conversion buffer, so it must not outlive the full expression.
                append_message(prefix, color, wxue::get_View(msg_text).substr(prefix.size()));
                handled = true;
                break;
            }
        }
        if (!handled)
        {
            m_textCtrl->AppendText(msg_text);
        }
    }

    if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WARNING))
    {
        m_menu_item_warnings->Check(true);
    }
    if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_EVENT))
    {
        m_menu_item_events->Check(true);
    }
    if ((UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_INFO))
    {
        m_menu_item_info->Check(true);
    }

    // On Windows, this saves converting the UTF8 to UTF16 and then back to ANSI.
    SetStcColors(m_scintilla, GenLang::xrc, false, true);

    wxPersistentRegisterAndRestore(this, "MsgWindow");

    // OnPageChanged() only fires after the first page switch, so seed these from the notebook's
    // initial page -- otherwise OnNodeSelected() can act on the stale defaults.
    m_isXrcPage = (m_aui_notebook->GetCurrentPage() == m_page_xrc);
    m_isNodeInfoPage = (m_aui_notebook->GetCurrentPage() == m_page_node);
}

MsgFrame::~MsgFrame()
{
    // Set the host's destroyed flag here as well as in OnClose(): a close can be vetoed, and a
    // frame can die without wxEVT_CLOSE ever being processed.
    if (m_pDestroyed)
    {
        *m_pDestroyed = true;
    }
}

void MsgFrame::AddWarningMsg(std::string_view msg)
{
    AppendPrefixMsg(m_textCtrl, "Warning: ", UserPrefs.is_HighContrast() ? "#569CD6" : *wxBLUE,
                    (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WARNING) != 0, msg);
}

void MsgFrame::AddAssertionMsg(std::string_view msg)
{
    AppendPrefixMsg(m_textCtrl, "Assertion: ", AssertionColour(),
                    (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WARNING) != 0, msg);
}

void MsgFrame::Add_wxWarningMsg(std::string_view msg)
{
    AppendPrefixMsg(m_textCtrl, "wxWarning: ", UserPrefs.is_HighContrast() ? "#569CD6" : *wxBLUE,
                    (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WARNING) != 0, msg);
}

void MsgFrame::Add_wxInfoMsg(std::string_view msg)
{
    AppendPrefixMsg(m_textCtrl, "wxInfo: ", UserPrefs.is_HighContrast() ? "#1cc462" : *wxCYAN,
                    (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_INFO) != 0, msg);
}

void MsgFrame::AddErrorMsg(std::string_view msg)
{
    // Note that we always display error messages
    AppendPrefixMsg(m_textCtrl, "Error: ", UserPrefs.is_HighContrast() ? "#FF0000" : *wxRED, true,
                    msg);
}

void MsgFrame::Add_wxErrorMsg(std::string_view msg)
{
    // Note that we always display error messages
    AppendPrefixMsg(m_textCtrl, "wxError: ", UserPrefs.is_HighContrast() ? "#FF0000" : *wxRED, true,
                    msg);
}

void MsgFrame::OnClose(wxCloseEvent& event)
{
    *m_pDestroyed = true;  // So that our host will know we've been destroyed

    event.Skip();
}

void MsgFrame::OnSaveAs(wxCommandEvent& /* event unused */)
{
    const wxString filename = wxSaveFileSelector("Save messages", "txt", wxEmptyString, this);
    if (filename.empty())
    {
        return;
    }

    wxue::StringVector file;

    // GetLineText() is O(n) per call, which made saving a long log O(n^2). Read the control's text
    // once instead and split it into lines.
    file.ReadString(m_textCtrl->GetValue());

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
    // Building a locale from the environment is expensive and can throw std::runtime_error, so
    // create it once (thread-safe lazy static init) and reuse it for every {:L} call below.
    static const std::locale loc("");

    wxString label;
    NodeMemory node_memory;

    Node* cur_sel = wxGetFrame().getSelectedNode();
    if (cur_sel)
    {
        if (m_isXrcPage)
        {
            const std::string doc_str =
                GenerateXrcStr(cur_sel, xrc::add_comments | xrc::use_xrc_dir);

            // The guard restores the read-only state even if AddTextRaw() throws (e.g. bad_alloc).
            const ScintillaReadOnlyGuard read_only_guard(m_scintilla);
            m_scintilla->ClearAll();
            // Clamp before the cast: AddTextRaw() takes an int, so a document larger than INT_MAX
            // would otherwise truncate.
            const size_t doc_size = doc_str.size();
            const int doc_length =
                (doc_size > static_cast<size_t>(std::numeric_limits<int>::max())) ?
                    std::numeric_limits<int>::max() :
                    static_cast<int>(doc_size);
            m_scintilla->AddTextRaw(doc_str.c_str(), doc_length);
            return;
        }

        label.clear();
        label << "Generator: gen_"
              << wxString::FromUTF8(cur_sel->get_DeclName().data(), cur_sel->get_DeclName().size());
        m_txt_generator->SetLabel(label);
        label.clear();
        // find() rather than at(): the selected node's gen type is not guaranteed to be a key of
        // map_GenTypes, and the std::out_of_range from at() would escape this wx event handler.
        decltype(GenEnum::map_GenTypes.find(cur_sel->get_GenType())) gen_type_iter =
            GenEnum::map_GenTypes.find(cur_sel->get_GenType());
        const std::string_view gen_type = (gen_type_iter != GenEnum::map_GenTypes.end()) ?
                                              gen_type_iter->second :
                                              std::string_view("unknown");
        label << "Type: " << wxString::FromUTF8(gen_type.data(), gen_type.size());
        m_txt_type->SetLabel(label);

        node_memory.size = 0;
        node_memory.children = 0;
        CalcNodeMemory(cur_sel, node_memory);
        label = std::format(loc, "Memory: {:L} ({:L} node{})", node_memory.size,
                            node_memory.children, node_memory.children == 1 ? "" : "s");
        m_txt_memory->SetLabel(label);

        if (auto* generator = cur_sel->get_Generator(); generator)
        {
            wxue::string gen_label = generator->GetHelpText(cur_sel);
            if (gen_label.empty())
            {
                gen_label << "wxWidgets";
            }
            m_hyperlink->SetLabel(gen_label.wx());
            wxString url("https://docs.wxwidgets.org/latest/");
            const wxue::string file = generator->GetHelpURL(cur_sel);
            if (!file.empty())
            {
                url << "class" << file.wx();
            }
            m_hyperlink->SetURL(url);
        }

        Node* project_node = Project.get_ProjectNode();
        if (project_node)
        {
            node_memory.size = 0;
            node_memory.children = 0;
            CalcNodeMemory(project_node, node_memory);

            label = std::format(loc, "Project: {:L} ({:L} nodes)", node_memory.size,
                                node_memory.children);
            m_txt_project->SetLabel(label);
        }
        else
        {
            m_txt_project->SetLabel(wxEmptyString);
        }

        Node* clipboard = wxGetFrame().getClipboard();
        if (clipboard)
        {
            node_memory.size = 0;
            node_memory.children = 0;
            CalcNodeMemory(clipboard, node_memory);
            label = std::format(loc, "Clipboard: {:L} ({:L} nodes)", node_memory.size,
                                node_memory.children);
            m_txt_clipboard->SetLabel(label);
        }
        else
        {
            m_txt_clipboard->SetLabel(wxEmptyString);
        }
    }
    else
    {
        // No selection: clear the labels so the previous node's info doesn't stay on display.
        m_txt_generator->SetLabel(wxEmptyString);
        m_txt_type->SetLabel(wxEmptyString);
        m_txt_memory->SetLabel(wxEmptyString);
        m_txt_project->SetLabel(wxEmptyString);
        m_txt_clipboard->SetLabel(wxEmptyString);
        m_hyperlink->SetLabel(wxEmptyString);
    }
}

void MsgFrame::OnParent(wxCommandEvent& /* event unused */)
{
    const Node* cur_sel = wxGetFrame().getSelectedNode();
    if (cur_sel)
    {
        Node* parent = cur_sel->get_Parent();
        if (!parent)
        {
            wxMessageBox("Current node doesn't have a parent!");
        }
        else
        {
            NodeInfo node_info_dlg(this);
            node_info_dlg.SetNode(parent);
            node_info_dlg.ShowModal();
        }
    }
}
