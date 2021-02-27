/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom property grid class for wxSize
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/property.h>  // wxPGProperty and related support classes

class CustomSizeProperty : public wxPGProperty
{
public:
    wxDECLARE_ABSTRACT_CLASS(CustomSizeProperty);

    CustomSizeProperty(const wxString& label, const wxSize& value);

    wxVariant ChildChanged(wxVariant& thisValue, int childIndex, wxVariant& childValue) const override;
    void RefreshChildren() override;

    const wxPGEditor* DoGetEditorClass() const override
    {
        return wxPGEditor_TextCtrl;
    }
};
