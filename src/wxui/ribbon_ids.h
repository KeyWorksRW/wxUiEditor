/////////////////////////////////////////////////////////////////////////////
// Purpose:   ribbon panel ids
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// These ids are used in the ribbon bar to indicate that they need to be specially handled in
// riobbon_tools.cpp (see RibbonPanel::OnToolClick). Note that gen_ ids are handled
// automatically, and so they don't need to be in this enumerated list.

#pragma once

enum
{
    NewPopupWin = wxID_HIGHEST + 1,
    NewCheckbox,
    NewButton,
    NewSpin,
    NewCombobox,
    NewListbox,
    NewDataCtrl,
    NewRibbonType,
    NewScrolled,
    NewStaticSizer,
    BarTools,
    AuiBarTools,
    CreateNewDialog,
    CreateNewFrame,
    CreateNewPanel,
    CreateNewRibbon,
    CreateNewFormRibbon,
    CreateNewWizard,

    CreateMdiFrame,
    CreateView,
    MdiMenuBar,

};
