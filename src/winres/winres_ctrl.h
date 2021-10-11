/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process Windows Resource control data
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "node.h"  // Node class

class WinResource;

class resCtrl
{
public:
    resCtrl();

    auto GetNode() const { return m_node.get(); }
    auto GetNodePtr() const { return m_node; }

    void ParseDirective(WinResource* pWinResource, ttlib::cview line);

    // left position in pixels
    auto GetLeft() const { return m_pixel_rect.GetLeft(); }
    // top position in pixels
    auto GetTop() const { return m_pixel_rect.GetTop(); }
    // right position in pixels
    auto GetRight() const { return (m_pixel_rect.GetLeft() + m_pixel_rect.GetWidth()); }
    // bottom position in pixels
    auto GetBottom() const { return (m_pixel_rect.GetTop() + m_pixel_rect.GetHeight()); }
    // width in pixels
    auto GetWidth() const { return m_pixel_rect.GetWidth(); }
    // height in pixels
    auto GetHeight() const { return m_pixel_rect.GetHeight(); }

    auto& GetDialogRect() { return m_du_rect; }

    // left position in dialog units
    auto du_left() const { return m_du_rect.GetLeft(); }
    // top position in dialog units
    auto du_top() const { return m_du_rect.GetTop(); }
    // width in dialog units
    auto du_width() const { return m_du_rect.GetWidth(); }
    // height in dialog units
    auto du_height() const { return m_du_rect.GetHeight(); }

    auto GetPostProcessStyle() const { return m_non_processed_style; }

    bool isAdded() const { return m_added; }
    void setAdded() { m_added = true; }

    bool isGen(GenName name) const noexcept { return GetNode()->isGen(name); }

    // Sets value only if the property exists.
    template <typename T>
    void prop_set_value(PropName name, T value)
    {
        GetNode()->prop_set_value(name, value);
    }

    static bool ParseDimensions(ttlib::cview line, wxRect& duRect, wxRect& pixelRect);

protected:
    // This will map window styles to wxWidgets styles and append them to prop_style
    void ParseStyles(ttlib::cview line);

    void ParseListViewStyles(ttlib::cview line);
    void ParseButtonStyles(ttlib::cview line);

    void AddSpecialStyles(ttlib::cview line);
    void AppendStyle(GenEnum::PropName prop_name, ttlib::cview style);

    // Set prop_ to common values (disabled, hidden, scroll, etc.)
    void ParseCommonStyles(ttlib::cview line);

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

#if defined(_DEBUG)
    ttlib::cstr m_original_line;
#endif  // _DEBUG

    // Caution -- wxRect is *NOT* the same as a Windows RECT structure. wxRect stores width and height, RECT stores right and
    // bottom positions.

    // These are in dialog coordinates
    wxRect m_du_rect { 0, 0, 0, 0 };

    // These are in pixel coordinates
    wxRect m_pixel_rect { 0, 0, 0, 0 };

    bool m_add_min_width_property;
    bool m_add_wrap_property;
    bool m_added { false };
};
