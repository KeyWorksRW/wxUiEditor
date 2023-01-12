/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handles property grid image properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "img_props.h"        // ImageProperties
#include "image_handler.h"    // ImageHandler class
#include "node.h"             // Node -- Node class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties

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
            auto img_bundle = ProjectImages.GetPropertyImageBundle(value);
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
        if (!image.file_exists() && Project.HasValue(prop_art_directory))
        {
            auto path = Project.ArtDirectory();
            path.append_filename(image);
            path.make_relative_wx(Project.ProjectPath());
            if (path.file_exists())
            {
                image = path.utf8_string();
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
