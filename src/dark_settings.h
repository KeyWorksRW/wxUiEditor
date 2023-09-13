/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dark Settings class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#if !wxCHECK_VERSION(3, 3, 0)
    #error "You must have wxWidgets 3.3.0 or later to include dark_settings.h"
#endif

#include <wx/msw/darkmode.h>

class DarkSettings : public wxDarkModeSettings
{
public:
    wxColour GetColour(wxSystemColour index);
};

extern DarkSettings* DarkModeSettings;

void wxColourToHSL(const wxColour& colour, double& hue, double& saturation, double& luminance);
wxColour HSLToWxColour(double hue, double saturation, double luminance);
wxColour wxColourToDarkForeground(const wxColour& colour);
wxColour wxColourToDarkBackground(const wxColour& colour);
