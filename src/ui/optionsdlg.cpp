/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog containing special Debugging commands
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>  // wxConfig base header

#include "optionsdlg.h"  // auto-generated: optionsdlg_base.h and optionsdlg_base.cpp

#include "appoptions.h"  // AppOptions -- Application-wide options
#include "mainframe.h"   // MainFrame -- Main window frame

void MainFrame::OnOptionsDlg(wxCommandEvent& WXUNUSED(event))
{
    OptionsDlg dlg(this);
    dlg.ShowModal();
}

void OptionsDlg::OnInit(wxInitDialogEvent& event)
{
    auto& options = GetAppOptions();
    m_class_access = options.get_ClassAccess();
    m_sizers_all_borders = options.get_SizersAllBorders();
    m_sizers_always_expand = options.get_SizersExpand();

    m_hdr_extension = options.get_HdrExtension();
    m_member_prefix = options.get_MemberPrefix();
    m_src_extension = options.get_SrcExtension();

    event.Skip();  // transfer all validator data to their windows and update UI
}

void OptionsDlg::OnAffirmative(wxCommandEvent& WXUNUSED(event))
{
    TransferDataFromWindow();

    auto& options = GetAppOptions();
    bool option_changed = false;

    if (m_class_access != options.get_ClassAccess())
    {
        options.set_ClassAccess(m_class_access);
        option_changed = true;
    }

    if (m_sizers_all_borders != options.get_SizersAllBorders())
    {
        options.set_SizersAllBorders(m_sizers_all_borders);
        option_changed = true;
    }

    if (m_sizers_always_expand != options.get_SizersExpand())
    {
        options.set_SizersExpand(m_sizers_always_expand);
        option_changed = true;
    }

    if (m_hdr_extension != options.get_HdrExtension())
    {
        options.set_HdrExtension(m_hdr_extension);
        option_changed = true;
    }

    if (m_hdr_extension != options.get_HdrExtension())
    {
        options.set_HdrExtension(m_hdr_extension);
        option_changed = true;
    }

    if (m_src_extension != options.get_SrcExtension())
    {
        options.set_SrcExtension(m_src_extension);
        option_changed = true;
    }

    if (m_member_prefix != options.get_MemberPrefix())
    {
        options.set_MemberPrefix(m_member_prefix);
        option_changed = true;
    }

    if (option_changed)
        options.WriteConfig();

    EndModal(wxID_OK);
}
