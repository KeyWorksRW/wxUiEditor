/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common mulit-language functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class Code;

const char* LangPtr(int language);
ttlib::cstr GetWidgetName(int language, ttlib::sview name);

// Python breaks classes and constants into wx.name -- e.g., wxID_ANY becomes wx.ID_ANY
ttlib::cstr GetPythonName(ttlib::sview name);

// Places the string in wxString::FromUTF8(), adds C++ escapes around any characters the
// compiler wouldn't accept as a normal part of a string, and wraps it all in _() if
// prop_internationalize is true.
//
// Will return "wxEmptyString" if prop_name is empty.
ttlib::cstr GeneratePythonQuotedString(const ttlib::cstr& str);

// Generate wxSizerFlags() function based on prop_proportion, prop_alignment, prop_flags,
// prop_borders and prop_border_size
ttlib::cstr GenerateSizerFlags(int language, Node* node);
