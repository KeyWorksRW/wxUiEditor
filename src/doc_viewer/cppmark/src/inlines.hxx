// Inline markdown parsing (emphasis, links, images, code spans)
// Purpose: Phase 2 of parsing - after blocks parsed, parse inline content within text nodes
// Key functions: cmark_parse_inlines() (main entry), cmark_parse_reference_inline() (for [ref]:
// url) Use case: Parser calls this on TEXT nodes to convert **bold**, [links](), etc. into AST
// nodes Dependencies:
// chunk.hxx, map.hxx for reference lookups

#pragma once

#include "chunk.hxx"
#include "map.hxx"

cmark_chunk cmark_clean_url(cmark_chunk* url);
cmark_chunk cmark_clean_title(cmark_chunk* title);

void cmark_parse_inlines(cmark_parser* parser, cmark_node* parent, cmark_map* refmap, int options);

size_t cmark_parse_reference_inline(cmark_chunk* input, cmark_map* refmap);

void cmark_inlines_add_special_character(unsigned char character, bool emphasis);
void cmark_inlines_remove_special_character(unsigned char character, bool emphasis);
