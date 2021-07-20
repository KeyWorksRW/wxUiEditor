/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handles property grid animation properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "anim_props.h"  // AnimationProperties
#include "mainapp.h"     // App -- Main application class
#include "node.h"        // Node -- Node class
#include "utils.h"       // Utility functions that work with properties

void AnimationProperties::InitValues(const char* value)
{
    ttlib::multistr mstr(value, ';');
    for (auto& iter: mstr)
    {
        iter.BothTrim();
    }

    if (mstr.size())
        image = mstr[0];

    if (mstr.size() > 1)
        convert = mstr[1];
}

ttlib::cstr AnimationProperties::CombineValues()
{
    // The user may have picked the filename from the autocomplete, in which case it's in the converted_art directory.
    // Or they may have chosen it from the File Open dialog which uses the converted_art as the default directory. In
    // either case, we need to change the path to the file's actual location.

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

    ttlib::cstr value;
    value << image << ';' << convert;
    return value;
}
