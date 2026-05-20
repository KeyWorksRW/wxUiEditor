/////////////////////////////////////////////////////////////////////////////
// Purpose:   Tokenizer for full-text search
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
#include <vector>

namespace ftsrch
{

    enum class TokenType : uint8_t
    {
        word,
        number,
        symbol,
        whitespace
    };

    struct Token
    {
        std::string_view text;
        size_t offset;
        TokenType type;
    };

    class Tokenizer
    {
    public:
        std::vector<Token> Tokenize(std::string_view utf8_text);
        static std::string ToLower(std::string_view utf8);
    };

    class GlyphClassifier
    {
    public:
        void Scan(std::span<const uint8_t> data);
        void BuildPartitions(uint32_t max_partitions = 32);
        uint32_t Classify(std::span<const uint8_t> data);
        uint32_t PartitionOf(uint8_t byte);

    private:
        uint32_t m_glyph_refs[256] = {};
        uint32_t m_glyph_refs_descending[256] = {};
        uint32_t m_classifications[256] = {};
        uint32_t m_glyph_partitions[33] = {};
        uint32_t m_partition_refs[32] = {};
        uint32_t m_partition_count = 0;
    };

}  // namespace ftsrch
