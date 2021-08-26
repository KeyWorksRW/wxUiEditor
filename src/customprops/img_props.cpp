/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handles property grid image properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "img_props.h"  // ImageProperties
#include "mainapp.h"    // App -- Main application class
#include "node.h"       // Node -- Node class
#include "utils.h"      // Utility functions that work with properties

void ImageProperties::InitValues(const char* value)
{
    ttlib::multistr mstr(value, ';');
    for (auto& iter: mstr)
    {
        iter.BothTrim();
    }

    if (mstr.size() > IndexType)
        type = mstr[IndexType];

    if (mstr.size() > IndexImage)
        image = mstr[IndexImage];

    if (mstr.size() > IndexImage + 1)
    {
        if (mstr[IndexScale][0] == '[')
        {
            m_size.x = atoi(mstr[IndexScale].c_str() + 1);
            if (auto pos_comma = mstr[IndexScale].find(','); ttlib::is_found(pos_comma))
            {
                m_size.y = atoi(mstr[IndexScale].c_str() + pos_comma + 1);
            }
            else
            {
                m_size.y = atoi(mstr[IndexScale + 1]);
            }
        }
        else
        {
            ttlib::multiview scale;
            if (mstr[IndexScale].contains(";"))
                scale.SetString(value, ';');
            else
                scale.SetString(value, ',');

            m_size.x = ttlib::atoi(scale[0]);
            if (scale.size() > 1)
                m_size.y = ttlib::atoi(scale[1]);
        }
    }
}

ttlib::cstr ImageProperties::CombineValues()
{
    if (type.size() && type != "Art")
    {
        if (!image.file_exists() && wxGetApp().GetProject()->HasValue(prop_original_art))
        {
            auto path = wxGetApp().GetProject()->prop_as_string(prop_original_art);
            path.append_filename(image);
            path.make_relative(wxGetApp().getProjectPath());
            if (path.file_exists())
            {
                image = path;
            }
        }
    }

    ttlib::cstr value;
    value << type << ';' << image << ";[" << m_size.x << ',' << m_size.y << "]";
    return value;
}

wxString ImageProperties::CombineScale()
{
    wxString value;
    value << m_size.x << ',' << m_size.y;
    return value;
}
