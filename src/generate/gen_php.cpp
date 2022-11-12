/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate PHP code files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "ttcwd_wx.h"  // cwd -- Class for storing and optionally restoring the current directory

#include "mainframe.h"

#include "gen_base.h"       // BaseCodeGenerator -- Generate Base class
#include "node.h"           // Node class
#include "project_class.h"  // Project class
#include "write_code.h"     // Write code to Scintilla or file

bool GeneratePhpFiles(wxWindow* /* parent */, bool /* NeedsGenerateCheck */, std::vector<ttlib::cstr>* /* pClassList */)
{
    return false;
}
