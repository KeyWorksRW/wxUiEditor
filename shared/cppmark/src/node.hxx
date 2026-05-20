// Abstract Syntax Tree (AST) node structure
// Purpose: Represents parsed markdown document as tree (e.g., HEADING->TEXT, LIST->ITEM->PARAGRAPH)
// Key type: cmark_node - parent/child/sibling pointers, type enum, node-specific data in union
// Tree structure: Doubly-linked siblings, parent/first_child/last_child pointers
// Node types: Block (DOCUMENT, PARAGRAPH, HEADING, LIST, etc.) vs Inline (TEXT, EMPH, STRONG, LINK,
// etc.) Status: Legacy code - uses manual malloc/calloc (Phase 1.5 modernization candidate)
// Dependencies: buffer.hxx, chunk.hxx, cmark-gfm.hxx

#pragma once

#include "buffer.hxx"
#include "chunk.hxx"
#include "cmark-gfm.hxx"

struct cmark_list
{
    cmark_list_type list_type;
    int marker_offset;
    int padding;
    int start;
    cmark_delim_type delimiter;
    unsigned char bullet_char;
    bool tight;
    bool checked;  // For task list extension
};

struct cmark_code
{
    cmark_chunk info;
    cmark_chunk literal;
    uint8_t fence_length;
    uint8_t fence_offset;
    unsigned char fence_char;
    int8_t fenced;
};

struct cmark_heading
{
    int level;
    bool setext;
};

struct cmark_link
{
    cmark_chunk url;
    cmark_chunk title;
};

struct cmark_custom
{
    cmark_chunk on_enter;
    cmark_chunk on_exit;
};

enum class CMarkNodeInternalFlags : std::uint8_t
{
    open = (1 << 0),
    last_line_blank = (1 << 1),
    last_line_checked = (1 << 2),

    // Extensions can register custom flags by calling `cmark_register_node_flag`.
    // This is the starting value for the custom flags.
    register_first = (1 << 3),
};

struct cmark_node
{
    CMarkStringBuffer content;

    struct cmark_node* next;
    struct cmark_node* prev;
    struct cmark_node* parent;
    struct cmark_node* first_child;
    struct cmark_node* last_child;

    void* user_data;
    cmark_free_func user_data_free_func;

    int start_line;
    int start_column;
    int end_line;
    int end_column;
    int internal_offset;
    uint16_t type;
    CMarkNodeInternalFlags flags;

    cmark_syntax_extension* extension;

    /**
     * Used during cmark_render() to cache the most recent non-NULL
     * extension, if you go up the parent chain like this:
     *
     * node->parent->...parent->extension
     */
    cmark_syntax_extension* ancestor_extension;

    union
    {
        int ref_ix;
        int def_count;
    } footnote;

    cmark_node* parent_footnote_def;

    union
    {
        cmark_chunk literal;
        cmark_list list;
        cmark_code code;
        cmark_heading heading;
        cmark_link link;
        cmark_custom custom;
        int html_block_type;
        int cell_index;  // For keeping track of TABLE_CELL table alignments
        void* opaque;
    } as;
};

// Syntax extensions can use this function to register a custom node flag. The
// flags are stored in the `flags` field of the `cmark_node` struct. The `flags`
// parameter should be the address of a global variable which will store the
// flag value.
void cmark_register_node_flag(CMarkNodeInternalFlags* flags);
int cmark_node_check(cmark_node* node, FILE* out);

static bool CMARK_NODE_TYPE_BLOCK_P(cmark_node_type node_type)
{
    return (node_type & CMARK_NODE_TYPE_MASK) == CMARK_NODE_TYPE_BLOCK;
}

[[maybe_unused]] static bool CMARK_NODE_BLOCK_P(cmark_node* node)
{
    return node != nullptr && CMARK_NODE_TYPE_BLOCK_P(static_cast<cmark_node_type>(node->type));
}

static bool CMARK_NODE_TYPE_INLINE_P(cmark_node_type node_type)
{
    return (node_type & CMARK_NODE_TYPE_MASK) == CMARK_NODE_TYPE_INLINE;
}

[[maybe_unused]] static bool CMARK_NODE_INLINE_P(cmark_node* node)
{
    return node != nullptr && CMARK_NODE_TYPE_INLINE_P(static_cast<cmark_node_type>(node->type));
}

bool cmark_node_can_contain_type(cmark_node* node, cmark_node_type child_type);

// Enable (or disable) extra safety checks. These extra checks cause extra
// performance overhead (in some cases quadratic), so they are only intended to
// be used during testing.
void cmark_enable_safety_checks(bool enable);
