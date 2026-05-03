// Text chunk type for efficient string slices
// Purpose: Non-owning view into markdown text (similar to std::string_view)
// Key type: cmark_chunk {data*, len, alloc} - when alloc=0 it's a view, when alloc=1 it owns memory
// Use case: Avoids copying strings during parsing - parser creates chunks pointing into input
// buffer Status: Phase 1.5 modernization - converted to std::unique_ptr for memory safety
// Dependencies: buffer.hxx, cmark-gfm.hxx, cmark_ctype.hxx

#include "chunk.hxx"

#include <bit>
#include <cassert>
#include <cstring>
#include <memory>

#include "buffer.hxx"
#include "cmark_ctype.hxx"

void cmark_chunk_free(cmark_chunk* chunk)
{
    if (chunk->alloc && chunk->data)
    {
        delete[] chunk->data;
    }

    chunk->data = nullptr;
    chunk->alloc = 0;
    chunk->len = 0;
}

void cmark_chunk_ltrim(cmark_chunk* chunk)
{
    assert(!chunk->alloc);

    while (chunk->len && cmark_isspace(chunk->data[0]))
    {
        chunk->data++;
        chunk->len--;
    }
}

void cmark_chunk_rtrim(cmark_chunk* chunk)
{
    assert(!chunk->alloc);

    while (chunk->len > 0)
    {
        if (!cmark_isspace(chunk->data[chunk->len - 1]))
        {
            break;
        }

        chunk->len--;
    }
}

void cmark_chunk_trim(cmark_chunk* chunk)
{
    cmark_chunk_ltrim(chunk);
    cmark_chunk_rtrim(chunk);
}

size_t cmark_chunk_strchr(cmark_chunk* chunk_source, int character_code, size_t offset)
{
    const unsigned char* found_ptr = static_cast<const unsigned char*>(
        std::memchr(chunk_source->data + offset, character_code, chunk_source->len - offset));
    return found_ptr ? static_cast<size_t>(found_ptr - chunk_source->data) : chunk_source->len;
}

const char* cmark_chunk_to_cstr(cmark_chunk* chunk)
{
    if (chunk->alloc)
    {
        return std::bit_cast<const char*>(chunk->data);
    }

    std::unique_ptr<unsigned char[]> owned_string =
        std::make_unique<unsigned char[]>(chunk->len + 1);
    if (chunk->len > 0)
    {
        std::memcpy(owned_string.get(), chunk->data, chunk->len);
    }
    owned_string[chunk->len] = 0;

    chunk->data = owned_string.release();  // Transfer ownership to chunk
    chunk->alloc = 1;

    return std::bit_cast<const char*>(chunk->data);
}

void cmark_chunk_set_cstr(cmark_chunk* chunk, const char* c_string)
{
    // Free old data if chunk owns it
    if (chunk->alloc && chunk->data)
    {
        delete[] chunk->data;
    }

    if (c_string == nullptr)
    {
        chunk->len = 0;
        chunk->data = nullptr;
        chunk->alloc = 0;
    }
    else
    {
        chunk->len = static_cast<size_t>(std::strlen(c_string));
        std::unique_ptr<unsigned char[]> new_data =
            std::make_unique<unsigned char[]>(chunk->len + 1);
        std::memcpy(new_data.get(), c_string, chunk->len + 1);
        chunk->data = new_data.release();  // Transfer ownership to chunk
        chunk->alloc = 1;
    }
}

cmark_chunk cmark_chunk_literal(const char* data)
{
    const size_t chunk_length = data ? static_cast<size_t>(std::strlen(data)) : 0;
    cmark_chunk chunk = { .data = std::bit_cast<unsigned char*>(const_cast<char*>(data)),
                          .len = chunk_length,
                          .alloc = 0 };  // NOLINT: Legacy cppmark code
    return chunk;
}

cmark_chunk cmark_chunk_dup(const cmark_chunk* chunk_source, size_t offset, size_t length)
{
    cmark_chunk result = { chunk_source->data + offset, length, 0 };
    return result;
}

cmark_chunk cmark_chunk_buf_detach(cmark_strbuf* buffer)
{
    cmark_chunk result = {};
    std::string detached = buffer->Detach();
    result.len = detached.size();
    std::unique_ptr<unsigned char[]> buf =
        std::make_unique_for_overwrite<unsigned char[]>(result.len + 1);
    std::memcpy(buf.get(), detached.data(), result.len + 1);
    result.data = buf.release();
    result.alloc = 1;
    return result;
}

cmark_chunk cmark_chunk_ltrim_new(cmark_chunk* chunk)
{
    cmark_chunk result = cmark_chunk_dup(chunk, 0, chunk->len);
    cmark_chunk_ltrim(&result);
    cmark_chunk_to_cstr(&result);
    return result;
}

cmark_chunk cmark_chunk_rtrim_new(cmark_chunk* chunk)
{
    cmark_chunk result = cmark_chunk_dup(chunk, 0, chunk->len);
    cmark_chunk_rtrim(&result);
    cmark_chunk_to_cstr(&result);
    return result;
}
