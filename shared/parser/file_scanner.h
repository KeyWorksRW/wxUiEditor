/////////////////////////////////////////////////////////////////////////////
// Purpose:   Scan directories for header files to process
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <filesystem>
#include <vector>

namespace docparser
{

    // Recursively scan a directory for .h files.
    // Returns paths relative to the input root.
    [[nodiscard]] std::vector<std::filesystem::path>
        ScanDirectory(const std::filesystem::path& root);

}  // namespace docparser
