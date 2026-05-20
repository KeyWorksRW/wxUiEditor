// Hash map for reference links and footnotes
// Purpose: Stores [link]: url definitions and [^footnote]: content during parsing
// Key type: cmark_map - linked list of entries with hash lookup
// Usage: Create map -> insert normalized labels -> lookup by label
// Dependencies: chunk.hxx for cmark_chunk

#pragma once

#include <cstddef>
#include <string>

#include "chunk.hxx"

struct cmark_map_entry
{
    struct cmark_map_entry* next;
    std::string label;
    size_t age;
    size_t size;
};

struct cmark_map;

typedef void (*cmark_map_free_f)(struct cmark_map* map_ptr, cmark_map_entry* entry);

struct cmark_map
{
    cmark_map_entry* refs;
    cmark_map_entry** sorted;
    size_t size;
    size_t ref_size;
    size_t max_ref_size;
    cmark_map_free_f free;
};

std::string normalize_map_label(cmark_chunk* reference_chunk);
cmark_map* cmark_map_new(cmark_map_free_f free_func);
void cmark_map_free(cmark_map* map_ptr);
cmark_map_entry* cmark_map_lookup(cmark_map* map_ptr, cmark_chunk* label_chunk);
