// AST node creation, manipulation, and traversal
// Purpose: Implements cmark_node operations - create, destroy, walk tree, get/set properties
// Key functions: cmark_node_new(), cmark_node_free(), cmark_node_append_child(),
// cmark_node_get_*/set_*() Memory: Modernized - uses smart pointers (std::unique_ptr,
// std::make_unique) Status: Phase 1.5 complete - removed cmark_mem allocator abstraction
// Dependencies: buffer.hxx, chunk.hxx, cmark-gfm.hxx, inlines.hxx, references.hxx, footnotes.hxx

#include <stdexcept>

#include "node.hxx"
#include "syntax_extension.hxx"
#include <memory>
#include <print>

// Expensive safety checks are off by default, but can be enabled by calling
// cmark_enable_safety_checks().
static bool enable_safety_checks = false;

void cmark_enable_safety_checks(bool enable)
{
    enable_safety_checks = enable;
}

static void S_node_unlink(cmark_node* node);

void cmark_register_node_flag(CMarkNodeInternalFlags* flags)
{
    static CMarkNodeInternalFlags nextflag = CMarkNodeInternalFlags::register_first;

    // flags should be a pointer to a global variable and this function
    // should only be called once to initialize its value.
    if (static_cast<uint16_t>(*flags))
    {
        throw std::runtime_error("flag initialization error in cmark_register_node_flag");
    }

    // Check that we haven't run out of bits.
    if (static_cast<uint16_t>(nextflag) == 0)
    {
        throw std::runtime_error("too many flags in cmark_register_node_flag");
    }

    *flags = nextflag;
    nextflag = static_cast<CMarkNodeInternalFlags>(static_cast<uint16_t>(nextflag) << 1);
}

bool cmark_node_can_contain_type(cmark_node* node, cmark_node_type child_type)
{
    if (child_type == CMARK_NODE_DOCUMENT)
    {
        return false;
    }

    if (node->extension && node->extension->can_contain_func)
    {
        return node->extension->can_contain_func(node->extension, node, child_type) != 0;
    }

    switch (node->type)
    {
        case CMARK_NODE_DOCUMENT:
        case CMARK_NODE_BLOCK_QUOTE:
        case CMARK_NODE_FOOTNOTE_DEFINITION:
        case CMARK_NODE_ITEM:
            return CMARK_NODE_TYPE_BLOCK_P(child_type) && child_type != CMARK_NODE_ITEM;

        case CMARK_NODE_LIST:
            return child_type == CMARK_NODE_ITEM;

        case CMARK_NODE_CUSTOM_BLOCK:
            return true;

        case CMARK_NODE_PARAGRAPH:
        case CMARK_NODE_HEADING:
        case CMARK_NODE_EMPH:
        case CMARK_NODE_STRONG:
        case CMARK_NODE_LINK:
        case CMARK_NODE_IMAGE:
        case CMARK_NODE_CUSTOM_INLINE:
            return CMARK_NODE_TYPE_INLINE_P(child_type);

        default:
            break;
    }

    return false;
}

static bool S_can_contain(cmark_node* node, cmark_node* child)
{
    if (node == nullptr || child == nullptr)
    {
        return false;
    }

    if (enable_safety_checks)
    {
        // Verify that child is not an ancestor of node or equal to node.
        const cmark_node* cur = node;
        do
        {
            if (cur == child)
            {
                return false;
            }
            cur = cur->parent;
        } while (cur != nullptr);
    }

    return cmark_node_can_contain_type(node, static_cast<cmark_node_type>(child->type));
}

cmark_node* cmark_node_new_with_mem_and_ext(cmark_node_type type, cmark_syntax_extension* extension)
{
    std::unique_ptr<cmark_node> node_ptr = std::make_unique<cmark_node>();
    cmark_node* node = node_ptr.get();
    node->content.Init(0);
    node->type = (uint16_t) type;
    node->extension = extension;

    switch (node->type)
    {
        case CMARK_NODE_HEADING:
            node->as.heading.level = 1;
            break;

        case CMARK_NODE_LIST:
            {
                cmark_list* list = &node->as.list;
                list->list_type = CMARK_BULLET_LIST;
                list->start = 0;
                list->tight = false;
                break;
            }

        default:
            break;
    }

    if (node->extension && node->extension->opaque_alloc_func)
    {
        node->extension->opaque_alloc_func(node->extension, node);
    }

    return node_ptr.release();
}

cmark_node* cmark_node_new_with_ext(cmark_node_type type, cmark_syntax_extension* extension)
{
    return cmark_node_new_with_mem_and_ext(type, extension);
}

cmark_node* cmark_node_new(cmark_node_type type)
{
    return cmark_node_new_with_ext(type, nullptr);
}

static void free_node_as(cmark_node* node)
{
    switch (node->type)
    {
        case CMARK_NODE_CODE_BLOCK:
            cmark_chunk_free(&node->as.code.info);
            cmark_chunk_free(&node->as.code.literal);
            break;
        case CMARK_NODE_TEXT:
        case CMARK_NODE_HTML_INLINE:
        case CMARK_NODE_CODE:
        case CMARK_NODE_HTML_BLOCK:
        case CMARK_NODE_FOOTNOTE_REFERENCE:
        case CMARK_NODE_FOOTNOTE_DEFINITION:
            cmark_chunk_free(&node->as.literal);
            break;
        case CMARK_NODE_LINK:
        case CMARK_NODE_IMAGE:
            cmark_chunk_free(&node->as.link.url);
            cmark_chunk_free(&node->as.link.title);
            break;
        case CMARK_NODE_CUSTOM_BLOCK:
        case CMARK_NODE_CUSTOM_INLINE:
            cmark_chunk_free(&node->as.custom.on_enter);
            cmark_chunk_free(&node->as.custom.on_exit);
            break;
        default:
            break;
    }
}

// Free a cmark_node list and any children.
static void S_free_nodes(cmark_node* node)
{
    cmark_node* next_node = nullptr;
    while (node != nullptr)
    {
        node->content.Free();

        if (node->user_data && node->user_data_free_func)
        {
            node->user_data_free_func(node->user_data);
        }

        if (node->as.opaque && node->extension && node->extension->opaque_free_func)
        {
            node->extension->opaque_free_func(node->extension, node);
        }

        free_node_as(node);

        if (node->last_child)
        {
            // Splice children into list
            node->last_child->next = node->next;
            node->next = node->first_child;
        }
        next_node = node->next;
        delete node;
        node = next_node;
    }
}

void cmark_node_free(cmark_node* node)
{
    S_node_unlink(node);
    node->next = nullptr;
    S_free_nodes(node);
}

cmark_node_type cmark_node_get_type(cmark_node* node)
{
    if (node == nullptr)
    {
        return CMARK_NODE_NONE;
    }

    return static_cast<cmark_node_type>(node->type);
}

int cmark_node_set_type(cmark_node* node, cmark_node_type type)
{
    if (node == nullptr)
    {
        return 0;
    }

    cmark_node_type initial_type = CMARK_NODE_NONE;

    if (type == node->type)
    {
        return 1;
    }

    initial_type = static_cast<cmark_node_type>(node->type);
    node->type = static_cast<uint16_t>(type);

    if (!S_can_contain(node->parent, node))
    {
        node->type = static_cast<uint16_t>(initial_type);
        return 0;
    }

    /* We rollback the type to free the union members appropriately */
    node->type = static_cast<uint16_t>(initial_type);
    free_node_as(node);

    node->type = static_cast<uint16_t>(type);

    return 1;
}

const char* cmark_node_get_type_string(cmark_node* node)
{
    if (node == nullptr)
    {
        return "NONE";
    }

    if (node->extension && node->extension->get_type_string_func)
    {
        return node->extension->get_type_string_func(node->extension, node);
    }

    switch (node->type)
    {
        case CMARK_NODE_NONE:
            return "none";
        case CMARK_NODE_DOCUMENT:
            return "document";
        case CMARK_NODE_BLOCK_QUOTE:
            return "block_quote";
        case CMARK_NODE_LIST:
            return "list";
        case CMARK_NODE_ITEM:
            return "item";
        case CMARK_NODE_CODE_BLOCK:
            return "code_block";
        case CMARK_NODE_HTML_BLOCK:
            return "html_block";
        case CMARK_NODE_CUSTOM_BLOCK:
            return "custom_block";
        case CMARK_NODE_PARAGRAPH:
            return "paragraph";
        case CMARK_NODE_HEADING:
            return "heading";
        case CMARK_NODE_THEMATIC_BREAK:
            return "thematic_break";
        case CMARK_NODE_TEXT:
            return "text";
        case CMARK_NODE_SOFTBREAK:
            return "softbreak";
        case CMARK_NODE_LINEBREAK:
            return "linebreak";
        case CMARK_NODE_CODE:
            return "code";
        case CMARK_NODE_HTML_INLINE:
            return "html_inline";
        case CMARK_NODE_CUSTOM_INLINE:
            return "custom_inline";
        case CMARK_NODE_EMPH:
            return "emph";
        case CMARK_NODE_STRONG:
            return "strong";
        case CMARK_NODE_LINK:
            return "link";
        case CMARK_NODE_IMAGE:
            return "image";
    }

    return "<unknown>";
}

cmark_node* cmark_node_next(cmark_node* node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    return node->next;
}

cmark_node* cmark_node_previous(cmark_node* node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    return node->prev;
}

cmark_node* cmark_node_parent(cmark_node* node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    return node->parent;
}

cmark_node* cmark_node_first_child(cmark_node* node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    return node->first_child;
}

cmark_node* cmark_node_last_child(cmark_node* node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    return node->last_child;
}

cmark_node* cmark_node_parent_footnote_def(cmark_node* node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    return node->parent_footnote_def;
}

void* cmark_node_get_user_data(cmark_node* node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    return node->user_data;
}

int cmark_node_set_user_data(cmark_node* node, void* user_data)
{
    if (node == nullptr)
    {
        return 0;
    }
    node->user_data = user_data;
    return 1;
}

int cmark_node_set_user_data_free_func(cmark_node* node, cmark_free_func free_func)
{
    if (node == nullptr)
    {
        return 0;
    }
    node->user_data_free_func = free_func;
    return 1;
}

const char* cmark_node_get_literal(cmark_node* node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    switch (node->type)
    {
        case CMARK_NODE_HTML_BLOCK:
        case CMARK_NODE_TEXT:
        case CMARK_NODE_HTML_INLINE:
        case CMARK_NODE_CODE:
        case CMARK_NODE_FOOTNOTE_REFERENCE:
        case CMARK_NODE_FOOTNOTE_DEFINITION:
            return cmark_chunk_to_cstr(&node->as.literal);

        case CMARK_NODE_CODE_BLOCK:
            return cmark_chunk_to_cstr(&node->as.code.literal);

        default:
            break;
    }

    return nullptr;
}

int cmark_node_set_literal(cmark_node* node, const char* content)
{
    if (node == nullptr)
    {
        return 0;
    }

    switch (node->type)
    {
        case CMARK_NODE_HTML_BLOCK:
        case CMARK_NODE_TEXT:
        case CMARK_NODE_HTML_INLINE:
        case CMARK_NODE_CODE:
        case CMARK_NODE_FOOTNOTE_REFERENCE:
            cmark_chunk_set_cstr(&node->as.literal, content);
            return 1;

        case CMARK_NODE_CODE_BLOCK:
            cmark_chunk_set_cstr(&node->as.code.literal, content);
            return 1;

        default:
            break;
    }

    return 0;
}

const char* cmark_node_get_string_content(cmark_node* node)
{
    if (node == nullptr)
    {
        return nullptr;
    }
    return static_cast<const char*>(static_cast<const void*>(node->content.get_ptr()));
}

int cmark_node_set_string_content(cmark_node* node, const char* content)
{
    if (node == nullptr)
    {
        return 0;
    }
    node->content.Sets(content);
    return 1;
}

int cmark_node_get_heading_level(cmark_node* node)
{
    if (node == nullptr)
    {
        return 0;
    }

    switch (node->type)
    {
        case CMARK_NODE_HEADING:
            return node->as.heading.level;

        default:
            break;
    }

    return 0;
}

int cmark_node_set_heading_level(cmark_node* node, int level)
{
    if (node == nullptr || level < 1 || level > 6)
    {
        return 0;
    }

    switch (node->type)
    {
        case CMARK_NODE_HEADING:
            node->as.heading.level = level;
            return 1;

        default:
            break;
    }

    return 0;
}

cmark_list_type cmark_node_get_list_type(cmark_node* node)
{
    if (node == nullptr)
    {
        return CMARK_NO_LIST;
    }

    if (node->type == CMARK_NODE_LIST)
    {
        return node->as.list.list_type;
    }

    return CMARK_NO_LIST;
}

int cmark_node_set_list_type(cmark_node* node, cmark_list_type type)
{
    if (!(type == CMARK_BULLET_LIST || type == CMARK_ORDERED_LIST))
    {
        return 0;
    }

    if (node == nullptr)
    {
        return 0;
    }

    if (node->type == CMARK_NODE_LIST)
    {
        node->as.list.list_type = type;
        return 1;
    }

    return 0;
}

cmark_delim_type cmark_node_get_list_delim(cmark_node* node)
{
    if (node == nullptr)
    {
        return CMARK_NO_DELIM;
    }

    if (node->type == CMARK_NODE_LIST)
    {
        return node->as.list.delimiter;
    }

    return CMARK_NO_DELIM;
}

int cmark_node_set_list_delim(cmark_node* node, cmark_delim_type delim)
{
    if (!(delim == CMARK_PERIOD_DELIM || delim == CMARK_PAREN_DELIM))
    {
        return 0;
    }

    if (node == nullptr)
    {
        return 0;
    }

    if (node->type == CMARK_NODE_LIST)
    {
        node->as.list.delimiter = delim;
        return 1;
    }

    return 0;
}

int cmark_node_get_list_start(cmark_node* node)
{
    if (node == nullptr)
    {
        return 0;
    }

    if (node->type == CMARK_NODE_LIST)
    {
        return node->as.list.start;
    }

    return 0;
}

int cmark_node_set_list_start(cmark_node* node, int start)
{
    if (node == nullptr || start < 0)
    {
        return 0;
    }

    if (node->type == CMARK_NODE_LIST)
    {
        node->as.list.start = start;
        return 1;
    }

    return 0;
}

int cmark_node_get_list_tight(cmark_node* node)
{
    if (node == nullptr)
    {
        return 0;
    }

    if (node->type == CMARK_NODE_LIST)
    {
        return node->as.list.tight;
    }

    return 0;
}

int cmark_node_set_list_tight(cmark_node* node, int tight)
{
    if (node == nullptr)
    {
        return 0;
    }

    if (node->type == CMARK_NODE_LIST)
    {
        node->as.list.tight = tight != 0;
        return 1;
    }

    return 0;
}

int cmark_node_get_item_index(cmark_node* node)
{
    if (node == nullptr)
    {
        return 0;
    }

    if (node->type == CMARK_NODE_ITEM)
    {
        return node->as.list.start;
    }

    return 0;
}

int cmark_node_set_item_index(cmark_node* node, int idx)
{
    if (node == nullptr || idx < 0)
    {
        return 0;
    }

    if (node->type == CMARK_NODE_ITEM)
    {
        node->as.list.start = idx;
        return 1;
    }

    return 0;
}

const char* cmark_node_get_fence_info(cmark_node* node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    if (node->type == CMARK_NODE_CODE_BLOCK)
    {
        return cmark_chunk_to_cstr(&node->as.code.info);
    }

    return nullptr;
}

int cmark_node_set_fence_info(cmark_node* node, const char* info)
{
    if (node == nullptr)
    {
        return 0;
    }

    if (node->type == CMARK_NODE_CODE_BLOCK)
    {
        cmark_chunk_set_cstr(&node->as.code.info, info);
        return 1;
    }

    return 0;
}

int cmark_node_get_fenced(cmark_node* node, int* length, int* offset, char* character)
{
    if (node == nullptr)
    {
        return 0;
    }

    if (node->type == CMARK_NODE_CODE_BLOCK)
    {
        if (length)
            *length = node->as.code.fence_length;
        if (offset)
            *offset = node->as.code.fence_offset;
        if (character)
            *character = node->as.code.fence_char;
        return node->as.code.fenced;
    }

    return 0;
}

int cmark_node_set_fenced(cmark_node* node, int fenced, int length, int offset, char character)
{
    if (node == nullptr)
    {
        return 0;
    }

    if (node->type == CMARK_NODE_CODE_BLOCK)
    {
        node->as.code.fenced = static_cast<int8_t>(fenced);
        node->as.code.fence_length = static_cast<uint8_t>(length);
        node->as.code.fence_offset = static_cast<uint8_t>(offset);
        node->as.code.fence_char = character;
        return 1;
    }

    return 0;
}

const char* cmark_node_get_url(cmark_node* node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    switch (node->type)
    {
        case CMARK_NODE_LINK:
        case CMARK_NODE_IMAGE:
            return cmark_chunk_to_cstr(&node->as.link.url);
        default:
            break;
    }

    return nullptr;
}

int cmark_node_set_url(cmark_node* node, const char* url)
{
    if (node == nullptr)
    {
        return 0;
    }

    switch (node->type)
    {
        case CMARK_NODE_LINK:
        case CMARK_NODE_IMAGE:
            cmark_chunk_set_cstr(&node->as.link.url, url);
            return 1;
        default:
            break;
    }

    return 0;
}

const char* cmark_node_get_title(cmark_node* node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    switch (node->type)
    {
        case CMARK_NODE_LINK:
        case CMARK_NODE_IMAGE:
            return cmark_chunk_to_cstr(&node->as.link.title);
        default:
            break;
    }

    return nullptr;
}

int cmark_node_set_title(cmark_node* node, const char* title)
{
    if (node == nullptr)
    {
        return 0;
    }

    switch (node->type)
    {
        case CMARK_NODE_LINK:
        case CMARK_NODE_IMAGE:
            cmark_chunk_set_cstr(&node->as.link.title, title);
            return 1;
        default:
            break;
    }

    return 0;
}

const char* cmark_node_get_on_enter(cmark_node* node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    switch (node->type)
    {
        case CMARK_NODE_CUSTOM_INLINE:
        case CMARK_NODE_CUSTOM_BLOCK:
            return cmark_chunk_to_cstr(&node->as.custom.on_enter);
        default:
            break;
    }

    return nullptr;
}

int cmark_node_set_on_enter(cmark_node* node, const char* on_enter)
{
    if (node == nullptr)
    {
        return 0;
    }

    switch (node->type)
    {
        case CMARK_NODE_CUSTOM_INLINE:
        case CMARK_NODE_CUSTOM_BLOCK:
            cmark_chunk_set_cstr(&node->as.custom.on_enter, on_enter);
            return 1;
        default:
            break;
    }

    return 0;
}

const char* cmark_node_get_on_exit(cmark_node* node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    switch (node->type)
    {
        case CMARK_NODE_CUSTOM_INLINE:
        case CMARK_NODE_CUSTOM_BLOCK:
            return cmark_chunk_to_cstr(&node->as.custom.on_exit);
        default:
            break;
    }

    return nullptr;
}

int cmark_node_set_on_exit(cmark_node* node, const char* on_exit)
{
    if (node == nullptr)
    {
        return 0;
    }

    switch (node->type)
    {
        case CMARK_NODE_CUSTOM_INLINE:
        case CMARK_NODE_CUSTOM_BLOCK:
            cmark_chunk_set_cstr(&node->as.custom.on_exit, on_exit);
            return 1;
        default:
            break;
    }

    return 0;
}

cmark_syntax_extension* cmark_node_get_syntax_extension(cmark_node* node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    return node->extension;
}

int cmark_node_set_syntax_extension(cmark_node* node, cmark_syntax_extension* extension)
{
    if (node == nullptr)
    {
        return 0;
    }

    node->extension = extension;
    return 1;
}

int cmark_node_get_start_line(cmark_node* node)
{
    if (node == nullptr)
    {
        return 0;
    }
    return node->start_line;
}

int cmark_node_get_start_column(cmark_node* node)
{
    if (node == nullptr)
    {
        return 0;
    }
    return node->start_column;
}

int cmark_node_get_end_line(cmark_node* node)
{
    if (node == nullptr)
    {
        return 0;
    }
    return node->end_line;
}

int cmark_node_get_end_column(cmark_node* node)
{
    if (node == nullptr)
    {
        return 0;
    }
    return node->end_column;
}

// Unlink a node without adjusting its next, prev, and parent pointers.
static void S_node_unlink(cmark_node* node)
{
    if (node == nullptr)
    {
        return;
    }

    if (node->prev)
    {
        node->prev->next = node->next;
    }
    if (node->next)
    {
        node->next->prev = node->prev;
    }

    // Adjust first_child and last_child of parent.
    cmark_node* parent = node->parent;
    if (parent)
    {
        if (parent->first_child == node)
        {
            parent->first_child = node->next;
        }
        if (parent->last_child == node)
        {
            parent->last_child = node->prev;
        }
    }
}

void cmark_node_unlink(cmark_node* node)
{
    S_node_unlink(node);

    node->next = nullptr;
    node->prev = nullptr;
    node->parent = nullptr;
}

int cmark_node_insert_before(cmark_node* node, cmark_node* sibling)
{
    if (node == nullptr || sibling == nullptr)
    {
        return 0;
    }

    if (!node->parent || !S_can_contain(node->parent, sibling))
    {
        return 0;
    }

    S_node_unlink(sibling);

    cmark_node* old_prev = node->prev;

    // Insert 'sibling' between 'old_prev' and 'node'.
    if (old_prev)
    {
        old_prev->next = sibling;
    }
    sibling->prev = old_prev;
    sibling->next = node;
    node->prev = sibling;

    // Set new parent.
    cmark_node* parent = node->parent;
    sibling->parent = parent;

    // Adjust first_child of parent if inserted as first child.
    if (parent && !old_prev)
    {
        parent->first_child = sibling;
    }

    return 1;
}

int cmark_node_insert_after(cmark_node* node, cmark_node* sibling)
{
    if (node == nullptr || sibling == nullptr)
    {
        return 0;
    }

    if (!node->parent || !S_can_contain(node->parent, sibling))
    {
        return 0;
    }

    S_node_unlink(sibling);

    cmark_node* old_next = node->next;

    // Insert 'sibling' between 'node' and 'old_next'.
    if (old_next)
    {
        old_next->prev = sibling;
    }
    sibling->next = old_next;
    sibling->prev = node;
    node->next = sibling;

    // Set new parent.
    cmark_node* parent = node->parent;
    sibling->parent = parent;

    // Adjust last_child of parent if inserted as last child.
    if (parent && !old_next)
    {
        parent->last_child = sibling;
    }

    return 1;
}

int cmark_node_replace(cmark_node* oldnode, cmark_node* newnode)
{
    if (!cmark_node_insert_before(oldnode, newnode))
    {
        return 0;
    }
    cmark_node_unlink(oldnode);
    return 1;
}

int cmark_node_prepend_child(cmark_node* node, cmark_node* child)
{
    if (!S_can_contain(node, child))
    {
        return 0;
    }

    S_node_unlink(child);

    cmark_node* old_first_child = node->first_child;

    child->next = old_first_child;
    child->prev = nullptr;
    child->parent = node;
    node->first_child = child;

    if (old_first_child)
    {
        old_first_child->prev = child;
    }
    else
    {
        // Also set last_child if node previously had no children.
        node->last_child = child;
    }

    return 1;
}

int cmark_node_append_child(cmark_node* node, cmark_node* child)
{
    if (!S_can_contain(node, child))
    {
        return 0;
    }

    S_node_unlink(child);

    cmark_node* old_last_child = node->last_child;

    child->next = nullptr;
    child->prev = old_last_child;
    child->parent = node;
    node->last_child = child;

    if (old_last_child)
    {
        old_last_child->next = child;
    }
    else
    {
        // Also set first_child if node previously had no children.
        node->first_child = child;
    }

    return 1;
}

static void S_print_error(FILE* out, cmark_node* node, const char* elem)
{
    if (out == nullptr)
    {
        return;
    }
    std::println(out, "Invalid '{}' in node type {} at {}:{}", elem,
                 cmark_node_get_type_string(node), node->start_line, node->start_column);
}

int cmark_node_check(cmark_node* node, FILE* out)
{
    cmark_node* cur = nullptr;
    int errors = 0;

    if (!node)
    {
        return 0;
    }

    cur = node;
    while (true)
    {
        if (cur->first_child)
        {
            if (cur->first_child->prev != nullptr)
            {
                S_print_error(out, cur->first_child, "prev");
                cur->first_child->prev = nullptr;
                ++errors;
            }
            if (cur->first_child->parent != cur)
            {
                S_print_error(out, cur->first_child, "parent");
                cur->first_child->parent = cur;
                ++errors;
            }
            cur = cur->first_child;
            continue;
        }

        // Walk up until we find a node with an unvisited sibling or reach root
        while (cur != node)
        {
            if (cur->next)
            {
                if (cur->next->prev != cur)
                {
                    S_print_error(out, cur->next, "prev");
                    cur->next->prev = cur;
                    ++errors;
                }
                if (cur->next->parent != cur->parent)
                {
                    S_print_error(out, cur->next, "parent");
                    cur->next->parent = cur->parent;
                    ++errors;
                }
                cur = cur->next;
                break;
            }

            if (cur->parent->last_child != cur)
            {
                S_print_error(out, cur->parent, "last_child");
                cur->parent->last_child = cur;
                ++errors;
            }
            cur = cur->parent;
        }

        if (cur == node)
        {
            break;
        }
    }

    return errors;
}
