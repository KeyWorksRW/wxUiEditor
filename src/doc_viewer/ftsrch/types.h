/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common type definitions for full-text search
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <string_view>

namespace ftsrch
{

    enum class Error : uint8_t
    {
        none,
        invalid_argument,
        io_error,
        corrupt_index,
        index_not_finalized,
        index_already_finalized
    };

    using ConceptId = uint32_t;
    using DocId = uint32_t;
    using Weight = uint16_t;

    inline constexpr Weight WT_ONE = 0xFFFF;
    inline constexpr uint32_t MAX_WORD_LEN = 128;

    using StemmerFn = std::function<std::string(std::string_view)>;

    inline constexpr ConceptId STOP_WORD = std::numeric_limits<ConceptId>::max();

    inline constexpr char KFTS_SIGNATURE[4] = { 'K', 'F', 'T', 'S' };
    inline constexpr uint32_t KFTS_VERSION = 1;

}  // namespace ftsrch
