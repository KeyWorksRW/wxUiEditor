/////////////////////////////////////////////////////////////////////////////
// Purpose:   Tokenizer for full-text search
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "tokenizer.h"

#include <uni_algo/case.h>
#include <uni_algo/ranges_word.h>

#include <algorithm>
#include <numeric>

namespace ftsrch
{

    // ---------------------------------------------------------------------------
    // Tokenizer
    // ---------------------------------------------------------------------------

    std::vector<Token> Tokenizer::Tokenize(std::string_view utf8_text)
    {
        std::vector<Token> tokens;

        una::ranges::word::utf8_view<std::string_view> word_view { utf8_text };
        for (decltype(word_view.begin()) iterator = word_view.begin(); iterator != word_view.end();
             ++iterator)
        {
            const std::string_view segment = *iterator;  // std::string_view of the segment
            if (segment.empty())
            {
                continue;
            }

            // Compute byte offset from original string
            const size_t seg_offset = static_cast<size_t>(segment.data() - utf8_text.data());

            TokenType type {};
            if (iterator.is_word_letter())
            {
                type = TokenType::word;
            }
            else if (iterator.is_word_number())
            {
                type = TokenType::number;
            }
            else if (iterator.is_segspace() || iterator.is_newline())
            {
                continue;  // skip whitespace
            }
            else if (iterator.is_punctuation())
            {
                type = TokenType::symbol;
            }
            else if (iterator.is_word())
            {
                type = TokenType::word;  // kana, ideographic, emoji
            }
            else
            {
                continue;  // other non-word segments
            }

            tokens.push_back(Token { .text = segment, .offset = seg_offset, .type = type });
        }

        return tokens;
    }

    std::string Tokenizer::ToLower(std::string_view utf8)
    {
        return una::cases::to_lowercase_utf8(utf8);
    }

    // ---------------------------------------------------------------------------
    // GlyphClassifier — ported from legacy classify.cpp
    // ---------------------------------------------------------------------------

    static constexpr uint32_t UNUSED_GLYPH = 0x8000'0000;

    void GlyphClassifier::Scan(std::span<const uint8_t> data)
    {
        for (const auto byte: data)
        {
            ++m_glyph_refs[byte];
        }
    }

    void GlyphClassifier::BuildPartitions(uint32_t max_partitions)
    {
        if (max_partitions > 32)
        {
            max_partitions = 32;
        }

        // Initialize descending index array
        std::iota(m_glyph_refs_descending, m_glyph_refs_descending + 256, 0U);

        // Sort indices by frequency (descending)
        std::sort(m_glyph_refs_descending, m_glyph_refs_descending + 256,
                  [this](uint32_t left_index, uint32_t right_index)
                  {
                      return m_glyph_refs[left_index] > m_glyph_refs[right_index];
                  });
        // Each partition groups glyphs of similar frequency
        m_partition_count = 0;
        m_glyph_partitions[0] = 0;

        // Find where frequencies drop to zero
        uint32_t non_zero_count = 0;
        for (uint32_t i = 0; i < 256; ++i)
        {
            if (m_glyph_refs[m_glyph_refs_descending[i]] > 0)
            {
                ++non_zero_count;
            }
        }

        if (non_zero_count == 0)
        {
            m_partition_count = 0;
            m_glyph_partitions[0] = 0;
            std::fill_n(m_classifications, 256, UNUSED_GLYPH);
            return;
        }

        // Distribute non-zero glyphs evenly across partitions
        const uint32_t actual_partitions = std::min(max_partitions, non_zero_count);
        const uint32_t glyphs_per_partition = non_zero_count / actual_partitions;
        uint32_t remainder = non_zero_count % actual_partitions;

        uint32_t glyph_index = 0;
        for (uint32_t part = 0; part < actual_partitions; ++part)
        {
            m_glyph_partitions[part] = glyph_index;
            const uint32_t partition_count = glyphs_per_partition + (remainder > 0 ? 1U : 0U);
            if (remainder > 0)
            {
                --remainder;
            }
            glyph_index += partition_count;
        }
        m_glyph_partitions[actual_partitions] = non_zero_count;
        m_partition_count = actual_partitions;

        // Compute reference counts per partition
        for (uint32_t part = 0; part < m_partition_count; ++part)
        {
            uint32_t partition_sum = 0;
            for (uint32_t i = m_glyph_partitions[part]; i < m_glyph_partitions[part + 1]; ++i)
            {
                partition_sum += m_glyph_refs[m_glyph_refs_descending[i]];
            }
            m_partition_refs[part] = partition_sum;
        }

        // Assign classification masks
        std::fill_n(m_classifications, 256, UNUSED_GLYPH);

        const uint32_t start_bit = (non_zero_count < 256) ? 30U : 31U;

        for (uint32_t part = 0; part < m_partition_count; ++part)
        {
            const uint32_t mask = (start_bit >= part) ? (uint32_t { 1 } << (start_bit - part)) : 0;
            for (uint32_t i = m_glyph_partitions[part]; i < m_glyph_partitions[part + 1]; ++i)
            {
                m_classifications[m_glyph_refs_descending[i]] = mask;
            }
        }
    }

    uint32_t GlyphClassifier::Classify(std::span<const uint8_t> data)
    {
        uint32_t result = 0;
        for (const auto byte: data)
        {
            result |= m_classifications[byte];
        }
        return result;
    }

    uint32_t GlyphClassifier::PartitionOf(uint8_t byte)
    {
        // Find which partition the byte belongs to
        for (uint32_t part = 0; part < m_partition_count; ++part)
        {
            for (uint32_t i = m_glyph_partitions[part]; i < m_glyph_partitions[part + 1]; ++i)
            {
                if (m_glyph_refs_descending[i] == byte)
                {
                    return part;
                }
            }
        }
        return m_partition_count;  // unused glyph
    }

}  // namespace ftsrch
