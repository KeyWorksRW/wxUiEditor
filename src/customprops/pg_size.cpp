/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom property grid class for wxSize
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "pg_size.h"

wxIMPLEMENT_ABSTRACT_CLASS(CustomSizeProperty, wxPGProperty);

CustomSizeProperty::CustomSizeProperty(const wxString& label, const wxSize& value) : wxPGProperty(label, wxPG_LABEL)
{
    m_value << value;
    AddPrivateChild(new wxIntProperty("width", wxPG_LABEL, value.x));
    AddPrivateChild(new wxIntProperty("height", wxPG_LABEL, value.y));
}

void CustomSizeProperty::RefreshChildren()
{
    auto& size = wxSizeRefFromVariant(m_value);
    Item(0)->SetValue(size.x);
    Item(1)->SetValue(size.y);
}

wxVariant CustomSizeProperty::ChildChanged(wxVariant& thisValue, int childIndex, wxVariant& childValue) const
{
    auto& size = wxSizeRefFromVariant(thisValue);
    if (childIndex == 0)
        size.x = childValue.GetLong();
    else
        size.y = childValue.GetLong();

    return wxVariant(size);
}
