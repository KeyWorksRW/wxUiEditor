/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom property grid class for wxPoint
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include <ttmultistr.h>  // multistr -- Breaks a single string into multiple strings

#include "pg_point.h"

#include "node.h"  // Node -- Node class

wxIMPLEMENT_ABSTRACT_CLASS(CustomPointProperty, wxPGProperty);

CustomPointProperty::CustomPointProperty(const wxString& label, NodeProperty* prop, data_type type) :
    wxPGProperty(label, wxPG_LABEL)
{
    m_value = prop->as_wxString();
    InitValues(prop->as_string());

    // For a property, the only difference between a size and a point is the label

    if (type == CustomPointProperty::type_point)
    {
        AddPrivateChild(new wxIntProperty("x", wxPG_LABEL, m_point.x));
        AddPrivateChild(new wxIntProperty("y", wxPG_LABEL, m_point.y));
    }
    else
    {
        AddPrivateChild(new wxIntProperty("width", wxPG_LABEL, m_point.x));
        AddPrivateChild(new wxIntProperty("height", wxPG_LABEL, m_point.y));
    }

    if (type != CustomPointProperty::type_scale)
    {
        AddPrivateChild(new wxBoolProperty("use dialog units", wxPG_LABEL, m_dialog_units));
        Item(2)->SetHelpString("When checked, values will be converted to dialog units before being used.");
    }
}

void CustomPointProperty::RefreshChildren()
{
    wxString value = m_value;
    if (value.size())
    {
        InitValues(value.utf8_string());
        Item(0)->SetValue(m_point.x);
        Item(1)->SetValue(m_point.y);
        Item(2)->SetValue(m_dialog_units);
    }
}

wxVariant CustomPointProperty::ChildChanged(wxVariant& /* thisValue */, int childIndex, wxVariant& childValue) const
{
    wxString value = childValue;
    if (value.empty())
        return value;

    wxPoint point { m_point };
    bool dialog_units = m_dialog_units;

    switch (childIndex)
    {
        case 0:
            point.x = childValue.GetLong();
            break;

        case 1:
            point.y = childValue.GetLong();
            break;

        case 2:
            dialog_units = childValue.GetBool();
    }

    value.clear();
    value << point.x << ';' << point.y;
    if (dialog_units)
        value << 'd';

    return value;
}

void CustomPointProperty::InitValues(ttlib::cview value)
{
    if (value.empty())
    {
        m_point.x = -1;
        m_point.y = -1;
        m_dialog_units = false;
    }
    else
    {
        ttlib::multistr mstr(value, ';');
        for (auto& iter: mstr)
        {
            iter.BothTrim();
        }

        m_point.x = mstr[0].atoi();
        m_point.y = mstr[1].atoi();
        m_dialog_units = ttlib::contains(value, "d", tt::CASE::either);
    }
}

ttlib::cstr CustomPointProperty::CombineValues()
{
    ttlib::cstr value;
    value << m_point.x << ';' << m_point.y;
    if (m_dialog_units)
        value << 'd';
    return value;
}
