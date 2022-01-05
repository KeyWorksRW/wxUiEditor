/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for editing Font Property
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/fontenum.h>

#include "ttstr.h"  // ttString -- wxString with additional methods similar to ttlib::cstr

#include "../ui/fontpropdlg_base.h"

#include "../utils/font_prop.h"   // FontProperty class

class NodeProperty;

class FontPropDlg : public FontPropDlgBase
{
public:
    FontPropDlg(wxWindow* parent, NodeProperty* prop);
    const wxString& GetResults() { return m_value; }

protected:
    void UpdateFontInfo();
    // Handlers for FontPropDlgBase events
    void OnCustomRadio(wxCommandEvent& WXUNUSED(event)) override;
    void OnFacename(wxCommandEvent& WXUNUSED(event)) override;
    void OnFamily(wxCommandEvent& WXUNUSED(event)) override;
    void OnInit(wxInitDialogEvent& WXUNUSED(event)) override;
    void OnPointSize(wxSpinDoubleEvent& WXUNUSED(event)) override;
    void OnEditPointSize(wxCommandEvent& WXUNUSED(event)) override;
    void OnStrikeThrough(wxCommandEvent& WXUNUSED(event)) override;
    void OnStyle(wxCommandEvent& WXUNUSED(event)) override;
    void OnSystemRadio(wxCommandEvent& WXUNUSED(event)) override;
    void OnSymbolSize(wxCommandEvent& WXUNUSED(event)) override;
    void OnUnderlined(wxCommandEvent& WXUNUSED(event)) override;
    void OnWeight(wxCommandEvent& WXUNUSED(event)) override;

    wxString m_value;

private:
    NodeProperty* m_prop;
    wxFontEnumerator m_font_enum;
    FontProperty m_system_font;
    FontProperty m_custom_font;
};
