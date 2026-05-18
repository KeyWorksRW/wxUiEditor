// Dynamic string buffer - MODERNIZED (Phase 2: std::string backend)
// Purpose: Provides growable byte buffers for markdown parsing
// Modernization: Replaced std::unique_ptr<std::vector<unsigned char>> with std::string
// Key type: CMarkStringBuffer (C++ class with std::string backend)
// Dependencies: cmark-gfm.hxx for size_t

#pragma once

#include <span>
#include <string>
#include <string_view>

#include "cmark-gfm.hxx"

// C++ class for dynamic string buffers using std::string
class CMarkStringBuffer
{
public:
    CMarkStringBuffer() = default;

    ~CMarkStringBuffer() = default;
    CMarkStringBuffer(const CMarkStringBuffer&) = delete;
    CMarkStringBuffer& operator=(const CMarkStringBuffer&) = delete;
    CMarkStringBuffer(CMarkStringBuffer&&) = default;
    CMarkStringBuffer& operator=(CMarkStringBuffer&&) = default;

    // Initialize with initial size
    void Init(size_t initial_size);

    // Grow the buffer to hold at least target_size bytes
    void Grow(size_t target_size);

    // Free/reset the buffer
    void Free();
    void Clear();
    void Reset() { Free(); }

    // Get buffer length (logical size, excludes null terminator)
    size_t Len() const { return m_buffer.size(); }

    // Compare two buffers
    int Cmp(const CMarkStringBuffer& other) const;

    // Swap contents with another buffer
    void Swap(CMarkStringBuffer& other);

    // Transfer buffer contents to caller. Buffer is left empty afterwards.
    [[nodiscard]] std::string Detach();

    // Copy buffer to C-string
    void CopyCStr(std::span<char> data) const;

    // Get C-string pointer (const)
    const char* CStr() const { return m_buffer.c_str(); }

    std::string& get_StringBuffer() { return m_buffer; }

    // Set buffer contents
    void Set(const char* data, size_t length);
    void Sets(const char* string);

    // Append operations
    void Putc(int character);
    void Put(const unsigned char* data, size_t length);
    void Put(std::string_view string);
    void Puts(const char* string);

    // String search operations
    size_t Strchr(int character, size_t position) const;
    size_t Strrchr(int character, size_t position) const;

    // Buffer manipulation
    void Drop(size_t count);
    void Truncate(size_t length);
    void Rtrim();
    void Trim();
    void NormalizeWhitespace();
    void Unescape();

    // Mutable access to raw buffer (caller must call Truncate after writing)
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    unsigned char* get_ptr() { return reinterpret_cast<unsigned char*>(m_buffer.data()); }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const unsigned char* get_ptr() const
    {
        return reinterpret_cast<const unsigned char*>(m_buffer.data());
    }

    size_t get_size() const { return m_buffer.size(); }

private:
    std::string m_buffer;
};
