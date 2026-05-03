/////////////////////////////////////////////////////////////////////////////
// Purpose:   Query processing for full-text search
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "collection.h"
#include "dictionary.h"
#include "types.h"

#include <cstdint>
#include <span>
#include <vector>

namespace ftsrch
{

    struct QueryResult;

    class Query
    {
    public:
        void AddTerm(ConceptId concept_id, float boost = 1.0F);
        void AddPrefixTerms(std::span<const ConceptId> concepts, float boost = 1.0F);
        std::vector<QueryResult> Execute(const Collection& collection, const Dictionary& dictionary,
                                         std::uint32_t max_results = 100);

    private:
        struct TermEntry
        {
            ConceptId concept_id;
            float boost;
        };

        struct PrefixGroup
        {
            std::vector<ConceptId> concepts;
            float boost;
        };

        std::vector<TermEntry> m_terms;
        std::vector<PrefixGroup> m_prefix_groups;
    };

}  // namespace ftsrch
