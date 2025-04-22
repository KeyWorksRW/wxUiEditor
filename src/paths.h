/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for directory and file properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/manager.h>  // wxPropertyGridManager

class Node;
class NodeProperty;

void AllowDirectoryChange(wxPropertyGridEvent& event, NodeProperty* prop, Node* node);
void AllowFileChange(wxPropertyGridEvent& event, NodeProperty* prop, Node* node);

void OnPathChanged(wxPropertyGridEvent& event, NodeProperty* prop, Node* node);

void ChangeDerivedDirectory(tt_string& path);
void ChangeBaseDirectory(tt_string& path);
