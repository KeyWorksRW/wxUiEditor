/////////////////////////////////////////////////////////////////////////////
// Purpose:   Implementation for the UnusedGenerators dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "generated/unused_gen_dlg_base.h"

class UnusedGenerators : public UnusedGeneratorsBase
{
public:
    UnusedGenerators();  // If you use this constructor, you must call Create(parent)
    UnusedGenerators(wxWindow* parent);

protected:
    // Handlers for UnusedGeneratorsBase events
    void OnInit(wxInitDialogEvent& event) override;
    void OnSave(wxCommandEvent& event) override;
};
