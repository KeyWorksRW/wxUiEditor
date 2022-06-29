/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handles property grid image properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "img_props.h"      // ImageProperties
#include "mainapp.h"        // App -- Main application class
#include "node.h"           // Node -- Node class
#include "project_class.h"  // Project class
#include "utils.h"          // Utility functions that work with properties

void ImageProperties::InitValues(const char* value)
{
    ttlib::multiview mstr(value, ';', tt::TRIM::both);

    if (mstr.size() > IndexImage)
        image = mstr[IndexImage];
    else
        image.clear();

    if (mstr.size() > IndexType)
    {
        type = mstr[IndexType];
        if (type == "Header" && image.extension().is_sameas(".xpm", tt::CASE::either))
            type = "XPM";

        if (type == "SVG" && mstr.size() > IndexImage + 1)
        {
            GetSizeInfo(m_size, mstr[IndexSize]);
        }
        else
        {
            auto img_bundle = wxGetApp().GetPropertyImageBundle(value);
            if (img_bundle)
            {
                m_size = img_bundle->bundle.GetDefaultSize();
            }
            else
            {
                m_size.x = -1;
                m_size.y = -1;
            }
        }
    }
}

ttlib::cstr ImageProperties::CombineValues()
{
    if (type.size() && type != "Art")
    {
        if (!image.file_exists() && wxGetApp().GetProject()->HasValue(prop_art_directory))
        {
            auto path = wxGetApp().GetProject()->prop_as_string(prop_art_directory);
            path.append_filename(image);
            path.make_relative(wxGetApp().getProjectPath());
            if (path.file_exists())
            {
                image = path;
            }
        }
    }

    ttlib::cstr value;
    image.backslashestoforward();
    value << type << ';' << image;
    if (type == "SVG")
        value << ";[" << m_size.x << ',' << m_size.y << "]";
    return value;
}

wxString ImageProperties::CombineDefaultSize()
{
    wxString value;
    value << m_size.x << ',' << m_size.y;
    return value;
}
