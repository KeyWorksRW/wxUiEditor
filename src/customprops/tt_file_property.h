/////////////////////////////////////////////////////////////////////////////
// Purpose:   Version of wxFileProperty specific to wxUiEditor
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/propgrid.h>  // for wxEditorDialogProperty

class NodeProperty;

class ttFileProperty : public wxEditorDialogProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS(ttFileProperty)

public:
    ttFileProperty(NodeProperty* prop);
    ttFileProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL,
                   const wxString& value = wxString());

    ~ttFileProperty() = default;

    void OnSetValue() override;
    wxString
        ValueToString(wxVariant& value,
                      wxPGPropValFormatFlags flags = wxPGPropValFormatFlags::Null) const override;
    bool StringToValue(wxVariant& variant, const wxString& text,
                       wxPGPropValFormatFlags flags = wxPGPropValFormatFlags::Null) const override;

    bool DoSetAttribute(const wxString& name, wxVariant& value) override;

    static wxValidator* GetClassValidator();
    virtual wxValidator* DoGetValidator() const override;

protected:
    bool DisplayEditorDialog(wxPropertyGrid* pg, wxVariant& value) override;

private:
    NodeProperty* m_prop;
};
