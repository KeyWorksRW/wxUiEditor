/////////////////////////////////////////////////////////////////////////////
// Purpose:   C++ RAII wrapper around miniz ZIP writer
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "zip_writer.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <vector>

#include <libdeflate.h>
#include <miniz.h>

struct ZipArchiveImpl
{
    mz_zip_archive archive {};
};

ZipWriter::ZipWriter() : impl_(std::make_unique<ZipArchiveImpl>()) {}

ZipWriter::~ZipWriter()
{
    if (is_open_)
    {
        if (!is_finalized_)
        {
            // Archive was never finalized — attempt cleanup but file will be invalid.
            mz_zip_writer_end(&impl_->archive);
        }
        mz_zip_writer_end(&impl_->archive);
    }
}

std::expected<void, std::string> ZipWriter::Open(const std::filesystem::path& file_path)
{
    if (is_open_)
    {
        return std::unexpected(std::string("Archive is already open"));
    }

    impl_->archive = mz_zip_archive {};

    const std::string path_str = file_path.string();
    if (!mz_zip_writer_init_file(&impl_->archive, path_str.c_str(), 0))
    {
        std::string error = "Failed to create ZIP file: ";
        error += path_str;
        const char* miniz_error = mz_zip_get_error_string(impl_->archive.m_last_error);
        if (miniz_error)
        {
            error += " (";
            error += miniz_error;
            error += ")";
        }
        return std::unexpected(std::move(error));
    }

    is_open_ = true;
    is_finalized_ = false;
    return {};
}

std::expected<void, std::string> ZipWriter::AddFile(std::string_view archive_name,
                                                    std::string_view content)
{
    if (!is_open_ || is_finalized_)
    {
        return std::unexpected(std::string("Archive is not open for writing"));
    }

    // Pre-compress with libdeflate, then add via AddPreCompressed
    std::vector<uint8_t> compressed =
        CompressForArchive(std::span<const char>(content), MAX_DEFLATE_LEVEL);

    if (compressed.empty())
    {
        return std::unexpected(std::string("Compression failed for: ") + std::string(archive_name));
    }

    // Compute CRC-32 of the uncompressed data
    const uint32_t crc32 = libdeflate_crc32(0, content.data(), content.size());

    return AddPreCompressed(archive_name, compressed, content.size(), crc32);
}

std::expected<void, std::string> ZipWriter::AddFileUncompressed(std::string_view archive_name,
                                                                const void* data, std::size_t size)
{
    if (!is_open_ || is_finalized_)
    {
        return std::unexpected(std::string("Archive is not open for writing"));
    }

    const std::string name_str(archive_name);

    // MZ_NO_COMPRESSION (level 0) stores the data without compression.
    if (!mz_zip_writer_add_mem(&impl_->archive, name_str.c_str(), data, size, MZ_NO_COMPRESSION))
    {
        std::string error = "Failed to add uncompressed file to archive: ";
        error += name_str;
        const char* miniz_error = mz_zip_get_error_string(impl_->archive.m_last_error);
        if (miniz_error)
        {
            error += " (";
            error += miniz_error;
            error += ")";
        }
        return std::unexpected(std::move(error));
    }

    return {};
}

std::expected<void, std::string> ZipWriter::AddDiskFile(std::string_view archive_name,
                                                        const std::filesystem::path& disk_path)
{
    if (!is_open_ || is_finalized_)
    {
        return std::unexpected(std::string("Archive is not open for writing"));
    }

    // Read the entire file into memory, then add it.
    std::ifstream file_stream(disk_path, std::ios::binary | std::ios::ate);
    if (!file_stream)
    {
        std::string error = "Cannot open source file: ";
        error += disk_path.string();
        return std::unexpected(std::move(error));
    }

    const std::streamsize file_size = file_stream.tellg();
    file_stream.seekg(0, std::ios::beg);

    std::vector<char> buffer(static_cast<std::size_t>(file_size));
    if (!file_stream.read(buffer.data(), file_size))
    {
        std::string error = "Failed to read source file: ";
        error += disk_path.string();
        return std::unexpected(std::move(error));
    }

    return AddFile(archive_name, std::string_view(buffer.data(), buffer.size()));
}

std::expected<void, std::string> ZipWriter::Finalize()
{
    if (!is_open_)
    {
        return std::unexpected(std::string("Archive is not open"));
    }

    if (is_finalized_)
    {
        return std::unexpected(std::string("Archive is already finalized"));
    }

    if (!mz_zip_writer_finalize_archive(&impl_->archive))
    {
        std::string error = "Failed to finalize ZIP archive";
        const char* miniz_error = mz_zip_get_error_string(impl_->archive.m_last_error);
        if (miniz_error)
        {
            error += " (";
            error += miniz_error;
            error += ")";
        }
        return std::unexpected(std::move(error));
    }

    if (!mz_zip_writer_end(&impl_->archive))
    {
        return std::unexpected(std::string("Failed to close ZIP archive"));
    }

    is_finalized_ = true;
    is_open_ = false;
    return {};
}

std::expected<void, std::string>
    ZipWriter::AddPreCompressed(std::string_view archive_name,
                                std::span<const uint8_t> compressed_data,
                                std::size_t uncompressed_size, uint32_t crc32)
{
    if (!is_open_ || is_finalized_)
    {
        return std::unexpected(std::string("Archive is not open for writing"));
    }

    const std::string name_str(archive_name);

    // MZ_ZIP_FLAG_COMPRESSED_DATA tells miniz the buffer is already deflated.
    // Use MZ_DEFAULT_LEVEL (positive) OR'd with the flag — MZ_DEFAULT_COMPRESSION (-1)
    // would be replaced by miniz and lose the flag.
    if (!mz_zip_writer_add_mem_ex(
            &impl_->archive, name_str.c_str(), compressed_data.data(), compressed_data.size(),
            nullptr, 0, static_cast<mz_uint>(MZ_DEFAULT_LEVEL) | MZ_ZIP_FLAG_COMPRESSED_DATA,
            static_cast<mz_uint64>(uncompressed_size), crc32))
    {
        std::string error = "Failed to add pre-compressed file to archive: ";
        error += name_str;
        const char* miniz_error = mz_zip_get_error_string(impl_->archive.m_last_error);
        if (miniz_error)
        {
            error += " (";
            error += miniz_error;
            error += ")";
        }
        return std::unexpected(std::move(error));
    }

    return {};
}

std::vector<uint8_t> ZipWriter::CompressForArchive(std::span<const char> source_data, int level)
{
    if (source_data.empty())
    {
        return {};
    }

    // libdeflate accepts deflate compression level (MIN_DEFLATE_LEVEL–MAX_DEFLATE_LEVEL).
    // Level MAX_DEFLATE_LEVEL uses optimal parsing (minimum-cost-path) for best ratio.
    // Clamp to valid range.
    level = std::clamp(level, MIN_DEFLATE_LEVEL, MAX_DEFLATE_LEVEL);

    libdeflate_compressor* compressor = libdeflate_alloc_compressor(level);
    if (!compressor)
    {
        return {};
    }

    const size_t bound = libdeflate_deflate_compress_bound(compressor, source_data.size());
    std::vector<uint8_t> result(bound);

    const size_t compressed_size = libdeflate_deflate_compress(
        compressor, source_data.data(), source_data.size(), result.data(), result.size());

    libdeflate_free_compressor(compressor);

    if (compressed_size == 0)
    {
        return {};
    }

    result.resize(compressed_size);
    return result;
}
