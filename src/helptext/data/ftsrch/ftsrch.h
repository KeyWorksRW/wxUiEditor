/////////////////////////////////////////////////////////////////////////////
// Purpose:   Full-text search implementation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "types.h"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ftsrch
{

    // Opaque index type.  Use CreateIndex or OpenIndex to obtain one.
    class Index;

    // Custom deleter for IndexPtr so `unique_ptr<Index>` works with
    // the forward-declared (incomplete) Index type.
    struct IndexDeleter
    {
        void operator()(Index* ptr) const noexcept;
    };

    // Owning smart-pointer to an Index.
    using IndexPtr = std::unique_ptr<Index, IndexDeleter>;

    struct IndexOptions
    {
        // Optional stemmer callback.  If empty, no stemming is applied.
        StemmerFn stemmer;
    };

    // A single search result returned by Search or SearchIncremental.
    struct QueryResult
    {
        DocId doc_id;       // Document identifier supplied at indexing time.
        float score;        // Relevance score (higher is better).
        std::string title;  // Document title supplied at indexing time.
    };

    // -----------------------------------------------------------------------
    //  Building
    // -----------------------------------------------------------------------

    // Create a new, empty index ready for document insertion.
    // options  Index options (stemmer callback, etc.).
    IndexPtr CreateIndex(IndexOptions options = {});

    // index  Index returned by CreateIndex (must not be finalized).
    // doc_id  Caller-chosen document identifier (must be unique).
    // title  Human-readable title stored alongside the document.
    std::expected<void, Error> AddDocument(Index& index, DocId doc_id, std::string_view title,
                                           std::string_view text);

    // Finalize and persist the index to a .kfts file.
    // After calling this the index is finalized and no more documents may
    // be added.  The file can later be loaded with OpenIndex.
    std::expected<void, Error> SaveIndex(Index& index, const std::filesystem::path& file_path);

    // -----------------------------------------------------------------------
    //  Searching
    // -----------------------------------------------------------------------

    // Open a previously saved .kfts index file for searching.
    // file_path  Path to a .kfts file produced by SaveIndex.
    std::expected<IndexPtr, Error> OpenIndex(const std::filesystem::path& file_path);

    // Open a .kfts index from an in-memory buffer.
    // This avoids writing to a temporary file when the index data is
    // already available in memory (e.g., extracted from a ZIP archive).
    // data  Buffer containing the serialized .kfts index.
    std::expected<IndexPtr, Error> OpenIndex(std::span<const std::byte> data);

    // Run a full-text search against a loaded index.
    // Query tokens support AND (required), NOT (prohibited), and OR (default,
    // optional).  Results are sorted by descending relevance score.
    // index  Index loaded via OpenIndex.
    // query  Space-separated search terms.
    std::expected<std::vector<QueryResult>, Error> Search(const Index& index,
                                                          std::string_view query);

    // Incremental (as-you-type) search.
    // All tokens except the last must match exactly; the last token is
    // treated as a prefix (matching any word that starts with it).
    // index  Index loaded via OpenIndex.
    // partial_query  Partial query string (last token = prefix).
    std::expected<std::vector<QueryResult>, Error>
        SearchIncremental(const Index& index, std::string_view partial_query);

}  // namespace ftsrch
