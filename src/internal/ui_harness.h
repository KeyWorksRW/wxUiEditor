/////////////////////////////////////////////////////////////////////////////
// Purpose:   Debug-only UI command harness for driving the editor from a script
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/string.h>

namespace ui_harness
{
    // Starts reading commands appended to script_path and writing results to log_path.
    // Called from App::OnRun() after the main frame has been shown. Only compiled into
    // INTERNAL_BLD_TESTING builds.
    void Start(const wxString& script_path, const wxString& log_path);
}  // namespace ui_harness
