/////////////////////////////////////////////////////////////////////////////
// Purpose:   Console reporter for the documentation parser pipeline.
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "parser_reporter.h"

#include <print>

namespace parser
{

    void AddErrorMessage(std::string_view msg)
    {
        std::print(stderr, "{}\n", msg);
    }

    void AddResultMessage(std::string_view msg)
    {
        std::print("{}\n", msg);
    }

}  // namespace parser
