// GFM table syntax extension - public header
// Purpose: Declares create_table_extension() and the three table node types
// Dependencies: cmark-gfm-extension_api.hxx

#pragma once

#include "cmark-gfm-extension_api.hxx"

// Global table node type variables — zero-initialized; assigned by
// cmark_syntax_extension_add_node() when create_table_extension() is first called.
extern cmark_node_type CMARK_NODE_TABLE;
extern cmark_node_type CMARK_NODE_TABLE_ROW;
extern cmark_node_type CMARK_NODE_TABLE_CELL;

// Create and return the fully-configured table syntax extension.
// The caller is responsible for attaching it to a parser with
// cmark_parser_attach_syntax_extension() and freeing it with cmark_syntax_extension_free()
// when the parser is done.
[[nodiscard]] cmark_syntax_extension* create_table_extension();
