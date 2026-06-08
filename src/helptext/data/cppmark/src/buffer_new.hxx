// Alternative buffer implementation using std::vector directly
// Purpose: Experimental modernized buffer (API-breaking, no cmark_mem allocator)
// Status: Not currently used - prefer buffer.hxx for compatibility
// Key difference: Direct std::vector<unsigned char> instead of unique_ptr wrapper
// Dependencies: cmark-gfm.hxx for size_t

#pragma once

#include <string>
#include <vector>

#include "cmark-gfm.hxx"

// Modernized buffer class using std::vector (no manual memory management)
// API-breaking change: No longer uses cmark_mem* allocator
class cmark_strbuf
{
public:
    std::vector<unsigned char> data;

    cmark_strbuf() { data.push_back('\0'); }  // Ensure null-termination
    explicit cmark_strbuf(size_t initial_size) : data()
    {
        data.reserve(initial_size + 1);
        data.push_back('\0');
    }

    // Direct data access
    auto ptr() -> unsigned char* { return data.data(); }
    auto ptr() const -> const unsigned char* { return data.data(); }

    // Size excludes null terminator
    auto size() const -> size_t { return data.empty() ? 0 : static_cast<size_t>(data.size() - 1); }
    auto asize() const -> size_t { return static_cast<size_t>(data.capacity()); }

    // Ensure null-termination after modifications
    void ensure_null_term()
    {
        if (data.empty() || data.back() != '\0')
            data.push_back('\0');
    }
};

// Deprecated - kept for compatibility during transition
extern unsigned char cmark_strbuf_initbuf[];

// CMARK_BUF_INIT macro removed - use cmark_strbuf() constructor directly

/**
 * Initialize a cmark_strbuf structure.
 * mem parameter ignored (deprecated - no longer needs custom allocator)
 */
void cmark_strbuf_init(cmark_mem* mem, cmark_strbuf* buf, size_t initial_size);

// Grow the buffer to hold at least `target_size` bytes.
void cmark_strbuf_grow(cmark_strbuf* buf, size_t target_size);

void cmark_strbuf_free(cmark_strbuf* buf);

void cmark_strbuf_reset(cmark_strbuf* buf);
void cmark_strbuf_swap(cmark_strbuf* buf_a, cmark_strbuf* buf_b);

size_t cmark_strbuf_len(const cmark_strbuf* buf);

int cmark_strbuf_cmp(const cmark_strbuf* a, const cmark_strbuf* b);

unsigned char* cmark_strbuf_detach(cmark_strbuf* buf);

void cmark_strbuf_copy_cstr(char* data, size_t datasize, const cmark_strbuf* buf);

[[maybe_unused]] static const char* cmark_strbuf_cstr(const cmark_strbuf* buf)
{
    return (char*) buf->ptr();
}

void cmark_strbuf_set(cmark_strbuf* buf, const unsigned char* data, size_t len);

void cmark_strbuf_sets(cmark_strbuf* buf, const char* string);

void cmark_strbuf_putc(cmark_strbuf* buf, int c);

void cmark_strbuf_put(cmark_strbuf* buf, const unsigned char* data, size_t len);

void cmark_strbuf_puts(cmark_strbuf* buf, const char* string);

void cmark_strbuf_puts(cmark_strbuf* buf, const char* string);

void cmark_strbuf_clear(cmark_strbuf* buf);

size_t cmark_strbuf_strchr(const cmark_strbuf* buf, int c, size_t pos);

size_t cmark_strbuf_strrchr(const cmark_strbuf* buf, int c, size_t pos);

void cmark_strbuf_drop(cmark_strbuf* buf, size_t n);

void cmark_strbuf_truncate(cmark_strbuf* buf, size_t len);

void cmark_strbuf_normalize_whitespace(cmark_strbuf* s);

void cmark_strbuf_rtrim(cmark_strbuf* buf);

void cmark_strbuf_trim(cmark_strbuf* buf);

void cmark_strbuf_normalize_whitespace(cmark_strbuf* s);

void cmark_strbuf_unescape(cmark_strbuf* s);

// Alternative C++ class with std::string backend (used in some places)
class CMarkStrbuf
{
public:
    std::string m_strbuf;

    void init(cmark_mem* mem, size_t initial_size = 0);
    void grow(size_t target_size);

    void free()
    {
        m_strbuf.clear();
        m_strbuf.shrink_to_fit();
    }

    void clear() { m_strbuf.clear(); }

    void copy_cstr(char* data, size_t datasize);

    void putc(int c) { m_strbuf.push_back(static_cast<char>(c)); }

    void put(const unsigned char* data, size_t len)
    {
        if (len > 0)
        {
            m_strbuf.append(reinterpret_cast<const char*>(data), len);
        }
    }

    void puts(cmark_strbuf* buf, const char* string)
    {
        if (string)
        {
            m_strbuf.append(string);
        }
    }

    void set(const unsigned char* data, size_t len);
    void sets(const char* data, size_t len);

    void swap(CMarkStrbuf& other) { m_strbuf.swap(other.m_strbuf); }

    std::string detach()
    {
        std::string result = m_strbuf;
        m_strbuf.clear();
        m_strbuf.shrink_to_fit();
        return result;
    }

    int cmp(const CMarkStrbuf& other) const { return m_strbuf.compare(other.m_strbuf); }

    size_t strchr(int c, size_t pos);
    size_t strrchr(int c, size_t pos);

    void truncate(size_t len);
    void drop(size_t n);

    void rtrim();
    void trim();

    void normalize_whitespace();
    void unescape();
};
