// Global extension registry
// Purpose: Manages list of available syntax extensions (register once, use in multiple parsers)
// Key functions: cmark_list_syntax_extensions() (enumerate)
// Status: Legacy code - uses manual malloc (Phase 1.5 modernization candidate)
// Dependencies: cmark-gfm.hxx

#pragma once

#include "cmark-gfm.hxx"

void cmark_release_plugins(void);

cmark_llist* cmark_list_syntax_extensions(void);
