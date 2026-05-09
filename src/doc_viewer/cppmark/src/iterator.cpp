// AST tree iterator implementation
// Purpose: Event-based traversal of markdown AST (ENTER node, visit children, EXIT node)
// Key functions: cmark_iter_new(), cmark_iter_next() (returns ENTER/EXIT/DONE events)
// Use case: Renderers use this to walk AST in depth-first order
// Status: Phase 1.5 modernization - converted to std::unique_ptr
// Dependencies: cmark-gfm.hxx, node.hxx, iterator.hxx

#include <cassert>
#include <memory>

#include "cmark-gfm.hxx"
#include "node.hxx"

#include "iterator.hxx"

cmark_iter* cmark_iter_new(cmark_node* root)
{
    if (root == nullptr)
    {
        return nullptr;
    }
    std::unique_ptr<cmark_iter> iter = std::make_unique<cmark_iter>();
    iter->root = root;
    iter->cur.ev_type = CMARK_EVENT_NONE;
    iter->cur.node = nullptr;
    iter->next.ev_type = CMARK_EVENT_ENTER;
    iter->next.node = root;
    return iter.release();
}

void cmark_iter_free(cmark_iter* iter)
{
    delete iter;
}

static bool IsLeafNode(const cmark_node* node)
{
    switch (node->type)
    {
        case CMARK_NODE_HTML_BLOCK:
        case CMARK_NODE_THEMATIC_BREAK:
        case CMARK_NODE_CODE_BLOCK:
        case CMARK_NODE_TEXT:
        case CMARK_NODE_SOFTBREAK:
        case CMARK_NODE_LINEBREAK:
        case CMARK_NODE_CODE:
        case CMARK_NODE_HTML_INLINE:
            return true;
    }
    return false;
}

cmark_event_type cmark_iter_next(cmark_iter* iter)
{
    const cmark_event_type event_type = iter->next.ev_type;
    cmark_node* node = iter->next.node;

    iter->cur.ev_type = event_type;
    iter->cur.node = node;

    if (event_type == CMARK_EVENT_DONE)
    {
        return event_type;
    }

    /* roll forward to next item, setting both fields */
    if (event_type == CMARK_EVENT_ENTER && !IsLeafNode(node))
    {
        if (node->first_child == nullptr)
        {
            /* stay on this node but exit */
            iter->next.ev_type = CMARK_EVENT_EXIT;
        }
        else
        {
            iter->next.ev_type = CMARK_EVENT_ENTER;
            iter->next.node = node->first_child;
        }
    }
    else if (node == iter->root)
    {
        /* don't move past root */
        iter->next.ev_type = CMARK_EVENT_DONE;
        iter->next.node = nullptr;
    }
    else if (node->next)
    {
        iter->next.ev_type = CMARK_EVENT_ENTER;
        iter->next.node = node->next;
    }
    else if (node->parent)
    {
        iter->next.ev_type = CMARK_EVENT_EXIT;
        iter->next.node = node->parent;
    }
    else
    {
        assert(false);
        iter->next.ev_type = CMARK_EVENT_DONE;
        iter->next.node = nullptr;
    }

    return event_type;
}

void cmark_iter_reset(cmark_iter* iter, cmark_node* current, cmark_event_type event_type)
{
    iter->next.ev_type = event_type;
    iter->next.node = current;
    cmark_iter_next(iter);
}

cmark_node* cmark_iter_get_node(cmark_iter* iter)
{
    return iter->cur.node;
}

cmark_event_type cmark_iter_get_event_type(cmark_iter* iter)
{
    return iter->cur.ev_type;
}

cmark_node* cmark_iter_get_root(cmark_iter* iter)
{
    return iter->root;
}

void cmark_consolidate_text_nodes(cmark_node* root)
{
    if (root == nullptr)
    {
        return;
    }
    cmark_iter* iter = cmark_iter_new(root);
    cmark_strbuf buf = cmark_strbuf();
    cmark_event_type event_type = CMARK_EVENT_NONE;
    cmark_node* current_node = nullptr;
    cmark_node* text_node = nullptr;
    cmark_node* next_node = nullptr;

    while ((event_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE)
    {
        current_node = cmark_iter_get_node(iter);
        if (event_type == CMARK_EVENT_ENTER && current_node->type == CMARK_NODE_TEXT &&
            current_node->next != nullptr && current_node->next->type == CMARK_NODE_TEXT)
        {
            buf.Clear();
            buf.Put(current_node->as.literal.data, current_node->as.literal.len);
            text_node = current_node->next;
            while (text_node != nullptr && text_node->type == CMARK_NODE_TEXT)
            {
                cmark_iter_next(iter);  // advance pointer
                buf.Put(text_node->as.literal.data, text_node->as.literal.len);
                current_node->end_column = text_node->end_column;
                next_node = text_node->next;
                cmark_node_free(text_node);
                text_node = next_node;
            }
            cmark_chunk_free(&current_node->as.literal);
            current_node->as.literal = cmark_chunk_buf_detach(&buf);
        }
    }

    buf.Free();
    cmark_iter_free(iter);
}

void cmark_node_own(cmark_node* root)
{
    if (root == nullptr)
    {
        return;
    }
    cmark_iter* iter = cmark_iter_new(root);
    cmark_event_type event_type = CMARK_EVENT_NONE;
    cmark_node* current_node = nullptr;

    while ((event_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE)
    {
        current_node = cmark_iter_get_node(iter);
        if (event_type == CMARK_EVENT_ENTER)
        {
            switch (current_node->type)
            {
                case CMARK_NODE_TEXT:
                case CMARK_NODE_HTML_INLINE:
                case CMARK_NODE_CODE:
                case CMARK_NODE_HTML_BLOCK:
                    cmark_chunk_to_cstr(&current_node->as.literal);
                    break;
                case CMARK_NODE_LINK:
                    cmark_chunk_to_cstr(&current_node->as.link.url);
                    cmark_chunk_to_cstr(&current_node->as.link.title);
                    break;
                case CMARK_NODE_CUSTOM_INLINE:
                    cmark_chunk_to_cstr(&current_node->as.custom.on_enter);
                    cmark_chunk_to_cstr(&current_node->as.custom.on_exit);
                    break;
            }
        }
    }

    cmark_iter_free(iter);
}
