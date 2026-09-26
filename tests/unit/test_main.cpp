/////////////////////////////////////////////////////////////////////////////
// Purpose:   Entry point for the wxui_tests binary
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// Deliberately no wxWidgets initialization here. wxInitialize()/wxEntryStart() connects to a
// display (X11 or Wayland on Unix), which does not exist on a headless CI runner, and nothing in
// wxui_core needs it -- wxString, wxFileName and wxDir all work without a wxApp.
//
// A custom main (rather than Catch2::Catch2WithMain) is used so that this decision is explicit
// and cannot be undone by accident.

#include <catch2/catch_session.hpp>

int main(int argc, char* argv[])
{
    return Catch::Session().run(argc, argv);
}
