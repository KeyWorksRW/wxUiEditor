// Low-level pattern scanners for markdown syntax
// Purpose: Fast lookahead parsing - scan for URLs, code fences, HTML tags, headings, etc.
// Key functions: _scan_* functions return length of matched pattern (or 0 if no match)
// Implementation: Generated from scanners.re (re2c tool) - do not edit _scan_* functions directly
// Usage: Parser calls scan_open_code_fence(), scan_atx_heading_start(), etc. to detect syntax
// Status: Legacy code (auto-generated)
// Dependencies: chunk.hxx, cmark-gfm.hxx

#pragma once

#include "chunk.hxx"
#include "cmark-gfm.hxx"

using scanner_func_t = size_t (*)(const unsigned char* input);

size_t _scan_at(scanner_func_t scanner, cmark_chunk* chunk, size_t offset);
size_t _scan_scheme(const unsigned char* input);
size_t _scan_autolink_uri(const unsigned char* input);
size_t _scan_autolink_email(const unsigned char* input);
size_t _scan_html_tag(const unsigned char* input);
size_t _scan_liberal_html_tag(const unsigned char* input);
size_t _scan_html_comment(const unsigned char* input);
size_t _scan_html_pi(const unsigned char* input);
size_t _scan_html_declaration(const unsigned char* input);
size_t _scan_html_cdata(const unsigned char* input);
size_t _scan_html_block_start(const unsigned char* input);
size_t _scan_html_block_start_7(const unsigned char* input);
size_t _scan_html_block_end_1(const unsigned char* input);
size_t _scan_html_block_end_2(const unsigned char* input);
size_t _scan_html_block_end_3(const unsigned char* input);
size_t _scan_html_block_end_4(const unsigned char* input);
size_t _scan_html_block_end_5(const unsigned char* input);
size_t _scan_link_title(const unsigned char* input);
size_t _scan_spacechars(const unsigned char* input);
size_t _scan_atx_heading_start(const unsigned char* input);
size_t _scan_setext_heading_line(const unsigned char* input);
size_t _scan_open_code_fence(const unsigned char* input);
size_t _scan_close_code_fence(const unsigned char* input);
size_t _scan_entity(const unsigned char* input);
size_t _scan_dangerous_url(const unsigned char* input);
size_t _scan_footnote_definition(const unsigned char* input);

inline size_t scan_scheme(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_scheme, chunk, offset);
}
inline size_t scan_autolink_uri(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_autolink_uri, chunk, offset);
}
inline size_t scan_autolink_email(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_autolink_email, chunk, offset);
}
inline size_t scan_html_tag(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_html_tag, chunk, offset);
}
inline size_t scan_liberal_html_tag(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_liberal_html_tag, chunk, offset);
}
inline size_t scan_html_comment(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_html_comment, chunk, offset);
}
inline size_t scan_html_pi(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_html_pi, chunk, offset);
}
inline size_t scan_html_declaration(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_html_declaration, chunk, offset);
}
inline size_t scan_html_cdata(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_html_cdata, chunk, offset);
}
inline size_t scan_html_block_start(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_html_block_start, chunk, offset);
}
inline size_t scan_html_block_start_7(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_html_block_start_7, chunk, offset);
}
inline size_t scan_html_block_end_1(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_html_block_end_1, chunk, offset);
}
inline size_t scan_html_block_end_2(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_html_block_end_2, chunk, offset);
}
inline size_t scan_html_block_end_3(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_html_block_end_3, chunk, offset);
}
inline size_t scan_html_block_end_4(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_html_block_end_4, chunk, offset);
}
inline size_t scan_html_block_end_5(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_html_block_end_5, chunk, offset);
}
inline size_t scan_link_title(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_link_title, chunk, offset);
}
inline size_t scan_spacechars(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_spacechars, chunk, offset);
}
inline size_t scan_atx_heading_start(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_atx_heading_start, chunk, offset);
}
inline size_t scan_setext_heading_line(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_setext_heading_line, chunk, offset);
}
inline size_t scan_open_code_fence(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_open_code_fence, chunk, offset);
}
inline size_t scan_close_code_fence(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_close_code_fence, chunk, offset);
}
inline size_t scan_entity(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_entity, chunk, offset);
}
inline size_t scan_dangerous_url(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_dangerous_url, chunk, offset);
}
inline size_t scan_footnote_definition(cmark_chunk* chunk, size_t offset)
{
    return _scan_at(_scan_footnote_definition, chunk, offset);
}
