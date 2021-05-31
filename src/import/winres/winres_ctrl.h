/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process Windows Resource control data
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "node.h"  // Node class

// Same as the Windows RECT structure -- this version declared to provide a cross-platform
// version
struct RC_RECT
{
    int left;
    int top;
    int right;
    int bottom;
};

class rcCtrl
{
public:
    rcCtrl();

    auto GetNode() const { return m_node; }

    void ParseControlCtrl(ttlib::cview line);
    void ParseEditCtrl(ttlib::cview line);
    void ParseGroupBox(ttlib::cview line);
    void ParsePushButton(ttlib::cview line);
    void ParseStaticCtrl(ttlib::cview line);

    auto GetLeft() const { return m_left; }
    auto GetTop() const { return m_top; }
    auto GetRight() const { return (m_left + m_width); }
    auto GetBottom() const { return (m_top + m_height); }
    auto GetWidth() const { return m_width; }
    auto GetHeight() const { return m_height; }

protected:
    void AppendStyle(GenEnum::PropName prop_name, ttlib::cview style);

    void ParseCommonStyles(ttlib::cview line);
    void GetDimensions(ttlib::cview line);

    ttlib::cview StepOverQuote(ttlib::cview line, ttlib::cstr& str);
    ttlib::cview StepOverComma(ttlib::cview line, ttlib::cstr& str);

private:
    NodeSharedPtr m_node;

    // left position in pixel coordinate
    int m_left;
    // top position in pixel coordinate
    int m_top;
    // width in pixels
    int m_width;
    // heihgt in pixels
    int m_height;

    // These are in dialog coordinates
    RC_RECT m_rc { 0, 0, 0, 0 };

    int m_minHeight;
    int m_minWidth;
};
