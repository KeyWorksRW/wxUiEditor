/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom property grid class for wxPoint/wxSize
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/artprov.h>
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "pg_point.h"

#include "image_handler.h"                    // ImageHandler class
#include "mainframe.h"                        // MainFrame -- Main window frame
#include "node.h"                             // Node -- Node class
#include "utils.h"                            // Utility functions that work with properties
#include "wxue_namespace/wxue_view_vector.h"  // wxue::ViewVector

wxIMPLEMENT_ABSTRACT_CLASS(CustomPointProperty, wxPGProperty);

CustomPointProperty::CustomPointProperty(const wxString& label, NodeProperty* prop, DataType type) :
    wxPGProperty(label, wxPG_LABEL)
{
    m_prop_type = type;

    if ((type == CustomPointProperty::type_SVG || type == CustomPointProperty::type_ART) &&
        prop->HasValue() && prop->as_string().contains("["))
    {
        wxue::string value(prop->as_string().substr(prop->as_string().find('[') + 1));
        if (value.back() == ']')
        {
            value.pop_back();
        }
        m_value = value;
        InitValues(value);
    }
    else if (type == CustomPointProperty::type_BITMAP && prop->HasValue())
    {
        wxue::ViewVector parts;
        parts.SetString(std::string_view { prop->as_string() }, ';');
        if (parts.size() > IndexImage)
        {
            auto* embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
            if (embed)
            {
                m_org_size = embed->get_wxSize();
            }
        }
        m_value = prop->as_wxString();
        InitValues(prop->as_string());
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

    // Starting with wxUiEditor 1.2.9.0, scaling information should never be stored in the property
    // itself as all scaling is done automatically.
    if (type != CustomPointProperty::type_SVG && type != CustomPointProperty::type_ART &&
        type != CustomPointProperty::type_BITMAP && !prop->getNode()->is_Gen(gen_wxWizard))
    {
        AddPrivateChild(new CustomBoolProperty("high dpi support", wxPG_LABEL, m_dpi_scaling));
        Item(2)->SetHelpString("When checked, values will be scaled on high DPI displays.");
    }
}

auto CustomPointProperty::RefreshChildren() -> void
{
    wxString value = m_value;
    if (value.size())
    {
        InitValues(value.utf8_string());
        Item(0)->SetValue(m_point.x);
        Item(1)->SetValue(m_point.y);
        if (m_prop_type != type_SVG && m_prop_type != type_ART && m_prop_type != type_BITMAP)
        {
            Item(2)->SetValue(m_dpi_scaling);
        }
    }
}

wxVariant CustomPointProperty::ChildChanged([[maybe_unused]] wxVariant& thisValue, int childIndex,
                                            wxVariant& childValue) const
{
    wxString value = childValue;
    if (value.empty())
    {
        return value;
    }

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
            break;
    }

    value.clear();
    value << point.x << ',' << point.y;
    if (!dpi_scaling && m_prop_type != type_SVG && m_prop_type != type_ART &&
        m_prop_type != type_BITMAP)
    {
        value << 'n';
    }

    return value;
}

auto CustomPointProperty::InitValues(wxue::string_view value) -> void
{
    if (value.empty())
    {
        m_point.x = -1;
        m_point.y = -1;
    }
    else
    {
        wxue::ViewVector parts;
        if (value.contains(";"))
        {
            parts.SetString(value, ';');
        }
        else
        {
            parts.SetString(value, ',');
        }

        if (m_prop_type == type_BITMAP)
        {
            m_point.x = m_org_size.x;
            m_point.y = m_org_size.y;
            return;
        }

        if (parts.size() < 2)
        {
            m_point.x = -1;
            m_point.y = -1;
            return;
        }

        if (m_prop_type == type_ART && parts.size() > 1)
        {
            m_point.x = wxue::atoi(parts[0]);
            m_point.y = wxue::atoi(parts[1]);
            return;
        }

        // We don't need to trim, because wxue::atoi() skips leading whitespace
        m_point.x = wxue::atoi(parts[0]);
        m_point.y = wxue::atoi(parts[1]);

        // If mainframe window was created before the project was loaded, then any values with 'd'
        // should already have been converted to pixels. This just ensures it still works in case we
        // missed something.
        ASSERT_MSG(!wxue::contains(value, 'd', wxue::CASE::either),
                   "'d' in size/point not converted when project loaded.");
        if (wxue::contains(value, 'd', wxue::CASE::either))
        {
            m_point = wxGetApp().getMainFrame()->ConvertDialogToPixels(m_point);
        }

        m_dpi_scaling = (wxue::contains(value, 'n', wxue::CASE::either) == false);
    }
}

auto CustomPointProperty::CombineValues() -> wxue::string
{
    wxue::string value;
    value << m_point.x << ',' << m_point.y;
    if (!m_dpi_scaling && m_prop_type != type_SVG && m_prop_type != type_ART &&
        m_prop_type != type_BITMAP)
    {
        value << 'n';
    }
    return value;
}

// The wxWidgets version uses "Not" when the value is false. This version uses "No" instead.
wxString CustomBoolProperty::ValueToString(wxVariant& value, wxPGPropValFormatFlags flags) const
{
    bool boolValue = value.GetBool();

    if (!!(flags & wxPGPropValFormatFlags::CompositeFragment))
    {
        if (boolValue)
        {
            return m_label;
        }
        if (!!(flags & wxPGPropValFormatFlags::UneditableCompositeFragment))
        {
            return wxString();
        }

        return (wxString("No ") + m_label);
    }

    if (!(flags & wxPGPropValFormatFlags::FullValue))
    {
        return wxPGGlobalVars->m_boolChoices[boolValue ? 1 : 0].GetText();
    }

    return boolValue ? "true" : "false";
}
