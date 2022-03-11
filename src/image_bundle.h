/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for working with wxBitmapBundle
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#if !wxCHECK_VERSION(3, 1, 6)
    #error "Don't include this file unless compiling with 3.1.6 or later."
#endif

#include <wx/bmpbndl.h>  // includes wx/bitmap.h, wxBitmapBundle class interface

struct ImageBundle
{
    wxBitmapBundle bundle;
    std::vector<ttlib::cstr> lst_filenames;
};
