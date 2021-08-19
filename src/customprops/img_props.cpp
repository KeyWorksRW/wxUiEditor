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
        ttlib::multistr dimensions(mstr[IndexScale], ',');
        for (auto& iter: dimensions)
        {
            iter.BothTrim();
        }

        if (dimensions.size() > 0)
            size.x = atoi(dimensions[0].c_str() + 1);  // step over the leading bracket
        if (dimensions.size() > 1)
            size.y = atoi(dimensions[1]);
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
    value << type << ';' << image << ";[" << size.x << ',' << size.y << "]";
    return value;
}
