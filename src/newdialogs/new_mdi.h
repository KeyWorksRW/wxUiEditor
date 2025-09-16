/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for creating a new MDI application
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "new_mdi_base.h"

class NewMdiForm : public NewMdiFormBase
{
public:
    NewMdiForm() : NewMdiFormBase() {}
    NewMdiForm(wxWindow* parent, wxWindowID id = wxID_ANY,
               const wxString& title = "New MDI Application",
               const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
               long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxDialogNameStr) :
        NewMdiFormBase()
    {
        Create(parent, id, title, pos, size, style, name);
    }

    void CreateNode();

protected:

void VerifyClassName() override;

    // Event handlers

    void OnOK(wxCommandEvent& event) override;
    void OnViewType(wxCommandEvent& event) override;
};
