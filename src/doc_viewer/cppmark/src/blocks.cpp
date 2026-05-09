// Purpose: Parses block structure - paragraphs, headings, lists, code blocks, blockquotes, etc.
// Parse flow: Read lines -> match block patterns -> build AST of block nodes -> later parse inline
// content Key functions: cmark_parse_document() (entry point), parse_block_starts(), finalize()
// Algorithm: See http://spec.commonmark.org/0.24/#phase-1-block-structure
// Dependencies: All cppmark headers - this is the main parser implementation

/**
 * Block parsing implementation.
 *
 * For a high-level overview of the block parsing process,
 * see http://spec.commonmark.org/0.24/#phase-1-block-structure
 */

#include <array>
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <memory>

#include "buffer.hxx"  // NOLINT (cppcheck-suppress)
#include "cmark-gfm.hxx"
#include "cmark_ctype.hxx"
#include "footnotes.hxx"
#include "houdini.hxx"
#include "inlines.hxx"
#include "node.hxx"
#include "parser.hxx"  // NOLINT (cppcheck-suppress)
#include "references.hxx"
#include "scanners.hxx"
#include "syntax_extension.hxx"  // NOLINT (cppcheck-suppress)
#include "utf8.hxx"

constexpr int CODE_INDENT = 4;
constexpr int TAB_STOP = 4;

// Very deeply nested lists can cause quadratic performance issues. This
// constant is used in open_new_blocks() to limit the nesting depth. It is
// unlikely that a non-contrived markdown document will be nested this deeply.
// Very deeply nested lists can cause quadratic performance issues. This
// constant is used in open_new_blocks() to limit the nesting depth. It is
// unlikely that a non-contrived markdown document will be nested this deeply.
constexpr int MAX_LIST_DEPTH = 100;

template <typename T>
constexpr T MIN(const T& left_value, const T& right_value)
{
    return (left_value < right_value) ? left_value : right_value;
}

inline unsigned char peek_at(const cmark_chunk* input_chunk, size_t offset)
{
    return input_chunk->data[offset];
}

static bool S_last_line_blank(const cmark_node* node)
{
    return (static_cast<uint16_t>(node->flags) &
            static_cast<uint16_t>(CMarkNodeInternalFlags::last_line_blank)) != 0;
}

static bool S_last_line_checked(const cmark_node* node)
{
    return (static_cast<uint16_t>(node->flags) &
            static_cast<uint16_t>(CMarkNodeInternalFlags::last_line_checked)) != 0;
}

static cmark_node_type S_type(const cmark_node* node)
{
    return (cmark_node_type) node->type;
}

static void S_set_last_line_blank(cmark_node* node, bool is_blank)
{
    if (is_blank)
    {
        node->flags = static_cast<CMarkNodeInternalFlags>(
            static_cast<uint16_t>(node->flags) |
            static_cast<uint16_t>(CMarkNodeInternalFlags::last_line_blank));
    }
    else
    {
        node->flags = static_cast<CMarkNodeInternalFlags>(
            static_cast<uint16_t>(node->flags) &
            ~static_cast<uint16_t>(CMarkNodeInternalFlags::last_line_blank));
    }
}

static void S_set_last_line_checked(cmark_node* node)
{
    node->flags = static_cast<CMarkNodeInternalFlags>(
        static_cast<uint16_t>(node->flags) |
        static_cast<uint16_t>(CMarkNodeInternalFlags::last_line_checked));
}

static bool S_is_line_end_char(char character)
{
    return (character == '\n' || character == '\r');
}

static bool S_is_space_or_tab(char character)
{
    return (character == ' ' || character == '\t');
}

static void S_parser_feed(cmark_parser* parser, const unsigned char* buffer, size_t buffer_length,
                          bool end_of_file);

static void S_process_line(cmark_parser* parser, const unsigned char* buffer, size_t bytes);

static cmark_node* make_block(cmark_node_type tag, int start_line, int start_column)
{
    std::unique_ptr<cmark_node> node_ptr = std::make_unique<cmark_node>();
    cmark_node* node = node_ptr.get();

    node->content.Init(32);
    node->type = (uint16_t) tag;
    node->flags = CMarkNodeInternalFlags::open;
    node->start_line = start_line;
    node->start_column = start_column;
    node->end_line = start_line;

    return node_ptr.release();
}

// Create a root document node.
static cmark_node* make_document()
{
    cmark_node* document_node = make_block(CMARK_NODE_DOCUMENT, 1, 1);
    return document_node;
}

int cmark_parser_attach_syntax_extension(cmark_parser* parser, cmark_syntax_extension* extension)
{
    parser->syntax_extensions = cmark_llist_append(parser->syntax_extensions, extension);
    if (extension->match_inline || extension->insert_inline_from_delim)
    {
        parser->inline_syntax_extensions =
            cmark_llist_append(parser->inline_syntax_extensions, extension);
    }

    return 1;
}

static void cmark_parser_dispose(cmark_parser* parser)
{
    if (parser->root)
    {
        cmark_node_free(parser->root);
    }

    if (parser->refmap)
    {
        cmark_map_free(parser->refmap);
    }
}

static void cmark_parser_reset(cmark_parser* parser)
{
    cmark_llist* saved_exts = parser->syntax_extensions;
    cmark_llist* saved_inline_exts = parser->inline_syntax_extensions;
    int saved_options = parser->options;

    cmark_parser_dispose(parser);

    memset((void*) parser, 0, sizeof(cmark_parser));

    parser->curline.Init(256);
    parser->linebuf.Init(0);

    cmark_node* document = make_document();

    parser->refmap = cmark_reference_map_new();
    parser->root = document;
    parser->current = document;

    parser->syntax_extensions = saved_exts;
    parser->inline_syntax_extensions = saved_inline_exts;
    parser->options = saved_options;
}

cmark_parser* cmark_parser_new_with_mem(int options)
{
    auto parser_ptr = std::make_unique<cmark_parser>();
    cmark_parser* parser = parser_ptr.get();
    parser->options = options;
    cmark_parser_reset(parser);
    return parser_ptr.release();
}

cmark_parser* cmark_parser_new(int options)
{
    return cmark_parser_new_with_mem(options);
}

void cmark_parser_free(cmark_parser* parser)
{
    cmark_parser_dispose(parser);
    parser->curline.Free();
    parser->linebuf.Free();
    cmark_llist_free(parser->syntax_extensions);
    cmark_llist_free(parser->inline_syntax_extensions);
    delete parser;
}

static cmark_node* finalize(cmark_parser* parser, cmark_node* block);

// Returns true if line has only space characters, else false.
static bool is_blank(cmark_strbuf* text_buffer, size_t offset)
{
    while (offset < text_buffer->get_size())
    {
        switch (text_buffer->get_ptr()[offset])
        {
            case '\r':
            case '\n':
                return true;
            case ' ':
                offset++;
                break;
            case '\t':
                offset++;
                break;
            default:
                return false;
        }
    }

    return true;
}

static bool accepts_lines(cmark_node_type block_type)
{
    return (block_type == CMARK_NODE_PARAGRAPH || block_type == CMARK_NODE_HEADING ||
            block_type == CMARK_NODE_CODE_BLOCK);
}

static bool contains_inlines(cmark_node* node)
{
    if (node->extension && node->extension->contains_inlines_func)
    {
        return node->extension->contains_inlines_func(node->extension, node) != 0;
    }

    return (node->type == CMARK_NODE_PARAGRAPH || node->type == CMARK_NODE_HEADING);
}

static void add_line(cmark_node* node, cmark_chunk* input_chunk, cmark_parser* parser)
{
    int spaces_to_tab_stop = 0;
    assert(static_cast<uint16_t>(node->flags) &
           static_cast<uint16_t>(CMarkNodeInternalFlags::open));
    if (parser->partially_consumed_tab)
    {
        ++parser->offset;  // skip over tab
        // add space characters:
        spaces_to_tab_stop = TAB_STOP - (parser->column % TAB_STOP);
        for (int space_index = 0; space_index < spaces_to_tab_stop; ++space_index)
        {
            node->content.Putc(' ');
        }
    }
    node->content.Put(input_chunk->data + parser->offset, input_chunk->len - parser->offset);
}

static void remove_trailing_blank_lines(cmark_strbuf* line_buffer)
{
    size_t scan_index = 0;
    unsigned char current_char = '\0';

    for (scan_index = line_buffer->get_size() - 1; scan_index >= 0; --scan_index)
    {
        current_char = line_buffer->get_ptr()[scan_index];

        if (current_char != ' ' && current_char != '\t' && !S_is_line_end_char(current_char))
        {
            break;
        }
    }

    if (scan_index < 0)
    {
        line_buffer->Clear();
        return;
    }

    for (; scan_index < line_buffer->get_size(); ++scan_index)
    {
        current_char = line_buffer->get_ptr()[scan_index];

        if (!S_is_line_end_char(current_char))
        {
            continue;
        }

        line_buffer->Truncate(scan_index);
        break;
    }
}

// Check to see if a node ends with a blank line, descending
// if needed into lists and sublists.
static bool S_ends_with_blank_line(cmark_node* node)
{
    if (S_last_line_checked(node))
    {
        return (S_last_line_blank(node));
    }
    if ((S_type(node) == CMARK_NODE_LIST || S_type(node) == CMARK_NODE_ITEM) && node->last_child)
    {
        S_set_last_line_checked(node);
        return (S_ends_with_blank_line(node->last_child));
    }

    S_set_last_line_checked(node);
    return (S_last_line_blank(node));
}

// returns true if content remains after link defs are resolved.
static bool resolve_reference_link_definitions(cmark_parser* parser, cmark_node* block)
{
    size_t parsed_bytes = 0;
    cmark_strbuf* node_content = &block->content;
    cmark_chunk chunk = { .data = node_content->get_ptr(),
                          .len = static_cast<size_t>(node_content->get_size()),
                          .alloc = 0 };
    while (chunk.len && chunk.data[0] == '[' &&
           (parsed_bytes = cmark_parse_reference_inline(&chunk, parser->refmap)))
    {
        chunk.data += parsed_bytes;
        chunk.len -= parsed_bytes;
    }
    node_content->Drop((node_content->get_size() - chunk.len));
    return !is_blank(&block->content, 0);
}

static cmark_node* finalize(cmark_parser* parser, cmark_node* block)
{
    size_t content_offset = 0;
    cmark_node* list_item = nullptr;
    cmark_node* child_item = nullptr;
    cmark_node* parent_node = nullptr;
    bool has_content = false;

    parent_node = block->parent;
    assert(static_cast<uint16_t>(block->flags) &
           static_cast<uint16_t>(
               CMarkNodeInternalFlags::open));  // shouldn't call finalize on closed blocks
    block->flags = static_cast<CMarkNodeInternalFlags>(
        static_cast<uint16_t>(block->flags) & ~static_cast<uint16_t>(CMarkNodeInternalFlags::open));

    if (parser->curline.get_size() == 0)
    {
        // end of input - line number has not been incremented
        block->end_line = parser->line_number;
        block->end_column = parser->last_line_length;
    }
    else if (S_type(block) == CMARK_NODE_DOCUMENT ||
             (S_type(block) == CMARK_NODE_CODE_BLOCK && block->as.code.fenced) ||
             (S_type(block) == CMARK_NODE_HEADING && block->as.heading.setext))
    {
        block->end_line = parser->line_number;
        block->end_column = parser->curline.get_size();
        if (block->end_column && parser->curline.get_ptr()[block->end_column - 1] == '\n')
        {
            block->end_column -= 1;
        }
        if (block->end_column && parser->curline.get_ptr()[block->end_column - 1] == '\r')
        {
            block->end_column -= 1;
        }
    }
    else
    {
        block->end_line = parser->line_number - 1;
        block->end_column = parser->last_line_length;
    }

    cmark_strbuf* node_content = &block->content;

    switch (S_type(block))
    {
        case CMARK_NODE_PARAGRAPH:
            {
                has_content = resolve_reference_link_definitions(parser, block);
                if (!has_content)
                {
                    // remove blank node (former reference def)
                    cmark_node_free(block);
                }
                break;
            }

        case CMARK_NODE_CODE_BLOCK:
            if (!block->as.code.fenced)
            {  // indented code
                remove_trailing_blank_lines(node_content);
                node_content->Putc('\n');
            }
            else
            {
                // first line of contents becomes info
                for (content_offset = 0;
                     static_cast<size_t>(content_offset) < node_content->get_size();
                     ++content_offset)
                {
                    if (S_is_line_end_char(node_content->get_ptr()[content_offset]))
                    {
                        break;
                    }
                }
                assert(static_cast<size_t>(content_offset) < node_content->get_size());

                cmark_strbuf info_buffer = cmark_strbuf();
                houdini_unescape_html_f(&info_buffer, node_content->get_ptr(), content_offset);
                info_buffer.Trim();
                info_buffer.Unescape();
                block->as.code.info = cmark_chunk_buf_detach(&info_buffer);

                if (node_content->get_ptr()[content_offset] == '\r')
                {
                    ++content_offset;
                }
                if (node_content->get_ptr()[content_offset] == '\n')
                {
                    ++content_offset;
                }
                node_content->Drop(content_offset);
            }
            block->as.code.literal = cmark_chunk_buf_detach(node_content);
            break;

        case CMARK_NODE_HTML_BLOCK:
            block->as.literal = cmark_chunk_buf_detach(node_content);
            break;

        case CMARK_NODE_LIST:             // determine tight/loose status
            block->as.list.tight = true;  // tight by default
            list_item = block->first_child;

            while (list_item)
            {
                // check for non-final non-empty list item ending with blank line:
                if (S_last_line_blank(list_item) && list_item->next)
                {
                    block->as.list.tight = false;
                    break;
                }
                // recurse into children of list item, to see if there are
                // spaces between them:
                child_item = list_item->first_child;
                while (child_item)
                {
                    if ((list_item->next || child_item->next) && S_ends_with_blank_line(child_item))
                    {
                        block->as.list.tight = false;
                        break;
                    }
                    child_item = child_item->next;
                }
                if (!(block->as.list.tight))
                {
                    break;
                }
                list_item = list_item->next;
            }

            break;

        default:
            break;
    }

    return parent_node;
}

// Add a node as child of another.  Return pointer to child.
static cmark_node* add_child(cmark_parser* parser, cmark_node* parent, cmark_node_type block_type,
                             int start_column)
{
    assert(parent);

    // if 'parent' isn't the kind of node that can accept this child,
    // then back up til we hit a node that can.
    while (!cmark_node_can_contain_type(parent, block_type))
    {
        parent = finalize(parser, parent);
    }

    cmark_node* child = make_block(block_type, parser->line_number, start_column);
    child->parent = parent;

    if (parent->last_child)
    {
        parent->last_child->next = child;
        child->prev = parent->last_child;
    }
    else
    {
        parent->first_child = child;
        child->prev = nullptr;
    }
    parent->last_child = child;
    return child;
}

void cmark_manage_extensions_special_characters(cmark_parser* parser, int add)
{
    cmark_llist* tmp_ext = nullptr;

    for (tmp_ext = parser->inline_syntax_extensions; tmp_ext; tmp_ext = tmp_ext->next)
    {
        cmark_syntax_extension* ext = static_cast<cmark_syntax_extension*>(tmp_ext->data);
        cmark_llist* tmp_char = nullptr;
        for (tmp_char = ext->special_inline_chars; tmp_char; tmp_char = tmp_char->next)
        {
            unsigned char character =
                (unsigned char) (size_t) tmp_char->data;  // NOLINT(cppcheck-suppress)
            if (add)
            {
                cmark_inlines_add_special_character(character, ext->emphasis);
            }
            else
            {
                cmark_inlines_remove_special_character(character, ext->emphasis);
            }
        }
    }
}

// Walk through node and all children, recursively, parsing
// string content into inline content where appropriate.
static void process_inlines(cmark_parser* parser, cmark_map* refmap, int options)
{
    cmark_iter* iter = cmark_iter_new(parser->root);
    cmark_node* cur = nullptr;
    cmark_event_type ev_type = CMARK_EVENT_DONE;

    cmark_manage_extensions_special_characters(parser, true);

    while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE)
    {
        cur = cmark_iter_get_node(iter);
        if (ev_type == CMARK_EVENT_ENTER)
        {
            if (contains_inlines(cur))
            {
                cmark_parse_inlines(parser, cur, refmap, options);
            }
        }
    }

    cmark_manage_extensions_special_characters(parser, false);

    cmark_iter_free(iter);
}

static int sort_footnote_by_ix(const void* arg_a, const void* arg_b)
{
    cmark_footnote* footnote_a = *(cmark_footnote**) arg_a;  // NOLINT(cppcheck-suppress)
    cmark_footnote* footnote_b = *(cmark_footnote**) arg_b;  // NOLINT(cppcheck-suppress)
    return (int) footnote_a->ix - (int) footnote_b->ix;
}

static void process_footnotes(cmark_parser* parser)
{
    // * Collect definitions in a map.
    // * Iterate the references in the document in order, assigning indices to
    //   definitions in the order they're seen.
    // * Write out the footnotes at the bottom of the document in index order.

    cmark_map* footnote_map = cmark_footnote_map_new();

    cmark_iter* iter = cmark_iter_new(parser->root);
    cmark_node* current_node = nullptr;
    cmark_event_type ev_type = CMARK_EVENT_DONE;

    while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE)
    {
        current_node = cmark_iter_get_node(iter);
        if (ev_type == CMARK_EVENT_EXIT && current_node->type == CMARK_NODE_FOOTNOTE_DEFINITION)
        {
            cmark_footnote_create(footnote_map, current_node);
        }
    }

    cmark_iter_free(iter);
    iter = cmark_iter_new(parser->root);
    unsigned int next_footnote_index = 0;

    while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE)
    {
        current_node = cmark_iter_get_node(iter);
        if (ev_type == CMARK_EVENT_EXIT && current_node->type == CMARK_NODE_FOOTNOTE_REFERENCE)
        {
            cmark_footnote* footnote =
                (cmark_footnote*) cmark_map_lookup(footnote_map, &current_node->as.literal);
            if (footnote)
            {
                if (!footnote->ix)
                {
                    footnote->ix = ++next_footnote_index;
                }

                // store a reference to this footnote reference's footnote definition
                // this is used by renderers when generating label ids
                current_node->parent_footnote_def = footnote->node;

                // keep track of a) count of how many times this footnote def has been
                // referenced, and b) which reference index this footnote ref is at.
                // this is used by renderers when generating links and backreferences.
                current_node->footnote.ref_ix = ++footnote->node->footnote.def_count;

                std::array<char, 32> format_buffer = {};
                snprintf(format_buffer.data(), format_buffer.size(), "%d", footnote->ix);
                cmark_chunk_free(&current_node->as.literal);
                cmark_strbuf replacement_text = cmark_strbuf();
                replacement_text.Puts(format_buffer.data());

                current_node->as.literal = cmark_chunk_buf_detach(&replacement_text);
            }
            else
            {
                std::unique_ptr<cmark_node> text_node_ptr = std::make_unique<cmark_node>();
                cmark_node* text_node = text_node_ptr.get();
                text_node->content.Init(0);
                text_node->type = (uint16_t) CMARK_NODE_TEXT;

                cmark_strbuf replacement_text = cmark_strbuf();
                replacement_text.Puts("[^");
                replacement_text.Put(current_node->as.literal.data, current_node->as.literal.len);
                replacement_text.Putc(']');

                text_node->as.literal = cmark_chunk_buf_detach(&replacement_text);
                cmark_node_insert_after(current_node, text_node_ptr.release());
                cmark_node_free(current_node);
            }
        }
    }

    cmark_iter_free(iter);

    if (footnote_map->sorted)
    {
        qsort(footnote_map->sorted, footnote_map->size, sizeof(cmark_map_entry*),
              sort_footnote_by_ix);
        for (unsigned int footnote_index = 0; footnote_index < footnote_map->size; ++footnote_index)
        {
            cmark_footnote* footnote = (cmark_footnote*) footnote_map->sorted[footnote_index];
            if (!footnote->ix)
            {
                cmark_node_unlink(footnote->node);
                continue;
            }
            cmark_node_append_child(parser->root, footnote->node);
            footnote->node = nullptr;
        }
    }

    cmark_unlink_footnotes_map(footnote_map);
    cmark_map_free(footnote_map);
}

// Attempts to parse a list item marker (bullet or enumerated).
// On success, returns length of the marker, and populates
// data with the details.  On failure, returns 0.
static size_t parse_list_marker(cmark_chunk* input, size_t pos, bool interrupts_paragraph,
                                cmark_list** dataptr)
{
    unsigned char marker_char = '\0';
    size_t startpos = 0;
    cmark_list* data = nullptr;
    size_t scan_pos = 0;

    startpos = pos;
    marker_char = peek_at(input, pos);

    if (marker_char == '*' || marker_char == '-' || marker_char == '+')
    {
        pos++;
        if (!cmark_isspace(peek_at(input, pos)))
        {
            return 0;
        }

        if (interrupts_paragraph)
        {
            scan_pos = pos;
            // require non-blank content after list marker:
            while (S_is_space_or_tab(peek_at(input, scan_pos)))
            {
                scan_pos++;
            }
            if (peek_at(input, scan_pos) == '\n')
            {
                return 0;
            }
        }

        auto data_ptr = std::make_unique<cmark_list>();
        data = data_ptr.get();
        data->marker_offset = 0;  // will be adjusted later
        data->list_type = CMARK_BULLET_LIST;
        data->bullet_char = marker_char;
        data->start = 0;
        data->delimiter = CMARK_NO_DELIM;
        data->tight = false;
        data_ptr.release();  // Will be freed later or copied to node
    }
    else if (cmark_isdigit(marker_char))
    {
        int start = 0;
        int digits = 0;

        do
        {
            start = (10 * start) + (peek_at(input, pos) - '0');
            pos++;
            digits++;
            // We limit to 9 digits to avoid overflow,
            // assuming max int is 2^31 - 1
            // This also seems to be the limit for 'start' in some browsers.
        } while (digits < 9 && cmark_isdigit(peek_at(input, pos)));

        if (interrupts_paragraph && start != 1)
        {
            return 0;
        }
        marker_char = peek_at(input, pos);
        if (marker_char == '.' || marker_char == ')')
        {
            pos++;
            if (!cmark_isspace(peek_at(input, pos)))
            {
                return 0;
            }
            if (interrupts_paragraph)
            {
                // require non-blank content after list marker:
                scan_pos = pos;
                while (S_is_space_or_tab(peek_at(input, scan_pos)))
                {
                    scan_pos++;
                }
                if (S_is_line_end_char(peek_at(input, scan_pos)))
                {
                    return 0;
                }
            }

            auto data_ptr = std::make_unique<cmark_list>();
            data = data_ptr.get();
            data->marker_offset = 0;  // will be adjusted later
            data->list_type = CMARK_ORDERED_LIST;
            data->bullet_char = 0;
            data->start = start;
            data->delimiter = (marker_char == '.' ? CMARK_PERIOD_DELIM : CMARK_PAREN_DELIM);
            data->tight = false;
            data_ptr.release();  // Will be freed later or copied to node
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }

    *dataptr = data;
    return (pos - startpos);
}

// Return 1 if list item belongs in list, else 0.
static int lists_match(cmark_list* list_data, cmark_list* item_data)
{
    return (list_data->list_type == item_data->list_type &&
            list_data->delimiter == item_data->delimiter &&
            // list_data->marker_offset == item_data.marker_offset &&
            list_data->bullet_char == item_data->bullet_char);
}

static cmark_node* finalize_document(cmark_parser* parser)
{
    while (parser->current != parser->root)
    {
        parser->current = finalize(parser, parser->current);
    }

    finalize(parser, parser->root);

    // Limit total size of extra content created from reference links to
    // document size to avoid superlinear growth. Always allow 100KB.
    if (parser->total_size > 100000)
    {
        parser->refmap->max_ref_size = parser->total_size;
    }
    else
    {
        parser->refmap->max_ref_size = 100000;
    }

    process_inlines(parser, parser->refmap, parser->options);
    if (parser->options & CMARK_OPT_FOOTNOTES)
    {
        process_footnotes(parser);
    }

    return parser->root;
}

cmark_node* cmark_parse_file(FILE* file, int options)
{
    std::array<unsigned char, 4096> buffer = {};
    cmark_parser* parser = cmark_parser_new(options);
    size_t bytes = 0;
    cmark_node* document = nullptr;

    while ((bytes = fread(buffer.data(), 1, buffer.size(), file)) > 0)
    {
        bool eof = bytes < buffer.size();
        S_parser_feed(parser, buffer.data(), bytes, eof);
        if (eof)
        {
            break;
        }
    }

    document = cmark_parser_finish(parser);
    cmark_parser_free(parser);
    return document;
}

cmark_node* cmark_parse_document(const char* buffer, size_t len, int options)
{
    cmark_parser* parser = cmark_parser_new(options);
    cmark_node* document = nullptr;

    S_parser_feed(parser, (const unsigned char*) buffer, len, true);

    document = cmark_parser_finish(parser);
    cmark_parser_free(parser);
    return document;
}

void cmark_parser_feed(cmark_parser* parser, const char* buffer, size_t len)
{
    S_parser_feed(parser, (const unsigned char*) buffer, len, false);
}

void cmark_parser_feed_reentrant(cmark_parser* parser, const char* buffer, size_t len)
{
    cmark_strbuf saved_linebuf;

    saved_linebuf.Init(0);
    saved_linebuf.Puts(parser->linebuf.CStr());
    parser->linebuf.Clear();

    S_parser_feed(parser, (const unsigned char*) buffer, len, true);

    parser->linebuf.Sets(saved_linebuf.CStr());
    saved_linebuf.Free();
}

static void S_parser_feed(cmark_parser* parser, const unsigned char* buffer, size_t buffer_length,
                          bool end_of_file)
{
    const unsigned char* buffer_end = buffer + buffer_length;
    static const uint8_t repl[] = { 239, 191, 189 };

    if (buffer_length > UINT_MAX - parser->total_size)
    {
        parser->total_size = UINT_MAX;
    }
    else
    {
        parser->total_size += buffer_length;
    }

    if (parser->last_buffer_ended_with_cr && *buffer == '\n')
    {
        // skip NL if last buffer ended with CR ; see #117
        buffer++;
    }
    parser->last_buffer_ended_with_cr = false;
    while (buffer < buffer_end)
    {
        const unsigned char* line_end = nullptr;
        size_t chunk_length = 0;
        bool should_process_line = false;
        for (line_end = buffer; line_end < buffer_end; ++line_end)
        {
            if (S_is_line_end_char(*line_end))
            {
                should_process_line = true;
                break;
            }
            if (*line_end == '\0' && line_end < buffer_end)
            {
                break;
            }
        }
        if (line_end >= buffer_end && end_of_file)
        {
            should_process_line = true;
        }

        chunk_length = static_cast<size_t>(line_end - buffer);
        if (should_process_line)
        {
            if (parser->linebuf.get_size() > 0)
            {
                parser->linebuf.Put(buffer, chunk_length);
                S_process_line(parser, parser->linebuf.get_ptr(), parser->linebuf.get_size());
                parser->linebuf.Clear();
            }
            else
            {
                S_process_line(parser, buffer, chunk_length);
            }
        }
        else
        {
            if (line_end < buffer_end && *line_end == '\0')
            {
                // omit NULL byte
                parser->linebuf.Put(buffer, chunk_length);
                // add replacement character
                parser->linebuf.Put(repl, 3);
            }
            else
            {
                parser->linebuf.Put(buffer, chunk_length);
            }
        }

        buffer += chunk_length;
        if (buffer < buffer_end)
        {
            if (*buffer == '\0')
            {
                // skip over NULL
                buffer++;
            }
            else
            {
                // skip over line ending characters
                if (*buffer == '\r')
                {
                    buffer++;
                    if (buffer == buffer_end)
                    {
                        parser->last_buffer_ended_with_cr = true;
                    }
                }
                if (buffer < buffer_end && *buffer == '\n')
                {
                    buffer++;
                }
            }
        }
    }
}

static void chop_trailing_hashtags(cmark_chunk* ch)
{
    size_t format_buffer_orig = 0;
    size_t format_buffer = 0;

    cmark_chunk_rtrim(ch);
    format_buffer_orig = format_buffer = ch->len - 1;

    // if string ends in space followed by #s, remove these:
    while (format_buffer >= 0 && peek_at(ch, format_buffer) == '#')
    {
        format_buffer--;
    }

    // Check for a space before the final #s:
    if (format_buffer != format_buffer_orig && format_buffer >= 0 &&
        S_is_space_or_tab(peek_at(ch, format_buffer)))
    {
        ch->len = format_buffer;
        cmark_chunk_rtrim(ch);
    }
}

// Check for thematic break.  On failure, return 0 and update
// thematic_break_kill_pos with the index at which the
// parse fails.  On success, return length of match.
// "...three or more hyphens, asterisks,
// or underscores on a line by themselves. If you wish, you may use
// spaces between the hyphens or asterisks."
static int S_scan_thematic_break(cmark_parser* parser, cmark_chunk* input, size_t offset)
{
    size_t scan_index = 0;
    char start_char = '\0';
    char nextc = '\0';
    int count = 0;
    scan_index = offset;
    start_char = peek_at(input, scan_index);
    if (start_char != '*' && start_char != '_' && start_char != '-')
    {
        parser->thematic_break_kill_pos = scan_index;
        return 0;
    }
    count = 1;
    while ((nextc = peek_at(input, ++scan_index)))
    {
        if (nextc == start_char)
        {
            count++;
        }
        else if (nextc != ' ' && nextc != '\t')
        {
            break;
        }
    }
    if (count >= 3 && (nextc == '\r' || nextc == '\n'))
    {
        return (scan_index - offset) + 1;
    }

    parser->thematic_break_kill_pos = scan_index;
    return 0;
}

// Find first nonspace character from current offset, setting
// parser->first_nonspace, parser->first_nonspace_column,
// parser->indent, and parser->blank. Does not advance parser->offset.
static void S_find_first_nonspace(cmark_parser* parser, cmark_chunk* input)
{
    char current_char = '\0';
    int spaces_to_next_tab_stop = TAB_STOP - (parser->column % TAB_STOP);

    if (parser->first_nonspace <= parser->offset)
    {
        parser->first_nonspace = parser->offset;
        parser->first_nonspace_column = parser->column;
        while ((current_char = peek_at(input, parser->first_nonspace)))
        {
            if (current_char == ' ')
            {
                ++parser->first_nonspace;
                ++parser->first_nonspace_column;
                spaces_to_next_tab_stop = spaces_to_next_tab_stop - 1;
                if (spaces_to_next_tab_stop == 0)
                {
                    spaces_to_next_tab_stop = TAB_STOP;
                }
            }
            else if (current_char == '\t')
            {
                ++parser->first_nonspace;
                parser->first_nonspace_column += spaces_to_next_tab_stop;
                spaces_to_next_tab_stop = TAB_STOP;
            }
            else
            {
                break;
            }
        }
    }

    parser->indent = parser->first_nonspace_column - parser->column;
    parser->blank = S_is_line_end_char(peek_at(input, parser->first_nonspace));
}

// Advance parser->offset and parser->column.  parser->offset is the
// byte position in input; parser->column is a virtual column number
// that takes into account tabs. (Multibyte characters are not taken
// into account, because the Markdown line prefixes we are interested in
// analyzing are entirely ASCII.)  The count parameter indicates
// how far to advance the offset.  If columns is true, then count
// indicates a number of columns; otherwise, a number of bytes.
// If advancing a certain number of columns partially consumes
// a tab character, parser->partially_consumed_tab is set to true.
static void S_advance_offset(cmark_parser* parser, cmark_chunk* input, size_t count, bool columns)
{
    char current_char = '\0';
    int spaces_to_next_tab_stop = 0;
    int columns_to_advance = 0;
    while (count > 0 && (current_char = peek_at(input, parser->offset)))
    {
        if (current_char == '\t')
        {
            spaces_to_next_tab_stop = TAB_STOP - (parser->column % TAB_STOP);
            if (columns)
            {
                parser->partially_consumed_tab =
                    static_cast<size_t>(spaces_to_next_tab_stop) > count;
                columns_to_advance =
                    static_cast<size_t>(std::min(static_cast<int>(count), spaces_to_next_tab_stop));
                parser->column += columns_to_advance;
                parser->offset += (parser->partially_consumed_tab ? 0 : 1);
                count -= columns_to_advance;
            }
            else
            {
                parser->partially_consumed_tab = false;
                parser->column += spaces_to_next_tab_stop;
                ++parser->offset;
                count -= 1;
            }
        }
        else
        {
            parser->partially_consumed_tab = false;
            ++parser->offset;
            ++parser->column;  // assume ascii; block starts are ascii
            count -= 1;
        }
    }
}

static bool S_last_child_is_open(cmark_node* container)
{
    return container->last_child && (static_cast<uint16_t>(container->last_child->flags) &
                                     static_cast<uint16_t>(CMarkNodeInternalFlags::open));
}

static bool parse_block_quote_prefix(cmark_parser* parser, cmark_chunk* input)
{
    bool res = false;
    size_t matched = 0;

    matched = parser->indent <= 3 && peek_at(input, parser->first_nonspace) == '>';
    if (matched)
    {
        S_advance_offset(parser, input, parser->indent + 1, true);

        if (S_is_space_or_tab(peek_at(input, parser->offset)))
        {
            S_advance_offset(parser, input, 1, true);
        }

        res = true;
    }
    return res;
}

static bool parse_footnote_definition_block_prefix(cmark_parser* parser, cmark_chunk* input,
                                                   [[maybe_unused]] cmark_node* container)
{
    if (parser->indent >= 4)
    {
        S_advance_offset(parser, input, 4, true);
        return true;
    }
    if (input->len > 0 &&
        (input->data[0] == '\n' || (input->data[0] == '\r' && input->data[1] == '\n')))
    {
        return true;
    }

    return false;
}

static bool parse_node_item_prefix(cmark_parser* parser, cmark_chunk* input, cmark_node* container)
{
    bool res = false;

    if (parser->indent >= container->as.list.marker_offset + container->as.list.padding)
    {
        S_advance_offset(parser, input,
                         container->as.list.marker_offset + container->as.list.padding, true);
        res = true;
    }
    else if (parser->blank && container->first_child != nullptr)
    {
        // if container->first_child is NULL, then the opening line
        // of the list item was blank after the list marker; in this
        // case, we are done with the list item.
        S_advance_offset(parser, input, parser->first_nonspace - parser->offset, false);
        res = true;
    }
    return res;
}

static bool parse_code_block_prefix(cmark_parser* parser, cmark_chunk* input, cmark_node* container,
                                    bool* should_continue)
{
    bool res = false;

    if (!container->as.code.fenced)
    {  // indented
        if (parser->indent >= CODE_INDENT)
        {
            S_advance_offset(parser, input, CODE_INDENT, true);
            res = true;
        }
        else if (parser->blank)
        {
            S_advance_offset(parser, input, parser->first_nonspace - parser->offset, false);
            res = true;
        }
    }
    else
    {  // fenced
        size_t matched = 0;

        if (parser->indent <= 3 &&
            (peek_at(input, parser->first_nonspace) == container->as.code.fence_char))
        {
            matched = scan_close_code_fence(input, parser->first_nonspace);
        }

        if (matched >= container->as.code.fence_length)
        {
            // closing fence - and since we're at
            // the end of a line, we can stop processing it:
            *should_continue = false;
            S_advance_offset(parser, input, matched, false);
            parser->current = finalize(parser, container);
        }
        else
        {
            // skip opt. spaces of fence parser->offset
            int remaining_fence_offset = container->as.code.fence_offset;

            while (remaining_fence_offset > 0 && S_is_space_or_tab(peek_at(input, parser->offset)))
            {
                S_advance_offset(parser, input, 1, true);
                --remaining_fence_offset;
            }
            res = true;
        }
    }

    return res;
}

static bool parse_html_block_prefix(cmark_parser* parser, cmark_node* container)
{
    bool res = false;
    int html_block_type = container->as.html_block_type;

    assert(html_block_type >= 1 && html_block_type <= 7);
    switch (html_block_type)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            // these types of blocks can accept blanks
            res = true;
            break;
        case 6:
        case 7:
            res = !parser->blank;
            break;
    }

    return res;
}

static bool parse_extension_block(cmark_parser* parser, cmark_node* container, cmark_chunk* input)
{
    bool res = false;

    if (container->extension->last_block_matches)
    {
        if (container->extension->last_block_matches(container->extension, parser, input->data,
                                                     input->len, container))
        {
            res = true;
        }
    }

    return res;
}

/**
 * For each containing node, try to parse the associated line start.
 *
 * Will not close unmatched blocks, as we may have a lazy continuation
 * line -> http://spec.commonmark.org/0.24/#lazy-continuation-line
 *
 * Returns: The last matching node, or NULL
 */
static cmark_node* check_open_blocks(cmark_parser* parser, cmark_chunk* input, bool* all_matched)
{
    bool should_continue = true;
    *all_matched = false;
    cmark_node* container = parser->root;
    cmark_node_type container_type = CMARK_NODE_NONE;
    bool done = false;

    while (S_last_child_is_open(container) && !done)
    {
        container = container->last_child;
        container_type = S_type(container);

        S_find_first_nonspace(parser, input);

        if (container->extension)
        {
            if (!parse_extension_block(parser, container, input))
            {
                done = true;
                continue;
            }
            continue;
        }

        switch (container_type)
        {
            case CMARK_NODE_BLOCK_QUOTE:
                if (!parse_block_quote_prefix(parser, input))
                {
                    done = true;
                }
                break;
            case CMARK_NODE_ITEM:
                if (!parse_node_item_prefix(parser, input, container))
                {
                    done = true;
                }
                break;
            case CMARK_NODE_CODE_BLOCK:
                if (!parse_code_block_prefix(parser, input, container, &should_continue))
                {
                    done = true;
                }
                break;
            case CMARK_NODE_HEADING:
                // a heading can never contain more than one line
                done = true;
                break;
            case CMARK_NODE_HTML_BLOCK:
                if (!parse_html_block_prefix(parser, container))
                {
                    done = true;
                }
                break;
            case CMARK_NODE_PARAGRAPH:
                if (parser->blank)
                {
                    done = true;
                }
                break;
            case CMARK_NODE_FOOTNOTE_DEFINITION:
                if (!parse_footnote_definition_block_prefix(parser, input, container))
                {
                    done = true;
                }
                break;
            default:
                break;
        }
    }

    if (!done)
    {
        *all_matched = true;
    }

    if (!*all_matched)
    {
        container = container->parent;  // back up to last matching node
    }

    if (!should_continue)
    {
        container = nullptr;
    }

    return container;
}

static void open_new_blocks(cmark_parser* parser, cmark_node** container, cmark_chunk* input,
                            bool all_matched)
{
    bool indented = false;
    cmark_list* list_data = nullptr;
    bool maybe_lazy = S_type(parser->current) == CMARK_NODE_PARAGRAPH;
    cmark_node_type container_type = S_type(*container);
    size_t matched = 0;
    int heading_level = 0;
    bool save_partially_consumed_tab = false;
    bool has_content = false;
    int save_offset = 0;
    int save_column = 0;
    size_t depth = 0;

    while (container_type != CMARK_NODE_CODE_BLOCK && container_type != CMARK_NODE_HTML_BLOCK)
    {
        depth++;
        S_find_first_nonspace(parser, input);
        indented = parser->indent >= CODE_INDENT;

        if (!indented && peek_at(input, parser->first_nonspace) == '>')
        {
            size_t blockquote_startpos = parser->first_nonspace;

            S_advance_offset(parser, input, parser->first_nonspace + 1 - parser->offset, false);
            // optional following character
            if (S_is_space_or_tab(peek_at(input, parser->offset)))
            {
                S_advance_offset(parser, input, 1, true);
            }
            *container =
                add_child(parser, *container, CMARK_NODE_BLOCK_QUOTE, blockquote_startpos + 1);
        }
        else if (!indented && (matched = scan_atx_heading_start(input, parser->first_nonspace)))
        {
            size_t hashpos = 0;
            int level = 0;
            size_t heading_startpos = parser->first_nonspace;

            S_advance_offset(parser, input, parser->first_nonspace + matched - parser->offset,
                             false);
            *container = add_child(parser, *container, CMARK_NODE_HEADING, heading_startpos + 1);

            hashpos = cmark_chunk_strchr(input, '#', parser->first_nonspace);

            while (peek_at(input, hashpos) == '#')
            {
                level++;
                hashpos++;
            }

            (*container)->as.heading.level = level;
            (*container)->as.heading.setext = false;
            (*container)->internal_offset = matched;
        }
        else if (!indented && (matched = scan_open_code_fence(input, parser->first_nonspace)))
        {
            *container =
                add_child(parser, *container, CMARK_NODE_CODE_BLOCK, parser->first_nonspace + 1);
            (*container)->as.code.fenced = true;
            (*container)->as.code.fence_char = peek_at(input, parser->first_nonspace);
            (*container)->as.code.fence_length =
                (matched > 255) ? 255 : static_cast<uint8_t>(matched);
            (*container)->as.code.fence_offset =
                static_cast<int8_t>(parser->first_nonspace - parser->offset);
            (*container)->as.code.info = cmark_chunk_literal("");
            S_advance_offset(parser, input, parser->first_nonspace + matched - parser->offset,
                             false);
        }
        else if (!indented &&
                 ((matched = scan_html_block_start(input, parser->first_nonspace)) ||
                  (container_type != CMARK_NODE_PARAGRAPH &&
                   (matched = scan_html_block_start_7(input, parser->first_nonspace)))))
        {
            *container =
                add_child(parser, *container, CMARK_NODE_HTML_BLOCK, parser->first_nonspace + 1);
            (*container)->as.html_block_type = matched;
            // note, we don't adjust parser->offset because the tag is part of the
            // text
        }
        else if (!indented && container_type == CMARK_NODE_PARAGRAPH &&
                 (heading_level = scan_setext_heading_line(input, parser->first_nonspace)))
        {
            // finalize paragraph, resolving reference links
            has_content = resolve_reference_link_definitions(parser, *container);

            if (has_content)
            {
                (*container)->type = static_cast<uint16_t>(CMARK_NODE_HEADING);
                (*container)->as.heading.level = heading_level;
                (*container)->as.heading.setext = true;
                S_advance_offset(parser, input, input->len - 1 - parser->offset, false);
            }
        }
        else if (!indented && !(container_type == CMARK_NODE_PARAGRAPH && !all_matched) &&
                 (parser->thematic_break_kill_pos <= parser->first_nonspace) &&
                 (matched = S_scan_thematic_break(parser, input, parser->first_nonspace)))
        {
            // it's only now that we know the line is not part of a setext heading:
            *container = add_child(parser, *container, CMARK_NODE_THEMATIC_BREAK,
                                   parser->first_nonspace + 1);
            S_advance_offset(parser, input, input->len - 1 - parser->offset, false);
        }
        else if (!indented && (parser->options & CMARK_OPT_FOOTNOTES) && depth < MAX_LIST_DEPTH &&
                 (matched = scan_footnote_definition(input, parser->first_nonspace)))
        {
            cmark_chunk footnote_label =
                cmark_chunk_dup(input, parser->first_nonspace + 2, matched - 2);

            while (footnote_label.data[footnote_label.len - 1] != ']')
            {
                --footnote_label.len;
            }
            --footnote_label.len;

            cmark_chunk_to_cstr(&footnote_label);

            S_advance_offset(parser, input, parser->first_nonspace + matched - parser->offset,
                             false);
            *container = add_child(parser, *container, CMARK_NODE_FOOTNOTE_DEFINITION,
                                   parser->first_nonspace + matched + 1);
            (*container)->as.literal = footnote_label;

            (*container)->internal_offset = matched;
        }
        else if ((!indented || container_type == CMARK_NODE_LIST) && parser->indent < 4 &&
                 depth < MAX_LIST_DEPTH &&
                 (matched =
                      parse_list_marker(input, parser->first_nonspace,
                                        (*container)->type == CMARK_NODE_PARAGRAPH, &list_data)))
        {
            // Note that we can have new list items starting with >= 4
            // spaces indent, as long as the list container is still open.
            int padding_width = 0;

            // compute padding:
            S_advance_offset(parser, input, parser->first_nonspace + matched - parser->offset,
                             false);

            save_partially_consumed_tab = parser->partially_consumed_tab;
            save_offset = parser->offset;
            save_column = parser->column;

            while (parser->column - save_column <= 5 &&
                   S_is_space_or_tab(peek_at(input, parser->offset)))
            {
                S_advance_offset(parser, input, 1, true);
            }

            padding_width = parser->column - save_column;
            if (padding_width >= 5 || padding_width < 1 ||
                // only spaces after list marker:
                S_is_line_end_char(peek_at(input, parser->offset)))
            {
                list_data->padding = matched + 1;
                parser->column = save_column;
                parser->offset = save_offset;
                parser->partially_consumed_tab = save_partially_consumed_tab;
                if (save_partially_consumed_tab)
                {
                    parser->column += 1;
                }
            }
            else
            {
                list_data->padding = matched + padding_width;
            }

            // check container; if it's a list, see if this list item belongs in it:
            if (container_type != CMARK_NODE_LIST ||
                !lists_match(&((*container)->as.list), list_data))
            {
                *container =
                    add_child(parser, *container, CMARK_NODE_LIST, parser->first_nonspace + 1);
                memcpy(&((*container)->as.list), list_data, sizeof(cmark_list));
            }

            // add the list item
            *container = add_child(parser, *container, CMARK_NODE_ITEM, parser->first_nonspace + 1);
            /* TODO: static_cast? */
            memcpy(&((*container)->as.list), list_data, sizeof(cmark_list));
            delete list_data;
        }
        else if (indented && !maybe_lazy && !parser->blank)
        {
            S_advance_offset(parser, input, CODE_INDENT, true);
            *container = add_child(parser, *container, CMARK_NODE_CODE_BLOCK, parser->offset + 1);
            (*container)->as.code.fenced = false;
            (*container)->as.code.fence_char = 0;
            (*container)->as.code.fence_length = 0;
            (*container)->as.code.fence_offset = 0;
            (*container)->as.code.info = cmark_chunk_literal("");
        }
        else
        {
            cmark_llist* extension_link = nullptr;
            cmark_node* new_container = nullptr;

            for (extension_link = parser->syntax_extensions; extension_link;
                 extension_link = extension_link->next)
            {
                cmark_syntax_extension* extension =
                    static_cast<cmark_syntax_extension*>(extension_link->data);

                if (extension->try_opening_block)
                {
                    new_container = extension->try_opening_block(
                        extension, indented, parser, *container, input->data, input->len);

                    if (new_container)
                    {
                        *container = new_container;
                        break;
                    }
                }
            }

            if (!new_container)
            {
                break;
            }
        }

        if (accepts_lines(S_type(*container)))
        {
            // if it's a line container, it can't contain other containers
            break;
        }

        container_type = S_type(*container);
        maybe_lazy = false;
    }
}

static void add_text_to_container(cmark_parser* parser, cmark_node* container,
                                  cmark_node* last_matched_container, cmark_chunk* input)
{
    cmark_node* ancestor_node = nullptr;
    // what remains at parser->offset is a text line.  add the text to the
    // appropriate container.

    S_find_first_nonspace(parser, input);

    if (parser->blank && container->last_child)
    {
        S_set_last_line_blank(container->last_child, true);
    }

    // block quote lines are never blank as they start with >
    // and we don't count blanks in fenced code for purposes of tight/loose
    // lists or breaking out of lists.  we also don't set last_line_blank
    // on an empty list item.
    const cmark_node_type ctype = S_type(container);
    const bool last_line_blank =
        (parser->blank && ctype != CMARK_NODE_BLOCK_QUOTE && ctype != CMARK_NODE_HEADING &&
         ctype != CMARK_NODE_THEMATIC_BREAK &&
         !(ctype == CMARK_NODE_CODE_BLOCK && container->as.code.fenced) &&
         !(ctype == CMARK_NODE_ITEM && container->first_child == nullptr &&
           container->start_line == parser->line_number));

    S_set_last_line_blank(container, last_line_blank);

    ancestor_node = container;
    while (ancestor_node->parent)
    {
        S_set_last_line_blank(ancestor_node->parent, false);
        ancestor_node = ancestor_node->parent;
    }

    // If the last line processed belonged to a paragraph node,
    // and we didn't match all of the line prefixes for the open containers,
    // and we didn't start any new containers,
    // and the line isn't blank,
    // then treat this as a "lazy continuation line" and add it to
    // the open paragraph.
    if (parser->current != last_matched_container && container == last_matched_container &&
        !parser->blank && S_type(parser->current) == CMARK_NODE_PARAGRAPH)
    {
        add_line(parser->current, input, parser);
    }
    else
    {  // not a lazy continuation
        // Finalize any blocks that were not matched and set cur to container:
        while (parser->current != last_matched_container)
        {
            parser->current = finalize(parser, parser->current);
            assert(parser->current != nullptr);
        }

        if (S_type(container) == CMARK_NODE_CODE_BLOCK)
        {
            add_line(container, input, parser);
        }
        else if (S_type(container) == CMARK_NODE_HTML_BLOCK)
        {
            add_line(container, input, parser);

            int matches_end_condition = 0;
            switch (container->as.html_block_type)
            {
                case 1:
                    // </script>, </style>, </pre>
                    matches_end_condition = scan_html_block_end_1(input, parser->first_nonspace);
                    break;
                case 2:
                    // -->
                    matches_end_condition = scan_html_block_end_2(input, parser->first_nonspace);
                    break;
                case 3:
                    // ?>
                    matches_end_condition = scan_html_block_end_3(input, parser->first_nonspace);
                    break;
                case 4:
                    // >
                    matches_end_condition = scan_html_block_end_4(input, parser->first_nonspace);
                    break;
                case 5:
                    // ]]>
                    matches_end_condition = scan_html_block_end_5(input, parser->first_nonspace);
                    break;
                default:
                    matches_end_condition = 0;
                    break;
            }

            if (matches_end_condition)
            {
                container = finalize(parser, container);
                assert(parser->current != nullptr);
            }
        }
        else if (parser->blank)
        {
            // ??? do nothing
        }
        else if (accepts_lines(S_type(container)))
        {
            if (S_type(container) == CMARK_NODE_HEADING && container->as.heading.setext == false)
            {
                chop_trailing_hashtags(input);
            }
            S_advance_offset(parser, input, parser->first_nonspace - parser->offset, false);
            add_line(container, input, parser);
        }
        else
        {
            // create paragraph container for line
            container =
                add_child(parser, container, CMARK_NODE_PARAGRAPH, parser->first_nonspace + 1);
            S_advance_offset(parser, input, parser->first_nonspace - parser->offset, false);
            add_line(container, input, parser);
        }

        parser->current = container;
    }
}

/* See http://spec.commonmark.org/0.24/#phase-1-block-structure */
static void S_process_line(cmark_parser* parser, const unsigned char* buffer, size_t bytes)
{
    cmark_node* last_matched_container = nullptr;
    bool all_matched = true;
    cmark_node* container = nullptr;
    cmark_chunk input = {};
    cmark_node* current_node = nullptr;

    parser->curline.Clear();

    if (parser->options & CMARK_OPT_VALIDATE_UTF8)
    {
        cmark_utf8proc_check(&parser->curline, buffer, bytes);
    }
    else
    {
        parser->curline.Put(buffer, bytes);
    }

    bytes = parser->curline.get_size();

    // ensure line ends with a newline:
    if (bytes == 0 || !S_is_line_end_char(parser->curline.get_ptr()[bytes - 1]))
    {
        parser->curline.Putc('\n');
    }

    parser->offset = 0;
    parser->column = 0;
    parser->first_nonspace = 0;
    parser->first_nonspace_column = 0;
    parser->thematic_break_kill_pos = 0;
    parser->indent = 0;
    parser->blank = false;
    parser->partially_consumed_tab = false;

    input.data = parser->curline.get_ptr();
    input.len = parser->curline.get_size();
    input.alloc = 0;

    // Skip UTF-8 BOM.
    if (parser->line_number == 0 && input.len >= 3 && memcmp(input.data, "\xef\xbb\xbf", 3) == 0)
    {
        parser->offset += 3;
    }

    parser->line_number++;

    last_matched_container = check_open_blocks(parser, &input, &all_matched);

    if (last_matched_container)
    {
        container = last_matched_container;

        current_node = parser->current;

        open_new_blocks(parser, &container, &input, all_matched);

        /* parser->current might have changed if feed_reentrant was called */
        if (current_node == parser->current)
        {
            add_text_to_container(parser, container, last_matched_container, &input);
        }
    }

    parser->last_line_length = input.len;
    if (parser->last_line_length && input.data[parser->last_line_length - 1] == '\n')
    {
        parser->last_line_length -= 1;
    }
    if (parser->last_line_length && input.data[parser->last_line_length - 1] == '\r')
    {
        parser->last_line_length -= 1;
    }

    parser->curline.Clear();
}

cmark_node* cmark_parser_finish(cmark_parser* parser)
{
    cmark_node* result_root = nullptr;
    cmark_llist* extensions = nullptr;

    /* Parser was already finished once */
    if (parser->root == nullptr)
    {
        return nullptr;
    }

    if (parser->linebuf.get_size())
    {
        S_process_line(parser, parser->linebuf.get_ptr(), parser->linebuf.get_size());
        parser->linebuf.Clear();
    }

    finalize_document(parser);

    cmark_consolidate_text_nodes(parser->root);

    parser->curline.Free();
    parser->linebuf.Free();

#if CMARK_DEBUG_NODES
    if (cmark_node_check(parser->root, stderr))
    {
        abort();
    }
#endif

    for (extensions = parser->syntax_extensions; extensions; extensions = extensions->next)
    {
        cmark_syntax_extension* extension = static_cast<cmark_syntax_extension*>(extensions->data);
        if (extension->postprocess_func)
        {
            cmark_node* processed = extension->postprocess_func(extension, parser, parser->root);
            if (processed)
            {
                parser->root = processed;
            }
        }
    }

    result_root = parser->root;
    parser->root = nullptr;

    cmark_parser_reset(parser);

    return result_root;
}

int cmark_parser_get_line_number(cmark_parser* parser)
{
    return parser->line_number;
}

size_t cmark_parser_get_offset(cmark_parser* parser)
{
    return parser->offset;
}

size_t cmark_parser_get_column(cmark_parser* parser)
{
    return parser->column;
}

int cmark_parser_get_first_nonspace(cmark_parser* parser)
{
    return parser->first_nonspace;
}

int cmark_parser_get_first_nonspace_column(cmark_parser* parser)
{
    return parser->first_nonspace_column;
}

int cmark_parser_get_indent(cmark_parser* parser)
{
    return parser->indent;
}

int cmark_parser_is_blank(cmark_parser* parser)
{
    return parser->blank;
}

int cmark_parser_has_partially_consumed_tab(cmark_parser* parser)
{
    return parser->partially_consumed_tab;
}

int cmark_parser_get_last_line_length(cmark_parser* parser)
{
    return parser->last_line_length;
}

cmark_node* cmark_parser_add_child(cmark_parser* parser, cmark_node* parent,
                                   cmark_node_type block_type, int start_column)
{
    return add_child(parser, parent, block_type, start_column);
}

void cmark_parser_advance_offset(cmark_parser* parser, const char* input, int count, int columns)
{
    cmark_chunk input_chunk = cmark_chunk_literal(input);

    S_advance_offset(parser, &input_chunk, count, columns != 0);
}

void cmark_parser_set_backslash_ispunct_func(cmark_parser* parser, cmark_ispunct_func func)
{
    parser->backslash_ispunct = func;
}

cmark_llist* cmark_parser_get_syntax_extensions(cmark_parser* parser)
{
    return parser->syntax_extensions;
}
