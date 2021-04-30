/////////////////////////////////////////////////////////////////////////////
// Purpose:   Map of bitmaps accessed by name
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/bitmap.h>  // wxBitmap class interface
#include <wx/image.h>   // wxImage class

constexpr const int CompImgSize = 22;
constexpr const int TOOL_SIZE = 22;

void InitializeIcons();

// Converts an image stored as an unsigned char array in a header file. It's designed to read
// header files created by wxUiEditor -- any other generated header file might or might not
// work.
wxImage GetHeaderImage(ttlib::cview filename);

// Returns wxImage(default.xpm) if string cannot be found. Default.xpm is a question mark.
wxImage GetXPMImage(ttlib::cview name);
