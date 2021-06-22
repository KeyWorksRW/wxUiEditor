/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main window frame
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/fdrepdlg.h>     // wxFindReplaceDialog class
#include <wx/filehistory.h>  // wxFileHistory class

#include "gen_enums.h"   // Enumerations for generators
#include "mainapp.h"     // App class
#include "undo_stack.h"  // UndoAction -- Maintain a undo and redo stack

#include "ui/mainframe_base.h"

using namespace GenEnum;

class CustomEvent;
class BasePanel;
class NavigationPanel;
class PropGridPanel;
class MockupPanel;
class MockupParent;
class FocusKillerEvtHandler;

class wxAuiNotebook;
class wxAuiNotebookEvent;
class wxInfoBar;
class wxSplitterWindow;

class ueStatusBar;
class NavigationPanel;
class RibbonPanel;

enum class MoveDirection
{
    Up = 1,
    Down,
    Left,
    Right
};

// Warning! This MUST be at least 3!
constexpr const size_t StatusPanels = 3;

inline constexpr auto txt_main_window_config = "/main_window";

class MainFrame : public MainFrameBase
{
public:
    MainFrame();
    ~MainFrame() override;

    MockupParent* GetMockup() { return m_mockupPanel; }
    PropGridPanel* GetPropPanel() { return m_property_panel; }
    NavigationPanel* GetNavigationPanel() { return m_nav_panel; }
    RibbonPanel* GetRibbonPanel() { return m_ribbon_panel; }

    void AddCustomEventHandler(wxEvtHandler* handler) { m_custom_event_handlers.push_back(handler); }

    void FireCreatedEvent(Node* node);
    void FireDeletedEvent(Node* node);
    void FireProjectLoadedEvent();
    void FireProjectUpdatedEvent();
    void FirePropChangeEvent(NodeProperty* prop);
    void FireSelectedEvent(Node* node);

    // These are just here for convenience so you don't have to remember whether you have the raw pointer or the shared
    // pointer.

    void FireCreatedEvent(NodeSharedPtr node) { FireCreatedEvent(node.get()); }
    void FireDeletedEvent(NodeSharedPtr node) { FireDeletedEvent(node.get()); }
    void FireSelectedEvent(NodeSharedPtr node) { FireSelectedEvent(node.get()); }

    void ChangeEventHandler(NodeEvent* event, const ttlib::cstr& value);

    // This will first call cmd->Change() and then push cmd onto the undo stack.
    inline void PushUndoAction(UndoActionPtr cmd)
    {
        m_isProject_modified = true;
        m_undo_stack.Push(cmd);
    }

    void Undo();
    void Redo();

    bool CanUndo() { return m_undo_stack.IsUndoAvailable(); }
    bool CanRedo() { return m_undo_stack.IsRedoAvailable(); }

    const NodeSharedPtr& GetSelectedNodePtr() { return m_selected_node; };
    Node* GetSelectedNode() { return (m_selected_node ? m_selected_node.get() : nullptr); };
    Node* GetSelectedForm();

    Node* GetClipboard() { return (m_clipboard ? m_clipboard.get() : nullptr); }
    size_t GetClipHash() { return (m_clipboard ? m_clip_hash : 0); }

    // Node will not be selected if it already is selected, unless force == true.
    // Returns true if selection changed, false if already selected or selection removed.
    bool SelectNode(Node* node, bool force = false, bool notify = true);

    bool SelectNode(NodeSharedPtr node, bool force = false, bool notify = true)
    {
        return SelectNode(node.get(), force, notify);
    }

    bool MoveNode(Node* node, MoveDirection where, bool check_only = false);
    bool MoveNode(MoveDirection where, bool check_only = false)
    {
        return MoveNode(m_selected_node.get(), where, check_only);
    }

    // Removes the node and places it in the internal clipboard
    void CutNode(Node* node) { RemoveNode(node, true); };

    // Erase the node without placing it in the clipboard
    void DeleteNode(Node* node) { RemoveNode(node, false); };

    // Cut or Delete a node.
    void RemoveNode(Node* node, bool isCutMode);

    // Call this MainFrame version if you don't have access to a node.
    void ModifyProperty(NodeProperty* prop, ttlib::cview value);

    void ChangeAlignment(Node* node, int align, bool vertical);

    bool GetLayoutSettings(int* flag, int* option, int* border, int* orient);

    // If there is a selection, this will create a new child node with special handling for
    // specific components.
    void CreateToolNode(GenEnum::GenName name);

    wxFileHistory& GetFileHistory() { return m_FileHistory; }

    // Display the text in a specific field of the status bar -- the default is the field
    // that aligns with the PropertyGrid panel.
    void SetStatusField(const ttlib::cstr text, int position = -1);

    // Search for a sizer to move the node into
    Node* FindChildSizerItem(Node* node);

    // This is the only variable length field, and therefore can hold the most text
    void SetRightStatusField(const ttlib::cstr text) { SetStatusField(text, m_posRightStatusField); }

    int GetDebugStatusField() { return m_posRightStatusField; }
    void UpdateStatusWidths();

    void CopyNode(Node* node);
    void PasteNode(Node* parent);

    bool CanCopyNode();
    bool CanPasteNode();

    // This does not use the internal clipboard
    void DuplicateNode(Node* node);

    void FindItemName(Node* node);

    void setStatusText(const ttlib::cstr& txt, int pane = 1);
    wxStatusBar* OnCreateStatusBar(int number, long style, wxWindowID id, const wxString& name) override;

    bool SaveWarning();
    void UpdateFrame();

    bool IsGenerateNeeded() { return !m_isProject_generated; }
    bool IsModified() { return m_isProject_modified; }

    // Used by LoadProject when an old version was converted
    void SetModified()
    {
        m_isProject_modified = true;
        UpdateFrame();
    }

    void SetImportedFlag(bool imported = true) { m_isImported = imported; }

    wxInfoBar* GetPropInfoBar() { return m_info_bar; }

    void ProjectLoaded();
    void ProjectSaved();

    void ToggleBorderFlag(Node* node, int border);

protected:
    void OnAbout(wxCommandEvent& event) override;
    void OnAppendFormBuilder(wxCommandEvent& event) override;
    void OnAppendGlade(wxCommandEvent& event) override;
    void OnAppendSmith(wxCommandEvent& event) override;
    void OnAppendXRC(wxCommandEvent& event) override;
    void OnChangeAlignment(wxCommandEvent& event) override;
    void OnChangeBorder(wxCommandEvent& event) override;
    void OnClose(wxCloseEvent& event) override;
    void OnCopy(wxCommandEvent& event) override;
    void OnCut(wxCommandEvent& event) override;
    void OnDbgImportWinRes(wxCommandEvent& event);
    void OnDbgCodeDiff(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event) override;
    void OnEmbedImageConverter(wxCommandEvent& event) override;
    void OnGenInhertedClass(wxCommandEvent& event) override;
    void OnGenerateCode(wxCommandEvent& event) override;
    void OnImportWindowsResource(wxCommandEvent& event) override;
    void OnInsertWidget(wxCommandEvent&) override;
    void OnNewProject(wxCommandEvent& event);
    void OnOpenProject(wxCommandEvent& event) override;
    void OnOpenRecentProject(wxCommandEvent& event);
    void OnOptionsDlg(wxCommandEvent& event) override;
    void OnPaste(wxCommandEvent& event) override;
    void OnSaveAsProject(wxCommandEvent& event) override;
    void OnSaveProject(wxCommandEvent& event) override;
    void OnToggleExpandLayout(wxCommandEvent&) override;

    void OnFindDialog(wxCommandEvent& event) override;
    void OnFind(wxFindDialogEvent& event);
    void OnFindClose(wxFindDialogEvent& event);

    void OnAuiNotebookPageChanged(wxAuiNotebookEvent& event);

    void OnNodeSelected(CustomEvent& event);

#if defined(_DEBUG)  // Starts debug section.

    void OnDebugCurrentTest(wxCommandEvent& e);

#endif

    wxWindow* CreateNoteBook(wxWindow* parent);

    void CreateSplitters();

    void UpdateLayoutTools();
    void UpdateMoveMenu();

private:
    wxSplitterWindow* m_MainSplitter { nullptr };
    wxSplitterWindow* m_SecondarySplitter { nullptr };

    wxAuiNotebook* m_notebook;
    PropGridPanel* m_property_panel;
    NavigationPanel* m_nav_panel;
    RibbonPanel* m_ribbon_panel;

    MockupParent* m_mockupPanel;

    BasePanel* m_generatedPanel { nullptr };
    BasePanel* m_derivedPanel { nullptr };

    int m_MainSashPosition { 300 };
    int m_SecondarySashPosition { 300 };

    // Currently the Tree and Property panels can only be on the left (see CreateSplitters function). However, we make
    // these variables in the off chance that we add a function that creates a different layout for the splitters.

    int m_posPropGridStatusField { 0 };  // This field dynamically aligns underneat the PropertyGrid panel
    int m_posRightStatusField { 1 };     // This field is variable length, so it can hold the most text

    wxFindReplaceData m_findData;
    wxFindReplaceDialog* m_findDialog { nullptr };

    wxFileHistory m_FileHistory;

    wxInfoBar* m_info_bar { nullptr };

    ueStatusBar* m_statBar { nullptr };

    // Custom events will be sent to each of these handlers
    std::vector<wxEvtHandler*> m_custom_event_handlers;

    UndoStack m_undo_stack;
    size_t m_undo_stack_size { 0 };  // this gets used to determine if save should be enabled/disabled

    // This is the node selected in the navigation panel
    NodeSharedPtr m_selected_node { nullptr };

    NodeSharedPtr m_clipboard;
    size_t m_clip_hash;  // generated clipboard hash

    bool m_isProject_generated { false };
    bool m_isProject_modified { false };

    // If true, the entire project was imported, and a Save As must be done before a Save is
    // allowed.
    bool m_isImported { false };

    bool m_has_clipboard_data { false };
};

// Same as wxGetApp() only this returns a reference to the frame window
inline MainFrame& wxGetFrame()
{
    ASSERT_MSG(wxGetApp().GetMainFrame(), "MainFrame hasn't been created yet.");
    return *wxGetApp().GetMainFrame();
}
