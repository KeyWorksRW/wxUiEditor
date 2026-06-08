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
#include <unordered_map>
#include <unordered_set>
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

    void Query::AddRequiredTerm(ConceptId concept_id, float boost)
    {
        m_required_terms.push_back({ concept_id, boost });
    }

    void Query::AddProhibitedTerm(ConceptId concept_id)
    {
        m_prohibited_concepts.push_back(concept_id);
    }

    std::vector<QueryResult> Query::Execute(const Collection& collection,
                                            [[maybe_unused]] const Dictionary& dictionary,
                                            std::uint32_t max_results)
    {
        const std::uint32_t doc_count = collection.DocCount();

        // Phase A+B: Collect required term doc sets and intersect into candidate_set
        std::unordered_set<DocId> candidate_set;
        const bool has_required = !m_required_terms.empty();

        if (has_required)
        {
            bool first = true;
            for (const auto& term: m_required_terms)
            {
                const std::span<const std::uint32_t> packed =
                    collection.GetCompressedDocList(term.concept_id);
                if (packed.empty())
                {
                    // Required term has no matching docs — intersection is empty
                    return {};
                }

                const std::uint32_t header_word = packed[0];
                const std::vector<std::uint32_t> doc_ids =
                    DecompressSortedIds(packed, 0, doc_count);

                if (first)
                {
                    candidate_set.insert(doc_ids.begin(), doc_ids.end());
                    first = false;
                }
                else
                {
                    // Intersect with existing set
                    for (std::unordered_set<DocId>::iterator it = candidate_set.begin();
                         it != candidate_set.end();)
                    {
                        if (!std::ranges::binary_search(doc_ids, *it))
                        {
                            it = candidate_set.erase(it);
                        }
                        else
                        {
                            ++it;
                        }
                    }
                    if (candidate_set.empty())
                    {
                        return {};
                    }
                }
            }
        }

        // Phase C: Collect prohibited docs for subtraction
        std::unordered_set<DocId> prohibited_set;
        const bool has_prohibited = !m_prohibited_concepts.empty();

        if (has_prohibited)
        {
            for (const ConceptId concept_id: m_prohibited_concepts)
            {
                const std::span<const std::uint32_t> packed =
                    collection.GetCompressedDocList(concept_id);
                if (packed.empty())
                {
                    continue;
                }

                const std::uint32_t header_word = packed[0];
                const std::vector<std::uint32_t> doc_ids =
                    DecompressSortedIds(packed, 0, doc_count);

                for (const DocId doc_id: doc_ids)
                {
                    prohibited_set.insert(doc_id);
                    if (has_required)
                    {
                        candidate_set.erase(doc_id);
                    }
                }
            }
        }

        // Phase D: Score only candidates
        std::unordered_map<DocId, float> doc_scores;

        // Helper: accumulate score from a concept's doc list,
        // skipping documents outside the candidate set.
        auto accumulate_concept = [&](ConceptId concept_id, float boost)
        {
            const std::span<const std::uint32_t> packed_doc_list =
                collection.GetCompressedDocList(concept_id);
            if (packed_doc_list.empty())
            {
                return;
            }

            const std::uint32_t header_word = packed_doc_list[0];

            const std::vector<std::uint32_t> doc_ids =
                DecompressSortedIds(packed_doc_list, 0, doc_count);

            const double weight_scale = static_cast<double>(WT_ONE);
            for (const DocId doc_id: doc_ids)
            {
                if (has_required && !candidate_set.contains(doc_id))
                {
                    continue;
                }
                if (has_prohibited && prohibited_set.contains(doc_id))
                {
                    continue;
                }

                const Weight doc_weight = collection.GetDocWeight(concept_id, doc_id);
                const float score =
                    (static_cast<float>(doc_weight) / static_cast<float>(weight_scale)) * boost;
                doc_scores[doc_id] += score;
            }
        };

        // Process exact terms (optional / OR)
        for (const auto& term: m_terms)
        {
            accumulate_concept(term.concept_id, term.boost);
        }

        // Process required terms (AND) — also scored for ranking
        for (const auto& term: m_required_terms)
        {
            accumulate_concept(term.concept_id, term.boost);
        }

        // Process prefix groups (OR semantics — any matching
        // concept contributes)
        for (const auto& group: m_prefix_groups)
        {
            for (const ConceptId concept_id: group.concepts)
            {
                accumulate_concept(concept_id, group.boost);
            }
        }

        // Phase E: Sort by score descending
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
