/////////////////////////////////////////////////////////////////////////////
// Purpose:   Verify a parser-produced ZIP archive via the DocArchive reader
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "parser_reporter.h"
#include "utils.h"

#include <cstring>
#include <string>

#include <miniz.h>

int RunZipTest(const std::filesystem::path& zip_path)
{
    parser::AddResultMessage("zip-test: opening " + zip_path.string());

    // Open the archive with the high-level DocArchive reader.
    std::expected<std::shared_ptr<DocArchive>, std::string> archive_result =
        OpenDocArchive(zip_path.string());
    if (!archive_result)
    {
        parser::AddErrorMessage("FAIL: " + archive_result.error());
        return 1;
    }
    const std::shared_ptr<DocArchive> archive = std::move(*archive_result);

    // Enumerate entries with miniz so we know what to extract.
    mz_zip_archive raw_archive {};
    const std::string path_str = zip_path.string();
    if (!mz_zip_reader_init_file(&raw_archive, path_str.c_str(), 0))
    {
        parser::AddErrorMessage("FAIL: miniz could not open archive for enumeration.");
        return 1;
    }

    const mz_uint entry_count = mz_zip_reader_get_num_files(&raw_archive);
    if (entry_count == 0)
    {
        parser::AddErrorMessage("FAIL: archive contains zero entries.");
        mz_zip_reader_end(&raw_archive);
        return 1;
    }

    bool all_ok = true;
    mz_uint files_tested = 0;

    for (mz_uint idx = 0; idx < entry_count; ++idx)
    {
        mz_zip_archive_file_stat stat {};
        if (!mz_zip_reader_file_stat(&raw_archive, idx, &stat))
        {
            parser::AddErrorMessage("FAIL: could not stat entry " + std::to_string(idx));
            all_ok = false;
            continue;
        }

        // Skip directories.
        if (mz_zip_reader_is_file_a_directory(&raw_archive, idx))
        {
            continue;
        }

        const std::string entry_name = stat.m_filename;
        const std::expected<std::string, std::string> content_result =
            ReadMarkdown(*archive, entry_name);
        if (!content_result && stat.m_uncomp_size > 0)
        {
            parser::AddErrorMessage("FAIL: " + entry_name + "  (" + content_result.error() + ")");
            all_ok = false;
        }
        else if (content_result)
        {
            parser::AddResultMessage("  OK: " + entry_name + "  (" +
                                     std::to_string(content_result->size()) + " bytes)");
        }
        ++files_tested;
    }

    mz_zip_reader_end(&raw_archive);

    parser::AddResultMessage(std::to_string(files_tested) + " entries tested.");

    if (!all_ok)
    {
        parser::AddErrorMessage("FAIL: one or more entries could not be extracted.");
        return 1;
    }

    parser::AddResultMessage("All entries extracted successfully.");
    return 0;
}
