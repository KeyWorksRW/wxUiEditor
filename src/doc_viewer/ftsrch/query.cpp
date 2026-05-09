/////////////////////////////////////////////////////////////////////////////
// Purpose:   Query processing for full-text search
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "query.h"

#include "compressor.h"
#include "ftsrch.h"

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <utility>

namespace ftsrch
{

    void Query::AddTerm(ConceptId concept_id, float boost)
    {
        m_terms.push_back({ concept_id, boost });
    }

    void Query::AddPrefixTerms(std::span<const ConceptId> concepts, float boost)
    {
        if (concepts.empty())
        {
            return;
        }
        PrefixGroup group;
        group.concepts.assign(concepts.begin(), concepts.end());
        group.boost = boost;
        m_prefix_groups.push_back(std::move(group));
    }

    std::vector<QueryResult> Query::Execute(const Collection& collection,
                                            [[maybe_unused]] const Dictionary& dictionary,
                                            std::uint32_t max_results)
    {
        // Accumulate scores per document
        std::unordered_map<DocId, float> doc_scores;

        const std::uint32_t doc_count = collection.DocCount();

        // Helper: accumulate score from a concept's doc list
        auto accumulate_concept =
            [&](ConceptId concept_id, float boost, const Collection& source_collection)
        {
            const std::span<const std::uint32_t> packed_doc_list =
                source_collection.GetCompressedDocList(concept_id);
            if (packed_doc_list.empty())
            {
                return;
            }

            // Get doc count for this concept from compressed header
            // DecompressSortedIds needs count + universe
            // The compressed header stores count in the first word
            // (lower 27 bits)
            const std::uint32_t header_word = packed_doc_list[0];
            const std::uint32_t count = (header_word & 0x07FF'FFFFU);

            const std::vector<std::uint32_t> doc_ids =
                DecompressSortedIds(packed_doc_list, count, doc_count);

            const double weight_scale = static_cast<double>(WT_ONE);
            for (const DocId doc_id: doc_ids)
            {
                const Weight doc_weight = source_collection.GetDocWeight(concept_id, doc_id);
                const float score =
                    (static_cast<float>(doc_weight) / static_cast<float>(weight_scale)) * boost;
                doc_scores[doc_id] += score;
            }
        };

        // Process exact terms
        for (const auto& term: m_terms)
        {
            accumulate_concept(term.concept_id, term.boost, collection);
        }

        // Process prefix groups (OR semantics — any matching
        // concept contributes)
        for (const auto& group: m_prefix_groups)
        {
            for (const ConceptId concept_id: group.concepts)
            {
                accumulate_concept(concept_id, group.boost, collection);
            }
        }

        // Sort by score descending
        std::vector<QueryResult> results;
        results.reserve(doc_scores.size());
        for (const auto& [doc_id, score]: doc_scores)
        {
            results.push_back({ doc_id, score, {} });
        }

        std::ranges::sort(results,
                          [](const auto& lhs, const auto& rhs)
                          {
                              return lhs.score > rhs.score;
                          });

        // Trim to max_results
        if (results.size() > max_results)
        {
            results.resize(max_results);
        }

        return results;
    }

}  // namespace ftsrch
