/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for creating a new project dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "newdialog_base.h"

class NewDialog : public NewDialogBase
{
public:
    NewDialog(wxWindow* parent = nullptr);
    void CreateNode();

protected:
    // Handlers for NewDialogBase events
    void OnInit(wxInitDialogEvent& WXUNUSED(event)) override;
};
