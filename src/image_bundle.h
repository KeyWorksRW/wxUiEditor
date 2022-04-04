/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for working with wxBitmapBundle
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/bmpbndl.h>  // includes wx/bitmap.h, wxBitmapBundle class interface

struct ImageBundle
{
    wxBitmapBundle bundle;
    std::vector<ttlib::cstr> lst_filenames;
};
