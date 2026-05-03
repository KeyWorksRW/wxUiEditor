// Purpose: Base renderer for all output formats - handles line wrapping, escaping, traversal
// Key functions: cmark_render() (main entry), cmark_render_ascii(), cmark_render_code_point()
// Use case: HTML/CommonMark/LaTeX renderers build on this framework
// Status: Legacy code - uses manual malloc (Phase 1.5 modernization candidate)
// Dependencies: buffer.hxx, chunk.hxx, cmark-gfm.hxx, node.hxx, syntax_extension.hxx, utf8.hxx,
// render.hxx

#include <cstdint>
#include <cstring>

#include "buffer.hxx"
#include "chunk.hxx"
#include "cmark-gfm.hxx"
#include "cmark_ctype.hxx"
#include "node.hxx"
#include "syntax_extension.hxx"
#include "utf8.hxx"

#include "render.hxx"

static void SetCarriageReturn(cmark_renderer* renderer)
{
    if (renderer->need_cr < 1)
    {
        renderer->need_cr = 1;
    }
}

static void SetBlankLine(cmark_renderer* renderer)
{
    if (renderer->need_cr < 2)
    {
        renderer->need_cr = 2;
    }
}

static void WriteOutput(cmark_renderer* renderer, cmark_node* node, const char* source_text,
                        bool wrap, cmark_escaping escape)
{
    const int length = static_cast<int>(std::strlen(source_text));
    unsigned char next_character = 0;
    int32_t code_point = 0;
    int source_index = 0;
    int last_nonspace = 0;
    int code_point_length = 0;
    cmark_chunk remainder = cmark_chunk_literal("");
    int tail_index = renderer->buffer->get_size() - 1;

    cmark_syntax_extension* extension = node->ancestor_extension;
    if (extension && !extension->commonmark_escape_func)
    {
        extension = nullptr;
    }

    wrap = wrap && !renderer->no_linebreaks;

    if (renderer->in_tight_list_item && renderer->need_cr > 1)
    {
        renderer->need_cr = 1;
    }
    while (renderer->need_cr)
    {
        if (tail_index < 0 || renderer->buffer->get_ptr()[tail_index] == '\n')
        {
            tail_index -= 1;
        }
        else
        {
            renderer->buffer->Putc('\n');
            if (renderer->need_cr > 1)
            {
                renderer->buffer->Put(renderer->prefix->get_ptr(), renderer->prefix->get_size());
            }
        }
        renderer->column = 0;
        renderer->last_breakable = 0;
        renderer->begin_line = true;
        renderer->begin_content = true;
        renderer->need_cr -= 1;
    }

    while (source_index < length)
    {
        if (renderer->begin_line)
        {
            renderer->buffer->Put(renderer->prefix->get_ptr(), renderer->prefix->get_size());
            // note: this assumes prefix is ascii:
            renderer->column = renderer->prefix->get_size();
        }

        code_point_length = cmark_utf8proc_iterate(
            static_cast<const uint8_t*>(static_cast<const void*>(source_text + source_index)),
            length - source_index, &code_point);
        if (code_point_length == -1)
        {
            // error condition
            return;  // return without rendering rest of string
        }

        if (extension && extension->commonmark_escape_func(extension, node, code_point))
        {
            renderer->buffer->Putc('\\');
        }

        next_character = source_text[source_index + code_point_length];
        if (code_point == 32 && wrap)
        {
            if (!renderer->begin_line)
            {
                last_nonspace = renderer->buffer->get_size();
                renderer->buffer->Putc(' ');
                ++renderer->column;
                renderer->begin_line = false;
                renderer->begin_content = false;
                // skip following spaces
                while (source_text[source_index + 1] == ' ')
                {
                    ++source_index;
                }
                // We don't allow breaks that make a digit the first character
                // because this causes problems with commonmark output.
                if (!cmark_isdigit(source_text[source_index + 1]))
                {
                    renderer->last_breakable = last_nonspace;
                }
            }
        }
        else if (escape == LITERAL)
        {
            if (code_point == 10)
            {
                renderer->buffer->Putc('\n');
                renderer->column = 0;
                renderer->begin_line = true;
                renderer->begin_content = true;
                renderer->last_breakable = 0;
            }
            else
            {
                cmark_render_code_point(renderer, code_point);
                renderer->begin_line = false;
                // we don't set 'begin_content' to false til we've
                // finished parsing a digit.  Reason:  in commonmark
                // we need to escape a potential list marker after
                // a digit:
                renderer->begin_content =
                    renderer->begin_content && cmark_isdigit(static_cast<char>(code_point)) == 1;
            }
        }
        else
        {
            renderer->outc(renderer, node, escape, code_point, next_character);
            renderer->begin_line = false;
            renderer->begin_content =
                renderer->begin_content && cmark_isdigit(static_cast<char>(code_point)) == 1;
        }

        // If adding the character went beyond width, look for an
        // earlier place where the line could be broken:
        if (renderer->width > 0 && renderer->column > renderer->width && !renderer->begin_line &&
            renderer->last_breakable > 0)
        {
            // copy from last_breakable to remainder
            cmark_chunk_set_cstr(&remainder, (char*) renderer->buffer->get_ptr() +
                                                 renderer->last_breakable + 1);
            // truncate at last_breakable
            renderer->buffer->Truncate(renderer->last_breakable);
            // add newline, prefix, and remainder
            renderer->buffer->Putc('\n');
            renderer->buffer->Put(renderer->prefix->get_ptr(), renderer->prefix->get_size());
            renderer->buffer->Put(remainder.data, remainder.len);
            renderer->column = renderer->prefix->get_size() + remainder.len;
            cmark_chunk_free(&remainder);
            renderer->last_breakable = 0;
            renderer->begin_line = false;
            renderer->begin_content = false;
        }

        source_index += code_point_length;
    }
}

// Assumes no newlines, assumes ascii content:
void cmark_render_ascii(cmark_renderer* renderer, const char* source_text)
{
    const int original_size = renderer->buffer->get_size();
    renderer->buffer->Puts(source_text);
    renderer->column += renderer->buffer->get_size() - original_size;
}

void cmark_render_code_point(cmark_renderer* renderer, uint32_t code_point)
{
    cmark_utf8proc_encode_char(code_point, renderer->buffer);
    ++renderer->column;
}

std::string cmark_render(cmark_node* root, int options, int width,
                         void (*output_character)(cmark_renderer*, cmark_node*, cmark_escaping,
                                                  int32_t, unsigned char),
                         int (*render_node)(cmark_renderer* renderer, cmark_node* node,
                                            cmark_event_type event_type, int options))
{
    cmark_strbuf prefix_buffer = cmark_strbuf();
    cmark_strbuf output_buffer = cmark_strbuf();
    cmark_node* current_node = nullptr;
    cmark_event_type event_type = CMARK_EVENT_NONE;
    cmark_iter* node_iterator = cmark_iter_new(root);

    cmark_renderer renderer = { .buffer = &output_buffer,
                                .prefix = &prefix_buffer,
                                .column = 0,
                                .width = width,
                                .need_cr = 0,
                                .last_breakable = 0,
                                .begin_line = true,
                                .begin_content = true,
                                .no_linebreaks = false,
                                .in_tight_list_item = false,
                                .outc = output_character,
                                .cr = SetCarriageReturn,
                                .blankline = SetBlankLine,
                                .out = WriteOutput,
                                .footnote_ix = 0 };

    while ((event_type = cmark_iter_next(node_iterator)) != CMARK_EVENT_DONE)
    {
        current_node = cmark_iter_get_node(node_iterator);
        if (current_node->extension)
        {
            current_node->ancestor_extension = current_node->extension;
        }
        else if (current_node->parent)
        {
            current_node->ancestor_extension = current_node->parent->ancestor_extension;
        }
        if (current_node->type == CMARK_NODE_ITEM)
        {
            // Calculate the list item's index, for the benefit of output formats
            // like commonmark and plaintext.
            if (current_node->prev)
            {
                cmark_node_set_item_index(current_node,
                                          1 + cmark_node_get_item_index(current_node->prev));
            }
            else
            {
                cmark_node_set_item_index(current_node,
                                          cmark_node_get_list_start(current_node->parent));
            }
        }
        if (!render_node(&renderer, current_node, event_type, options))
        {
            // a false value causes us to skip processing
            // the node's contents.  this is used for
            // autolinks.
            cmark_iter_reset(node_iterator, current_node, CMARK_EVENT_EXIT);
        }
    }

    // ensure final newline
    if (renderer.buffer->get_size() == 0 ||
        renderer.buffer->get_ptr()[renderer.buffer->get_size() - 1] != '\n')
    {
        renderer.buffer->Putc('\n');
    }

    std::string result = renderer.buffer->Detach();

    cmark_iter_free(node_iterator);
    renderer.prefix->Free();
    renderer.buffer->Free();

    return result;
}
