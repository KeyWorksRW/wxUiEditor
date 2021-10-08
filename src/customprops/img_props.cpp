/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handles property grid image properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "img_props.h"  // ImageProperties
#include "mainapp.h"    // App -- Main application class
#include "node.h"       // Node -- Node class
#include "utils.h"      // Utility functions that work with properties

void ImageProperties::InitValues(const char* value)
{
    ttlib::multiview mstr(value, ';', tt::TRIM::both);

    if (mstr.size() > IndexImage)
        image = mstr[IndexImage];

    if (mstr.size() > IndexType)
    {
        type = mstr[IndexType];
        if (type == "Header" && image.extension().is_sameas(".xpm", tt::CASE::either))
            type = "XPM";
    }

    if (mstr.size() > IndexImage + 1)
    {
        GetScaleInfo(m_size, mstr[IndexScale]);
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
