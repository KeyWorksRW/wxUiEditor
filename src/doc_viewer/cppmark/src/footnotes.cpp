// Footnote extension implementation
// Purpose: Creates cmark_footnote entries when parser finds [^footnote]: content
// Key functions: cmark_footnote_create(), cmark_footnote_map_new(), cmark_unlink_footnotes_map()
// Status: Phase 1.5 modernization - converted to std::unique_ptr
// Dependencies: footnotes.hxx, cmark-gfm.hxx, parser.hxx

#include <memory>

#include "cmark-gfm.hxx"
#include "footnotes.hxx"
#include "node.hxx"

static void footnote_free(cmark_map* /*footnote_map*/, cmark_map_entry* ref_entry)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* ref = reinterpret_cast<cmark_footnote*>(ref_entry);
    if (ref != nullptr)
    {
        if (ref->node)
        {
            cmark_node_free(ref->node);
        }
        delete ref;
    }
}

void cmark_footnote_create(cmark_map* footnote_map, cmark_node* node)
{
    if (footnote_map == nullptr)
    {
        return;
    }

    std::string reflabel = normalize_map_label(&node->as.literal);
    if (reflabel.empty())
    {
        return;
    }

    assert(footnote_map->sorted == nullptr);

    std::unique_ptr<cmark_footnote> ref_ptr = std::make_unique<cmark_footnote>();
    cmark_footnote* ref = ref_ptr.get();
    ref->entry.label = std::move(reflabel);
    ref->node = node;
    ref->entry.age = footnote_map->size;
    ref->entry.next = footnote_map->refs;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    footnote_map->refs = reinterpret_cast<cmark_map_entry*>(ref_ptr.release());
    ++footnote_map->size;
}

cmark_map* cmark_footnote_map_new()
{
    return cmark_map_new(footnote_free);
}

// Before calling `cmark_map_free` on a map with `cmark_footnotes`, first
// unlink all of the footnote nodes before freeing their memory.
//
// Sometimes, two (unused) footnote nodes can end up referencing each other,
// which as they get freed up by calling `cmark_map_free` -> `footnote_free` ->
// etc, can lead to a use-after-free error.
//
// Better to `unlink` every footnote node first, setting their next, prev, and
// parent pointers to NULL, and only then walk thru & free them up.
void cmark_unlink_footnotes_map(cmark_map* footnote_map)
{
    cmark_map_entry* ref = nullptr;
    cmark_map_entry* next = nullptr;

    ref = footnote_map->refs;
    while (ref)
    {
        next = ref->next;
        if (((cmark_footnote*) ref)->node)
        {
            cmark_node_unlink(((cmark_footnote*) ref)->node);
        }
        ref = next;
    }
}
