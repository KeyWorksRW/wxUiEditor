/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom property grid class for wxPoint/wxSize
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/property.h>
#include <wx/propgrid/props.h>

class NodeProperty;

class CustomPointProperty : public wxPGProperty
{
public:
    wxDECLARE_ABSTRACT_CLASS(CustomSizeProperty);

    enum DataType
    {
        type_size,
        type_point,
        type_SVG,
        type_ART,
        type_BITMAP
    };

    CustomPointProperty(const wxString& label, NodeProperty* prop,
                        DataType type = CustomPointProperty::type_size);

    wxVariant ChildChanged(wxVariant& thisValue, int childIndex,
                           wxVariant& childValue) const override;
    void RefreshChildren() override;

    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrl; }

    void InitValues(tt_string_view value);
    tt_string CombineValues();

private:
    wxPoint m_point { wxDefaultPosition };
    wxSize m_org_size { wxDefaultSize };  // original size of an embedded image
    bool m_dpi_scaling { true };
    DataType m_prop_type;
};

// Custom version that uses "No " instead of "Not " as the prefix if the value is false
class CustomBoolProperty : public wxBoolProperty
{
public:
    CustomBoolProperty(const wxString& label = wxPG_LABEL, const wxString& name = wxPG_LABEL,
                       bool value = false) : wxBoolProperty(label, name, value)
    {
    }

    wxString
        ValueToString(wxVariant& value,
                      wxPGPropValFormatFlags flags = wxPGPropValFormatFlags::Null) const override;
};
