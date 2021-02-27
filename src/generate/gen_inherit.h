/////////////////////////////////////////////////////////////////////////////
// Purpose:   Inherited class code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <optional>

#include "node_classes.h"  // Forward defintions of Node classes

// This generates code for the header file for Get() and Set() functions using function names
// specified by the user in the project file.
std::optional<ttlib::cstr> GenGetSetCode(Node* node);

std::optional<ttlib::cstr> GenInheritSettings(Node* node);
