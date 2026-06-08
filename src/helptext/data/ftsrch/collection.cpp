/////////////////////////////////////////////////////////////////////////////
// Purpose:   Collection class implementation for building and managing the inverted index
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "collection.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstring>

#include "compressor.h"

namespace ftsrch
{

    void Collection::BeginDocument(DocId doc_id)
    {
        assert(!m_finalized);
        m_documents.push_back({ doc_id, {} });
        m_current_doc = &m_documents.back();
    }

    void Collection::RecordConcept(ConceptId concept_id)
    {
        if (m_current_doc == nullptr)
        {
            return;
        }
        assert(concept_id != STOP_WORD);  // callers must filter stop words before recording
        ++m_current_doc->term_freqs[concept_id];
        m_max_concept_id = std::max(m_max_concept_id, concept_id);
    }

    void Collection::EndDocument()
    {
        m_current_doc = nullptr;
    }

    void Collection::Finalize(const WeightConfig& config)
    {
        if (m_finalized)
        {
            return;
        }

        m_doc_count = static_cast<uint32_t>(m_documents.size());
        m_concept_count = m_max_concept_id + 1;
        m_concept_entries.resize(m_concept_count);

        if (m_doc_count == 0)
        {
            m_finalized = true;
            return;
        }

        // Step 1: Compute document frequency per concept
        std::vector<uint32_t> document_frequency(m_concept_count, 0);
        for (const auto& doc_rec: m_documents)
        {
            for (const auto& [concept_id, term_frequency]: doc_rec.term_freqs)
            {
                ++document_frequency[concept_id];
            }
        }

        // Step 2: Compute IDF per concept
        const double num_docs = static_cast<double>(m_doc_count);
        std::vector<double> inverse_document_frequency(m_concept_count, 0.0);
        for (uint32_t concept_id = 0; concept_id < m_concept_count; ++concept_id)
        {
            if (document_frequency[concept_id] == 0)
            {
                continue;
            }
            if (config.idf_scheme == IdfScheme::standard)
            {
                inverse_document_frequency[concept_id] =
                    std::log(num_docs / static_cast<double>(document_frequency[concept_id]));
            }
            else
            {
                // Probabilistic
                const double document_frequency_value =
                    static_cast<double>(document_frequency[concept_id]);
                inverse_document_frequency[concept_id] =
                    std::log((num_docs - document_frequency_value) / document_frequency_value);
                inverse_document_frequency[concept_id] =
                    std::max(inverse_document_frequency[concept_id], 0.0);
            }
        }

        // Step 3: Compute TF-IDF weights per doc-concept pair and build
        //         inverted index
        // First pass: collect per-concept sorted doc lists
        std::vector<std::vector<std::pair<DocId, double>>> concept_docs(m_concept_count);

        for (const auto& doc_rec: m_documents)
        {
            // Find max term freq in this document (for normalization)
            uint16_t max_term_frequency = 0;
            for (const auto& [concept_id, term_frequency]: doc_rec.term_freqs)
            {
                max_term_frequency = std::max(max_term_frequency, term_frequency);
            }

            // Compute TF for each concept in this document
            for (const auto& [concept_id, raw_frequency]: doc_rec.term_freqs)
            {
                double term_freq = 0.0;
                const double raw_frequency_value = static_cast<double>(raw_frequency);
                const double max_frequency_value = static_cast<double>(max_term_frequency);

                if (config.tf_scheme == TfScheme::binary)
                {
                    term_freq = 1.0;
                }
                else if (config.tf_scheme == TfScheme::max_norm)
                {
                    term_freq = raw_frequency_value / max_frequency_value;
                }
                else
                {
                    // aug_norm: 0.5 + 0.5 * (tf / max_tf)
                    term_freq = 0.5 + (0.5 * (raw_frequency_value / max_frequency_value));
                }

                const double weight = term_freq * inverse_document_frequency[concept_id];
                concept_docs[concept_id].emplace_back(doc_rec.doc_id, weight);
            }
        }

        // Step 4: Normalize per document, convert to fixed-point
        // First gather per-doc weight sums for normalization
        std::unordered_map<DocId, double> doc_norm_factor;

        if (config.norm_scheme == NormScheme::cosine)
        {
            // Compute L2 norm per document
            std::unordered_map<DocId, double> doc_squared_weight_sum;
            for (uint32_t concept_id = 0; concept_id < m_concept_count; ++concept_id)
            {
                for (const auto& [document_id, document_weight]: concept_docs[concept_id])
                {
                    doc_squared_weight_sum[document_id] += document_weight * document_weight;
                }
            }
            for (auto& [document_id, squared_weight_sum]: doc_squared_weight_sum)
            {
                const double norm = std::sqrt(squared_weight_sum);
                doc_norm_factor[document_id] = (norm > 0.0) ? (1.0 / norm) : 1.0;
            }
        }
        else if (config.norm_scheme == NormScheme::sum)
        {
            // Compute sum of weights per document
            std::unordered_map<DocId, double> document_weight_sum;
            for (uint32_t concept_id = 0; concept_id < m_concept_count; ++concept_id)
            {
                for (const auto& [document_id, document_weight]: concept_docs[concept_id])
                {
                    document_weight_sum[document_id] += document_weight;
                }
            }
            for (auto& [document_id, total_weight]: document_weight_sum)
            {
                doc_norm_factor[document_id] = (total_weight > 0.0) ? (1.0 / total_weight) : 1.0;
            }
        }
        // NormScheme::none: no normalization

        // Step 5: Build final inverted index with compressed doc lists
        for (uint32_t concept_id = 0; concept_id < m_concept_count; ++concept_id)
        {
            std::vector<std::pair<DocId, double>>& concept_document_weights =
                concept_docs[concept_id];
            if (concept_document_weights.empty())
            {
                continue;
            }

            // Sort by doc_id for delta compression
            std::ranges::sort(concept_document_weights);

            ConceptEntry& entry = m_concept_entries[concept_id];
            entry.doc_count = static_cast<uint32_t>(concept_document_weights.size());

            // Build sorted doc ID list and weight vector (already sorted by doc_id)
            std::vector<uint32_t> sorted_doc_ids;
            sorted_doc_ids.reserve(concept_document_weights.size());
            entry.doc_weights.reserve(concept_document_weights.size());

            for (const auto& [document_id, raw_weight]: concept_document_weights)
            {
                sorted_doc_ids.push_back(document_id);

                double final_weight = raw_weight;
                if (config.norm_scheme != NormScheme::none)
                {
                    if (const std::unordered_map<DocId, double>::const_iterator norm_iter =
                            doc_norm_factor.find(document_id);
                        norm_iter != doc_norm_factor.end())
                    {
                        final_weight *= norm_iter->second;
                    }
                }

                // Clamp to [0, 1] and convert to uint16_t
                final_weight = std::max(final_weight, 0.0);
                final_weight = std::min(final_weight, 1.0);
                const Weight encoded_weight =
                    static_cast<Weight>(final_weight * static_cast<double>(WT_ONE));
                entry.doc_weights.emplace_back(document_id, encoded_weight);
            }

            // Compress the sorted doc ID list
            const std::uint32_t universe = m_doc_count;
            entry.compressed_docs = CompressSortedIds(sorted_doc_ids, universe);
        }

        // Free build-phase data
        m_documents.clear();
        m_documents.shrink_to_fit();

        m_finalized = true;
    }

    std::uint32_t Collection::DocCount() const
    {
        return m_doc_count;
    }

    std::uint32_t Collection::ConceptCount() const
    {
        return m_concept_count;
    }

    std::span<const std::uint32_t> Collection::GetCompressedDocList(ConceptId concept_id) const
    {
        if (concept_id >= m_concept_count)
        {
            return {};
        }
        return m_concept_entries[concept_id].compressed_docs;
    }

    Weight Collection::GetDocWeight(ConceptId concept_id, DocId doc_id) const
    {
        const ConceptEntry& entry = m_concept_entries[concept_id];
        // Binary search on sorted vector — faster than hash lookup for small-N
        // and eliminates per-element heap allocations that slow shutdown.
        const std::vector<std::pair<DocId, Weight>>::const_iterator it =
            std::lower_bound(entry.doc_weights.begin(), entry.doc_weights.end(), doc_id,
                             [](const std::pair<DocId, Weight>& pair, DocId id)
                             {
                                 return pair.first < id;
                             });
        if (it != entry.doc_weights.end() && it->first == doc_id)
        {
            return it->second;
        }
        return 0;
    }

    std::vector<std::uint8_t> Collection::Serialize() const
    {
        constexpr size_t U32_BYTES = sizeof(uint32_t);
        constexpr size_t U16_BYTES = sizeof(uint16_t);
        constexpr size_t HEADER_BYTES = 2 * U32_BYTES;        // m_doc_count + m_concept_count
        constexpr size_t ENTRY_HEADER_BYTES = 2 * U32_BYTES;  // doc_count + packed_size
        constexpr size_t WEIGHT_PAIR_BYTES = U32_BYTES + U16_BYTES;  // doc_id + weight

        std::vector<uint8_t> result;

        // Pre-compute serialized size to avoid reallocations
        size_t total_bytes = HEADER_BYTES;
        for (const auto& entry: m_concept_entries)
        {
            total_bytes += ENTRY_HEADER_BYTES + (entry.compressed_docs.size() * U32_BYTES) +
                           (static_cast<size_t>(entry.doc_count) * WEIGHT_PAIR_BYTES);
        }
        result.reserve(total_bytes);

        auto WriteU32 = [&](uint32_t value)
        {
            result.push_back(static_cast<uint8_t>(value & 0xFF));
            result.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
            result.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
            result.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
        };

        auto WriteU16 = [&](uint16_t value)
        {
            result.push_back(static_cast<uint8_t>(value & 0xFF));
            result.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        };

        // Header
        WriteU32(m_doc_count);
        WriteU32(m_concept_count);

        // Per concept: doc_count, compressed_docs, weight pairs
        for (uint32_t concept_id = 0; concept_id < m_concept_count; ++concept_id)
        {
            const ConceptEntry& entry = m_concept_entries[concept_id];
            WriteU32(entry.doc_count);

            const std::uint32_t packed_size =
                static_cast<std::uint32_t>(entry.compressed_docs.size());
            WriteU32(packed_size);
            for (auto packed_word: entry.compressed_docs)
            {
                WriteU32(packed_word);
            }

            // Write weight pairs: (doc_id, weight) for each doc
            for (const auto& [doc_id, weight]: entry.doc_weights)
            {
                WriteU32(doc_id);
                WriteU16(weight);
            }
        }

        return result;
    }

    Collection Collection::Deserialize(std::span<const std::byte> data)
    {
        Collection collection;

        constexpr size_t U32_BYTES = sizeof(uint32_t);
        constexpr size_t U16_BYTES = sizeof(uint16_t);
        constexpr size_t HEADER_BYTES = 2 * U32_BYTES;        // m_doc_count + m_concept_count
        constexpr size_t ENTRY_HEADER_BYTES = 2 * U32_BYTES;  // doc_count + packed_size
        constexpr size_t WEIGHT_PAIR_BYTES = U32_BYTES + U16_BYTES;  // doc_id + weight

        if (data.size() < HEADER_BYTES)
        {
            return collection;
        }

        const auto ReadU32 = [](const std::byte* data_ptr) -> std::uint32_t
        {
            return static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data_ptr[0])) |
                   (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data_ptr[1])) << 8U) |
                   (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data_ptr[2])) << 16U) |
                   (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data_ptr[3])) << 24U);
        };

        const auto ReadU16 = [](const std::byte* data_ptr) -> std::uint16_t
        {
            return static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(data_ptr[0])) |
                   (static_cast<std::uint16_t>(std::to_integer<std::uint8_t>(data_ptr[1])) << 8U);
        };

        const std::size_t total_bytes = data.size();
        const std::byte* data_bytes = data.data();
        std::size_t offset = 0;

        collection.m_doc_count = ReadU32(data_bytes + offset);
        offset += U32_BYTES;
        collection.m_concept_count = ReadU32(data_bytes + offset);
        offset += U32_BYTES;

        // Guard against corrupt concept_count triggering a huge allocation:
        // each concept entry occupies at least ENTRY_HEADER_BYTES
        if (collection.m_concept_count > (total_bytes - offset) / ENTRY_HEADER_BYTES)
        {
            return collection;
        }

        collection.m_concept_entries.resize(collection.m_concept_count);

        for (std::uint32_t concept_id = 0; concept_id < collection.m_concept_count; ++concept_id)
        {
            ConceptEntry& entry = collection.m_concept_entries[concept_id];

            if (offset + ENTRY_HEADER_BYTES > total_bytes)
            {
                return collection;
            }
            entry.doc_count = ReadU32(data_bytes + offset);
            offset += U32_BYTES;
            const std::uint32_t packed_size = ReadU32(data_bytes + offset);
            offset += U32_BYTES;

            if (static_cast<std::size_t>(packed_size) * U32_BYTES > total_bytes - offset)
            {
                return collection;
            }
            entry.compressed_docs.resize(packed_size);
            for (auto& word: entry.compressed_docs)
            {
                word = ReadU32(data_bytes + offset);
                offset += U32_BYTES;
            }

            // WEIGHT_PAIR_BYTES per pair: U32_BYTES (doc_id) + U16_BYTES (weight)
            if (static_cast<std::size_t>(entry.doc_count) * WEIGHT_PAIR_BYTES >
                total_bytes - offset)
            {
                return collection;
            }
            entry.doc_weights.reserve(entry.doc_count);
            for (std::uint32_t index = 0; index < entry.doc_count; ++index)
            {
                const DocId doc_id = ReadU32(data_bytes + offset);
                offset += U32_BYTES;
                const Weight weight = ReadU16(data_bytes + offset);
                offset += U16_BYTES;
                entry.doc_weights.emplace_back(doc_id, weight);
            }
            // Sort by DocId for binary-search lookup; also handles old .kfts
            // files where weight pairs were written in hash-table iteration order.
            std::ranges::sort(entry.doc_weights, {}, &std::pair<DocId, Weight>::first);
        }

        collection.m_finalized = true;
        return collection;
    }

}  // namespace ftsrch
