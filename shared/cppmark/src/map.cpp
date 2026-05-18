// Hash map implementation for references and footnotes
// Purpose: Stores [ref]: url definitions and [^footnote] content with fast lookup by label
// Key functions: cmark_map_new(), cmark_map_lookup() (case-insensitive label matching)
// Implementation: Chained hashing with bucket array, normalized labels (case-fold, collapse
// whitespace) Status: Phase 1.5 modernization - converted to std::unique_ptr (future: replace with
// std::unordered_map) Dependencies: chunk.hxx, map.hxx, utf8.hxx for case folding

#include <cassert>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <memory>

#include "map.hxx"
#include "parser.hxx"
#include "utf8.hxx"

// normalize map label:  collapse internal whitespace to single space,
// remove leading/trailing whitespace, case fold
// Return nullptr if the label is actually empty (i.e. composed solely from
// whitespace)
std::string normalize_map_label(cmark_chunk* reference_chunk)
{
    if (reference_chunk == nullptr || reference_chunk->len == 0)
    {
        return {};
    }

    CMarkStringBuffer normalized;
    cmark_utf8proc_case_fold(&normalized, reference_chunk->data, reference_chunk->len);
    normalized.Trim();
    normalized.NormalizeWhitespace();

    std::string result = normalized.Detach();
    if (result.empty() || result[0] == '\0')
    {
        return {};
    }
    return result;
}

static int labelcmp(const std::string& label_a, const std::string& label_b)
{
    if (label_a < label_b)
    {
        return -1;
    }
    if (label_a > label_b)
    {
        return 1;
    }
    return 0;
}

static int refcmp(const void* left_ptr, const void* right_ptr)
{
    const cmark_map_entry* left_entry = *static_cast<cmark_map_entry* const*>(left_ptr);
    const cmark_map_entry* right_entry = *static_cast<cmark_map_entry* const*>(right_ptr);
    const int comparison = labelcmp(left_entry->label, right_entry->label);
    return comparison != 0 ?
               comparison :
               (static_cast<int>(left_entry->age) - static_cast<int>(right_entry->age));
}

static int refsearch(const void* label, const void* entry_ptr)
{
    const char* key = static_cast<const char*>(label);
    const cmark_map_entry* entry = *static_cast<cmark_map_entry* const*>(entry_ptr);
    const int result = std::strcmp(key, entry->label.c_str());
    return result;
}

static void sort_map(cmark_map* map_ptr)
{
    size_t index = 0;
    size_t last = 0;
    const size_t size = map_ptr->size;
    cmark_map_entry* entry = map_ptr->refs;

    std::unique_ptr<cmark_map_entry*[]> sorted_ptr = std::make_unique<cmark_map_entry*[]>(size);
    cmark_map_entry** sorted = sorted_ptr.get();

    while (entry)
    {
        sorted[index++] = entry;
        entry = entry->next;
    }

    qsort(sorted, size, sizeof(cmark_map_entry*), refcmp);

    for (index = 1; index < size; ++index)
    {
        if (labelcmp(sorted[index]->label, sorted[last]->label) != 0)
        {
            sorted[++last] = sorted[index];
        }
    }

    map_ptr->sorted = sorted_ptr.release();
    map_ptr->size = last + 1;
}

cmark_map_entry* cmark_map_lookup(cmark_map* map_ptr, cmark_chunk* label_chunk)
{
    cmark_map_entry* const* entry = nullptr;
    const cmark_map_entry* result_entry = nullptr;

    if (label_chunk == nullptr || label_chunk->len < 1 || label_chunk->len > MAX_LINK_LABEL_LENGTH)
    {
        return nullptr;
    }
    if (map_ptr == nullptr || !map_ptr->size)
    {
        return nullptr;
    }

    const std::string normalized_label = normalize_map_label(label_chunk);
    if (normalized_label.empty())
    {
        return nullptr;
    }

    if (!map_ptr->sorted)
    {
        sort_map(map_ptr);
    }

    entry =
        static_cast<cmark_map_entry**>(bsearch(normalized_label.c_str(), map_ptr->sorted,
                                               map_ptr->size, sizeof(cmark_map_entry*), refsearch));

    if (entry != nullptr)
    {
        result_entry = entry[0];
        if (result_entry->size > map_ptr->max_ref_size - map_ptr->ref_size)
        {
            return nullptr;
        }
        map_ptr->ref_size += result_entry->size;
    }

    return const_cast<cmark_map_entry*>(result_entry);
}

void cmark_map_free(cmark_map* map_ptr)
{
    cmark_map_entry* entry = nullptr;

    if (map_ptr == nullptr)
    {
        return;
    }

    entry = map_ptr->refs;
    while (entry)
    {
        cmark_map_entry* const next_entry = entry->next;
        map_ptr->free(map_ptr, entry);
        entry = next_entry;
    }

    delete[] map_ptr->sorted;
    delete map_ptr;
}

cmark_map* cmark_map_new(cmark_map_free_f free_func)
{
    std::unique_ptr<cmark_map> map_ptr = std::make_unique<cmark_map>();
    map_ptr->free = free_func;
    map_ptr->max_ref_size = UINT_MAX;
    return map_ptr.release();
}
