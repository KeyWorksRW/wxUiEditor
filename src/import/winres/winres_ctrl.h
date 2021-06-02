/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process Windows Resource control data
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "node.h"  // Node class

class WinResource;

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

    auto GetNode() const { return m_node.get(); }
    auto GetNodePtr() const { return m_node; }

    void ParseDirective(WinResource* pWinResource, ttlib::cview line);

    auto GetLeft() const { return m_left; }
    auto GetTop() const { return m_top; }
    auto GetRight() const { return (m_left + m_width); }
    auto GetBottom() const { return (m_top + m_height); }
    auto GetWidth() const { return m_width; }
    auto GetHeight() const { return m_height; }

protected:
    void AppendStyle(GenEnum::PropName prop_name, ttlib::cview style);

    // Set prop_ to common values (disabled, hidden, scroll, etc.)
    void ParseCommonStyles(ttlib::cview line);

    // Sets m_left, m_top, m_width and m_height in pixel dimensions
    void GetDimensions(ttlib::cview line);

    // This will set prop_id, and return a cview to the position past the id
    ttlib::cview GetID(ttlib::cview line);

    // This will set prop_label, and return a cview to the position past the id
    ttlib::cview GetLabel(ttlib::cview line);

    // Returns a view past the closing quote, or an empty view if there was no closing quote
    ttlib::cview StepOverQuote(ttlib::cview line, ttlib::cstr& str);

    // Retrieves any string between commas, returns view past the closing comma
    ttlib::cview StepOverComma(ttlib::cview line, ttlib::cstr& str);

    // Icon controls require too much special processing to be inside the ParseDirective()
    // function.
    void ParseIconControl(ttlib::cview line);

private:
    NodeSharedPtr m_node;
    WinResource* m_pWinResource;

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
};
