/////////////////////////////////////////////////////////////////////////////
// Purpose:   ribbon panel ids
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// These ids are used in the ribbon bar to indicate that they need to be specially handled in
// riobbon_tools.cpp (see RibbonPanel::OnToolClick). Note that gen_ ids are handled
// automatically, and so they don't need to be in this enumerated list.

#pragma once

enum
{
    NewPopupWin = START_RIBBON_IDS + 100,
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

    // All the Create enums need to be grouped together, with MdiMenuBar as the last one so that
    // they can be handed as a range to the Add menu Bind command.

    CreateNewDialog,
    CreateNewFrame,
    CreateNewPanel,
    CreateNewPropertySheet,
    CreateNewRibbon,
    CreateNewFormRibbon,
    CreateNewWizard,
    CreateMdiFrame,
    CreateView,

    MdiMenuBar,

};
