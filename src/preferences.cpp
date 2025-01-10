/////////////////////////////////////////////////////////////////////////////
// Purpose:   Set/Get wxUiEditor preferences
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// See also: ui/preferences_dlg.cpp and ui/preferences_dlg.h

#include <wx/config.h>  // wxConfig base header

#include "preferences.h"

Prefs& UserPrefs = Prefs::getInstance();

void Prefs::ReadConfig()
{
    auto* config = wxConfig::Get();
    config->SetPath("/preferences");

    m_flags = config->ReadLong("flags", PREFS_MSG_WINDOW | PREFS_MSG_INFO | PREFS_MSG_EVENT | PREFS_MSG_WARNING);
    m_project_flags = config->ReadLong("project_flags", PREFS_PJT_MEMBER_PREFIX);
    m_preview_type = static_cast<PREVIEW_TYPE>(config->ReadLong("preview_type", PREVIEW_TYPE_XRC));

    m_sizers_all_borders = config->ReadBool("all_borders", true);
    m_sizers_always_expand = config->ReadBool("always_expand", true);
    m_var_prefix = config->ReadBool("var_prefix", true);
    m_fullpath_title = config->ReadBool("fullpath_title", false);
    m_add_comments = config->ReadBool("add_comments", false);
    m_svg_images = config->ReadBool("svg_images", false);

    m_enable_wakatime = config->ReadBool("enable_wakatime", true);
    m_dark_mode = config->ReadBool("dark_mode", false);
    m_high_constrast = config->ReadBool("high_contrast", false);
    m_is_load_last_project = config->ReadBool("load_last_project", false);
    m_is_right_propgrid = config->ReadBool("right_propgrid", false);
    m_is_cpp_snake_case = config->ReadBool("cpp_snake_case", true);

    m_cpp_widgets_version = config->Read("cpp_widgets_version", "3.2");
    m_python_version = config->Read("python_version", "4.2");
    m_ruby_version = config->Read("ruby_version", "1.2");
    if (m_ruby_version == "0.9")  // we no longer support the wxRuby3 beta
    {
        m_ruby_version = "1.2";
    }

    m_colour_cpp = config->Read("cpp_colour", "#FF00FF");
    m_colour_cpp_comment = config->Read("cpp_comment_colour", "#008000");
    m_colour_cpp_keyword = config->Read("cpp_keyword_colour", "#0000FF");
    m_colour_cpp_number = config->Read("cpp_number_colour", "#FF0000");
    m_colour_cpp_string = config->Read("cpp_string_colour", "#008000");

    m_colour_python = config->Read("python_colour", "#FF00FF");
    m_colour_python_comment = config->Read("python_comment_colour", "#008000");
    m_colour_python_keyword = config->Read("python_keyword_colour", "#0000FF");
    m_colour_python_number = config->Read("python_number_colour", "#FF0000");
    m_colour_python_string = config->Read("python_string_colour", "#008000");

    m_colour_ruby = config->Read("ruby_colour", "#FF00FF");
    m_colour_ruby_comment = config->Read("ruby_comment_colour", "#008000");
    m_colour_ruby_number = config->Read("ruby_number_colour", "#FF0000");
    m_colour_ruby_string = config->Read("ruby_string_colour", "#008000");

    m_colour_fortran = config->Read("fortran_colour", "#FF00FF");
    m_colour_fortran_comment = config->Read("fortran_comment_colour", "#008000");
    m_colour_fortran_keyword = config->Read("fortran_keyword_colour", "#0000FF");
    m_colour_fortran_number = config->Read("fortran_number_colour", "#FF0000");
    m_colour_fortran_string = config->Read("fortran_string_colour", "#008000");

    m_colour_haskell = config->Read("haskell_colour", "#FF00FF");
    m_colour_haskell_comment = config->Read("haskell_comment_colour", "#008000");
    m_colour_haskell_keyword = config->Read("haskell_keyword_colour", "#0000FF");
    m_colour_haskell_number = config->Read("haskell_number_colour", "#FF0000");
    m_colour_haskell_string = config->Read("haskell_string_colour", "#008000");

    m_colour_lua = config->Read("lua_colour", "#FF00FF");
    m_colour_lua_comment = config->Read("lua_comment_colour", "#008000");
    m_colour_lua_keyword = config->Read("lua_keyword_colour", "#0000FF");
    m_colour_lua_number = config->Read("lua_number_colour", "#FF0000");
    m_colour_lua_string = config->Read("lua_string_colour", "#008000");

    m_colour_perl = config->Read("perl_colour", "#FF00FF");
    m_colour_perl_comment = config->Read("perl_comment_colour", "#008000");
    m_colour_perl_keyword = config->Read("perl_keyword_colour", "#0000FF");
    m_colour_perl_number = config->Read("perl_number_colour", "#FF0000");
    m_colour_perl_string = config->Read("perl_string_colour", "#008000");

    m_colour_rust = config->Read("rust_colour", "#FF00FF");
    m_colour_rust_comment = config->Read("rust_comment_colour", "#008000");
    m_colour_rust_keyword = config->Read("rust_keyword_colour", "#0000FF");
    m_colour_rust_number = config->Read("rust_number_colour", "#FF0000");
    m_colour_rust_string = config->Read("rust_string_colour", "#008000");

    m_colour_xrc_attribute = config->Read("xrc_colour", "#FF00FF");
    m_colour_xrc_dblstring = config->Read("xrc_dblstring_colour", "#008000");
    m_colour_xrc_tag = config->Read("xrc_tag_colour", "#0000FF");

    m_cpp_line_length = config->Read("cpp_line_length", 110);
    m_python_line_length = config->Read("python_line_length", 90);
    m_ruby_line_length = config->Read("ruby_line_length", 80);
    m_fortran_line_length = config->Read("fortran_line_length", 100);
    m_haskell_line_length = config->Read("haskell_line_length", 80);
    m_lua_line_length = config->Read("lua_line_length", 100);
    m_perl_line_length = config->Read("perl_line_length", 80);
    m_rust_line_length = config->Read("rust_line_length", 80);

    m_icon_size = config->Read("icon_size", 20);

    m_code_display_font = config->Read("code_display_font", "");

    config->SetPath("/");
}

void Prefs::WriteConfig()
{
    auto* config = wxConfig::Get();
    config->SetPath("/preferences");

    config->Write("all_borders", m_sizers_all_borders);
    config->Write("always_expand", m_sizers_always_expand);
    config->Write("var_prefix", m_var_prefix);
    config->Write("fullpath_title", m_fullpath_title);
    config->Write("add_comments", m_add_comments);
    config->Write("svg_images", m_svg_images);

    config->Write("enable_wakatime", m_enable_wakatime);
    config->Write("high_contrast", m_high_constrast);
    config->Write("load_last_project", m_is_load_last_project);
    config->Write("right_propgrid", m_is_right_propgrid);

    config->Write("cpp_snake_case", m_is_cpp_snake_case);

    config->Write("cpp_widgets_version", m_cpp_widgets_version.make_wxString());
    config->Write("python_version", m_python_version.make_wxString());
    config->Write("ruby_version", m_ruby_version.make_wxString());

    config->Write("cpp_colour", m_colour_cpp.GetAsString(wxC2S_HTML_SYNTAX));
    config->Write("cpp_comment_colour", m_colour_cpp_comment.GetAsString(wxC2S_HTML_SYNTAX));
    config->Write("cpp_keyword_colour", m_colour_cpp_keyword.GetAsString(wxC2S_HTML_SYNTAX));
    config->Write("cpp_number_colour", m_colour_cpp_number.GetAsString(wxC2S_HTML_SYNTAX));
    config->Write("cpp_string_colour", m_colour_cpp_string.GetAsString(wxC2S_HTML_SYNTAX));

    config->Write("python_colour", m_colour_python.GetAsString(wxC2S_HTML_SYNTAX));
    config->Write("python_comment_colour", m_colour_python_comment.GetAsString(wxC2S_HTML_SYNTAX));
    config->Write("python_keyword_colour", m_colour_python_keyword.GetAsString(wxC2S_HTML_SYNTAX));
    config->Write("python_number_colour", m_colour_python_number.GetAsString(wxC2S_HTML_SYNTAX));
    config->Write("python_string_colour", m_colour_python_string.GetAsString(wxC2S_HTML_SYNTAX));

    config->Write("ruby_colour", m_colour_ruby.GetAsString(wxC2S_HTML_SYNTAX));
    config->Write("ruby_comment_colour", m_colour_ruby_comment.GetAsString(wxC2S_HTML_SYNTAX));
    config->Write("ruby_number_colour", m_colour_ruby_number.GetAsString(wxC2S_HTML_SYNTAX));
    config->Write("ruby_string_colour", m_colour_ruby_string.GetAsString(wxC2S_HTML_SYNTAX));

    config->Write("xrc_colour", m_colour_xrc_attribute.GetAsString(wxC2S_HTML_SYNTAX));
    config->Write("xrc_dblstring_colour", m_colour_xrc_dblstring.GetAsString(wxC2S_HTML_SYNTAX));
    config->Write("xrc_tag_colour", m_colour_xrc_tag.GetAsString(wxC2S_HTML_SYNTAX));

    config->Write("cpp_line_length", m_cpp_line_length);
    config->Write("python_line_length", m_python_line_length);
    config->Write("ruby_line_length", m_ruby_line_length);

    config->Write("icon_size", m_icon_size);

    if (m_dark_mode_pending & PENDING_DARK_MODE_ENABLE)
    {
        config->Write("dark_mode", m_dark_mode_pending & PENDING_DARK_MODE_ON ? true : false);
    }

    config->Write("code_display_font", m_code_display_font.make_wxString());

    config->SetPath("/");
}

wxColour Prefs::GetColour(wxSystemColour index)
{
    if (!is_DarkMode())
    {
        switch (index)
        {
            case wxSYS_COLOUR_WINDOW:
                if (is_HighContrast())
                    return *wxWHITE;
                break;

            case wxSYS_COLOUR_WINDOWTEXT:
                if (is_HighContrast())
                    return *wxBLACK;
                break;

            default:
                break;
        }
        return wxSystemSettings::GetColour(index);
    }

    // These generally match wxWidgets/src/msw/darkmode.cpp unless is_HighContrast() is set

    switch (index)
    {
        case wxSYS_COLOUR_BTNSHADOW:
            return *wxBLACK;

        case wxSYS_COLOUR_ACTIVECAPTION:
        case wxSYS_COLOUR_APPWORKSPACE:
        case wxSYS_COLOUR_INFOBK:
        case wxSYS_COLOUR_LISTBOX:
        case wxSYS_COLOUR_BTNFACE:
            if (is_HighContrast())
                return *wxBLACK;
            else
                return wxColour(0x202020);

        case wxSYS_COLOUR_BTNTEXT:
        case wxSYS_COLOUR_CAPTIONTEXT:
        case wxSYS_COLOUR_HIGHLIGHTTEXT:
        case wxSYS_COLOUR_INFOTEXT:
        case wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT:
        case wxSYS_COLOUR_LISTBOXTEXT:
        case wxSYS_COLOUR_MENUTEXT:
            if (is_HighContrast())
                return *wxWHITE;
            else
                return wxColour(0xe0e0e0);

        case wxSYS_COLOUR_WINDOW:
            if (is_HighContrast())
                return *wxBLACK;
            else
                return wxColour(0x202020);

        case wxSYS_COLOUR_WINDOWTEXT:
            if (is_HighContrast())
                return *wxWHITE;
            else
                return wxColour(0xe0e0e0);

        case wxSYS_COLOUR_HOTLIGHT:
            return wxColour(0x474747);

        case wxSYS_COLOUR_SCROLLBAR:
            return wxColour(0x4d4d4d);

        case wxSYS_COLOUR_INACTIVECAPTION:
        case wxSYS_COLOUR_MENU:
            return wxColour(0x2b2b2b);

        case wxSYS_COLOUR_MENUBAR:
            return wxColour(0x626262);

        case wxSYS_COLOUR_MENUHILIGHT:
            return wxColour(0x353535);

        case wxSYS_COLOUR_BTNHIGHLIGHT:
        case wxSYS_COLOUR_HIGHLIGHT:
            return wxColour(0x777777);

        case wxSYS_COLOUR_INACTIVECAPTIONTEXT:
            return wxColour(0xaaaaaa);

        case wxSYS_COLOUR_3DDKSHADOW:
        case wxSYS_COLOUR_3DLIGHT:
        case wxSYS_COLOUR_ACTIVEBORDER:
        case wxSYS_COLOUR_DESKTOP:
        case wxSYS_COLOUR_GRADIENTACTIVECAPTION:
        case wxSYS_COLOUR_GRADIENTINACTIVECAPTION:
        case wxSYS_COLOUR_GRAYTEXT:
        case wxSYS_COLOUR_INACTIVEBORDER:
        case wxSYS_COLOUR_WINDOWFRAME:
            return wxColour();

        case wxSYS_COLOUR_MAX:
            break;
    }

    FAIL_MSG("unreachable");
    return wxColour();
}

void wxColourToHSL(const wxColour& colour, double& hue, double& saturation, double& luminance)
{
    double r = colour.Red() / 255.0;
    double g = colour.Green() / 255.0;
    double b = colour.Blue() / 255.0;
    double cmax = std::max({ r, g, b });
    double cmin = std::min({ r, g, b });
    double delta = cmax - cmin;

    // Calculate hue
    if (delta == 0)
    {
        hue = 0;
    }
    else if (cmax == r)
    {
        hue = fmod((g - b) / delta, 6);
    }
    else if (cmax == g)
    {
        hue = (b - r) / delta + 2;
    }
    else
    {
        hue = (r - g) / delta + 4;
    }
    hue *= 60;
    if (hue < 0)
    {
        hue += 360;
    }

    // Calculate luminance
    luminance = (cmax + cmin) / 2;

    // Calculate saturation
    if (delta == 0)
    {
        saturation = 0;
    }
    else
    {
        saturation = delta / (1 - fabs(2 * luminance - 1));
    }
}

wxColour HSLToWxColour(double hue, double saturation, double luminance)
{
    double c = (1 - fabs(2 * luminance - 1)) * saturation;
    double x = c * (1 - fabs(fmod(hue / 60, 2) - 1));
    double m = luminance - c / 2;
    double r, g, b;

    if (hue < 60)
    {
        r = c;
        g = x;
        b = 0;
    }
    else if (hue < 120)
    {
        r = x;
        g = c;
        b = 0;
    }
    else if (hue < 180)
    {
        r = 0;
        g = c;
        b = x;
    }
    else if (hue < 240)
    {
        r = 0;
        g = x;
        b = c;
    }
    else if (hue < 300)
    {
        r = x;
        g = 0;
        b = c;
    }
    else
    {
        r = c;
        g = 0;
        b = x;
    }

    r += m;
    g += m;
    b += m;

    r = std::clamp(r, 0.0, 1.0);
    g = std::clamp(g, 0.0, 1.0);
    b = std::clamp(b, 0.0, 1.0);

    return wxColour(static_cast<unsigned char>(r * 255), static_cast<unsigned char>(g * 255),
                    static_cast<unsigned char>(b * 255));
}

wxColour wxColourToDarkForeground(const wxColour& colour)
{
    double hue, saturation, luminance;
    wxColourToHSL(colour, hue, saturation, luminance);

    if (UserPrefs.is_HighContrast())
    {
        if (luminance < 0.85)
        {
            luminance = 0.85;
        }
    }
    else
    {
        luminance = 0.75;
    }

    return HSLToWxColour(hue, saturation, luminance);
}

wxColour wxColourToDarkBackground(const wxColour& colour)
{
    double hue, saturation, luminance;
    wxColourToHSL(colour, hue, saturation, luminance);

    if (UserPrefs.is_HighContrast())
    {
        if (luminance > 0.05)
        {
            luminance = 0.05;
        }
    }
    else
    {
        luminance = 0.20;
    }

    return HSLToWxColour(hue, saturation, luminance);
}
