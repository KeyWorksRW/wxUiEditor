/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom property grid class for wxPoint
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "pg_point.h"

wxIMPLEMENT_ABSTRACT_CLASS(CustomPointProperty, wxPGProperty);

CustomPointProperty::CustomPointProperty(const wxString& label, const wxPoint& value) : wxPGProperty(label, wxPG_LABEL)
{
    m_value << value;
    AddPrivateChild(new wxIntProperty("x", wxPG_LABEL, value.x));
    AddPrivateChild(new wxIntProperty("y", wxPG_LABEL, value.y));
}

void CustomPointProperty::RefreshChildren()
{
    auto& point = wxPointRefFromVariant(m_value);
    Item(0)->SetValue(point.x);
    Item(1)->SetValue(point.y);
}

wxVariant CustomPointProperty::ChildChanged(wxVariant& thisValue, int childIndex, wxVariant& childValue) const
{
    auto& point = wxPointRefFromVariant(thisValue);
    if (childIndex == 0)
        point.x = childValue.GetLong();
    else
        point.y = childValue.GetLong();

    return wxVariant(point);
}
