/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handles property grid image properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "img_props.h"  // ImageProperties
#include "mainapp.h"    // MoveDirection -- Main application class
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

    if (mstr.size() > IndexConvert)
        convert = mstr[IndexConvert];

    if (mstr.size() > IndexSize)
    {
        ttlib::multistr dimensions(mstr[IndexSize], ',');
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
    // The user may have picked the filename from the autocomplete, in which case it's in the converted_art directory.
    // Or they may have chosen it from the File Open dialog which uses the converted_art as the default directory. In
    // either case, we need to change the path to the file's actual location.

    if (type.size() && type != "Art")
    {
        if (!image.file_exists() && wxGetApp().GetProject()->HasValue(prop_converted_art))
        {
            auto path = wxGetApp().GetProject()->prop_as_string(prop_converted_art);
            path.append_filename(image);
            path.make_relative(wxGetApp().getProjectPath());
            if (path.file_exists())
            {
                image = path;
            }
        }
    }

    ttlib::cstr value;
    value << type << ';' << image << ';' << convert << ";[" << size.x << ',' << size.y << "]";
    return value;
}
