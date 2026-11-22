/////////////////////////////////////////////////////////////////////////////
// Purpose:   Watch linked .wxui files for external changes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>  // for std::ranges::find
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
            linked_file.MakeAbsolute(project_path.wx());
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
        if (std::ranges::find(watched_directories, link.directory) != watched_directories.end())
        {
            continue;
        }

        watched_directories.push_back(link.directory);
        m_watcher->Add(wxFileName::DirName(link.directory));
    }
}

void LinkedProjectWatcher::OnFileSystemEvent(wxFileSystemWatcherEvent& event)
{
    if (event.IsError())
    {
        return;
    }

    // The callback must not modify the node tree. It only records which link folders are
    // affected and lets the debounce timer post the reload to the MainFrame.
    const wxString changed_path = event.GetPath().GetFullPath();
    const wxString new_path = event.GetNewPath().GetFullPath();

    bool is_queued = false;
    for (const WatchedLink& link: m_links)
    {
        // A rename event reports the old path in GetPath() and the new path in GetNewPath(), so
        // both must be matched -- an editor that saves by renaming would otherwise be missed.
        if (changed_path.CmpNoCase(link.file) == 0 || new_path.CmpNoCase(link.file) == 0)
        {
            m_pending_folders.push_back(link.folder);
            is_queued = true;
        }
    }

    if (is_queued)
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

    // The same file can report several changes, so post one reload per folder.
    std::vector<Node*> unique_folders;
    for (const auto& folder: pending_folders)
    {
        if (std::ranges::find(unique_folders, folder) == unique_folders.end())
        {
            unique_folders.push_back(folder);
        }
    }

    for (const auto& folder: unique_folders)
    {
        frame->GetEventHandler()->QueueEvent(new CustomEvent(EVT_ReloadLinkedProject, folder));
    }
}
