/////////////////////////////////////////////////////////////////////////////
// Purpose:   Utility functions that work with properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/settings.h>

#include <ttcstr.h>  // cstr -- Classes for handling zero-terminated char strings.

class wxColour;
class wxImage;
class wxPoint;
class wxSize;

// Used to index fields in a bitmap property
enum PropIndex
{
    IndexType = 0,
    IndexImage = 1,
    IndexArtID = 1,
    IndexConvert = 2,
    IndexArtClient = 2,
    IndexSize
};

ttlib::cstr ClearPropFlag(std::string_view flag, std::string_view currentValue);
ttlib::cstr ClearMultiplePropFlags(std::string_view flags, std::string_view currentValue);
ttlib::cstr SetPropFlag(std::string_view flag, std::string_view currentValue);

// Convert a double to a string without needing to switch locales
ttlib::cstr DoubleToStr(double val);

bool isPropFlagSet(std::string_view flag, std::string_view currentValue);

wxPoint ConvertToPoint(std::string_view value);
wxSize ConvertToSize(std::string_view value);
wxSystemColour ConvertToSystemColour(std::string_view value);
int ConvertBitlistToInt(std::string_view list);

ttlib::cstr ConvertPointToString(const wxPoint& point);
ttlib::cstr ConvertSizeToString(const wxSize& size);
ttlib::cstr ConvertColourToString(const wxColour& colour);
ttlib::cstr ConvertSystemColourToString(long colour);
const char* ConvertFontFamilyToString(wxFontFamily family);

// Add C++ escapes around any characters the compiler wouldn't accept as a normal part of a string.
ttlib::cstr ConvertToCodeString(std::string_view text);

// Used to add escapes to a string that will be handed off to a wxWidgets property editor
ttlib::cstr CreateEscapedText(ttlib::cview str);

// if value begins with 'wx' then it is assumed to be a wxSystemColour
wxColour ConvertToColour(std::string_view value);

// Replace escape slashes with the actual character. Affects \\, \\n, \\r, and \\t
ttlib::cstr ConvertEscapeSlashes(ttlib::cview str);

std::vector<std::string> ConvertToArrayString(std::string_view value);

// Converts a GZIP unsigned char array into an image.
wxImage LoadGzipImage(const unsigned char* data, size_t size_data);

// Converts a PNG unsigned char array into an image.
wxImage LoadPngHdrImage(const unsigned char* data, size_t size_data);

// Converts a GZIP unsigned char array into a string.
std::string LoadGzipString(const unsigned char* data, size_t size_data);
