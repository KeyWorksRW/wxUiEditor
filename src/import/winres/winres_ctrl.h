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

    auto GetPostProcessStyle() { return m_non_processed_style; }

protected:
    // This will map window styles to wxWidgets styles and append them to prop_style
    void ParseStyles(ttlib::cview line);

    void ParseListViewStyles(ttlib::cview line);
    void ParseButtonStyles(ttlib::cview line);

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

    // Similar to ParseIconControl only in this case line is pointing to the id, and the Node
    // has already been created.
    //
    // Works with either SS_BITMAP or SS_ICON.
    void ParseImageControl(ttlib::cview line);

    // Icon controls require too much special processing to be inside the ParseDirective()
    // function.
    void ParseIconControl(ttlib::cview line);

private:
    NodeSharedPtr m_node;
    WinResource* m_pWinResource;

    // Some styles like UDS_AUTOBUDDY have to be post-processed during actual layout.
    ttlib::cstr m_non_processed_style;

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
