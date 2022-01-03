/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for editing Font Property
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "font_prop_dlg.h"  // auto-generated: ../ui/fontpropdlg_base.h and ../ui/fontpropdlg_base.cpp

#include "../nodes/node.h"        // Node class
#include "../nodes/node_event.h"  // NodeEventInfo -- NodeEvent and NodeEventInfo classes
#include "../utils/font_prop.h"   // FontProperty class

FontPropDlg::FontPropDlg(wxWindow* parent, NodeProperty* prop) : FontPropDlgBase(parent)
{
    m_value = prop->as_wxString();
}

void FontPropDlg::OnCustomRadio(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnCustomRadio
}

void FontPropDlg::OnFamily(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnFamily
}

void FontPropDlg::OnStyle(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnStyle
}

void FontPropDlg::OnWeight(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnWeight
}

void FontPropDlg::OnFacename(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnFacename
}

void FontPropDlg::OnPointSize(wxSpinDoubleEvent& WXUNUSED(event))
{
    // TODO: Implement OnPointSize
}

void FontPropDlg::OnUnderlined(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnUnderlined
}

void FontPropDlg::OnStrikeThrough(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnStrikeThrough
}

void FontPropDlg::OnSystemRadio(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnSystemRadio
}

void FontPropDlg::OnSysFont(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnSysFont
}

void FontPropDlg::OnSysPointSize(wxSpinDoubleEvent& WXUNUSED(event))
{
    // TODO: Implement OnSysPointSize
}
