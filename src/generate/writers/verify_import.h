//////////////////////////////////////////////////////////////////////////
// Purpose:   Verify that importing a project file did not change
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "wx/cmdline.h"

#include "verify_codegen.h"  // verify_codegen::VerifyResult

// Imports the project file named by the first command-line parameter and either writes it to a
// golden .wxui file (--save_import) or compares it against that golden (--verify_import). No code
// generation is performed, so no language compiler and no display are required.
//
// Returns the verify_codegen::VerifyResult exit code: 0 success, 1 differences, 2 source or
// golden not found, 3 invalid arguments.
[[nodiscard]] verify_codegen::VerifyResult VerifyImport(wxCmdLineParser& parser,
                                                        bool& is_project_loaded);
