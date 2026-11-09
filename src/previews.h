/////////////////////////////////////////////////////////////////////////////
// Purpose:   Top level Preview functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [11-09-2026]

#pragma once

#include <string>

#include <wx/string.h>  // wxString class

class Node;
class wxPanel;
class wxWindow;

extern const char* txt_dlg_name;  // "_wxue_temp_dlg"

void PreviewXrc(Node* form_node);

// form_node is required for a wxWizard
void PreviewXrc(std::string& doc_str, GenEnum::GenName gen_name, Node* form_node = nullptr);

// Idempotent -- registers InitAllHandlers() plus the extra XRC handlers this application needs.
void InitXrcHandlers();

// Creates the XRC panel resource named panel_name as a child of parent, from xrc_str.
// Returns nullptr on failure; on failure err_msg describes the reason for the user.
// Caller owns the returned window. Must not disturb the modal preview resource.
wxPanel* LoadXrcPanel(const std::string& xrc_str, wxWindow* parent, const wxString& panel_name,
                      wxString& err_msg);

void Preview(Node* form_node);
