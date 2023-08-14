/////////////////////////////////////////////////////////////////////////////
// Purpose:   Modified version of wxColourPickerCtrl
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

/*

    Currently (08-12-2023) wxColourPickerCtrl only supports about half
    of the W3C CSS colors (see https://www.w3.org/TR/css-color-3/ ).
    This class has a map of all the CSS colors which are mapped to a
    CSS Hex string. This allows the user to specify any of the CSS colors,
    and convert that to a hex (#RRGGBB) string that wxColour will accept.

    Note that the text control has auto-complete enabled with all the CSS
    color names.

*/

#pragma once

#include <map>

#include <wx/clrpicker.h>   // wxColourBase definition
#include <wx/colour.h>      // wxColourBase definition
#include <wx/pickerbase.h>  // wxPickerBase class

#if !defined(wxCLRP_SHOW_LABEL)
    #define wxCLRP_SHOW_LABEL 0x0008
    #define wxCLRP_SHOW_ALPHA 0x0010
#endif

class kwColourPickerCtrl : public wxPickerBase
{
public:
    // This can convert any CSS color name, #rrggbb, rgb(r,g,b), or a wxSYS_COLOUR_* to a
    // wxColour.
    static wxColour GetColorFromString(const wxString& color_string);

    // Given a named CSS color, return the hex string (#RRGGBB).
    static std::string GetHexColorFromString(const wxString& color_string);

public:
    kwColourPickerCtrl() {}
    kwColourPickerCtrl(wxWindow* parent, wxWindowID id, const wxColour& col = *wxBLACK,
                       const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
                       long style = wxCLRP_DEFAULT_STYLE, const wxValidator& validator = wxDefaultValidator,
                       const wxString& name = wxASCII_STR(wxColourPickerCtrlNameStr))
    {
        Create(parent, id, col, pos, size, style, validator, name);
    }

    bool Create(wxWindow* parent, wxWindowID id, const wxColour& col = *wxBLACK, const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize, long style = wxCLRP_DEFAULT_STYLE,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxColourPickerCtrlNameStr));

    // This can be a named CSS color, hex string, RGB string, or system
    // color (such as wxSYS_COLOUR_BTNTEXT).
    void SetColour(const wxString& color_string);

    void SetColour(const wxColour& color);

    wxColour GetColour() const { return static_cast<wxColourPickerWidget*>(m_picker)->GetColour(); }

protected:
    // Event handlers
    void OnColorChange(wxColourPickerEvent& event);

    // The following two overrides are required when deriving from wxPickerBase

    void UpdatePickerFromTextCtrl() override;
    void UpdateTextCtrlFromPicker() override;

    long GetPickerStyle(long style) const override { return (style & (wxCLRP_SHOW_LABEL | wxCLRP_SHOW_ALPHA)); }
};

// This is a CSS_name/hex_string map of all the CSS colors.
extern const std::map<std::string, std::string, std::less<>> kw_css_colors;
