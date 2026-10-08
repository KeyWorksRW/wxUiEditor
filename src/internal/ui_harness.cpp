/////////////////////////////////////////////////////////////////////////////
// Purpose:   Debug-only UI command harness for driving the editor from a script
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "ui_harness.h"

#include <cstdio>  // fputs, fflush
#include <functional>
#include <map>
#include <string>
#include <vector>

#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/file.h>
#include <wx/log.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>  // wxMessageDialogBase
#include <wx/timer.h>

#include "cli_ui_guard.h"         // cli_ui::SetScriptedDialogAnswer, cli_ui::SetDialogObserver
#include "gen_enums.h"            // prop_label
#include "mainframe.h"            // MainFrame, wxGetFrame, evt_flags
#include "node.h"                 // Node
#include "panels/navpopupmenu.h"  // NavPopupMenu
#include "project_handler.h"      // Project
#include "undo_stack.h"           // UndoStack, UndoAction, UndoActionPtr

// Converts an 8-bit view to a wxString. Node names and undo strings are std::string_view, and
// the harness log is UTF-8, so every string crossing the boundary goes through here.
static wxString ToWx(std::string_view view)
{
    return wxString::FromUTF8(view.data(), view.size());
}

// Walks up to the project node accumulating child indices, returning e.g. "/0/1/2". The project
// node itself is "/". Canonical index paths are emitted (not names) because get_NodeName() is
// not unique -- several sizers can share the same declared name.
static wxString CanonicalPath(Node* node)
{
    Node* project_node = Project.get_ProjectNode();
    if (node == nullptr || project_node == nullptr)
    {
        return {};
    }

    std::vector<size_t> indices;
    Node* current = node;
    while (current != nullptr && current != project_node)
    {
        Node* parent = current->get_Parent();
        if (parent == nullptr)
        {
            return {};  // the node is not under the project node
        }

        const std::vector<NodeSharedPtr>& siblings = parent->get_ChildNodePtrs();
        size_t index = 0;
        bool found = false;
        for (size_t i = 0; i < siblings.size(); ++i)
        {
            if (siblings[i].get() == current)
            {
                index = i;
                found = true;
                break;
            }
        }
        if (!found)
        {
            return {};
        }

        indices.push_back(index);
        current = parent;
    }

    wxString result = "/";
    for (std::vector<size_t>::const_reverse_iterator iter = indices.rbegin();
         iter != indices.rend(); ++iter)
    {
        if (!result.EndsWith("/"))
        {
            result += "/";
        }
        result += wxString::Format("%d", static_cast<int>(*iter));
    }
    return result;
}

// Resolves a '/'-separated path from the project node. A numeric token is a child index; any
// other token matches a child by its declared name. Empty or "." returns the current selection.
static Node* ResolveNode(const wxString& path)
{
    wxString trimmed = path;
    trimmed.Trim(true);
    trimmed.Trim(false);

    if (trimmed.empty() || trimmed == ".")
    {
        return wxGetFrame().getSelectedNode();
    }

    Node* node = Project.get_ProjectNode();
    if (node == nullptr)
    {
        return nullptr;
    }

    std::vector<wxString> tokens;
    {
        wxString current;
        for (const wxUniChar ch: trimmed)
        {
            if (ch == '/')
            {
                tokens.push_back(current);
                current.clear();
                continue;
            }
            current.Append(ch);
        }
        tokens.push_back(current);
    }
    for (const wxString& token: tokens)
    {
        if (token.empty())
        {
            continue;
        }

        bool is_index = true;
        for (const wxUniChar ch: token)
        {
            if (ch < '0' || ch > '9')
            {
                is_index = false;
                break;
            }
        }

        if (is_index)
        {
            unsigned long index = 0;
            if (!token.ToULong(&index) || index >= node->get_ChildCount())
            {
                return nullptr;
            }
            node = node->get_Child(index);
            continue;
        }

        const wxue::string want_storage = token.utf8_string();
        const std::string_view want = want_storage.ToStdView();
        Node* found = nullptr;
        for (const NodeSharedPtr& child: node->get_ChildNodePtrs())
        {
            if (child->get_NodeName() == want)
            {
                found = child.get();
                break;
            }
        }
        if (found == nullptr)
        {
            return nullptr;
        }
        node = found;
    }
    return node;
}

// Splits on spaces/tabs, honouring double-quoted arguments (a '"' toggles quoting; quotes are
// stripped; no escape handling). Keeps argument values with spaces intact.
static std::vector<wxString> Tokenize(const wxString& line)
{
    std::vector<wxString> tokens;
    wxString current;
    bool in_quotes = false;
    for (const wxUniChar ch: line)
    {
        if (ch == '"')
        {
            in_quotes = !in_quotes;
            continue;
        }
        if (!in_quotes && (ch == ' ' || ch == '\t'))
        {
            if (!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }
        current.Append(ch);
    }
    if (!current.empty())
    {
        tokens.push_back(current);
    }
    return tokens;
}

// Accepts a decimal id, or a 0x-prefixed hex id. Returns false for anything else (including
// names), so a caller can fall back to a label lookup.
static bool ParseIdSpec(const wxString& spec, int& id)
{
    wxString text = spec;
    text.Trim(true);
    text.Trim(false);
    if (text.empty())
    {
        return false;
    }

    int base = 10;
    if (text.StartsWith("0x") || text.StartsWith("0X"))
    {
        text = text.Mid(2);
        base = 16;
    }
    if (text.empty())
    {
        return false;
    }

    for (const wxUniChar ch: text)
    {
        const bool is_digit = (ch >= '0' && ch <= '9');
        const bool is_hex = (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
        if (!is_digit && !(base == 16 && is_hex))
        {
            return false;
        }
    }

    long value = 0;
    if (!text.ToLong(&value, base))
    {
        return false;
    }
    id = static_cast<int>(value);
    return true;
}

// The accelerators live only in the menu bar; deriving them here avoids a hardcoded table.
static wxString AccelText(wxMenuItem* item)
{
    const wxString label = item->GetItemLabel();
    const int tab = label.Find('\t');
    if (tab == wxNOT_FOUND)
    {
        return {};
    }
    return label.Mid(tab + 1);
}

static wxString NormalizeAccel(const wxString& text)
{
    wxString result = text;
    result.MakeLower();
    result.Replace(" ", "");
    if (result == "del")
    {
        result = "delete";  // tolerate the common shorthand
    }
    return result;
}

// Case-insensitive match against the clean leaf label or the full "Menu/Sub/Item" path.
static bool SpecMatches(const wxString& spec, const wxString& leaf, const wxString& path)
{
    return spec.IsSameAs(leaf, false) || spec.IsSameAs(path, false);
}

static void CollectLabelMatches(wxMenu* menu, const wxString& spec, const wxString& path_prefix,
                                std::vector<int>& ids)
{
    const wxMenuItemList& items = menu->GetMenuItems();
    for (wxMenuItemList::compatibility_iterator iter = items.GetFirst(); iter;
         iter = iter->GetNext())
    {
        wxMenuItem* item = static_cast<wxMenuItem*>(iter->GetData());
        if (item == nullptr || item->IsSeparator())
        {
            continue;
        }

        const wxString leaf = item->GetItemLabelText();
        const wxString path = path_prefix.empty() ? leaf : path_prefix + "/" + leaf;

        if (item->IsSubMenu())
        {
            CollectLabelMatches(item->GetSubMenu(), spec, path, ids);
            continue;
        }

        if (SpecMatches(spec, leaf, path))
        {
            ids.push_back(item->GetId());
        }
    }
}

static void CollectMenuBarMatches(wxMenuBar* bar, const wxString& spec, std::vector<int>& ids)
{
    for (size_t i = 0; i < bar->GetMenuCount(); ++i)
    {
        CollectLabelMatches(bar->GetMenu(i), spec, bar->GetMenuLabelText(i), ids);
    }
}

// Recursive label lookup. Returns wxID_NONE when nothing matches or when the match is ambiguous
// -- a caller that needs the candidate ids must use CollectMenuBarMatches directly.
static int FindIdByLabel(wxMenu* menu, const wxString& spec)
{
    std::vector<int> ids;
    CollectLabelMatches(menu, spec, wxString(), ids);
    if (ids.size() == 1)
    {
        return ids.front();
    }
    return wxID_NONE;
}

static void CollectLeaves(wxMenu* menu, std::vector<wxMenuItem*>& leaves)
{
    const wxMenuItemList& items = menu->GetMenuItems();
    for (wxMenuItemList::compatibility_iterator iter = items.GetFirst(); iter;
         iter = iter->GetNext())
    {
        wxMenuItem* item = static_cast<wxMenuItem*>(iter->GetData());
        if (item == nullptr || item->IsSeparator())
        {
            continue;
        }
        if (item->IsSubMenu())
        {
            CollectLeaves(item->GetSubMenu(), leaves);
            continue;
        }
        leaves.push_back(item);
    }
}

static void DispatchMenuId(int id, int checked_state)
{
    wxCommandEvent event(wxEVT_MENU, id);
    event.SetEventObject(&wxGetFrame());
    if (checked_state >= 0)
    {
        event.SetInt(checked_state);  // used by toggle handlers that read IsChecked()
    }
    // ProcessEvent (not wxPostEvent) so a menu handler's effect is observable immediately while
    // the agent is stopped in the debugger. wxWindowBase hides wxEvtHandler::ProcessEvent as
    // protected, so call it through the event-handler base explicitly.
    static_cast<wxEvtHandler&>(wxGetFrame()).ProcessEvent(event);
}

static wxString NodeLabel(Node* node)
{
    if (!node->HasProp(prop_label))
    {
        return {};
    }
    if (NodeProperty* prop = node->get_PropPtr(prop_label); prop != nullptr)
    {
        return ToWx(prop->as_string().ToStdView());
    }
    return {};
}

using FuncRegistry = std::map<wxString, std::function<void()>>;

// Extension point: map a name to any zero-argument callable that performs a UI action which is
// not reachable through a menu id. Add entries here as needed.
static const FuncRegistry& GetFuncRegistry()
{
    static const FuncRegistry registry {
        { "undo",
          []
          {
              wxGetFrame().Undo();
          } },
        { "redo",
          []
          {
              wxGetFrame().Redo();
          } },
    };
    return registry;
}

// Drives the editor's own UI from commands appended to a script file. The timer executes at most
// one command per tick so the agent can set a breakpoint in ExecuteOne() and observe exactly one
// debugger stop per command.
class UiHarness : public wxEvtHandler
{
public:
    UiHarness(const wxString& script_path, const wxString& log_path);

    [[nodiscard]] bool IsScriptOpen() const { return m_script_is_open; }

    // Wires up the timer and installs the dialog observer. Kept separate from the constructor so
    // all resource acquisition happens there and nothing in Launch() can fail.
    void Launch();

private:
    static constexpr int TIMER_INTERVAL_MS { 100 };

    void OnTimer([[maybe_unused]] wxTimerEvent& event);
    void ExecuteOne(const wxString& command_line);

    void DumpTreeRecursive(Node* node);
    void DumpMenuBar();
    void DumpMenuRecursive(wxMenu* menu, const wxString& path_prefix);
    void DumpNavMenuRecursive(wxMenu* menu, const wxString& path_prefix);

    void LogLine(const wxString& line);

    wxString m_script_path;
    wxString m_log_path;
    wxFile m_script;
    wxFile m_log;
    wxTimer m_timer;
    wxString m_pending;  // partial (not-yet-newline-terminated) line carried across ticks
    bool m_script_is_open { false };
    bool m_log_is_open { false };
};

// Kept for the life of the process -- the harness owns the timer and must outlive every dialog
// call. Allocated once by ui_harness::Start() and deliberately never freed.
static UiHarness* g_ui_harness { nullptr };

UiHarness::UiHarness(const wxString& script_path, const wxString& log_path) :
    m_script_path(script_path),
    m_log_path(log_path)
{
    m_log_is_open = m_log.Open(m_log_path, wxFile::write);
    m_script_is_open = m_script.Open(m_script_path, wxFile::read);

    if (m_script_is_open)
    {
        LogLine(wxString::Format("START script=%s log=%s ui_allowed=%d", m_script_path, m_log_path,
                                 Project.is_UiAllowed() ? 1 : 0));
    }
    else
    {
        LogLine("ERR harness cannot open script " + m_script_path);
    }
}

void UiHarness::Launch()
{
    Bind(wxEVT_TIMER, &UiHarness::OnTimer, this);
    m_timer.SetOwner(this);

    cli_ui::SetDialogObserver(
        [this](const wxString& message, int answer)
        {
            LogLine(wxString::Format("DIALOG msg=\"%s\" answer=%d", message, answer));
        });

    if (m_script_is_open)
    {
        m_timer.Start(TIMER_INTERVAL_MS);
    }
}

void UiHarness::LogLine(const wxString& line)
{
    const wxString terminated = line + "\n";
    const wxScopedCharBuffer buf = terminated.utf8_str();

    if (m_log_is_open)
    {
        m_log.Write(buf.data(), buf.length());
        m_log.Flush();  // the agent reads the log while the process is paused at a breakpoint
        return;
    }

    fputs(buf.data(), stderr);
    fflush(stderr);
}

void UiHarness::OnTimer([[maybe_unused]] wxTimerEvent& event)
{
    char buf[4096];
    const ssize_t count = m_script.Read(buf, sizeof(buf));
    if (count > 0)
    {
        m_pending += wxString::FromUTF8(buf, static_cast<size_t>(count));
    }

    // Execute at most ONE complete line per tick: the agent sets a breakpoint in ExecuteOne(), so
    // one command per tick means one debugger stop per command.
    size_t pos = 0;
    while ((pos = m_pending.find('\n')) != wxString::npos)
    {
        wxString line = m_pending.Left(pos);
        m_pending = m_pending.Mid(pos + 1);
        line.Replace("\r", "");  // tolerate CRLF scripts
        line.Trim(true);
        line.Trim(false);
        if (line.empty() || line.StartsWith("#"))
        {
            continue;  // skip blank/comment lines, keep looking
        }
        LogLine("> " +
                line);  // log BEFORE dispatch, so the pending command is visible while paused
        ExecuteOne(line);
        return;
    }
}

void UiHarness::ExecuteOne(const wxString& command_line)
{
    const std::vector<wxString> tokens = Tokenize(command_line);
    if (tokens.empty())
    {
        return;
    }

    wxString verb = tokens[0];
    verb.MakeLower();

    if (verb == "marker")
    {
        wxString text;
        for (size_t i = 1; i < tokens.size(); ++i)
        {
            if (!text.empty())
            {
                text += " ";
            }
            text += tokens[i];
        }
        LogLine("MARKER " + text);
        return;
    }

    if (verb == "quit")
    {
        wxGetFrame().Close();
        LogLine("OK quit");
        return;
    }

    if (verb == "undo")
    {
        wxGetFrame().Undo();
        LogLine("OK undo");
        return;
    }

    if (verb == "redo")
    {
        wxGetFrame().Redo();
        LogLine("OK redo");
        return;
    }

    if (verb == "select")
    {
        if (tokens.size() < 2)
        {
            LogLine("ERR select missing path");
            return;
        }
        Node* node = ResolveNode(tokens[1]);
        if (node == nullptr)
        {
            LogLine("ERR select node not found: " + tokens[1]);
            return;
        }
        wxGetFrame().SelectNode(node, evt_flags::fire_event | evt_flags::force_selection);
        LogLine("OK select " + CanonicalPath(node));
        return;
    }

    if (verb == "navmenu")
    {
        Node* node = nullptr;
        wxString spec;
        if (tokens.size() >= 3)
        {
            node = ResolveNode(tokens[1]);
            spec = tokens[2];
        }
        else if (tokens.size() == 2)
        {
            node = wxGetFrame().getSelectedNode();
            spec = tokens[1];
        }
        else
        {
            LogLine("ERR navmenu missing spec");
            return;
        }
        if (node == nullptr)
        {
            LogLine("ERR navmenu node not found");
            return;
        }

        // Mirrors NavigationPanel::OnRightClick: UpdateUI runs OnUpdateEvent, which encodes the
        // per-item preconditions so an out-of-context command reports as disabled.
        NavPopupMenu menu(node);
        menu.UpdateUI(&menu);

        int id = 0;
        if (!ParseIdSpec(spec, id))
        {
            id = FindIdByLabel(&menu, spec);
        }
        if (id == wxID_NONE)
        {
            LogLine("ERR navmenu unknown command: " + spec);
            return;
        }
        if (!menu.InvokeCommand(id))
        {
            LogLine(wxString::Format("ERR navmenu command not runnable id=%d", id));
            return;
        }
        LogLine(wxString::Format("OK navmenu id=%d", id));
        return;
    }

    if (verb == "menu")
    {
        if (tokens.size() < 2)
        {
            LogLine("ERR menu missing spec");
            return;
        }

        int checked_state = -1;
        if (tokens.size() >= 3)
        {
            if (tokens[2].IsSameAs("checked", false))
            {
                checked_state = 1;
            }
            else if (tokens[2].IsSameAs("unchecked", false))
            {
                checked_state = 0;
            }
        }

        int id = 0;
        if (!ParseIdSpec(tokens[1], id))
        {
            wxMenuBar* bar = wxGetFrame().GetMenuBar();
            if (bar == nullptr)
            {
                LogLine("ERR menu no menu bar");
                return;
            }
            std::vector<int> matched_ids;
            CollectMenuBarMatches(bar, tokens[1], matched_ids);
            if (matched_ids.size() > 1)
            {
                wxString list;
                for (int matched_id: matched_ids)
                {
                    if (!list.empty())
                    {
                        list += ",";
                    }
                    list += wxString::Format("%d", matched_id);
                }
                LogLine("ERR menu ambiguous ids=" + list);
                return;
            }
            if (matched_ids.empty())
            {
                LogLine("ERR menu not found: " + tokens[1]);
                return;
            }
            id = matched_ids.front();
        }

        DispatchMenuId(id, checked_state);
        LogLine(wxString::Format("OK menu id=%d", id));
        return;
    }

    if (verb == "key")
    {
        if (tokens.size() < 2)
        {
            LogLine("ERR key missing accelerator");
            return;
        }

        // Do NOT synthesize a wxKeyEvent: real accelerators are handled in
        // wxWindow::PreProcessEvent, which a synthetic event bypasses. Deriving the target from
        // the live menu bar's accelerator text is the only deterministic option.
        wxMenuBar* bar = wxGetFrame().GetMenuBar();
        if (bar == nullptr)
        {
            LogLine("ERR key no menu bar");
            return;
        }

        std::vector<wxMenuItem*> leaves;
        for (size_t i = 0; i < bar->GetMenuCount(); ++i)
        {
            CollectLeaves(bar->GetMenu(i), leaves);
        }

        const wxString want = NormalizeAccel(tokens[1]);
        std::vector<int> matched_ids;
        for (wxMenuItem* item: leaves)
        {
            const wxString accel = AccelText(item);
            if (!accel.empty() && NormalizeAccel(accel) == want)
            {
                matched_ids.push_back(item->GetId());
            }
        }

        if (matched_ids.empty())
        {
            LogLine("ERR key not found in menu bar: " + tokens[1]);
            return;
        }
        if (matched_ids.size() > 1)
        {
            wxString list;
            for (int matched_id: matched_ids)
            {
                if (!list.empty())
                {
                    list += ",";
                }
                list += wxString::Format("%d", matched_id);
            }
            LogLine("ERR key ambiguous ids=" + list);
            return;
        }

        DispatchMenuId(matched_ids.front(), -1);
        LogLine(wxString::Format("OK key id=%d", matched_ids.front()));
        return;
    }

    if (verb == "func")
    {
        if (tokens.size() < 2)
        {
            LogLine("ERR func missing name");
            return;
        }
        const FuncRegistry& registry = GetFuncRegistry();
        const FuncRegistry::const_iterator iter = registry.find(tokens[1]);
        if (iter == registry.end())
        {
            LogLine("ERR func unknown name: " + tokens[1]);
            return;
        }
        iter->second();
        LogLine("OK func " + tokens[1]);
        return;
    }

    if (verb == "dialogs")
    {
        if (tokens.size() < 2)
        {
            LogLine("ERR dialogs missing answer");
            return;
        }

        if (tokens[1].IsSameAs("off", false))
        {
            cli_ui::SetScriptedDialogAnswer(wxID_NONE);
            LogLine("OK dialogs off");
            return;
        }

        int answer = 0;
        if (ParseIdSpec(tokens[1], answer))
        {
            cli_ui::SetScriptedDialogAnswer(answer);
            LogLine(wxString::Format("OK dialogs answer=%d", answer));
            return;
        }

        struct DialogAnswer
        {
            const char* name;
            int id;
        };
        static constexpr DialogAnswer ANSWERS[] = {
            { "wxID_OK", wxID_OK },
            { "wxID_CANCEL", wxID_CANCEL },
            { "wxID_YES", wxID_YES },
            { "wxID_NO", wxID_NO },
        };
        for (const DialogAnswer& option: ANSWERS)
        {
            if (tokens[1].IsSameAs(option.name, false))
            {
                cli_ui::SetScriptedDialogAnswer(option.id);
                LogLine(wxString::Format("OK dialogs answer=%d", option.id));
                return;
            }
        }

        LogLine("ERR dialogs unknown answer: " + tokens[1]);
        return;
    }

    if (verb == "dump")
    {
        if (tokens.size() < 2)
        {
            LogLine("ERR dump missing subject");
            return;
        }

        wxString subject = tokens[1];
        subject.MakeLower();

        if (subject == "tree")
        {
            Node* node = nullptr;
            if (tokens.size() >= 3)
            {
                node = ResolveNode(tokens[2]);
            }
            else
            {
                node = Project.get_ProjectNode();
            }
            if (node == nullptr)
            {
                LogLine("ERR dump tree node not found");
                return;
            }
            DumpTreeRecursive(node);
            return;
        }

        if (subject == "menubar")
        {
            DumpMenuBar();
            return;
        }

        if (subject == "navmenu")
        {
            Node* node = nullptr;
            if (tokens.size() >= 3)
            {
                node = ResolveNode(tokens[2]);
            }
            else
            {
                node = wxGetFrame().getSelectedNode();
            }
            if (node == nullptr)
            {
                LogLine("ERR dump navmenu node not found");
                return;
            }
            NavPopupMenu menu(node);
            menu.UpdateUI(&menu);
            DumpNavMenuRecursive(&menu, wxString());
            return;
        }

        if (subject == "undo")
        {
            const UndoStack& stack = wxGetFrame().getUndoStack();
            int index = 0;
            for (const UndoActionPtr& action: stack.GetUndoVector())
            {
                LogLine(wxString::Format("UNDO i=%d str=\"%s\"", index,
                                         ToWx(action->GetUndoString().ToStdView())));
                ++index;
            }
            index = 0;
            for (const UndoActionPtr& action: stack.GetRedoVector())
            {
                LogLine(wxString::Format("REDO i=%d str=\"%s\"", index,
                                         ToWx(action->GetUndoString().ToStdView())));
                ++index;
            }
            return;
        }

        if (subject == "selection")
        {
            Node* node = wxGetFrame().getSelectedNode();
            if (node == nullptr)
            {
                LogLine("ERR dump selection no selection");
                return;
            }
            LogLine(wxString::Format("SEL path=%s class=%s var=%s", CanonicalPath(node),
                                     ToWx(node->get_DeclName()), ToWx(node->get_NodeName())));
            return;
        }

        LogLine("ERR dump unknown subject: " + tokens[1]);
        return;
    }

    LogLine("ERR unknown verb \"" + verb + "\"");
}

void UiHarness::DumpTreeRecursive(Node* node)
{
    LogLine(wxString::Format("TREE %s | class=%s | var=%s | label=\"%s\"", CanonicalPath(node),
                             ToWx(node->get_DeclName()), ToWx(node->get_NodeName()),
                             NodeLabel(node)));

    for (const NodeSharedPtr& child: node->get_ChildNodePtrs())
    {
        DumpTreeRecursive(child.get());
    }
}

void UiHarness::DumpMenuBar()
{
    wxMenuBar* bar = wxGetFrame().GetMenuBar();
    if (bar == nullptr)
    {
        LogLine("ERR dump menubar no menu bar");
        return;
    }

    for (size_t i = 0; i < bar->GetMenuCount(); ++i)
    {
        DumpMenuRecursive(bar->GetMenu(i), bar->GetMenuLabelText(i));
    }
}

void UiHarness::DumpMenuRecursive(wxMenu* menu, const wxString& path_prefix)
{
    const wxMenuItemList& items = menu->GetMenuItems();
    for (wxMenuItemList::compatibility_iterator iter = items.GetFirst(); iter;
         iter = iter->GetNext())
    {
        wxMenuItem* item = static_cast<wxMenuItem*>(iter->GetData());
        if (item == nullptr || item->IsSeparator())
        {
            continue;
        }

        const wxString leaf = item->GetItemLabelText();
        const wxString path = path_prefix.empty() ? leaf : path_prefix + "/" + leaf;

        if (item->IsSubMenu())
        {
            DumpMenuRecursive(item->GetSubMenu(), path);
            continue;
        }

        LogLine(wxString::Format("MENUITEM path=\"%s\" id=%d label=\"%s\" accel=\"%s\"", path,
                                 item->GetId(), leaf, AccelText(item)));
    }
}

void UiHarness::DumpNavMenuRecursive(wxMenu* menu, const wxString& path_prefix)
{
    const wxMenuItemList& items = menu->GetMenuItems();
    for (wxMenuItemList::compatibility_iterator iter = items.GetFirst(); iter;
         iter = iter->GetNext())
    {
        wxMenuItem* item = static_cast<wxMenuItem*>(iter->GetData());
        if (item == nullptr || item->IsSeparator())
        {
            continue;
        }

        const wxString leaf = item->GetItemLabelText();
        const wxString path = path_prefix.empty() ? leaf : path_prefix + "/" + leaf;

        if (item->IsSubMenu())
        {
            DumpNavMenuRecursive(item->GetSubMenu(), path);
            continue;
        }

        LogLine(wxString::Format("NAVMENU path=\"%s\" id=%d enabled=%d label=\"%s\"", path,
                                 item->GetId(), item->IsEnabled() ? 1 : 0, leaf));
    }
}

namespace ui_harness
{
    void Start(const wxString& script_path, const wxString& log_path)
    {
        if (g_ui_harness != nullptr)
        {
            return;  // only one harness per process
        }
        g_ui_harness = new UiHarness(script_path, log_path);
        g_ui_harness->Launch();
    }
}  // namespace ui_harness
