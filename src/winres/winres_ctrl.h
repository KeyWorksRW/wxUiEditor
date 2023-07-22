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

    auto getNode() const { return m_node.get(); }
    auto GetNodePtr() const { return m_node; }

    void ParseDirective(WinResource* pWinResource, tt_string_view line);

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
    // right position in dialog units
    auto du_right() const { return m_du_rect.GetLeft() + m_du_rect.GetWidth(); }
    // top position in dialog units
    auto du_top() const { return m_du_rect.GetTop(); }
    // bottom position in dialog units
    auto du_bottom() const { return m_du_rect.GetTop() + m_du_rect.GetHeight(); }
    // width in dialog units
    auto du_width() const { return m_du_rect.GetWidth(); }
    // height in dialog units
    auto du_height() const { return m_du_rect.GetHeight(); }

    auto GetPostProcessStyle() const { return m_non_processed_style; }

    bool isAdded() const { return m_added; }
    void setAdded() { m_added = true; }

    bool isGen(GenName name) const noexcept { return getNode()->isGen(name); }

    // Sets value only if the property exists.
    template <typename T>
    void set_value(PropName name, T value)
    {
        getNode()->set_value(name, value);
    }

    bool ParseDimensions(tt_string_view line, wxRect& duRect, wxRect& pixelRect);
#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    auto& GetOrginalLine() { return m_original_line; }
#endif

    NodeSharedPtr SetNodePtr(NodeSharedPtr node)
    {
        m_node = node;
        return m_node;
    }

protected:
    // This will map window styles to wxWidgets styles and append them to prop_style
    void ParseStyles(tt_string_view line);

    void ParseListViewStyles(tt_string_view line);
    void ParseButtonStyles(tt_string_view line);

    void AddSpecialStyles(tt_string_view line);
    void AppendStyle(GenEnum::PropName prop_name, tt_string_view style);

    // Set prop_ to common values (disabled, hidden, scroll, etc.)
    void ParseCommonStyles(tt_string_view line);

    // This will set prop_id, and return a sview to the position past the id
    tt_string_view GetID(tt_string_view line);

    // This will set prop_label, and return a sview to the position past the id
    tt_string_view GetLabel(tt_string_view line);

    // Returns a view past the closing quote, or an empty view if there was no closing quote
    tt_string_view StepOverQuote(tt_string_view line, tt_string& str);

    // Retrieves any string between commas, returns view past the closing comma
    tt_string_view StepOverComma(tt_string_view line, tt_string& str);

    // Similar to ParseIconControl only in this case line is pointing to the id, and the Node
    // has already been created.
    //
    // Works with either SS_BITMAP or SS_ICON.
    void ParseImageControl(tt_string_view line);

    // Icon controls require too much special processing to be inside the ParseDirective()
    // function.
    void ParseIconControl(tt_string_view line);

private:
    NodeSharedPtr m_node;
    WinResource* m_pWinResource;

    // Some styles like UDS_AUTOBUDDY have to be post-processed during actual layout.
    tt_string m_non_processed_style;

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    tt_string m_original_line;
#endif

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
