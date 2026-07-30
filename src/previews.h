/////////////////////////////////////////////////////////////////////////////
// Purpose:   Top level Preview functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-12-2026]

#pragma once

class Node;

extern const char* txt_dlg_name;  // "_wxue_temp_dlg"

void PreviewXrc(Node* form_node);

// form_node is required for a wxWizard
void PreviewXrc(std::string& doc_str, GenEnum::GenName gen_name, Node* form_node = nullptr);

void Preview(Node* form_node);
