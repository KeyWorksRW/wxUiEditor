/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process Windows Resource control data
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#if 0
    #include "rcData.h"
#else
struct RC_RECT
{
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
};
#endif

class rcCtrl
{
public:
    rcCtrl();

    void ParseStaticCtrl(ttlib::cview line);
    void ParseEditCtrl(ttlib::cview line);
    void ParsePushButton(ttlib::cview line);

    // These are public so that the WinResource class can easily access them while it converts parse forms into
    // wxUiEditor objects

    ttlib::cstr m_ID;
    ttlib::cstr m_Class;
    ttlib::cstr m_Value;
    ttlib::cstr m_Label;
    ttlib::cstr m_ToolTip;

    ttlib::cstr m_Styles;
    ttlib::cstr m_WinStyles;
    ttlib::cstr m_WinExStyles;

    int m_Wrap { -1 };
    int m_MaxLength { -1 };
    int m_minWidth;
    int m_minHeight;

    RC_RECT m_rc { 0, 0, 0, 0 };

    bool m_isMultiLine { false };
    bool m_isDefault { false };
    bool m_isChecked { false };
    bool m_isEnabled { true };
    bool m_isHidden { false };
    bool m_isPlaced { false };

protected:
    void ParseCommonStyles(ttlib::cview line);
    void GetDimensions(ttlib::cview line);

    ttlib::cview StepOverQuote(ttlib::cview line, ttlib::cstr& str);
    ttlib::cview StepOverComma(ttlib::cview line, ttlib::cstr& str);

    inline void AddStyle(std::string_view style)
    {
        if (!m_Styles.empty())
            m_Styles.append("|");
        m_Styles.append(style);
    }

    inline void AddWinStyle(std::string_view style)
    {
        if (!m_WinStyles.empty())
            m_WinStyles.append("|");
        m_WinStyles.append(style);
    }
};
