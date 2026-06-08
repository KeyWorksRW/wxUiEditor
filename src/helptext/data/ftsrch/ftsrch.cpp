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

#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
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
        mutable std::unordered_map<DocId, std::string> title_lookup;
        mutable bool title_lookup_valid = false;

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
        index.title_lookup_valid = false;

        return {};
    }

    std::expected<void, Error> SaveIndex(Index& index, const std::filesystem::path& file_path)
    {
        if (index.state == IndexState::building)
        {
            index.dictionary.Finalize();
            index.collection.Finalize(index.weight_config);
            index.phrase_table.Build();
        }

        std::expected<void, Error> save_result =
            IndexFile::Save(file_path, index.meta, index.dictionary, index.collection,
                            index.phrase_table, index.titles);
        if (save_result)
        {
            index.state = IndexState::finalized;
        }
        return save_result;
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
        if (!index.title_lookup_valid)
        {
            index.title_lookup.clear();
            for (const auto& [stored_doc_id, stored_title]: index.titles)
            {
                index.title_lookup[stored_doc_id] = stored_title;
            }
            index.title_lookup_valid = true;
        }

        if (index.title_lookup.contains(doc_id))
        {
            return index.title_lookup.at(doc_id);
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

        std::unordered_set<ConceptId> seen_concepts;

        // Operator-parsing state machine:
        // Interpose between tokenization and dictionary lookup so that
        // "AND", "NOT", and "OR" are never looked up in the dictionary.
        enum class OpState
        {
            optional,
            required,
            prohibited
        };
        OpState op_state = OpState::optional;

        for (const Token& token: tokens)
        {
            if (token.type != TokenType::word)
            {
                continue;
            }

            // Check for operator keywords (full-word, uppercase only)
            if (token.text.size() == 3)
            {
                if (token.text[0] == 'A' && token.text[1] == 'N' && token.text[2] == 'D')
                {
                    op_state = OpState::required;
                    continue;
                }
                if (token.text[0] == 'N' && token.text[1] == 'O' && token.text[2] == 'T')
                {
                    op_state = OpState::prohibited;
                    continue;
                }
            }
            if (token.text.size() == 2)
            {
                if (token.text[0] == 'O' && token.text[1] == 'R')
                {
                    // OR is the default -- reset to optional, never a term
                    op_state = OpState::optional;
                    continue;
                }
            }

            const std::optional<ConceptId> concept_id = index.dictionary.Lookup(token.text);
            if (concept_id && *concept_id != STOP_WORD)
            {
                if (!seen_concepts.insert(*concept_id).second)
                {
                    op_state = OpState::optional;
                    continue;
                }

                switch (op_state)
                {
                    case OpState::required:
                        parsed_query.AddRequiredTerm(*concept_id);
                        break;
                    case OpState::prohibited:
                        parsed_query.AddProhibitedTerm(*concept_id);
                        break;
                    case OpState::optional:
                        parsed_query.AddTerm(*concept_id);
                        break;
                }
                // Reset to optional after consuming a term
                op_state = OpState::optional;
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

        // First pass: filter to word tokens, parsing AND/NOT/OR operators
        // to determine the state for each content token.
        struct ContentToken
        {
            std::string_view text;
            bool is_required;
            bool is_prohibited;
        };

        enum class OpState
        {
            optional,
            required,
            prohibited
        };
        OpState op_state = OpState::optional;
        std::vector<ContentToken> content_tokens {};

        for (const Token& token: tokens)
        {
            if (token.type != TokenType::word)
            {
                continue;
            }

            // Check for operator keywords (full-word, uppercase only)
            if (token.text.size() == 3)
            {
                if (token.text[0] == 'A' && token.text[1] == 'N' && token.text[2] == 'D')
                {
                    op_state = OpState::required;
                    continue;
                }
                if (token.text[0] == 'N' && token.text[1] == 'O' && token.text[2] == 'T')
                {
                    op_state = OpState::prohibited;
                    continue;
                }
            }
            if (token.text.size() == 2)
            {
                if (token.text[0] == 'O' && token.text[1] == 'R')
                {
                    op_state = OpState::optional;
                    continue;
                }
            }

            content_tokens.push_back({
                token.text,
                op_state == OpState::required,
                op_state == OpState::prohibited,
            });
            op_state = OpState::optional;
        }

        if (content_tokens.empty())
        {
            return std::vector<QueryResult> {};
        }

        Query parsed_query {};

        std::unordered_set<ConceptId> seen_concepts;

        // All content tokens except last: exact match
        for (std::size_t token_index = 0; token_index + 1 < content_tokens.size(); ++token_index)
        {
            const ContentToken& content_token = content_tokens[token_index];
            const std::optional<ConceptId> concept_id = index.dictionary.Lookup(content_token.text);
            if (concept_id && *concept_id != STOP_WORD)
            {
                if (!seen_concepts.insert(*concept_id).second)
                {
                    continue;
                }

                if (content_token.is_required)
                {
                    parsed_query.AddRequiredTerm(*concept_id);
                }
                else if (content_token.is_prohibited)
                {
                    parsed_query.AddProhibitedTerm(*concept_id);
                }
                else
                {
                    parsed_query.AddTerm(*concept_id);
                }
            }
        }

        // Last content token: prefix match
        const ContentToken& last_ct = content_tokens.back();
        const std::vector<ConceptId> prefix_concepts = index.dictionary.PrefixMatch(last_ct.text);
        if (!prefix_concepts.empty())
        {
            if (last_ct.is_required)
            {
                for (const ConceptId cid_val: prefix_concepts)
                {
                    if (seen_concepts.insert(cid_val).second)
                    {
                        parsed_query.AddRequiredTerm(cid_val);
                    }
                }
            }
            else if (last_ct.is_prohibited)
            {
                for (const ConceptId cid_val: prefix_concepts)
                {
                    if (seen_concepts.insert(cid_val).second)
                    {
                        parsed_query.AddProhibitedTerm(cid_val);
                    }
                }
            }
            else
            {
                std::vector<ConceptId> filtered_prefix_concepts;
                filtered_prefix_concepts.reserve(prefix_concepts.size());
                for (const ConceptId cid_val: prefix_concepts)
                {
                    if (seen_concepts.insert(cid_val).second)
                    {
                        filtered_prefix_concepts.push_back(cid_val);
                    }
                }
                if (!filtered_prefix_concepts.empty())
                {
                    parsed_query.AddPrefixTerms(filtered_prefix_concepts);
                }
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

}  // namespace ftsrch
