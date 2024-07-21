/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom property grid class for wxPoint/wxSize
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/artprov.h>
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "pg_point.h"

#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node -- Node class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties

// No scaling is always supported if there is a 'n' at the end of the size/point string. However, no
// UI is shown to the user unless NO_SCALING_OPTION is defined.

// #define NO_SCALING_OPTION

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

#if NO_SCALING_OPTION
    // Starting with wxUiEditor 1.2.9.0, scaling information should never be stored in the property
    // itself as all scaling is done automatically.
    if (type != CustomPointProperty::type_SVG)
    {
        AddPrivateChild(new wxBoolProperty("support high dpi", wxPG_LABEL, m_dpi_scaling));
        Item(2)->SetHelpString("When checked, values will be scaled on high DPI displays.");
    }
#endif
}

void CustomPointProperty::RefreshChildren()
{
    wxString value = m_value;
    if (value.size())
    {
        InitValues(value.utf8_string());
        Item(0)->SetValue(m_point.x);
        Item(1)->SetValue(m_point.y);
#if NO_SCALING_OPTION
        if (m_prop_type != type_SVG)
            Item(2)->SetValue(m_dpi_scaling);
#endif
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

#if NO_SCALING_OPTION
        case 2:
            dpi_scaling = childValue.GetBool();
            break;
#endif
    }

    value.clear();
    value << point.x << ',' << point.y;
    if (!dpi_scaling)
        value << 'n';

    return value;
}

void CustomPointProperty::InitValues(tt_string_view value)
{
    if (value.empty())
    {
        m_point.x = -1;
        m_point.y = -1;
    }
    else
    {
        tt_view_vector parts;
        if (value.contains(";"))
            parts.SetString(value, ';');
        else
            parts.SetString(value, ',');

        if (parts.size() < 2 || m_prop_type == type_BITMAP)
        {
            m_point.x = -1;
            m_point.y = -1;
            return;
        }

        if (m_prop_type == type_ART && parts.size() > 1)
        {
            m_point.x = tt::atoi(parts[0]);
            m_point.y = tt::atoi(parts[1]);
            return;
        }

        // We don't need to trim, because tt::atoi() skips leading whitespace
        m_point.x = tt::atoi(parts[0]);
        m_point.y = tt::atoi(parts[1]);

        // If mainframe window was created before the project was loaded, then any values with 'd' should already have been
        // converted to pixels. This just ensures it still works in case we missed something.
        ASSERT_MSG(!tt::contains(value, 'd', tt::CASE::either), "'d' in size/point not converted when project loaded.");
        if (tt::contains(value, 'd', tt::CASE::either))
        {
            m_point = wxGetApp().getMainFrame()->ConvertDialogToPixels(m_point);
        }

        if (tt::contains(value, 'n', tt::CASE::either))
            m_dpi_scaling = false;
    }
}

tt_string CustomPointProperty::CombineValues()
{
    tt_string value;
    value << m_point.x << ',' << m_point.y;
    if (!m_dpi_scaling)
        value << 'n';
    return value;
}
