/////////////////////////////////////////////////////////////////////////////
// Purpose:   Utility functions that work with properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/settings.h>

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

tt_string ClearPropFlag(tt_string_view flag, tt_string_view currentValue);
tt_string ClearMultiplePropFlags(tt_string_view flags, tt_string_view currentValue);
tt_string SetPropFlag(tt_string_view flag, tt_string_view currentValue);

// Convert a double to a string without needing to switch locales
tt_string DoubleToStr(double val);

bool isPropFlagSet(tt_string_view flag, tt_string_view currentValue);

wxSystemColour ConvertToSystemColour(tt_string_view value);
int ConvertBitlistToInt(tt_string_view list);

tt_string ConvertColourToString(const wxColour& colour);
tt_string ConvertSystemColourToString(long colour);
const char* ConvertFontFamilyToString(wxFontFamily family);

// Used to add escapes to a string that will be handed off to a wxWidgets property editor
tt_string CreateEscapedText(tt_string_view str);

// if value begins with 'wx' then it is assumed to be a wxSystemColour
wxColour ConvertToColour(tt_string_view value);

// Replace escape slashes with the actual character. Affects \\, \\n, \\r, and \\t
tt_string ConvertEscapeSlashes(tt_string_view str);

std::vector<tt_string> ConvertToArrayString(tt_string_view value);

// Converts an unsigned char array into an image. This is typically used for loading internal
// #included images
wxImage LoadHeaderImage(const unsigned char* data, size_t size_data);

// If the property specifies dialog units, then parent will be used to do the conversion
wxPoint DlgPoint(wxObject* parent, Node* node, GenEnum::PropName prop);

// If the property specifies dialog units, then parent will be used to do the conversion
wxSize DlgSize(wxObject* parent, Node* node, GenEnum::PropName prop);

// Convert the parts[IndexSize] or equivalent string into wxSize dimensions
void GetSizeInfo(wxSize& size, tt_string_view size_description);

// Convert the parts[IndexSize] or equivalent string into wxSize dimensions
wxSize get_image_prop_size(tt_string_view size_description);

// Friendly name/wxSYS_COLOUR_ pairs (e.g. "tooltip"/wxSYS_COLOUR_INFOBK)
extern std::map<std::string, const char*> g_sys_colour_pair;

// Friendly name/wxSTC_WRAP_ constant
extern std::map<std::string, const char*> g_stc_wrap_mode;

bool isConvertibleMime(const tt_wxString& suffix);

// Checks whether a string is a valid C++ variable name.
bool isValidVarName(const std::string& str);

tt_string CreateBaseFilename(Node* form_node, const tt_string& class_name);

tt_string CreateDerivedFilename(Node* form_node, const tt_string& class_name);
