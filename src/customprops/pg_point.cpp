/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom property grid class for wxPoint/wxSize
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "pg_point.h"

#include "node.h"             // Node -- Node class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties
#include "mainframe.h"        // MainFrame -- Main window frame

wxIMPLEMENT_ABSTRACT_CLASS(CustomPointProperty, wxPGProperty);

CustomPointProperty::CustomPointProperty(const wxString& label, NodeProperty* prop, DataType type) :
    wxPGProperty(label, wxPG_LABEL)
{
    m_prop_type = type;

    if (type == CustomPointProperty::type_SVG && prop->hasValue() && prop->as_string().contains("["))
    {
        tt_string value(prop->as_string().substr(prop->as_string().find('[') + 1));
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

    if (type != CustomPointProperty::type_SVG)
    {
        AddPrivateChild(new wxBoolProperty("support high dpi", wxPG_LABEL, m_dpi_scaling));
        Item(2)->SetHelpString("When checked, values will be scaled on high DPI displays.");
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
        if (m_prop_type != type_SVG)
            Item(2)->SetValue(m_dpi_scaling);
    }
}

wxVariant CustomPointProperty::ChildChanged(wxVariant& /* thisValue */, int childIndex, wxVariant& childValue) const
{
    wxString value = childValue;
    if (value.empty())
        return value;

    wxPoint point { m_point };
    bool dpi_scaling = m_dpi_scaling;

    switch (childIndex)
    {
        case 0:
            point.x = childValue.GetLong();
            break;

        case 1:
            point.y = childValue.GetLong();
            break;

        case 2:
            dpi_scaling = childValue.GetBool();
    }

    value.clear();
    value << point.x << ',' << point.y;
    if (dpi_scaling)
        value << 's';

    return value;
}

void CustomPointProperty::InitValues(tt_string_view value)
{
    if (value.empty())
    {
        m_point.x = -1;
        m_point.y = -1;
        m_dpi_scaling = Project.as_bool(prop_dialog_units);
    }
    else
    {
        tt_view_vector parts;
        if (value.contains(";"))
            parts.SetString(value, ';');
        else
            parts.SetString(value, ',');

        if (parts.size() < 2)
        {
            m_point.x = -1;
            m_point.y = -1;
            m_dpi_scaling = Project.as_bool(prop_dialog_units);
            return;
        }

        // We don't need to trim, because tt::atoi() skips leading whitespace
        m_point.x = tt::atoi(parts[0]);
        m_point.y = tt::atoi(parts[1]);
        m_dpi_scaling = tt::contains(value, "s", tt::CASE::either);
        if (!m_dpi_scaling && tt::contains(value, "d", tt::CASE::either))
        {
            m_point = wxGetApp().getMainFrame()->ConvertDialogToPixels(m_point);
            m_dpi_scaling = true;
        }

    }
}

tt_string CustomPointProperty::CombineValues()
{
    tt_string value;
    value << m_point.x << ',' << m_point.y;
    if (m_dpi_scaling)
        value << 's';
    return value;
}
