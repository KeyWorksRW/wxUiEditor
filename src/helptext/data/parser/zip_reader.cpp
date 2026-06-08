/////////////////////////////////////////////////////////////////////////////
// Purpose:   Read markdown content out of a documentation ZIP archive.
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "utils.h"

#include <cstring>
#include <expected>
#include <format>
#include <mutex>
#include <string>
#include <vector>

#include <miniz.h>

// Return a human-readable error string from the archive's last miniz error.
static std::string MinizError(mz_zip_archive& archive)
{
    const mz_zip_error code = mz_zip_get_last_error(&archive);
    const char* message = mz_zip_get_error_string(code);
    return message ? std::string(message) : "unknown miniz error";
}

// RAII wrapper around a miniz reader opened against a .zip file on disk.
// The archive stays memory-mapped (or seekable file-backed, depending on
// miniz internals) for the lifetime of the object; callers read individual
// entries via ReadMarkdown().
class DocArchive
{
public:
    DocArchive() = default;

    ~DocArchive()
    {
        if (is_open_)
        {
            mz_zip_reader_end(&archive_);
        }
    }

    DocArchive(const DocArchive&) = delete;
    DocArchive& operator=(const DocArchive&) = delete;
    DocArchive(DocArchive&&) = delete;
    DocArchive& operator=(DocArchive&&) = delete;

    [[nodiscard]] std::expected<void, std::string> Open(const std::filesystem::path& zip_path)
    {
        std::memset(&archive_, 0, sizeof(archive_));
        const std::string path_str = zip_path.string();
        if (!mz_zip_reader_init_file(&archive_, path_str.c_str(), 0))
        {
            return std::unexpected(
                std::format("Failed to open ZIP '{}': {}", path_str, MinizError(archive_)));
        }
        is_open_ = true;
        return {};
    }

    [[nodiscard]] std::expected<std::string, std::string>
        Extract(std::string_view archive_name) const
    {
        if (!is_open_)
        {
            return std::unexpected(std::string("Archive is not open"));
        }

        // miniz reader APIs are not documented as thread-safe against the same
        // archive handle — serialize calls on a single DocArchive so the GUI
        // can safely share the instance across threads.
        const std::scoped_lock<std::mutex> lock(mutex_);

        const std::string name(archive_name);
        const int index = mz_zip_reader_locate_file(&archive_, name.c_str(), nullptr, 0);
        if (index < 0)
        {
            return std::unexpected(std::format("Entry '{}' not found in archive", name));
        }

        mz_zip_archive_file_stat stat {};
        if (!mz_zip_reader_file_stat(&archive_, static_cast<mz_uint>(index), &stat))
        {
            return std::unexpected(
                std::format("Failed to stat '{}': {}", name, MinizError(archive_)));
        }

        std::string output;
        output.resize(static_cast<std::size_t>(stat.m_uncomp_size));
        if (stat.m_uncomp_size == 0)
        {
            return output;
        }

        if (!mz_zip_reader_extract_to_mem(&archive_, static_cast<mz_uint>(index), output.data(),
                                          output.size(), 0))
        {
            return std::unexpected(
                std::format("Failed to extract '{}': {}", name, MinizError(archive_)));
        }
        return output;
    }

private:
    mutable mz_zip_archive archive_ {};
    mutable std::mutex mutex_;
    bool is_open_ { false };
};

std::expected<std::shared_ptr<DocArchive>, std::string>
    OpenDocArchive(const std::filesystem::path& zip_path)
{
    std::shared_ptr<DocArchive> archive = std::make_shared<DocArchive>();
    std::expected<void, std::string> result = archive->Open(zip_path);
    if (!result)
    {
        return std::unexpected(std::move(result.error()));
    }
    return archive;
}

std::expected<std::string, std::string> ReadMarkdown(const DocArchive& archive,
                                                     std::string_view archive_name)
{
    return archive.Extract(archive_name);
}
