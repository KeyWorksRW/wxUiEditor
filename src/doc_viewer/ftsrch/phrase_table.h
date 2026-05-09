/////////////////////////////////////////////////////////////////////////////
// Purpose:   Phrase table for token frequency analysis and compression
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ftsrch
{

    enum class EncodingClass : std::uint8_t
    {
        literal,
        index_low,
        index_medium,
        null_class
    };

    class PhraseTable
    {
    public:
        void Scan(std::string_view text);
        void Build();
        std::vector<std::uint8_t> Compress(std::string_view text);
        std::string Decompress(std::span<const std::byte> data);
        std::vector<std::uint8_t> Serialize() const;
        static PhraseTable Deserialize(std::span<const std::byte> data);

    private:
        // Build phase
        std::unordered_map<std::string, std::uint32_t> m_token_freq;
        bool m_built = false;

        // After build
        std::vector<std::string> m_tokens_by_rank;
        std::unordered_map<std::string, std::uint32_t> m_token_to_index;

        static constexpr std::uint32_t INDEX_LOW_MAX = 128;
        static constexpr std::uint32_t INDEX_MEDIUM_MAX = 128 + 16384;
        static constexpr std::uint8_t LITERAL_MARKER = 0xC0;
        static constexpr std::uint8_t SPACE_MARKER = 0xFE;
    };

}  // namespace ftsrch
