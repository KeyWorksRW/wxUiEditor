// Reference link storage ([link]: url)
// Purpose: Stores [ref]: url "title" definitions parsed from markdown
// Key type: cmark_reference - map entry linking label to url/title chunks
// Usage: Parser stores definitions -> inline parser looks up [ref] links
// Status: Legacy code - uses manual malloc (Phase 1.5 modernization candidate)
// Dependencies: map.hxx for hash map storage

#pragma once

#include "map.hxx"

struct cmark_reference
{
    cmark_map_entry entry;
    cmark_chunk url;
    cmark_chunk title;
};

void cmark_reference_create(cmark_map* ref_map, cmark_chunk* ref_label, cmark_chunk* reference_url,
                            cmark_chunk* title);
cmark_map* cmark_reference_map_new();
