// Generic rendering framework for outputting markdown AST
// Purpose: Base renderer for all output formats (HTML, CommonMark, LaTeX, etc.)
// Key types: cmark_renderer (text output), cmark_html_renderer (HTML-specific)
// Render flow: Traverse AST -> call format-specific callbacks -> build output buffer
// Status: Legacy code - uses manual malloc (Phase 1.5 modernization candidate)
// Dependencies: buffer.hxx, chunk.hxx

#pragma once

#include <cstdint>
#include <stdlib.h>
#include <string>

#include "buffer.hxx"
#include "chunk.hxx"

typedef enum
{
    LITERAL,
    NORMAL,
    TITLE,
    URL
} cmark_escaping;

struct cmark_renderer
{
    CMarkStringBuffer* buffer;
    CMarkStringBuffer* prefix;
    int column;
    int width;
    int need_cr;
    size_t last_breakable;
    bool begin_line;
    bool begin_content;
    bool no_linebreaks;
    bool in_tight_list_item;
    void (*outc)(struct cmark_renderer* renderer, cmark_node* node, cmark_escaping escape,
                 int32_t code_point, unsigned char next_character);
    void (*cr)(struct cmark_renderer* renderer);
    void (*blankline)(struct cmark_renderer* renderer);
    void (*out)(struct cmark_renderer* renderer, cmark_node* node, const char* source_text,
                bool wrap, cmark_escaping escape);
    unsigned int footnote_ix;
};

struct cmark_html_renderer
{
    CMarkStringBuffer* html;
    cmark_node* plain;
    cmark_llist* filter_extensions;
    unsigned int footnote_ix;
    unsigned int written_footnote_ix;
    void* opaque;
};

void cmark_render_ascii(cmark_renderer* renderer, const char* source_text);

void cmark_render_code_point(cmark_renderer* renderer, uint32_t code_point);

std::string cmark_render(cmark_node* root, int options, int width,
                         void (*output_character)(cmark_renderer*, cmark_node*, cmark_escaping,
                                                  int32_t, unsigned char),
                         int (*render_node)(cmark_renderer* renderer, cmark_node* node,
                                            cmark_event_type event_type, int options));
