/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid utilities
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

class wxPropertyGridInterface;

void AfterCreationAddItems(wxPropertyGridInterface* pgi, Node* node);

// This will return true if wx/propgrid/advprops.h needs to be included
bool CheckAdvancePropertyInclude(Node* node);
