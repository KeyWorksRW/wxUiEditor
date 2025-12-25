
/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains various images handling functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/mstream.h>  // Memory stream classes

class wxAnimation;
class wxIcon;
class wxImage;

// This is the size of component images
constexpr const int GenImageSize = 22;

// This will first look for an associated PNG Header array, and if unavailable an XPM file.
// If neither can be found, a 16x16 question mark image is returned.
[[nodiscard]] auto GetInternalImage(wxue::string_view name) -> wxImage;

// If this is a PNG Header file, the alpha channel will be converted to a mask
[[nodiscard]] auto GetIconImage(wxue::string_view name) -> wxIcon;

// Converts the ASCII header file into binary data and loads it as an image. It's designed to
// read header files created by wxUiEditor or wxFormBuilder -- any other generated header
// file might or might not work.
[[nodiscard]] auto GetHeaderImage(wxue::string_view filename, size_t* p_original_size = nullptr,
                                  wxue::string* p_mime_type = nullptr) -> wxImage;

// Converts the ASCII header file into binary data and loads it as an animation. It's designed to
// read header files created by wxUiEditor or wxFormBuilder -- any other generated header
// file might or might not work.
[[nodiscard]] auto GetAnimationImage(wxAnimation& animation, wxue::string_view filename) -> bool;

[[nodiscard]] auto LoadAnimationImage(wxAnimation& animation, const unsigned char* data,
                                      size_t size_data) -> wxAnimation;

// Convert a data array into a wxImage
[[nodiscard]] inline auto GetImageFromArray(const unsigned char* data, size_t size_data) -> wxImage
{
    wxMemoryInputStream strm(data, size_data);
    wxImage image;
    image.LoadFile(strm);
    return image;
}

// This is almost identical to GetImageFromArray() -- the only difference is that this one
// first tries to load the image via the PNG handler.
[[nodiscard]] auto LoadHeaderImage(const unsigned char* data, size_t size_data) -> wxImage;

extern const std::map<std::string_view, std::function<wxBitmapBundle(int width, int height)>>
    map_svg_functions;

[[nodiscard]] inline auto GetSvgFunction(wxue::string_view name)
    -> std::function<wxBitmapBundle(int width, int height)>
{
    auto it = map_svg_functions.find(name);
    if (it != map_svg_functions.end())
    {
        return it->second;
    }
    return std::function<wxBitmapBundle(int width, int height)>();
}

[[nodiscard]] auto GetSvgImage(wxue::string_view name, int width = 22, int height = 22)
    -> wxBitmapBundle;
[[nodiscard]] auto GetSvgImage(wxue::string_view name, const wxSize& size) -> wxBitmapBundle;
