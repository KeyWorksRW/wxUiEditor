/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom property grid class for wxPoint
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/property.h>

class NodeProperty;

class CustomPointProperty : public wxPGProperty
{
public:
    wxDECLARE_ABSTRACT_CLASS(CustomSizeProperty);

    enum DataType
    {
        type_size,
        type_point,
        type_SVG
    };

    CustomPointProperty(const wxString& label, NodeProperty* prop, DataType type = CustomPointProperty::type_size);

    wxVariant ChildChanged(wxVariant& thisValue, int childIndex, wxVariant& childValue) const override;
    void RefreshChildren() override;

    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrl; }

    void InitValues(tt_string_view value);
    tt_string CombineValues();

private:
    wxPoint m_point { wxDefaultPosition };
    bool m_dialog_units { false };
    DataType m_prop_type;
};
