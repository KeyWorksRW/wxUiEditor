/////////////////////////////////////////////////////////////////////////////
// Purpose:   Load and validate linked (included) .wxui project files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>  // for std::vector

#include "wxue_namespace/wxue_string.h"  // wxue::string

class Node;

// Controls whether the loader may ask the user before the folder's existing children are
// discarded. A silent reload (file watcher, initial project load) never prompts.
enum class LinkReload
{
    interactive,
    silent,
};

// Loads the .wxui file referenced by folder's link_project property. The file is re-read from
// disk and fully re-validated on every call -- no parsed document or validation result is cached.
//
// On success, the linked forms and folders are adopted as children of folder, image paths are
// rebased relative to the current project, and true is returned. On any parse, version or
// content-validation failure, no child is adopted, the reason is reported, and false is returned.
//
// With reload == LinkReload::interactive and allow_ui true, the user is asked before the folder's
// existing children are discarded, and false is returned when the user declines. Pass
// LinkReload::silent for a reload that must not prompt; a caller that first unloads the folder's
// children itself (ProjectHandler::ReloadLinkedProject) has nothing left to confirm.
bool LoadLinkedProjectNode(Node* folder, bool allow_ui,
                           LinkReload reload = LinkReload::interactive);

// Removes every child previously adopted from a linked file. The folder itself is kept.
void UnloadLinkedProjectNode(Node* folder);

// Loads every link folder in the current project tree. Nested links are loaded recursively.
// Returns false if any linked file could not be loaded.
bool LoadAllLinkedProjects(bool allow_ui);

// Re-reads and re-validates the linked file of every link folder in the current project tree.
// Nothing in the node tree is modified. Every folder whose linked file no longer validates is
// added to invalid_folders, and a descriptive message is appended to errors. Returns true when
// every linked file validates.
bool ValidateAllLinkedProjects(std::vector<Node*>& invalid_folders,
                               std::vector<wxue::string>& errors);
