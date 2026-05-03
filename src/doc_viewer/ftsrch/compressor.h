/////////////////////////////////////////////////////////////////////////////
// Purpose:   Compressor class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace ftsrch
{

    class BitWriter
    {
    public:
        void WriteBits(uint32_t value, int nbits);
        void WriteUnary(uint32_t count);
        std::vector<uint32_t> Finish();
        size_t BitCount() const;

    private:
        std::vector<uint32_t> m_buffer;
        uint32_t m_current = 0;
        int m_bit_pos = 0;
    };

    class BitReader
    {
    public:
        explicit BitReader(std::span<const uint32_t> data);

        uint32_t ReadBits(int nbits);
        uint32_t ReadUnary();
        size_t BitsRemaining() const;

    private:
        std::span<const uint32_t> m_data;
        size_t m_word_index = 0;
        int m_bit_pos = 0;
        uint32_t m_current = 0;

        void LoadNext();
    };

    std::vector<uint32_t> CompressSortedIds(std::span<const uint32_t> sorted_ids,
                                            uint32_t universe_size);

    std::vector<uint32_t> DecompressSortedIds(std::span<const uint32_t> packed,
                                              [[maybe_unused]] uint32_t count,
                                              [[maybe_unused]] uint32_t universe_size);

}  // namespace ftsrch
