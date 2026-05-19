/////////////////////////////////////////////////////////////////////////////
// Purpose:   Console reporter for the documentation parser pipeline.
//            Replaces wxGetApp() message accumulation with std::print.
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string_view>

namespace parser
{

    // Simple console reporter. Messages go to stdout; errors to stderr.
    // Designed so a wxWidgets variant can be slotted in later by providing
    // the same function names with wxGetApp() delegation.
    void AddErrorMessage(std::string_view msg);
    void AddResultMessage(std::string_view msg);

}  // namespace parser
