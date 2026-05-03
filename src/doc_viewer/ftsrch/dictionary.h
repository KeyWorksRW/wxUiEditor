/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dictionary class implementation for mapping words to concept IDs
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "types.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ftsrch
{

    class Dictionary
    {
    public:
        explicit Dictionary(StemmerFn stemmer = {});

        // Build phase
        ConceptId AddWord(std::string_view word);
        void AddStopWords(std::span<const std::string> words);
        void Finalize();

        // Query phase (after finalize)
        std::optional<ConceptId> Lookup(std::string_view word) const;
        std::vector<ConceptId> PrefixMatch(std::string_view prefix) const;

        // Serialization
        std::vector<std::uint8_t> Serialize() const;
        static Dictionary Deserialize(std::span<const std::byte> data);

        std::size_t NumWords() const;
        std::size_t NumConcepts() const;

    private:
        std::string StemWord(std::string_view lowered) const;

        StemmerFn m_stemmer;

        // Build phase
        std::unordered_map<std::string, ConceptId> m_stem_to_concept;
        std::unordered_map<std::string, ConceptId> m_word_to_concept;
        std::unordered_set<std::string> m_stop_stems;
        ConceptId m_next_concept_id = 0;

        // After finalize (also used at query time after deserialize)
        std::vector<std::string> m_sorted_words;
        std::vector<ConceptId> m_sorted_concept_ids;
        std::uint32_t m_concept_id_bits = 0;
        std::uint32_t m_num_concepts = 0;
        bool m_finalized = false;
    };

}  // namespace ftsrch
