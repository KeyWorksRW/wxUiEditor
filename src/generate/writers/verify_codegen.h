//////////////////////////////////////////////////////////////////////////
// Purpose:   Verify that code generation did not change
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "wx/cmdline.h"

namespace verify_codegen
{
    enum VerifyResult : int  // NOLINT (uses a larger base type than necessary) // cppcheck-suppress
                             // [large-enum-base]
    {
        VERIFY_SUCCESS = 0,
        VERIFY_FAILURE = 1,
        VERIFY_FILE_NOT_FOUND = 2,
        VERIFY_INVALID = 3,
    };
}  // namespace verify_codegen

[[nodiscard]] auto VerifyCodeGen(wxCmdLineParser& parser, bool& is_project_loaded)
    -> verify_codegen::VerifyResult;
