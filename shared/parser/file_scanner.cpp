/////////////////////////////////////////////////////////////////////////////
// Purpose:   Scan directories for header files to process
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "file_scanner.h"

#include <algorithm>

namespace fs = std::filesystem;

namespace docparser
{
    std::vector<fs::path> ScanDirectory(const fs::path& root)
    {
        std::vector<fs::path> result;

        if (!fs::exists(root) || !fs::is_directory(root))
        {
            return result;
        }

        for (const fs::directory_entry& entry: fs::recursive_directory_iterator(root))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".h")
            {
                result.push_back(fs::relative(entry.path(), root));
            }
        }

        std::ranges::sort(result);
        return result;
    }

}  // namespace docparser
