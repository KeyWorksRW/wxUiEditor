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

    Text,           // "text" -- uses a multi-line editor, newlines are escaped
    RawText,        // "text" -- uses a multi-line editor, but with raw newline characters
    String,         // "string" -- like "text", but with no editor button
    Wxstring,       // "wxString" -- places the string in quotes when used
    Translate,      // "translate" places the string in quotes and surrounds it with _() macro

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
