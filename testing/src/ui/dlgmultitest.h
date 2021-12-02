/////////////////////////////////////////////////////////////////////////////
// Purpose:   Multiple Tests dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dlgmultitest_base.h"

class DlgMultiTest : public DlgMultiTestBase
{
public:
    DlgMultiTest(wxWindow* parent = nullptr);

protected:
    void OnInit(wxInitDialogEvent& event) override;
    // Handlers for DlgMultiTestBase events
};
