// Generic linked list implementation
// Purpose: Simple linked list for syntax extensions, inline special characters, etc.
// Key type: cmark_llist - singly-linked list with data pointer
// Status: Phase 1.5 modernization - converted to std::unique_ptr (future: replace with std::list)
// Dependencies: cmark-gfm.hxx

#include <memory>

#include "cmark-gfm.hxx"

cmark_llist* cmark_llist_append(cmark_llist* head, void* data)
{
    cmark_llist* tmp = nullptr;
    auto new_node = std::make_unique<cmark_llist>();

    new_node->data = data;
    new_node->next = nullptr;

    if (!head)
    {
        return new_node.release();
    }

    for (tmp = head; tmp->next; tmp = tmp->next)
    {
        // Advance to end of list
    }

    tmp->next = new_node.release();

    return head;
}

void cmark_llist_free_full(cmark_llist* head, cmark_free_func free_func)
{
    cmark_llist* tmp = nullptr;
    cmark_llist* prev = nullptr;

    for (tmp = head; tmp;)
    {
        if (free_func)
        {
            free_func(tmp->data);
        }

        prev = tmp;
        tmp = tmp->next;
        delete prev;
    }
}

void cmark_llist_free(cmark_llist* head)
{
    cmark_llist_free_full(head, nullptr);
}
