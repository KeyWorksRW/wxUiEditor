/////////////////////////////////////////////////////////////////////////////
// Purpose:   Watch linked .wxui files for external changes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [11-21-2026]

#include <algorithm>  // for std::ranges::find
#include <format>     // for std::format
#include <utility>    // for std::move

#include "linked_watcher.h"

#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class

using namespace GenEnum;

// Multiple writes to the same file arrive within milliseconds of each other. Waiting this long
// before reloading collapses them into a single reload.
static constexpr int DEBOUNCE_MS = 300;

wxDEFINE_EVENT(EVT_ReloadLinkedProject, CustomEvent);

// Collects every folder that references a linked file. Nested links carry no link_project
// property once adopted, so only the top-level link folders are collected -- reloading one
// reloads the nested links it contains as well.
static void CollectLinkFolders(Node* node, std::vector<Node*>& link_folders)
{
    for (const auto& child: node->get_ChildNodePtrs())
    {
        if (child->is_LinkedProject())
        {
            link_folders.push_back(child.get());
        }
        else if (child->is_Folder())
        {
            CollectLinkFolders(child.get(), link_folders);
        }
    }
}

// A watch event reports a path in whatever form the OS uses, and the stored link path is exactly
// what the project file contained -- both must be normalized or a legitimate change is missed.
static wxString NormalizePath(wxFileName path)
{
    path.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);
    return path.GetFullPath();
}

LinkedProjectWatcher& LinkedProjectWatcher::getInstance()
{
    static LinkedProjectWatcher instance;
    return instance;
}

LinkedProjectWatcher::LinkedProjectWatcher()
{
    m_debounce_timer.SetOwner(this);
    Bind(wxEVT_TIMER, &LinkedProjectWatcher::OnDebounceTimer, this);
}

LinkedProjectWatcher::~LinkedProjectWatcher()
{
    // The instance is a function-local static, so it is destroyed during static teardown -- which
    // can happen after the event loop and the wx subsystems are gone. StopAll() touches the
    // watcher, so it only runs while the wx application object is still alive; the unique_ptr is
    // always released here so no wxFileSystemWatcher outlives this object. The frame close path
    // also calls StopAll() (see MainFrame::OnClose) so no queued reload can fire against a tree
    // that is being torn down.
    if (wxTheApp != nullptr)
    {
        StopAll();
    }
    m_watcher.reset();
}

void LinkedProjectWatcher::SyncWithProject()
{
    StopAll();

    // A headless run has no MainFrame, so there is nothing to watch and nothing to notify.
    if (!wxGetMainFrame())
    {
        return;
    }

    Node* const project_node = Project.get_ProjectNode();
    if (!project_node)
    {
        return;
    }

    const wxue::string project_path = Project.get_ProjectPath();

    std::vector<Node*> link_folders;
    CollectLinkFolders(project_node, link_folders);

    for (const auto& folder: link_folders)
    {
        const wxue::string link_value = folder->as_string(prop_link_project);
        if (link_value.empty())
        {
            continue;
        }

        wxFileName linked_file(link_value.wx());
        if (linked_file.IsRelative())
        {
            if (project_path.empty())
            {
                MSG_WARNING(std::format(
                    "Linked project: unable to watch \"{}\" -- the project has not been saved",
                    link_value.ToStdString()));
                continue;
            }
            // MakeAbsolute() expects a base directory, not the project file's own path.
            linked_file.MakeAbsolute(wxFileName(project_path.wx()).GetPath());
        }
        if (!linked_file.FileExists())
        {
            // A missing file is reported when the project is loaded -- there is nothing to
            // watch here.
            continue;
        }

        WatchedLink link;
        link.file = linked_file.GetFullPath();
        link.directory = linked_file.GetPath();
        link.folder = folder;
        m_links.push_back(link);
    }

    // wxFileSystemWatcher must not be created before the event loop is running (and must not be
    // created at all in a headless run), so the watches are applied from a queued call.
    CallAfter(&LinkedProjectWatcher::ApplyWatches);
}

// StopAll() must also be called from the frame close path: the queued folder pointers only stay
// valid while the node tree exists.
void LinkedProjectWatcher::StopAll()
{
    if (m_watcher)
    {
        m_watcher->RemoveAll();
    }

    m_debounce_timer.Stop();
    m_links.clear();
    m_pending_folders.clear();
}

void LinkedProjectWatcher::ApplyWatches()
{
    if (!m_watcher)
    {
        if (m_links.empty())
        {
            // Nothing to watch -- do not create the watcher (this also covers a queued call that
            // runs after StopAll cleared the list).
            return;
        }

        m_watcher = std::make_unique<wxFileSystemWatcher>();
        m_watcher->Bind(wxEVT_FSWATCHER, &LinkedProjectWatcher::OnFileSystemEvent, this);
    }

    m_watcher->RemoveAll();

    std::vector<wxString> watched_directories;
    for (const WatchedLink& link: m_links)
    {
        if (link.directory.empty())
        {
            MSG_WARNING(std::format("Linked project: unable to watch \"{}\" -- no directory",
                                    link.file.ToStdString()));
            continue;
        }

        if (std::ranges::find(watched_directories, link.directory) != watched_directories.end())
        {
            continue;
        }

        watched_directories.push_back(link.directory);
        if (!m_watcher->Add(wxFileName::DirName(link.directory)))
        {
            MSG_WARNING(std::format("Unable to watch directory: {}", link.directory.ToStdString()));
        }
    }
}

void LinkedProjectWatcher::OnFileSystemEvent(wxFileSystemWatcherEvent& event)
{
    if (event.IsError())
    {
        // A failed watch (typically the directory was removed) is silent otherwise, and the
        // directory would never be watched again if it reappears.
        MSG_WARNING(std::format("Linked project watch error for: {}",
                                event.GetPath().GetFullPath().ToStdString()));
        CallAfter(&LinkedProjectWatcher::ApplyWatches);
        return;
    }

    // Access and attribute changes are reported far more often than content changes; only the
    // types that can alter the file may trigger a reload.
    const int change_type = event.GetChangeType();
    if (change_type != wxFSW_EVENT_MODIFY && change_type != wxFSW_EVENT_CREATE &&
        change_type != wxFSW_EVENT_RENAME && change_type != wxFSW_EVENT_DELETE)
    {
        return;
    }

    // The callback must not modify the node tree. It only records which link folders are
    // affected and lets the debounce timer post the reload to the MainFrame.
    const wxString changed_path = NormalizePath(event.GetPath());
    const wxString new_path = NormalizePath(event.GetNewPath());

    bool is_queued = false;
    for (const WatchedLink& link: m_links)
    {
        const wxString link_file = NormalizePath(wxFileName(link.file));

        // A rename event reports the old path in GetPath() and the new path in GetNewPath(), so
        // both must be matched -- an editor that saves by renaming would otherwise be missed.
        bool is_match = false;
        if (wxFileName::IsCaseSensitive())
        {
            is_match = (changed_path == link_file) || (new_path == link_file);
        }
        else
        {
            is_match = changed_path.CmpNoCase(link_file) == 0 || new_path.CmpNoCase(link_file) == 0;
        }
        if (!is_match)
        {
            continue;
        }

        // Re-pushing a folder already pending would only grow the list without adding a reload.
        if (std::ranges::find(m_pending_folders, link.folder) == m_pending_folders.end())
        {
            m_pending_folders.push_back(link.folder);
        }
        is_queued = true;
    }

    // Re-arming a running timer would push the reload back on every write of a burst, so only the
    // first event starts it.
    if (is_queued && !m_debounce_timer.IsRunning())
    {
        m_debounce_timer.StartOnce(DEBOUNCE_MS);
    }
}

void LinkedProjectWatcher::OnDebounceTimer([[maybe_unused]] wxTimerEvent& event)
{
    const std::vector<Node*> pending_folders = std::move(m_pending_folders);
    m_pending_folders.clear();

    MainFrame* const frame = wxGetMainFrame();
    if (!frame)
    {
        return;
    }

    // The same file can report several changes, so post one reload per folder. A folder that is no
    // longer part of m_links was removed or retargeted since the event arrived, so drop it.
    std::vector<Node*> unique_folders;
    for (const auto& folder: pending_folders)
    {
        if (std::ranges::find(m_links, folder, &WatchedLink::folder) == m_links.end())
        {
            continue;
        }
        if (std::ranges::find(unique_folders, folder) == unique_folders.end())
        {
            unique_folders.push_back(folder);
        }
    }

    // LIFETIME: the queued node pointers are only valid while the node tree is unchanged.
    // StopAll() -- called by SyncWithProject() and from the frame close path -- must clear
    // m_pending_folders, and queued EVT_ReloadLinkedProject events must be drained before the tree
    // is rebuilt or destroyed.
    for (const auto& folder: unique_folders)
    {
        frame->GetEventHandler()->QueueEvent(new CustomEvent(EVT_ReloadLinkedProject, folder));
    }
}
