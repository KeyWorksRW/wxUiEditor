// AST tree iterator for traversing markdown nodes
// Purpose: Provides event-based traversal (ENTER/EXIT events for each node)
// Key type: cmark_iter - maintains current position and next event
// Usage: Create iterator on root -> call cmark_iter_next() -> get ENTER/EXIT events
// Dependencies: cmark-gfm.hxx

#pragma once

#include "cmark-gfm.hxx"

struct cmark_iter_state
{
    cmark_event_type ev_type;
    cmark_node* node;
};

struct cmark_iter
{
    cmark_node* root;
    cmark_iter_state cur;
    cmark_iter_state next;
};
