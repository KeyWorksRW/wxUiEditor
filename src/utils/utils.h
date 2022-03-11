/////////////////////////////////////////////////////////////////////////////
// Purpose:   Utility functions that work with properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/settings.h>

#include "ttcstr.h"   // ttlib::cstr -- std::string with additional methods
#include "ttsview.h"  // sview -- std::string_view with additional methods

#include "gen_enums.h"  // Enumerations for generators

class wxColour;
class wxImage;
class wxPoint;
class wxSize;
class Node;

// Used to index fields in a bitmap property
enum PropIndex
{
    IndexType = 0,
    IndexImage,
    IndexArtID = IndexImage,
    IndexSize
};

ttlib::cstr ClearPropFlag(ttlib::cview flag, ttlib::cview currentValue);
ttlib::cstr ClearMultiplePropFlags(ttlib::cview flags, ttlib::cview currentValue);
ttlib::cstr SetPropFlag(ttlib::cview flag, ttlib::cview currentValue);

// Convert a double to a string without needing to switch locales
ttlib::cstr DoubleToStr(double val);

bool isPropFlagSet(ttlib::cview flag, ttlib::cview currentValue);

wxSystemColour ConvertToSystemColour(ttlib::cview value);
int ConvertBitlistToInt(ttlib::cview list);

ttlib::cstr ConvertColourToString(const wxColour& colour);
ttlib::cstr ConvertSystemColourToString(long colour);
const char* ConvertFontFamilyToString(wxFontFamily family);

// Used to add escapes to a string that will be handed off to a wxWidgets property editor
ttlib::cstr CreateEscapedText(ttlib::cview str);

// if value begins with 'wx' then it is assumed to be a wxSystemColour
wxColour ConvertToColour(ttlib::cview value);

// Replace escape slashes with the actual character. Affects \\, \\n, \\r, and \\t
ttlib::cstr ConvertEscapeSlashes(ttlib::cview str);

std::vector<ttlib::cstr> ConvertToArrayString(ttlib::cview value);

// Use ConvertToArrayString() to get a vector, this function to get a wxArrayString
wxArrayString ConvertToWxArrayString(ttlib::cview value);

// Converts an unsigned char array into an image. This is typically used for loading internal
// #included images
wxImage LoadHeaderImage(const unsigned char* data, size_t size_data);

// If the property specifies dialog units, then parent will be used to do the conversion
wxPoint DlgPoint(wxObject* parent, Node* node, GenEnum::PropName prop);

// If the property specifies dialog units, then parent will be used to do the conversion
wxSize DlgSize(wxObject* parent, Node* node, GenEnum::PropName prop);

// Convert the parts[IndexSize] or equivalent string into wxSize dimensions
void GetSizeInfo(wxSize& size, ttlib::sview description);

// Friendly name/wxSYS_COLOUR_ pairs (e.g. "tooltip"/wxSYS_COLOUR_INFOBK)
extern std::map<std::string, const char*> g_sys_colour_pair;

// Friendly name/wxSTC_WRAP_ constant
extern std::map<std::string, const char*> g_stc_wrap_mode;
