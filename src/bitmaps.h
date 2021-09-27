/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains various images handling functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

class wxAnimation;
class wxIcon;
class wxImage;

// This is the size of component images
constexpr const int CompImgSize = 22;

// This will first look for an associated PNG Header array, and if unavailable an XPM file.
// If neither can be found, a 16x16 question mark image is returned.
wxImage GetInternalImage(ttlib::cview name);

// If this is a PNG Header file, the alpha channel will be converted to a mask
wxIcon GetIconImage(ttlib::cview name);

// Converts the ASCII header file into binary data and loads it as an image. It's designed to
// read header files created by wxUiEditor or wxFormBuilder -- any other generated header
// file might or might not work.
wxImage GetHeaderImage(ttlib::cview filename, size_t* p_original_size = nullptr, ttString* p_mime_type = nullptr);

// Converts the binary data into an image. This is typically use for loading internal
// #included images
wxImage LoadHeaderImage(const unsigned char* data, size_t size_data);

// Converts the ASCII header file into binary data and loads it as an animation. It's designed to
// read header files created by wxUiEditor or wxFormBuilder -- any other generated header
// file might or might not work.
bool GetAnimationImage(wxAnimation& animation, ttlib::cview filename);

wxAnimation LoadAnimationImage(wxAnimation& animation, const unsigned char* data, size_t size_data);

// Defined in all generated code
inline wxImage GetImageFromArray(const unsigned char* data, size_t size_data);
