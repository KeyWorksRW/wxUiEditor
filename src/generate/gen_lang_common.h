/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common mulit-language functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "base_generator.h"  // BaseGenerator -- Base Generator class

const char* LangPtr(int language);
ttlib::cstr GetWidgetName(int language, ttlib::sview name);

// This is *NOT* the same as get_node_name() -- this will handle wxStaticBox and
// wxCollapsiblePane parents as well as "normal" parents
ttlib::cstr GetParentName(int language, Node* node);

// Generates " = new class(" -- with class being the derived_class (if specified) or the
// normal class name. If use_generic specified, Generic will be inserted into the class name.
ttlib::cstr GenerateNewAssignment(int language, Node* node, bool use_generic = false);

// Places the string in wxString::FromUTF8(), adds C++ escapes around any characters the
// compiler wouldn't accept as a normal part of a string, and wraps it all in _() if
// prop_internationalize is true.
//
// Will return "wxEmptyString" if prop_name is empty.
ttlib::cstr GenerateQuotedString(int language, const ttlib::cstr& str);

// Generate wxSizerFlags() function based on prop_proportion, prop_alignment, prop_flags,
// prop_borders and prop_border_size
ttlib::cstr GenerateSizerFlags(int language, Node* node);
