/////////////////////////////////////////////////////////////////////////////
// Purpose:   Scan directories for header files to process
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "file_scanner.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <system_error>

// CR: [10-27-2026]

namespace fs = std::filesystem;

namespace docparser
{
    std::vector<fs::path> ScanDirectory(const fs::path& root)
    {
        std::vector<fs::path> result;

        // An empty result means the root is missing, unreadable, or holds no header files.
        std::error_code root_ec;
        if (!fs::is_directory(root, root_ec) || root_ec)
        {
            return result;
        }

        // Directory symlinks and junctions are not followed (recursive_directory_iterator default).
        fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied,
                                            root_ec);
        const fs::recursive_directory_iterator end;

        while (!root_ec && it != end)
        {
            const fs::directory_entry& entry = *it;
            const fs::path& entry_path = entry.path();

            std::error_code entry_ec;
            if (entry.is_regular_file(entry_ec) && !entry_ec)
            {
                std::string extension = entry_path.extension().string();
                for (char& character: extension)
                {
                    character =
                        static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
                }

                if (extension == ".h" && !entry_path.stem().empty())
                {
                    const fs::path relative_path = entry_path.lexically_relative(root);
                    if (!relative_path.empty() && !relative_path.is_absolute() &&
                        relative_path.begin()->string() != "..")
                    {
                        result.push_back(relative_path);
                    }
                }
            }

            it.increment(root_ec);
        }

        // Sort on the generic (forward-slash) form so the order is platform-independent.
        std::ranges::sort(result,
                          [](const fs::path& left_path, const fs::path& right_path)
                          {
                              return left_path.generic_string() < right_path.generic_string();
                          });
        result.erase(std::unique(result.begin(), result.end(),
                                 [](const fs::path& left_path, const fs::path& right_path)
                                 {
                                     return left_path.generic_string() ==
                                            right_path.generic_string();
                                 }),
                     result.end());
        return result;
    }

}  // namespace docparser
