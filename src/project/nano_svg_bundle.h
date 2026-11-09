/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxBitmapBundle implementation that rasterizes SVG with NanoSVG
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>

#include <wx/bmpbndl.h>

// Creates a bundle that rasterizes the SVG data with NanoSVG instead of LunaSVG. This is only used
// for display in wxUiEditor -- it lets the Mockup preview what a project targeting wxWidgets 3.2
// will actually draw, since wxWidgets 3.2 rasterizes SVG with its embedded NanoSVG.
// data does not have to be NUL terminated; size bytes are copied and parsed.
wxBitmapBundle CreateNanoSVGBundle(const char* data, size_t size, const wxSize& default_size);
