/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains code common between all new_ dialogs
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

class Node;

// Updates prop_base_file and possibly prop_derived_class_name and prop_derived_file
// depending on value of prop_class_name
void UpdateFormClass(Node* form_node);

// Returns true or false depending on whether any other form has the same class name.
bool IsClassNameUnique(wxString classname);
