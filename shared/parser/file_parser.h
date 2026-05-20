/////////////////////////////////////////////////////////////////////////////
// Purpose:   Parse source files, extract declarations and comments
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "doc_types.h"

#include <filesystem>

namespace docparser
{

    // rel_path  Relative path from input root (used as filename identifier)
    FileContent ParseFile(const std::filesystem::path& path, const std::filesystem::path& rel_path);

}  // namespace docparser
