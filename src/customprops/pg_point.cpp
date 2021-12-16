/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom property grid class for wxPoint
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include <ttmultistr.h>  // multistr -- Breaks a single string into multiple strings

#include "pg_point.h"

#include "node.h"   // Node -- Node class
#include "utils.h"  // Utility functions that work with properties

wxIMPLEMENT_ABSTRACT_CLASS(CustomPointProperty, wxPGProperty);

CustomPointProperty::CustomPointProperty(const wxString& label, NodeProperty* prop, DataType type) :
    wxPGProperty(label, wxPG_LABEL)
{
    m_prop_type = type;

    if (type == CustomPointProperty::type_scale && prop->HasValue() && prop->as_string().contains("["))
    {
        ttlib::cstr value(prop->as_string().substr(prop->as_string().find('[') + 1));
        if (value.back() == ']')
            value.pop_back();
        m_value = value;
        InitValues(value);
    }
    else
    {
        m_value = prop->as_wxString();
        InitValues(prop->as_string());
    }

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
        AddPrivateChild(new wxBoolProperty("using dialog units", wxPG_LABEL, m_dialog_units));
        Item(2)->SetHelpString("When checked, values will be converted to dialog units by calling ConvertPixelsToDialog().");
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
        if (m_prop_type != type_scale)
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
    value << point.x << ',' << point.y;
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
        m_dialog_units = true;
    }
    else
    {
        ttlib::multiview parts;
        if (value.contains(";"))
            parts.SetString(value, ';');
        else
            parts.SetString(value, ',');

        // We don't need to trim, because ttlib::atoi() skips leading whitespace
        m_point.x = ttlib::atoi(parts[0]);
        m_point.y = ttlib::atoi(parts[1]);
        m_dialog_units = ttlib::contains(value, "d", tt::CASE::either);
    }
}

ttlib::cstr CustomPointProperty::CombineValues()
{
    ttlib::cstr value;
    value << m_point.x << ',' << m_point.y;
    if (m_dialog_units)
        value << 'd';
    return value;
}
