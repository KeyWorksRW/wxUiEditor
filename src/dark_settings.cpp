/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dark Settings class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#if !wxCHECK_VERSION(3, 3, 0)
    #error "You must have wxWidgets 3.3.0 or later to include dark_settings.h"
#endif

#include <algorithm>

#include "dark_settings.h"
#include "preferences.h"  // Set/Get wxUiEditor preferences

wxColour DarkSettings::GetColour(wxSystemColour index)
{
    switch (index)
    {
        case wxSYS_COLOUR_WINDOW:
        case wxSYS_COLOUR_LISTBOX:
        case wxSYS_COLOUR_BTNFACE:
            if (UserPrefs.is_HighContrast())
                return wxColour(0, 0, 0);
            else
                return wxColour(0x202020);

#if 0
            case wxSYS_COLOUR_ACTIVECAPTION:
            case wxSYS_COLOUR_APPWORKSPACE:
            case wxSYS_COLOUR_INFOBK:
                // Default colour used here is 0x202020.
                return wxColour(0x202020);
#endif

        default:
            return wxDarkModeSettings::GetColour(index);
    }
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
