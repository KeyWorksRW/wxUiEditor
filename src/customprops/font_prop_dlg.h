/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for editing Font Property
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../ui/fontpropdlg_base.h"

class NodeProperty;

class FontPropDlg : public FontPropDlgBase
{
public:
    FontPropDlg(wxWindow* parent, NodeProperty* prop);
    const wxString& GetResults() { return m_value; }

protected:
    // Handlers for FontPropDlgBase events
    void OnCustomRadio(wxCommandEvent& WXUNUSED(event)) override;
    void OnFamily(wxCommandEvent& WXUNUSED(event)) override;
    void OnStyle(wxCommandEvent& WXUNUSED(event)) override;
    void OnWeight(wxCommandEvent& WXUNUSED(event)) override;
    void OnFacename(wxCommandEvent& WXUNUSED(event)) override;
    void OnPointSize(wxSpinDoubleEvent& WXUNUSED(event)) override;
    void OnUnderlined(wxCommandEvent& WXUNUSED(event)) override;
    void OnStrikeThrough(wxCommandEvent& WXUNUSED(event)) override;
    void OnSystemRadio(wxCommandEvent& WXUNUSED(event)) override;
    void OnSysFont(wxCommandEvent& WXUNUSED(event)) override;
    void OnSysPointSize(wxSpinDoubleEvent& WXUNUSED(event)) override;

    wxString m_value;

private:
    NodeProperty* m_prop;
};
