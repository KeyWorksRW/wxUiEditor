/////////////////////////////////////////////////////////////////////////////
// Purpose:   Undo/Redo stack information dialog class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "generated/undo_info_base.h"

class UndoInfo : public UndoInfoBase
{
public:
    UndoInfo();  // If you use this constructor, you must call Create(parent)
    UndoInfo(wxWindow* parent);

protected:
    // Handlers for UndoInfoBase events
    void OnInit(wxInitDialogEvent& event) override;
};
