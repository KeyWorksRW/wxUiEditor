/////////////////////////////////////////////////////////////////////////////
// Purpose:   Phrase table for token frequency analysis and compression
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "phrase_table.h"

#include "tokenizer.h"

#include <algorithm>
#include <cstddef>
#include <utility>

namespace ftsrch
{

    void PhraseTable::Scan(std::string_view text)
    {
        Tokenizer tokenizer;
        const std::vector<Token> tokens = tokenizer.Tokenize(text);
        for (const auto& token: tokens)
        {
            if (token.type == TokenType::whitespace)
            {
                continue;
            }
            ++m_token_freq[std::string(token.text)];
        }
    }

    void PhraseTable::Build()
    {
        // Sort tokens by frequency descending, tiebreak alphabetically
        std::vector<std::pair<std::string, std::uint32_t>> freq_list(m_token_freq.begin(),
                                                                     m_token_freq.end());

        std::ranges::sort(freq_list,
                          [](const std::pair<std::string, std::uint32_t>& left_pair,
                             const std::pair<std::string, std::uint32_t>& right_pair)
                          {
                              if (left_pair.second != right_pair.second)
                              {
                                  return left_pair.second > right_pair.second;
                              }
                              return left_pair.first < right_pair.first;
                          });

        m_tokens_by_rank.clear();
        m_tokens_by_rank.reserve(freq_list.size());

        for (const auto& entry: freq_list)
        {
            m_tokens_by_rank.push_back(entry.first);
            m_token_to_index[entry.first] = static_cast<std::uint32_t>(m_tokens_by_rank.size() - 1);
        }

        m_built = true;
    }

    std::vector<std::uint8_t> PhraseTable::Compress(std::string_view text)
    {
        std::vector<std::uint8_t> result;

        Tokenizer tokenizer;
        const std::vector<Token> tokens = tokenizer.Tokenize(text);

        std::size_t current_pos { 0 };

        for (const auto& token: tokens)
        {
            if (token.type == TokenType::whitespace)
            {
                continue;
            }

            if (token.offset > current_pos)
            {
                const std::string_view text_gap =
                    text.substr(current_pos, token.offset - current_pos);
                if (text_gap == " ")
                {
                    result.push_back(SPACE_MARKER);
                }
                else
                {
                    result.push_back(LITERAL_MARKER);
                    result.push_back(static_cast<std::uint8_t>(text_gap.size()));
                    result.insert(result.end(), text_gap.begin(), text_gap.end());
                }
            }

            const std::string token_str(token.text);
            const std::unordered_map<std::string, std::uint32_t>::const_iterator token_iter =
                m_token_to_index.find(token_str);

            if (token_iter != m_token_to_index.end())
            {
                const std::uint32_t index = token_iter->second;
                if (index < INDEX_LOW_MAX)
                {
                    result.push_back(static_cast<std::uint8_t>(index));
                }
                else if (index < INDEX_MEDIUM_MAX)
                {
                    const std::uint32_t adjusted_index = index - INDEX_LOW_MAX;
                    result.push_back(static_cast<std::uint8_t>(0x80 | (adjusted_index >> 8)));
                    result.push_back(static_cast<std::uint8_t>(adjusted_index & 0xFF));
                }
                else
                {
                    result.push_back(LITERAL_MARKER);
                    result.push_back(static_cast<std::uint8_t>(token_str.size()));
                    result.insert(result.end(), token_str.begin(), token_str.end());
                }
            }
            else
            {
                result.push_back(LITERAL_MARKER);
                result.push_back(static_cast<std::uint8_t>(token_str.size()));
                result.insert(result.end(), token_str.begin(), token_str.end());
            }

            current_pos = token.offset + token.text.size();
        }

        if (current_pos < text.size())
        {
            const std::string_view trailing = text.substr(current_pos);
            if (trailing == " ")
            {
                result.push_back(SPACE_MARKER);
            }
            else
            {
                result.push_back(LITERAL_MARKER);
                result.push_back(static_cast<std::uint8_t>(trailing.size()));
                result.insert(result.end(), trailing.begin(), trailing.end());
            }
        }

        return result;
    }

    std::string PhraseTable::Decompress(std::span<const std::byte> data)
    {
        std::string result;
        std::size_t position { 0 };

        while (position < data.size())
        {
            const std::uint8_t selector = std::to_integer<std::uint8_t>(data[position]);

            if (selector == SPACE_MARKER)
            {
                result += ' ';
                ++position;
                continue;
            }

            if (selector == LITERAL_MARKER)
            {
                ++position;
                if (position >= data.size())
                {
                    break;
                }
                const std::size_t length =
                    static_cast<std::size_t>(std::to_integer<std::uint8_t>(data[position]));
                ++position;
                if (position + length > data.size())
                {
                    break;
                }
                for (std::size_t literal_index { 0 }; literal_index < length; ++literal_index)
                {
                    result.push_back(static_cast<char>(
                        std::to_integer<std::uint8_t>(data[position + literal_index])));
                }
                position += length;
                continue;
            }

            if ((selector & 0x80) != 0)
            {
                // IndexMedium: 2-byte encoding
                const std::uint32_t high = static_cast<std::uint32_t>(selector & 0x3F);
                ++position;
                if (position >= data.size())
                {
                    break;
                }
                const std::uint32_t low_bits =
                    static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data[position]));
                ++position;
                const std::uint32_t index = ((high << 8) | low_bits) + INDEX_LOW_MAX;
                if (index < m_tokens_by_rank.size())
                {
                    result += m_tokens_by_rank[index];
                }
                continue;
            }

            // IndexLow: 1-byte encoding (0x00-0x7F)
            const std::uint32_t index = static_cast<std::uint32_t>(selector);
            ++position;
            if (index < m_tokens_by_rank.size())
            {
                result += m_tokens_by_rank[index];
            }
        }

        return result;
    }

    std::vector<std::uint8_t> PhraseTable::Serialize() const
    {
        std::vector<std::uint8_t> result;

        auto write_u32 = [&](std::uint32_t value)
        {
            result.push_back(static_cast<std::uint8_t>(value & 0xFF));
            result.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
            result.push_back(static_cast<std::uint8_t>((value >> 16) & 0xFF));
            result.push_back(static_cast<std::uint8_t>((value >> 24) & 0xFF));
        };

        const std::uint32_t token_count = static_cast<std::uint32_t>(m_tokens_by_rank.size());
        write_u32(token_count);

        for (const auto& token: m_tokens_by_rank)
        {
            result.insert(result.end(), token.begin(), token.end());
            result.push_back(0);
        }

        return result;
    }

    PhraseTable PhraseTable::Deserialize(std::span<const std::byte> data)
    {
        PhraseTable table;

        if (data.size() < 4)
        {
            return table;
        }

        const auto read_u32 = [](const std::byte* data_ptr) -> std::uint32_t
        {
            return static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data_ptr[0])) |
                   (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data_ptr[1])) << 8U) |
                   (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data_ptr[2])) << 16U) |
                   (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data_ptr[3])) << 24U);
        };

        std::size_t position { 0 };
        const std::uint32_t token_count = read_u32(data.data() + position);
        position += 4;

        table.m_tokens_by_rank.reserve(token_count);

        for (std::uint32_t token_index { 0 }; token_index < token_count; ++token_index)
        {
            std::string token;
            while (position < data.size() && data[position] != std::byte { 0 })
            {
                token.push_back(static_cast<char>(std::to_integer<std::uint8_t>(data[position])));
                ++position;
            }

            if (position >= data.size())
            {
                return table;
            }

            // skip null separator
            ++position;

            table.m_tokens_by_rank.emplace_back(std::move(token));
            table.m_token_to_index[table.m_tokens_by_rank.back()] = token_index;
        }

        table.m_built = true;
        return table;
    }

}  // namespace ftsrch
