/////////////////////////////////////////////////////////////////////////////
// Purpose:   Full-text search implementation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "ftsrch.h"

#include "collection.h"
#include "dictionary.h"
#include "index_file.h"
#include "phrase_table.h"
#include "query.h"
#include "tokenizer.h"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ftsrch
{

    enum class IndexState : std::uint8_t
    {
        building,
        finalized
    };

    class Index
    {
    public:
        IndexState state = IndexState::building;
        Dictionary dictionary;
        Collection collection;
        PhraseTable phrase_table;
        Tokenizer tokenizer;
        IndexMeta meta;
        WeightConfig weight_config;
        std::vector<std::pair<DocId, std::string>> titles;

        explicit Index(StemmerFn stemmer_fn) : dictionary(std::move(stemmer_fn)) {}

        Index() = default;
    };

    void IndexDeleter::operator()(Index* ptr) const noexcept
    {
        delete ptr;
    }

    IndexPtr CreateIndex(IndexOptions options)
    {
        return IndexPtr(new Index(std::move(options.stemmer)));
    }

    std::expected<void, Error> AddDocument(Index& index, DocId doc_id, std::string_view title,
                                           std::string_view text)
    {
        if (index.state != IndexState::building)
        {
            return std::unexpected(Error::index_already_finalized);
        }

        index.collection.BeginDocument(doc_id);

        // Tokenize and index title words
        const std::vector<Token> title_tokens = index.tokenizer.Tokenize(title);
        for (const Token& token: title_tokens)
        {
            if (token.type != TokenType::word)
            {
                continue;
            }

            const ConceptId concept_id = index.dictionary.AddWord(token.text);
            if (concept_id != STOP_WORD)
            {
                index.collection.RecordConcept(concept_id);
            }
        }

        // Tokenize and index body text words
        const std::vector<Token> text_tokens = index.tokenizer.Tokenize(text);
        for (const Token& token: text_tokens)
        {
            if (token.type != TokenType::word)
            {
                continue;
            }

            const ConceptId concept_id = index.dictionary.AddWord(token.text);
            if (concept_id != STOP_WORD)
            {
                index.collection.RecordConcept(concept_id);
            }
        }

        index.collection.EndDocument();

        // Accumulate phrase table statistics
        index.phrase_table.Scan(title);
        index.phrase_table.Scan(text);

        // Store title for result display
        index.titles.emplace_back(doc_id, std::string(title));

        return {};
    }

    std::expected<void, Error> SaveIndex(Index& index, const std::filesystem::path& file_path)
    {
        if (index.state == IndexState::building)
        {
            index.dictionary.Finalize();
            index.collection.Finalize(index.weight_config);
            index.phrase_table.Build();
            index.state = IndexState::finalized;
        }

        return IndexFile::Save(file_path, index.meta, index.dictionary, index.collection,
                               index.phrase_table, index.titles);
    }

    std::expected<IndexPtr, Error> OpenIndex(const std::filesystem::path& file_path)
    {
        std::expected<IndexFile, Error> result = IndexFile::Open(file_path);
        if (!result)
        {
            return std::unexpected(result.error());
        }

        IndexPtr index(new Index());
        index->state = IndexState::finalized;
        index->dictionary = result->TakeDictionary();
        index->collection = result->TakeCollection();
        index->phrase_table = result->TakePhraseTable();
        index->titles = result->TakeTitles();
        index->meta = result->TakeMeta();

        return index;
    }

    std::expected<IndexPtr, Error> OpenIndex(std::span<const std::byte> data)
    {
        std::expected<IndexFile, Error> result = IndexFile::Open(data);
        if (!result)
        {
            return std::unexpected(result.error());
        }

        IndexPtr index(new Index());
        index->state = IndexState::finalized;
        index->dictionary = result->TakeDictionary();
        index->collection = result->TakeCollection();
        index->phrase_table = result->TakePhraseTable();
        index->titles = result->TakeTitles();
        index->meta = result->TakeMeta();

        return index;
    }

    static std::string FindTitle(const Index& index, DocId doc_id)
    {
        for (const auto& [stored_doc_id, stored_title]: index.titles)
        {
            if (stored_doc_id == doc_id)
            {
                return stored_title;
            }
        }

        return {};
    }

    std::expected<std::vector<QueryResult>, Error> Search(const Index& index,
                                                          std::string_view query)
    {
        if (index.state != IndexState::finalized)
        {
            return std::unexpected(Error::index_not_finalized);
        }

        Tokenizer tokenizer {};
        const std::vector<Token> tokens = tokenizer.Tokenize(query);

        Query parsed_query {};
        for (const Token& token: tokens)
        {
            if (token.type != TokenType::word)
            {
                continue;
            }

            const std::optional<ConceptId> concept_id = index.dictionary.Lookup(token.text);
            if (concept_id && *concept_id != STOP_WORD)
            {
                parsed_query.AddTerm(*concept_id);
            }
        }

        const std::vector<QueryResult> raw_results =
            parsed_query.Execute(index.collection, index.dictionary);

        std::vector<QueryResult> output {};
        output.reserve(raw_results.size());
        for (const QueryResult& raw_result: raw_results)
        {
            output.push_back(
                { raw_result.doc_id, raw_result.score, FindTitle(index, raw_result.doc_id) });
        }

        return output;
    }

    std::expected<std::vector<QueryResult>, Error> SearchIncremental(const Index& index,
                                                                     std::string_view partial_query)
    {
        if (index.state != IndexState::finalized)
        {
            return std::unexpected(Error::index_not_finalized);
        }

        Tokenizer tokenizer {};
        const std::vector<Token> tokens = tokenizer.Tokenize(partial_query);

        // Filter to word tokens only
        std::vector<Token> word_tokens {};
        for (const Token& token: tokens)
        {
            if (token.type == TokenType::word)
            {
                word_tokens.push_back(token);
            }
        }

        if (word_tokens.empty())
        {
            return std::vector<QueryResult> {};
        }

        Query parsed_query {};

        // All tokens except last: exact match
        for (std::size_t token_index = 0; token_index + 1 < word_tokens.size(); ++token_index)
        {
            const std::optional<ConceptId> concept_id =
                index.dictionary.Lookup(word_tokens[token_index].text);
            if (concept_id && *concept_id != STOP_WORD)
            {
                parsed_query.AddTerm(*concept_id);
            }
        }

        // Last token: prefix match
        const Token& last_token = word_tokens.back();
        std::vector<ConceptId> prefix_concepts = index.dictionary.PrefixMatch(last_token.text);
        if (!prefix_concepts.empty())
        {
            parsed_query.AddPrefixTerms(prefix_concepts);
        }

        const std::vector<QueryResult> raw_results =
            parsed_query.Execute(index.collection, index.dictionary);

        std::vector<QueryResult> output {};
        output.reserve(raw_results.size());
        for (const QueryResult& raw_result: raw_results)
        {
            output.push_back(
                { raw_result.doc_id, raw_result.score, FindTitle(index, raw_result.doc_id) });
        }

        return output;
    }

}  // namespace ftsrch
