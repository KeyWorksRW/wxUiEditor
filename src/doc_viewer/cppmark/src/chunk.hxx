// Text chunk type for efficient string slices
// Purpose: Non-owning view into markdown text (similar to std::string_view)
// Key type: cmark_chunk {data*, len, alloc} - when alloc=0 it's a view, when alloc=1 it owns memory
// Use case: Avoids copying strings during parsing - parser creates chunks pointing into input
// buffer Status: Legacy code - uses manual memory management (Phase 1.5 modernization candidate)
// Dependencies: buffer.hxx, cmark-gfm.hxx

#pragma once

#include <cassert>

#include "buffer.hxx"
#include "cmark-gfm.hxx"

#define CMARK_CHUNK_EMPTY { nullptr, 0, 0 }

struct cmark_chunk
{
    unsigned char* data;
    size_t len;
    size_t alloc;  // also implies a NULL-terminated string
};

// Function declarations
void cmark_chunk_free(cmark_chunk* chunk);
void cmark_chunk_ltrim(cmark_chunk* chunk);
void cmark_chunk_rtrim(cmark_chunk* chunk);
void cmark_chunk_trim(cmark_chunk* chunk);
size_t cmark_chunk_strchr(cmark_chunk* chunk_source, int character_code, size_t offset);
const char* cmark_chunk_to_cstr(cmark_chunk* chunk);
void cmark_chunk_set_cstr(cmark_chunk* chunk, const char* c_string);
cmark_chunk cmark_chunk_literal(const char* data);
cmark_chunk cmark_chunk_dup(const cmark_chunk* chunk_source, size_t offset, size_t length);
cmark_chunk cmark_chunk_buf_detach(cmark_strbuf* buffer);
cmark_chunk cmark_chunk_ltrim_new(cmark_chunk* chunk);
cmark_chunk cmark_chunk_rtrim_new(cmark_chunk* chunk);
