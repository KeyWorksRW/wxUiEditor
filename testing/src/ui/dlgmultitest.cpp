/////////////////////////////////////////////////////////////////////////////
// Purpose:   Multiple Tests dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "dlgmultitest.h"  // auto-generated: dlgmultitest_base.h and dlgmultitest_base.cpp

DlgMultiTest::DlgMultiTest(wxWindow* parent) : DlgMultiTestBase(parent) {}

void DlgMultiTest::OnInit(wxInitDialogEvent& event)
{
    auto sel = m_listview->FindItem(-1, "meaning");
    if (sel >= 0)
        m_listview->Select(sel);

    event.Skip();
}
