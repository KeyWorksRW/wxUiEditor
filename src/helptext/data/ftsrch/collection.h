/////////////////////////////////////////////////////////////////////////////
// Purpose:   Collection class implementation for building and managing the inverted index
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "types.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ftsrch
{

    enum class TfScheme : std::uint8_t
    {
        binary,
        max_norm,
        aug_norm
    };
    enum class IdfScheme : std::uint8_t
    {
        standard,
        probabilistic
    };
    enum class NormScheme : std::uint8_t
    {
        none,
        cosine,
        sum
    };

    struct WeightConfig
    {
        TfScheme tf_scheme = TfScheme::aug_norm;
        IdfScheme idf_scheme = IdfScheme::standard;
        NormScheme norm_scheme = NormScheme::cosine;
    };

    class Collection
    {
    public:
        void BeginDocument(DocId doc_id);
        void RecordConcept(ConceptId concept_id);
        void EndDocument();
        void Finalize(const WeightConfig& config = {});

        std::uint32_t DocCount() const;
        std::uint32_t ConceptCount() const;
        std::span<const std::uint32_t> GetCompressedDocList(ConceptId concept_id) const;
        Weight GetDocWeight(ConceptId concept_id, DocId doc_id) const;

        std::vector<std::uint8_t> Serialize() const;
        static Collection Deserialize(std::span<const std::byte> data);

    private:
        // Build phase — per-document concept frequencies
        struct DocRecord
        {
            DocId doc_id = 0;
            std::unordered_map<ConceptId, std::uint16_t> term_freqs;
        };

        std::vector<DocRecord> m_documents;
        DocRecord* m_current_doc = nullptr;
        uint32_t m_max_concept_id = 0;

        // After finalize — inverted index per concept
        struct ConceptEntry
        {
            std::vector<std::uint32_t> compressed_docs;
            std::uint32_t doc_count = 0;
            // weight lookup: sorted by DocId for binary-search access
            std::vector<std::pair<DocId, Weight>> doc_weights;
        };

        std::vector<ConceptEntry> m_concept_entries;
        std::uint32_t m_doc_count = 0;
        std::uint32_t m_concept_count = 0;
        bool m_finalized = false;
    };

}  // namespace ftsrch
