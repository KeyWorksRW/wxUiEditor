// Footnote extension support
// Purpose: Handles [^1]: footnote definitions and [^1] references in markdown
// Key type: cmark_footnote - map entry linking footnote label to AST node
// Dependencies: map.hxx for hash map storage

#pragma once

#include "map.hxx"

struct cmark_footnote
{
    cmark_map_entry entry;
    cmark_node* node;
    unsigned int ix;
};

void cmark_footnote_create(cmark_map* footnote_map, cmark_node* node);
cmark_map* cmark_footnote_map_new(void);

void cmark_unlink_footnotes_map(cmark_map* footnote_map);
