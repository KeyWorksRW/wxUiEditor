/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for creating a new project wxFrame
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "newframe_base.h"

class NewFrame : public NewFrameBase
{
public:
    NewFrame(wxWindow* parent = nullptr);
    void CreateNode();

protected:
    // Handlers for NewFrameBase events
    void OnCheckMainFrame(wxCommandEvent& WXUNUSED(event)) override;
};
