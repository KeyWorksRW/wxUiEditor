/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main window frame
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/fdrepdlg.h>     // wxFindReplaceDialog class
#include <wx/filehistory.h>  // wxFileHistory class
#include <wx/timer.h>        // wxTimer class

#include "gen_enums.h"   // Enumerations for generators
#include "mainapp.h"     // App class
#include "undo_stack.h"  // UndoAction -- Maintain a undo and redo stack
#include "wakatime.h"    // WakaTime -- Updates WakaTime metrics

#include "wxui/mainframe_base.h"

using namespace GenEnum;

class BasePanel;
class DocViewPanel;
class CustomEvent;
class FocusKillerEvtHandler;
class GridBagAction;
class MockupPanel;
class MockupParent;
class NavigationPanel;
class PropGridPanel;
class ImportPanel;

class wxAuiNotebook;
class wxAuiNotebookEvent;
class wxInfoBar;
class wxSplitterWindow;
class wxFileHistroy;

class ueStatusBar;
class NavigationPanel;
class RibbonPanel;

class ChangeParentAction;
class ChangePositionAction;
class ModifyProperties;

// Declared in clipboard.h. Returns true if the external clipboard condtains data that we can
// paste. This will either be from a different instance of wxUiEditor, or from wxFormBuilder
// or wxSmith.
auto isClipboardDataAvailable() -> bool;

// Warning! This MUST be at least 3!
inline constexpr int STATUS_PANELS = 3;

inline constexpr auto txt_main_window_config = "/main_window";

namespace evt_flags
{
    enum : std::uint8_t
    {
        no_event = 0,
        fire_event = 1 << 0,
        queue_event = 1 << 1,      // Queue event (overrides fire_event)
        force_selection = 1 << 2,  // Force node selection even if already selected
    };
};

class MainFrame : public MainFrameBase
{
public:
    MainFrame();
    ~MainFrame() override;
    MainFrame(const MainFrame&) = delete;
    auto operator=(const MainFrame&) -> MainFrame& = delete;
    MainFrame(MainFrame&&) = delete;
    auto operator=(MainFrame&&) -> MainFrame& = delete;

    wxWindow* getWindow() { return wxDynamicCast(this, wxWindow); }

    auto getMockup() -> MockupParent* { return m_mockupPanel; }
    auto get_PropPanel() -> PropGridPanel* { return m_property_panel; }
    auto getNavigationPanel() -> NavigationPanel* { return m_nav_panel; }
    auto getRibbonPanel() -> RibbonPanel* { return m_ribbon_panel; }

    auto GetFirstCodePanel() -> BasePanel*;
    auto GetCppPanel() -> BasePanel* { return m_cppPanel; }
    auto GetPerlPanel() -> BasePanel* { return m_perlPanel; }
    auto GetPythonPanel() -> BasePanel* { return m_pythonPanel; }
    auto GetRubyPanel() -> BasePanel* { return m_rubyPanel; }
    auto GetXrcPanel() -> BasePanel* { return m_xrcPanel; }

    void UpdateLanguagePanels();

    auto getTopNotebook() -> wxAuiNotebook* { return m_notebook; }
    auto getDocViewPanel() -> DocViewPanel* { return m_docviewPanel; }

    auto getImportPanel() -> ImportPanel* { return m_importPanel; }

    void AddCustomEventHandler(wxEvtHandler* handler)
    {
        m_custom_event_handlers.push_back(handler);
    }
    void RemoveCustomEventHandler(wxEvtHandler* handler);

    void FireChangeEventHandler(NodeEvent* event);
    void FireCreatedEvent(Node* node);
    void FireDeletedEvent(Node* node);
    void FireGridBagActionEvent(GridBagAction* undo_cmd);
    void FireParentChangedEvent(ChangeParentAction* undo_cmd);
    void FirePositionChangedEvent(ChangePositionAction* undo_cmd);
    void FireProjectLoadedEvent();
    void FireProjectUpdatedEvent();
    void FirePropChangeEvent(NodeProperty* prop);
    void FireMultiPropEvent(ModifyProperties* undo_cmd);
    void FireSelectedEvent(Node* node, size_t flags = evt_flags::fire_event);

    // These are just here for convenience so you don't have to remember whether you have the raw
    // pointer or the shared pointer.

    void FireCreatedEvent(const NodeSharedPtr& node) { FireCreatedEvent(node.get()); }
    void FireDeletedEvent(const NodeSharedPtr& node) { FireDeletedEvent(node.get()); }
    void FireSelectedEvent(const NodeSharedPtr& node, size_t flags = evt_flags::fire_event)
    {
        FireSelectedEvent(node.get(), flags);
    }

    void ChangeEventHandler(NodeEvent* event, const tt_string& value);

    // This will first call cmd->Change() and then push cmd onto the undo stack.
    void PushUndoAction(UndoActionPtr cmd, bool add_to_stack = true);

    void Undo();
    void Redo();

    bool CanUndo() { return m_undo_stack.IsUndoAvailable(); }
    bool CanRedo() { return m_undo_stack.IsRedoAvailable(); }

    UndoStack& getUndoStack() { return m_undo_stack; }

    auto getSelectedNodePtr() -> const NodeSharedPtr& { return m_selected_node; };
    auto getSelectedNode() -> Node* { return (m_selected_node ? m_selected_node.get() : nullptr); };
    auto getSelectedForm() -> Node*;

    auto getClipboardPtr() -> NodeSharedPtr { return (m_clipboard ? m_clipboard : nullptr); }
    auto getClipboard() -> Node* { return (m_clipboard ? m_clipboard.get() : nullptr); }
    auto getClipHash() -> size_t { return (m_clipboard ? m_clip_hash : 0); }

    // No event will be fired if the node is already selected, unless evt_flags::force_selection
    // is set.
    //
    // Returns true if selection changed, false if already selected or selection
    // removed.
    auto SelectNode(Node* node, size_t flags = evt_flags::fire_event) -> bool;

    bool SelectNode(const NodeSharedPtr& node, size_t flags = evt_flags::fire_event)
    {
        return SelectNode(node.get(), flags);
    }

    auto MoveNode(Node* node, MoveDirection where, bool check_only = false) -> bool;
    auto MoveNode(MoveDirection where, bool check_only = false) -> bool
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
    void ModifyProperty(NodeProperty* prop, tt_string_view value);

    void ChangeAlignment(Node* node, int align, bool vertical);

    bool GetLayoutSettings(int* flag, int* option, int* border, int* orient);

    // If there is a selection, this will create a new child node with special handling for
    // specific components.
    void CreateToolNode(GenEnum::GenName name);

    auto getFileHistory() -> wxFileHistory& { return m_FileHistory; }

    // This does an exact comparison, so file needs to be identical to what was added to the
    // history.
    void RemoveFileFromHistory(tt_string file);

    // Display the text in a specific field of the status bar -- the default is the field
    // that aligns with the PropertyGrid panel.
    void setStatusField(const std::string& text, int position = -1);

    auto GetMenuDpiSize() -> const wxSize& { return m_dpi_menu_size; }
    auto GetRibbonDpiSize() -> const wxSize& { return m_dpi_ribbon_size; }
    auto GetToolbarDpiSize() -> const wxSize& { return m_dpi_toolbar_size; }

    // This is the only variable length field, and therefore can hold the most text
    void setRightStatusField(const tt_string& text) { setStatusField(text, m_posRightStatusField); }

    [[nodiscard]] auto getDebugStatusField() const -> int { return m_posRightStatusField; }
    void UpdateStatusWidths();

    void CopyNode(Node* node);
    void PasteNode(Node* parent);

    auto CanCopyNode() -> bool;

    // Returns true if there is a selected node, and there is data in either the internal or
    // external clipboard.
    auto CanPasteNode() -> bool;

    // Returns true if there is data in either the internal or external clipboard.
    auto isPasteAvailable() -> bool { return (m_clipboard.get() || isClipboardDataAvailable()); }

    // This does not use the internal clipboard
    void DuplicateNode(Node* node);

    void setStatusText(const tt_string& txt, int pane = 1);
    auto OnCreateStatusBar(int number, long style, wxWindowID win_id, const wxString& name)
        -> wxStatusBar* override;

    bool SaveWarning();
    void UpdateFrame();
    void OnProjectLoaded();

    [[nodiscard]] auto isModified() const -> bool { return m_isProject_modified; }

    // Used by LoadProject when an old version was converted
    void setModified()
    {
        m_isProject_modified = true;
        UpdateFrame();
    }

    void setImportedFlag(bool imported = true) { m_isImported = imported; }

    auto GetPropInfoBar() -> wxInfoBar* { return m_info_bar; }

    // Shows info bar message above code display panels
    // icon is one of wxICON_INFORMATION, wxICON_WARNING, wxICON_ERROR, or wxICON_QUESTION
    void ShowInfoBarMsg(const tt_string& msg, int icon = wxICON_WARNING);
    void DismissInfoBar();

    auto GetAppendImportHistory() -> wxFileHistory* { return &m_ImportHistory; }

    void ProjectLoaded();
    void ProjectSaved();

    void ToggleBorderFlag(Node* node, int border);

    void PreviewCpp(Node* form_node);

    // The following event handlers are used when previewing an XRC form

    void OnXrcKeyUp(wxKeyEvent& event);
    void OnPreviewWinClose(wxCloseEvent& event);

    // If the Window is deactivated (switching to another window will do this), this will
    // destroy the preview window.
    void OnPreviewWinActivate(wxActivateEvent& event);

    void setPreviewDlgPtr(wxDialog* dlg) { m_pxrc_dlg = dlg; }
    void setPreviewWinPtr(wxFrame* frame) { m_pxrc_win = frame; }

    void OnPreviewXrc(wxCommandEvent& event) override;

    void OnOpenProject(wxCommandEvent& event) override;
    void OnSaveProject(wxCommandEvent& event) override;
    void OnGenerateCode(wxCommandEvent& event) override;

    void OnGenSingleCpp(wxCommandEvent& event);
    void OnGenSinglePerl(wxCommandEvent& event);
    void OnGenSinglePython(wxCommandEvent& event);
    void OnGenSingleRuby(wxCommandEvent& event);
    void OnGenSingleXRC(wxCommandEvent& event);

    void OnInsertWidget(wxCommandEvent& /*event*/) override;

protected:
    void OnAbout(wxCommandEvent& event) override;
    void OnAppendCrafter(wxCommandEvent& event) override;
    void OnAppendDialogBlocks(wxCommandEvent& event) override;
    void OnAppendFormBuilder(wxCommandEvent& event) override;
    void OnAppendGlade(wxCommandEvent& event) override;
    void OnAppendSmith(wxCommandEvent& event) override;
    void OnAppendXRC(wxCommandEvent& event) override;
    void OnBrowseDocs(wxCommandEvent& event) override;
    void OnBrowsePython(wxCommandEvent& event) override;
    void OnBrowseRuby(wxCommandEvent& event) override;
    void OnChangeAlignment(wxCommandEvent& event) override;
    void OnChangeBorder(wxCommandEvent& event) override;
    void OnClose(wxCloseEvent& event) override;
    void OnCopy(wxCommandEvent& event) override;
    void OnCut(wxCommandEvent& event) override;
    void OnDelete(wxCommandEvent& event) override;
    void OnDifferentProject(wxCommandEvent& event) override;
    void OnDuplicate(wxCommandEvent& event) override;
    void OnEditCustomIds(wxCommandEvent& event) override;
    void OnFindDialog(wxCommandEvent& event) override;
    void OnImportProject(wxCommandEvent& event);
    void OnImportRecent(wxCommandEvent& event);
    void OnImportWindowsResource(wxCommandEvent& event) override;
    void OnNewProject(wxCommandEvent& event);
    void OnOpenRecentProject(wxCommandEvent& event);
    void OnPaste(wxCommandEvent& event) override;
    void OnPreferencesDlg(wxCommandEvent& event) override;
    void OnReloadProject(wxCommandEvent& event) override;
    void OnSaveAsProject(wxCommandEvent& event) override;
    void OnToggleExpandLayout(wxCommandEvent& /*event*/) override;
    void OnUpdateBrowseDocs(wxUpdateUIEvent& event) override;
    void OnUpdateBrowsePython(wxUpdateUIEvent& event) override;
    void OnUpdateBrowseRuby(wxUpdateUIEvent& event) override;

    void OnFind(wxFindDialogEvent& event);
    void OnFindClose(wxFindDialogEvent& event);
    void OnAuiNotebookPageChanged(wxAuiNotebookEvent& event);

    void OnNodeSelected(CustomEvent& event);
    void OnQueueSelect(CustomEvent& event);

    void OnFindWidget(wxCommandEvent& event);

    void OnXrcPreview(wxCommandEvent& event);
    void OnTestXrcImport(wxCommandEvent& event);
    void OnTestXrcDuplicate(wxCommandEvent& event);

#if defined(_DEBUG)  // Starts debug section.

    void OnVerifyTTWX(wxCommandEvent& event);
    void OnConvertImageDlg(wxCommandEvent& event);

#endif

    wxWindow* CreateNoteBook(wxWindow* parent);

    void CreateSplitters();

    void UpdateLayoutTools();
    void UpdateMoveMenu();

    /*

        It's fine to call UpdateWakaTime() frequently since it keeps a timer so that wakatime
       logging is only updated once every two minutes. The exception is when a file is saved -- this
       will always notify wakatime.

        The following functions call UpdateWakaTime() automatically:

            UpdateFrame()
            ProjectSaved()
            OnGenerateCode()
            RemoveNode()
            ChangeEventHandler()

    */

    void UpdateWakaTime(bool FileSavedEvent = false);

private:
    // Helper methods for OnGenerateCode
    static auto GenerateFromOutputType(GenResults& results) -> bool;
    auto GenerateFromDialog(GenResults& results) -> bool;
    static void SaveGenerationPreferences();
    void ShowGenerationResults(const GenResults& results);
    void UpdateGenerationStatus();
    void OnGenerationTimer(wxTimerEvent& event);

    wxSplitterWindow* m_SecondarySplitter { nullptr };

    wxAuiNotebook* m_notebook;
    PropGridPanel* m_property_panel;
    RibbonPanel* m_ribbon_panel;
    std::unique_ptr<WakaTime> m_wakatime { nullptr };

    MockupParent* m_mockupPanel;
    DocViewPanel* m_docviewPanel { nullptr };

    BasePanel* m_cppPanel { nullptr };

    // Language panels -- whether they are actually created is dependent on defitions in pch.h as
    // well as user preferences.
    BasePanel* m_perlPanel { nullptr };
    BasePanel* m_pythonPanel { nullptr };
    BasePanel* m_rubyPanel { nullptr };
    BasePanel* m_xrcPanel { nullptr };

    ImportPanel* m_importPanel { nullptr };

    wxTimer m_generation_timer;

    int m_MainSashPosition { 300 };
    int m_SecondarySashPosition { 300 };

    // Currently the Tree and Property panels can only be on the left (see CreateSplitters
    // function). However, we make these variables in the off chance that we add a function that
    // creates a different layout for the splitters.

    int m_posPropGridStatusField {
        0
    };  // This field dynamically aligns underneat the PropertyGrid panel
    int m_posRightStatusField { 1 };  // This field is variable length, so it can hold the most text

    wxFindReplaceData m_findData;
    wxFindReplaceDialog* m_findDialog { nullptr };

    wxFileHistory m_FileHistory;
    wxFileHistory m_ImportHistory;
    wxMenu* m_submenu_import_recent;

    wxSize m_dpi_menu_size;
    wxSize m_dpi_ribbon_size;
    wxSize m_dpi_toolbar_size;

    wxInfoBar* m_info_bar { nullptr };
    bool m_info_bar_dismissed { true };

    ueStatusBar* m_statBar { nullptr };

    // Custom events will be sent to each of these handlers
    std::vector<wxEvtHandler*> m_custom_event_handlers;

    UndoStack m_undo_stack;
    size_t m_undo_stack_size {
        0
    };  // this gets used to determine if save should be enabled/disabled

    // This is the node selected in the navigation panel
    NodeSharedPtr m_selected_node { nullptr };

    NodeSharedPtr m_clipboard;
    size_t m_clip_hash;  // generated clipboard hash

    wxDialog* m_pxrc_dlg { nullptr };
    wxFrame* m_pxrc_win { nullptr };

    bool m_isXrcResourceInitalized { false };

    bool m_isProject_modified { false };

    bool m_iswakatime_bound { false };

    // If true, the entire project was imported, and a Save As must be done before a Save is
    // allowed.
    bool m_isImported { false };
};

// Returns a reference to the mainframe window
inline auto wxGetFrame() -> MainFrame&
{
    ASSERT_MSG(wxGetApp().getMainFrame(), "MainFrame hasn't been created yet.");
    return *wxGetApp().getMainFrame();
}

// Returns a pointer to the mainframe window or nullptr if it hasn't been created yet
inline auto wxGetMainFrame() -> MainFrame*
{
    return wxGetApp().getMainFrame();
}

extern const std::string_view txtEmptyProject;  // "Empty Project"
