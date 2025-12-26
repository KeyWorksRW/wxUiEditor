/////////////////////////////////////////////////////////////////////////////
// Purpose:   Process Windows Resource control data
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements resCtrl, representing a single Windows Resource control during
// .rc import. The class stores m_node (NodeSharedPtr for wxUiEditor component), dual coordinate
// systems (m_du_rect in dialog units, m_pixel_rect in pixels), m_original_line (unparsed .rc text),
// m_non_processed_style (deferred styles like UDS_AUTOBUDDY), and m_added flag (layout tracking).
// ParseDirective() is the main entry point calling GetID/GetLabel/ParseDimensions/ParseStyles to
// extract control properties from .rc syntax. Style parsing methods (ParseStyles,
// ParseListViewStyles, ParseButtonStyles, ParseCommonStyles, AddSpecialStyles) map Windows control
// styles (WS_BORDER, BS_PUSHBUTTON, LVS_REPORT) to wxWidgets equivalents. Helper methods
// (StepOverQuote, StepOverComma, ParseImageControl, ParseIconControl) parse .rc format. Accessor
// methods (GetLeft, du_top, etc.) provide coordinate queries. The class bridges Windows Resource
// control definitions to wxUiEditor Node properties.

#pragma once

#include "node.h"                        // Node class
#include "wxue_namespace/wxue_string.h"  // wxue::string and wxue::string_view

class WinResource;

class resCtrl
{
public:
    resCtrl();

    auto getNode() const { return m_node.get(); }
    auto GetNodePtr() const { return m_node; }

    void ParseDirective(WinResource* pWinResource, wxue::string_view line);

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

    bool is_Gen(GenName name) const noexcept { return getNode()->is_Gen(name); }

    // Sets value only if the property exists.
    template <typename T>
    void set_value(PropName name, T value)
    {
        getNode()->set_value(name, value);
    }

    auto ParseDimensions(wxue::string_view line, wxRect& duRect, wxRect& pixelRect) -> bool;
    auto& GetOrginalLine() { return m_original_line; }

    NodeSharedPtr SetNodePtr(NodeSharedPtr node)
    {
        m_node = node;
        return m_node;
    }

protected:
    // This will map window styles to wxWidgets styles and append them to prop_style
    void ParseStyles(wxue::string_view line);

    void ParseListViewStyles(wxue::string_view line);
    void ParseButtonStyles(wxue::string_view line);

    void AddSpecialStyles(wxue::string_view line);
    auto AppendStyle(GenEnum::PropName prop_name, wxue::string_view style) -> void;

    // Set prop_ to common values (disabled, hidden, scroll, etc.)
    auto ParseCommonStyles(wxue::string_view line) -> void;

    // This will set prop_id, and return a sview to the position past the id
    auto GetID(wxue::string_view line) -> wxue::string_view;

    // This will set prop_label, and return a sview to the position past the id
    auto GetLabel(wxue::string_view line) -> wxue::string_view;

    // Returns a view past the closing quote, or an empty view if there was no closing quote
    auto StepOverQuote(wxue::string_view line, wxue::string& str) -> wxue::string_view;

    // Retrieves any string between commas, returns view past the closing comma
    auto StepOverComma(wxue::string_view line, wxue::string& str) -> wxue::string_view;

    // Similar to ParseIconControl only in this case line is pointing to the id, and the Node
    // has already been created.
    //
    // Works with either SS_BITMAP or SS_ICON.
    void ParseImageControl(wxue::string_view line);

    // Icon controls require too much special processing to be inside the ParseDirective()
    // function.
    void ParseIconControl(wxue::string_view line);

private:
    NodeSharedPtr m_node;
    WinResource* m_pWinResource;

    // Some styles like UDS_AUTOBUDDY have to be post-processed during actual layout.
    wxue::string m_non_processed_style;

    wxue::string m_original_line;

    // Caution -- wxRect is *NOT* the same as a Windows RECT structure. wxRect stores width and
    // height, RECT stores right and bottom positions.

    // These are in dialog coordinates
    wxRect m_du_rect { 0, 0, 0, 0 };

    // These are in pixel coordinates
    wxRect m_pixel_rect { 0, 0, 0, 0 };

    bool m_add_min_width_property;
    bool m_add_wrap_property;
    bool m_added { false };
};
