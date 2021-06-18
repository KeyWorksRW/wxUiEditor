/////////////////////////////////////////////////////////////////////////////
// Purpose:   Miscellaneous functions for displaying UI
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/msgdlg.h>
#include <wx/string.h>

#include "ttstrings.h"  // Functions accessing translatable strings

#include "strings.h"  // Localized strings

/////////////////////// inline functions ////////////////////////////////////////

// If you are using any of the _tt() macros for localizable strings, then using appMsgBox
// instead of wxMessageBox will ensure your UTF8 localized strings are properly converted to
// UTF16 on Windows.
inline int appMsgBox(const ttlib::cstr str, ttlib::cview caption = txtAppname, int style = wxICON_WARNING | wxOK,
                     wxWindow* parent = nullptr)
{
    return wxMessageBox(str.wx_str(), caption.c_str(), style, parent);
}
