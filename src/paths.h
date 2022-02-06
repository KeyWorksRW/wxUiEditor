/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for directory and file properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/manager.h>  // wxPropertyGridManager

#include "../nodes/node_classes.h"  // Forward defintions of Node classes

// Called by PropGridPanel when the user attempts to change art_directory, base_directory, or
// derived_directory.
void AllowDirectoryChange(wxPropertyGridEvent& event, NodeProperty* prop, Node* node);

void AllowFileChange(wxPropertyGridEvent& event, NodeProperty* prop, Node* node);
