/////////////////////////////////////////////////////////////////////////////
// Purpose:   Watch linked .wxui files for external changes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>  // for std::unique_ptr
#include <vector>  // for std::vector

#include <wx/event.h>      // for wxEvtHandler and wxDECLARE_EVENT
#include <wx/filename.h>   // for wxFileName
#include <wx/fswatcher.h>  // for wxFileSystemWatcher
#include <wx/timer.h>      // for wxTimer

#include "cstm_event.h"  // CustomEvent -- Custom Event class

class Node;

// Posted to the MainFrame when a watched linked file has changed on disk. The event's node is
// the link folder whose linked file changed. The node tree must only be modified by the handler
// for this event -- never by the file system watcher callback itself.
wxDECLARE_EVENT(EVT_ReloadLinkedProject, CustomEvent);

// Watches the .wxui file referenced by every link folder's link_project property. A change is
// debounced (several writes in quick succession produce a single reload), and the affected link
// folder is handed to the MainFrame, which reloads just that subtree on the GUI thread.
class LinkedProjectWatcher : public wxEvtHandler
{
public:
    static LinkedProjectWatcher& getInstance();

    LinkedProjectWatcher(const LinkedProjectWatcher&) = delete;
    LinkedProjectWatcher& operator=(const LinkedProjectWatcher&) = delete;
    LinkedProjectWatcher(LinkedProjectWatcher&&) = delete;
    LinkedProjectWatcher& operator=(LinkedProjectWatcher&&) = delete;

    // Removes every watch, then watches the linked file of every link folder in the current
    // project tree. Does nothing when there is no MainFrame (e.g. a headless command-line run).
    void SyncWithProject();

    // Removes every watch, cancels a pending reload and discards queued folders.
    void StopAll();

private:
    LinkedProjectWatcher();
    ~LinkedProjectWatcher() override;

    // One watched linked file. The watch is placed on the file's directory -- wxFileSystemWatcher
    // only supports watching directories -- and events are matched against file.
    struct WatchedLink
    {
        wxString file;
        wxString directory;
        Node* folder { nullptr };
    };

    void OnFileSystemEvent(wxFileSystemWatcherEvent& event);
    void OnDebounceTimer([[maybe_unused]] wxTimerEvent& event);

    // Creates the wxFileSystemWatcher (which requires a running event loop) and (re)adds every
    // watched directory.
    void ApplyWatches();

    // Lazily created so no wxFileSystemWatcher is constructed before the event loop is running
    // or during a headless command-line run.
    std::unique_ptr<wxFileSystemWatcher> m_watcher;

    wxTimer m_debounce_timer;

    std::vector<WatchedLink> m_links;

    // Link folders whose linked file changed. Only filled by the watcher callback, drained by
    // OnDebounceTimer -- which posts EVT_ReloadLinkedProject to the MainFrame.
    std::vector<Node*> m_pending_folders;
};
