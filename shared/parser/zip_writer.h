/////////////////////////////////////////////////////////////////////////////
// Purpose:   C++ RAII wrapper around ZIP archive writer
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

struct ZipArchiveImpl;

// RAII wrapper around ZIP archive writer.
//
// Usage:
//   ZipWriter zip;
//   if (!zip.Open("output.zip")) { /* error */ }
//   zip.AddFile("docs/wxButton.md", markdown_text);
//   zip.AddFileUncompressed("search_index.kfts", binary_data, binary_size);
//   if (!zip.Finalize()) { /* error */ }
//   // destructor cleans up
class ZipWriter
{
public:
    ZipWriter();
    ~ZipWriter();

    static constexpr int MIN_DEFLATE_LEVEL = 1;
    static constexpr int MAX_DEFLATE_LEVEL = 12;

    // Non-copyable, non-movable (contains opaque C struct)
    ZipWriter(const ZipWriter&) = delete;
    ZipWriter& operator=(const ZipWriter&) = delete;
    ZipWriter(ZipWriter&&) = delete;
    ZipWriter& operator=(ZipWriter&&) = delete;

    [[nodiscard]] std::expected<void, std::string> Open(const std::filesystem::path& file_path);

    // Add a text file to the archive with DEFLATE compression.
    // archive_name  Path within the archive (e.g., "docs/wxButton.md")
    // content  File content (UTF-8 text)
    [[nodiscard]] std::expected<void, std::string> AddFile(std::string_view archive_name,
                                                           std::string_view content);

    // Add a binary file to the archive with no compression (STORE method).
    // Useful for pre-compressed or binary index files like .kfts.
    [[nodiscard]] std::expected<void, std::string>
        AddFileUncompressed(std::string_view archive_name, const void* data, std::size_t size);

    // Add a file from disk to the archive with DEFLATE compression.
    [[nodiscard]] std::expected<void, std::string>
        AddDiskFile(std::string_view archive_name, const std::filesystem::path& disk_path);

    // Add pre-compressed (raw DEFLATE) data to the archive.
    // Use this when compression was performed on a separate thread.
    // uncompressed_size  Original size before compression
    // crc32  CRC-32 of the original uncompressed data
    [[nodiscard]] std::expected<void, std::string>
        AddPreCompressed(std::string_view archive_name, std::span<const uint8_t> compressed_data,
                         std::size_t uncompressed_size, uint32_t crc32);

    // Compress data into raw DEFLATE format suitable for AddPreCompressed.
    // Thread-safe — operates only on local data with no shared state.
    // level  Compression level (MIN_DEFLATE_LEVEL–MAX_DEFLATE_LEVEL, default MAX = optimal)
    // Compressed bytes (raw DEFLATE), or empty vector on failure
    static std::vector<uint8_t> CompressForArchive(std::span<const char> source_data,
                                                   int level = MAX_DEFLATE_LEVEL);

    // Finalize the archive (writes central directory). Must be called before
    // destruction for the archive to be valid.
    [[nodiscard]] std::expected<void, std::string> Finalize();

    bool IsOpen() const { return is_open_; }

private:
    std::unique_ptr<ZipArchiveImpl> impl_;
    bool is_open_ = false;
    bool is_finalized_ = false;
};
