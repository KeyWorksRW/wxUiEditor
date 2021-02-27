/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process a Windows Resource form  (usually a dialog)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <tttextfile.h>  // ttTextFile -- Similar to wxTextFile, but uses UTF8 strings

#include "winres_ctrl.h"

class rcForm
{
public:
    rcForm();

    void ParseDialog(ttlib::textfile& txtfile, size_t& curTxtLine);

    // These are public so that the WinResource class can easily access them while it converts parse forms into
    // wxUiEditor objects

    ttlib::cstr m_Name;
    ttlib::cstr m_Title;
    ttlib::cstr m_Font;
    ttlib::cstr m_ID;

    ttlib::cstr m_BaseName;     // Generated filename
    ttlib::cstr m_DerivedName;  // Derived filename

    ttlib::cstr m_Styles;
    ttlib::cstr m_ExStyles;
    ttlib::cstr m_WinStyles;
    ttlib::cstr m_WinExStyles;

    ttlib::cstr m_Center;  // wxBOTH, wxHORIZONTAL, or wxVERTICAL

    std::vector<rcCtrl> m_ctrls;

    RC_RECT m_rc { 0, 0, 0, 0 };

protected:
    void AddStyle(ttlib::textfile& txtfile, size_t& curTxtLine);
    void ParseControls(ttlib::textfile& txtfile, size_t& curTxtLine);
    void GetDimensions(ttlib::cview line);
};
