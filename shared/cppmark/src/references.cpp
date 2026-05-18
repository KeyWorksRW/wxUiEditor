// Reference link creation and storage
// Purpose: Creates cmark_reference entries in hash map when parser finds [ref]: url "title"
// Key functions: cmark_reference_create(), cmark_reference_map_new()
// Status: Phase 1.5 modernization - converted to std::unique_ptr
// Dependencies: chunk.hxx, cmark-gfm.hxx, inlines.hxx, parser.hxx, references.hxx

#include <memory>

#include "chunk.hxx"
#include "inlines.hxx"

#include "references.hxx"

static void reference_free([[maybe_unused]] cmark_map* ref_map, cmark_map_entry* ref_entry)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* reference = reinterpret_cast<cmark_reference*>(ref_entry);
    if (reference != nullptr)
    {
        cmark_chunk_free(&reference->url);
        cmark_chunk_free(&reference->title);
        delete reference;
    }
}

void cmark_reference_create(cmark_map* ref_map, cmark_chunk* ref_label, cmark_chunk* reference_url,
                            cmark_chunk* title)
{
    if (ref_map == nullptr)
    {
        return;
    }

    std::string normalized_label = normalize_map_label(ref_label);
    if (normalized_label.empty())
    {
        return;
    }

    assert(ref_map->sorted == nullptr);

    std::unique_ptr<cmark_reference> reference_ptr = std::make_unique<cmark_reference>();
    cmark_reference* reference = reference_ptr.get();
    reference->entry.label = std::move(normalized_label);
    reference->url = cmark_clean_url(reference_url);
    reference->title = cmark_clean_title(title);
    reference->entry.age = ref_map->size;
    reference->entry.next = ref_map->refs;
    reference->entry.size = reference->url.len + reference->title.len;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ref_map->refs = reinterpret_cast<cmark_map_entry*>(reference_ptr.release());
    ++ref_map->size;
}

cmark_map* cmark_reference_map_new()
{
    return cmark_map_new(reference_free);
}
