/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog containing special Debugging commands
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>  // wxConfig base header

#include "optionsdlg.h"  // auto-generated: optionsdlg_base.h and optionsdlg_base.cpp

#include "mainframe.h"  // MainFrame -- Main window frame

void MainFrame::OnOptionsDlg(wxCommandEvent& WXUNUSED(event))
{
    OptionsDlg dlg(this);
    dlg.ShowModal();
}

void OptionsDlg::OnInit(wxInitDialogEvent& event)
{
    auto& preferences = wxGetApp().Preferences();
    m_sizers_all_borders = preferences.is_SizersAllBorders();
    m_sizers_always_expand = preferences.is_SizersExpand();
    m_isWakaTimeEnabled = preferences.is_WakaTimeEnabled();

    event.Skip();  // transfer all validator data to their windows and update UI
}

void OptionsDlg::OnAffirmative(wxCommandEvent& WXUNUSED(event))
{
    TransferDataFromWindow();

    auto& preferences = wxGetApp().Preferences();
    bool option_changed = false;

    if (m_sizers_all_borders != preferences.is_SizersAllBorders())
    {
        preferences.set_SizersAllBorders(m_sizers_all_borders);
        option_changed = true;
    }

    if (m_sizers_always_expand != preferences.is_SizersExpand())
    {
        preferences.set_SizersExpand(m_sizers_always_expand);
        option_changed = true;
    }

    if (m_isWakaTimeEnabled != preferences.is_WakaTimeEnabled())
    {
        preferences.set_isWakaTimeEnabled(m_isWakaTimeEnabled);
        option_changed = true;
    }

    if (option_changed)
    {
        preferences.WriteConfig();
    }

    EndModal(wxID_OK);
}
