/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for editing Font Property
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/fontenum.h>

#include "font_prop.h"  // FontProperty class

#include "wxui/fontpropdlg_base.h"  // FontPropDlgBase

class NodeProperty;

class FontPropDlg : public FontPropDlgBase
{
public:
    FontPropDlg(wxWindow* parent, NodeProperty* prop);
    FontPropDlg(wxWindow* parent, const wxString& font_description);
    const wxString& GetResults() { return m_value; }
    wxue::string_view GetFontDescription() { return m_font_description; }

protected:
    void Initialize();  // Call this from either of the constructors

    void UpdateFontInfo();
    // Handlers for FontPropDlgBase events
    void OnCustomRadio(wxCommandEvent& /* event unused */) override;
    void OnFacename(wxCommandEvent& /* event unused */) override;
    void OnFamily(wxCommandEvent& /* event unused */) override;
    void OnInit(wxInitDialogEvent& /* event unused */) override;
    void OnPointSize(wxSpinDoubleEvent& /* event unused */) override;
    void OnEditPointSize(wxCommandEvent& /* event unused */) override;
    void OnStrikeThrough(wxCommandEvent& /* event unused */) override;
    void OnStyle(wxCommandEvent& /* event unused */) override;
    void OnSystemRadio(wxCommandEvent& /* event unused */) override;
    void OnSymbolSize(wxCommandEvent& /* event unused */) override;
    void OnUnderlined(wxCommandEvent& /* event unused */) override;
    void OnWeight(wxCommandEvent& /* event unused */) override;

    void OnOK(wxCommandEvent& /* event unused */) override;

    wxString m_value;
    wxue::string m_font_description;

private:
    wxFontEnumerator m_font_enum;
    FontProperty m_system_font;
    FontProperty m_custom_font;
};
