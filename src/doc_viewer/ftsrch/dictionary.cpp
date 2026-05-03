/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dictionary class implementation for mapping words to concept IDs
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "dictionary.h"

#include "compressor.h"
#include "tokenizer.h"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstring>
#include <set>
#include <utility>

namespace ftsrch
{

    Dictionary::Dictionary(StemmerFn stemmer) : m_stemmer(std::move(stemmer)) {}

    std::string Dictionary::StemWord(std::string_view lowered) const
    {
        if (m_stemmer)
        {
            return m_stemmer(lowered);
        }
        return std::string(lowered);
    }

    ConceptId Dictionary::AddWord(std::string_view word)
    {
        const std::string lowered_word = Tokenizer::ToLower(word);

        // Check if this exact word was already added
        if (const std::unordered_map<std::string, ConceptId>::iterator map_iterator =
                m_word_to_concept.find(lowered_word);
            map_iterator != m_word_to_concept.end())
        {
            return map_iterator->second;
        }

        const std::string stemmed_word = StemWord(lowered_word);

        // Check stop list
        if (m_stop_stems.contains(stemmed_word))
        {
            m_word_to_concept[lowered_word] = STOP_WORD;
            return STOP_WORD;
        }

        // Get or create concept for this stem
        auto [stem_iterator, inserted] =
            m_stem_to_concept.try_emplace(stemmed_word, m_next_concept_id);
        if (inserted)
        {
            ++m_next_concept_id;
        }
        const ConceptId concept_id = stem_iterator->second;

        // Map the lowered word to its concept
        m_word_to_concept[lowered_word] = concept_id;

        // Also ensure the stem itself is in the word map (for lookup by stem)
        m_word_to_concept.try_emplace(stemmed_word, concept_id);

        return concept_id;
    }

    void Dictionary::AddStopWords(std::span<const std::string> words)
    {
        for (const auto& word: words)
        {
            const std::string lowered_word = Tokenizer::ToLower(word);
            const std::string stemmed_word = StemWord(lowered_word);
            m_stop_stems.insert(stemmed_word);
        }
    }

    void Dictionary::Finalize()
    {
        if (m_finalized)
        {
            return;
        }

        m_num_concepts = m_next_concept_id;

        // Collect non-stop words into sortable pairs
        std::vector<std::pair<std::string, ConceptId>> word_pairs;
        word_pairs.reserve(m_word_to_concept.size());
        for (const auto& [word, concept_id]: m_word_to_concept)
        {
            if (concept_id != STOP_WORD)
            {
                word_pairs.emplace_back(word, concept_id);
            }
        }

        // Sort lexicographically by word
        std::ranges::sort(word_pairs,
                          [](const std::pair<std::string, ConceptId>& lhs_pair,
                             const std::pair<std::string, ConceptId>& rhs_pair)
                          {
                              return lhs_pair.first < rhs_pair.first;
                          });

        // Split into parallel arrays
        m_sorted_words.clear();
        m_sorted_concept_ids.clear();
        m_sorted_words.reserve(word_pairs.size());
        m_sorted_concept_ids.reserve(word_pairs.size());

        for (auto& [word, concept_id]: word_pairs)
        {
            m_sorted_words.push_back(std::move(word));
            m_sorted_concept_ids.push_back(concept_id);
        }

        // Compute bits needed per concept ID: ceil(log2(num_concepts + 1))
        if (m_num_concepts == 0)
        {
            m_concept_id_bits = 1U;
        }
        else
        {
            m_concept_id_bits = std::bit_width(m_num_concepts);
        }

        m_finalized = true;
    }

    std::optional<ConceptId> Dictionary::Lookup(std::string_view word) const
    {
        const std::string lowered_word = Tokenizer::ToLower(word);
        const std::string stemmed_word = StemWord(lowered_word);

        // Check stop list
        if (m_stop_stems.contains(stemmed_word))
        {
            return STOP_WORD;
        }

        // Binary search in sorted word list for the stem
        std::vector<std::string>::const_iterator word_iterator =
            std::ranges::lower_bound(m_sorted_words, stemmed_word);
        if (word_iterator != m_sorted_words.end() && *word_iterator == stemmed_word)
        {
            const std::size_t concept_index =
                static_cast<std::size_t>(std::distance(m_sorted_words.begin(), word_iterator));
            return m_sorted_concept_ids[concept_index];
        }

        // Also try the lowered form (unstemmed) for exact match
        if (lowered_word != stemmed_word)
        {
            word_iterator = std::ranges::lower_bound(m_sorted_words, lowered_word);
            if (word_iterator != m_sorted_words.end() && *word_iterator == lowered_word)
            {
                const std::size_t concept_index =
                    static_cast<std::size_t>(std::distance(m_sorted_words.begin(), word_iterator));
                return m_sorted_concept_ids[concept_index];
            }
        }

        return std::nullopt;
    }

    std::vector<ConceptId> Dictionary::PrefixMatch(std::string_view prefix) const
    {
        const std::string lowered_prefix = Tokenizer::ToLower(prefix);

        std::vector<std::string>::const_iterator word_iterator =
            std::ranges::lower_bound(m_sorted_words, lowered_prefix);

        std::set<ConceptId> unique_concepts;
        while (word_iterator != m_sorted_words.end() && word_iterator->starts_with(lowered_prefix))
        {
            const std::size_t concept_index =
                static_cast<std::size_t>(std::distance(m_sorted_words.begin(), word_iterator));
            unique_concepts.insert(m_sorted_concept_ids[concept_index]);
            ++word_iterator;
        }

        return { unique_concepts.begin(), unique_concepts.end() };
    }

    std::vector<std::uint8_t> Dictionary::Serialize() const
    {
        std::vector<std::uint8_t> result;

        const std::uint32_t word_count = static_cast<std::uint32_t>(m_sorted_words.size());

        const auto write_u32 = [&](std::uint32_t value)
        {
            result.push_back(static_cast<std::uint8_t>(value & 0xFFU));
            result.push_back(static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
            result.push_back(static_cast<std::uint8_t>((value >> 16U) & 0xFFU));
            result.push_back(static_cast<std::uint8_t>((value >> 24U) & 0xFFU));
        };

        // Header
        write_u32(word_count);
        write_u32(m_num_concepts);
        write_u32(m_concept_id_bits);

        // Sorted words (null-terminated)
        for (const auto& word: m_sorted_words)
        {
            result.insert(result.end(), word.begin(), word.end());
            result.push_back(0U);
        }

        // Bit-pack concept IDs
        BitWriter writer;
        for (const ConceptId concept_id: m_sorted_concept_ids)
        {
            writer.WriteBits(concept_id, static_cast<int>(m_concept_id_bits));
        }
        const std::vector<std::uint32_t> packed_words = writer.Finish();

        // Write packed data length and data
        write_u32(static_cast<std::uint32_t>(packed_words.size()));
        for (const std::uint32_t packed_word: packed_words)
        {
            write_u32(packed_word);
        }

        return result;
    }

    Dictionary Dictionary::Deserialize(std::span<const std::byte> data)
    {
        Dictionary dict;

        if (data.size() < 12)
        {
            return dict;
        }

        const auto read_u32 = [](const std::byte* data_ptr) -> std::uint32_t
        {
            return static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data_ptr[0])) |
                   (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data_ptr[1])) << 8U) |
                   (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data_ptr[2])) << 16U) |
                   (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data_ptr[3])) << 24U);
        };

        std::size_t offset = 0;
        const std::uint32_t word_count = read_u32(data.data() + offset);
        offset += 4;
        dict.m_num_concepts = read_u32(data.data() + offset);
        offset += 4;
        dict.m_concept_id_bits = read_u32(data.data() + offset);
        offset += 4;

        // Read sorted words (null-terminated)
        dict.m_sorted_words.reserve(word_count);
        for (std::uint32_t index = 0; index < word_count; ++index)
        {
            const char* word_start = std::bit_cast<const char*>(data.data() + offset);
            const std::size_t word_length = std::strlen(word_start);
            dict.m_sorted_words.emplace_back(word_start, word_length);
            offset += word_length + 1;
        }

        // Read packed concept IDs
        const std::uint32_t packed_count = read_u32(data.data() + offset);
        offset += 4;
        std::vector<std::uint32_t> packed_words(packed_count);
        for (std::uint32_t index = 0; index < packed_count; ++index)
        {
            packed_words[index] = read_u32(data.data() + offset);
            offset += 4;
        }

        // Unpack concept IDs
        dict.m_sorted_concept_ids.reserve(word_count);
        if (!packed_words.empty() && word_count > 0U)
        {
            BitReader reader(packed_words);
            for (std::uint32_t index = 0; index < word_count; ++index)
            {
                dict.m_sorted_concept_ids.push_back(
                    reader.ReadBits(static_cast<int>(dict.m_concept_id_bits)));
            }
        }

        dict.m_finalized = true;
        return dict;
    }

    std::size_t Dictionary::NumWords() const
    {
        if (m_finalized)
        {
            return m_sorted_words.size();
        }
        return m_word_to_concept.size();
    }

    std::size_t Dictionary::NumConcepts() const
    {
        if (m_finalized)
        {
            return m_num_concepts;
        }
        return m_next_concept_id;
    }

}  // namespace ftsrch
