/////////////////////////////////////////////////////////////////////////////
// Purpose:   Property types
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// The XML component definition files specify the type of a property. Those types get mapped to these
// enumerated types so that we can use the property Type in a switch statement.

#pragma once

enum class Type : size_t
{
    None,
    Bool,

    String,
    String_Escapes,  // This doubles the backslash in escaped characters: \n, \t, \r, and "\""
    String_Edit,     // This includes a button that triggers a small text editor dialog
    String_Edit_Escapes,  // Includes editor dialog and also escapes characters
    String_Edit_Single,  // Includes single-line text editor, does not process escapes

    Int,
    Uint,
    Image,
    Bitlist,
    Uintlist,
    Uintpairlist,
    Option,
    ID,
    Wxpoint,
    Wxsize,
    Wxfont,
    Wxcolour,
    Wxparent,
    Wxparent_sb,
    Wxparent_cp,
    Path,
    File,
    Bitmap,
    Stringlist,
    Float,
    Parent,
    Edit_option  // same as Option, except that property grid uses wxEditEnumProperty instead of wxEnumProperty
};
