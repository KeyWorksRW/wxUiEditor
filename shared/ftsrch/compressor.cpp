/////////////////////////////////////////////////////////////////////////////
// Purpose:   Compressor class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "compressor.h"

#include <bit>
#include <cassert>
#include <utility>

namespace ftsrch
{

    // ---------------------------------------------------------------------------
    // BitWriter
    // ---------------------------------------------------------------------------

    void BitWriter::WriteBits(uint32_t value, int nbits)
    {
        assert(nbits >= 0 && nbits <= 32);
        if (nbits == 0)
        {
            return;
        }

        // Mask value to nbits (guard against caller passing extra high bits)
        if (nbits < 32)
        {
            value &= (uint32_t { 1 } << nbits) - 1;
        }

        // Place bits starting at m_bit_pos (LSB-first packing)
        m_current |= (value << m_bit_pos);
        m_bit_pos += nbits;

        if (m_bit_pos >= 32)
        {
            m_buffer.push_back(m_current);
            m_bit_pos -= 32;
            if (m_bit_pos > 0)
            {
                // Remaining high bits of value that didn't fit
                m_current = value >> (nbits - m_bit_pos);
            }
            else
            {
                m_current = 0;
            }
        }
    }

    void BitWriter::WriteUnary(uint32_t count)
    {
        // Write 'count' one-bits followed by a zero-bit
        // E.g. count=3 writes 1110
        while (count > 0)
        {
            const auto avail = static_cast<uint32_t>(32 - m_bit_pos);
            if (count >= avail)
            {
                // Fill rest of current word with ones
                m_current |=
                    (avail == 32) ? ~uint32_t { 0 } : ((uint32_t { 1 } << avail) - 1) << m_bit_pos;
                m_buffer.push_back(m_current);
                m_current = 0;
                count -= avail;
                m_bit_pos = 0;
            }
            else
            {
                m_current |= ((uint32_t { 1 } << count) - 1) << m_bit_pos;
                m_bit_pos += static_cast<int>(count);
                count = 0;
            }
        }
        // Write the terminating zero bit
        WriteBits(0, 1);
    }

    std::vector<uint32_t> BitWriter::Finish()
    {
        if (m_bit_pos > 0)
        {
            m_buffer.push_back(m_current);
        }
        std::vector<uint32_t> result = std::move(m_buffer);
        m_current = 0;
        m_bit_pos = 0;
        return result;
    }

    size_t BitWriter::BitCount() const
    {
        return (m_buffer.size() * 32) + static_cast<size_t>(m_bit_pos);
    }

    // ---------------------------------------------------------------------------
    // BitReader
    // ---------------------------------------------------------------------------

    BitReader::BitReader(std::span<const uint32_t> data) : m_data(data)
    {
        if (!m_data.empty())
        {
            m_current = m_data[0];
            m_word_index = 1;
        }
    }

    void BitReader::LoadNext()
    {
        if (m_word_index < m_data.size())
        {
            m_current = m_data[m_word_index];
            ++m_word_index;
        }
        else
        {
            m_current = 0;
        }
        m_bit_pos = 0;
    }

    uint32_t BitReader::ReadBits(int nbits)
    {
        assert(nbits >= 0 && nbits <= 32);
        if (nbits == 0)
        {
            return 0;
        }

        // Extract bits from current position
        const int available_bits = 32 - m_bit_pos;
        uint32_t result = 0;

        if (nbits <= available_bits)
        {
            if (nbits == 32)
            {
                result = m_current;
            }
            else
            {
                result = (m_current >> m_bit_pos) & ((uint32_t { 1 } << nbits) - 1);
            }
            m_bit_pos += nbits;
            if (m_bit_pos == 32)
            {
                LoadNext();
            }
        }
        else
        {
            // Spans word boundary
            result = m_current >> m_bit_pos;  // low part from current word
            const int consumed_bits = available_bits;
            LoadNext();
            const int remaining_bits = nbits - consumed_bits;
            if (remaining_bits > 0)
            {
                const uint32_t high_mask = (remaining_bits == 32) ?
                                               ~uint32_t { 0 } :
                                               (uint32_t { 1 } << remaining_bits) - 1;
                result |= (m_current & high_mask) << consumed_bits;
                m_bit_pos = remaining_bits;
                if (m_bit_pos == 32)
                {
                    LoadNext();
                }
            }
        }

        return result;
    }

    uint32_t BitReader::ReadUnary()
    {
        // Count consecutive one-bits, stopping at the first zero-bit
        uint32_t count = 0;

        for (;;)
        {
            // Examine bits from m_bit_pos onward in current word
            const uint32_t remaining_bits = m_current >> m_bit_pos;
            const int available_bits = 32 - m_bit_pos;

            // Count trailing ones in remaining_bits
            // trailing_ones = count of trailing 1-bits = ctz(~remaining_bits)
            const auto ones = static_cast<uint32_t>(std::countr_one(remaining_bits));

            if (std::cmp_less(ones, available_bits))
            {
                // Found a zero bit within this word
                count += ones;
                m_bit_pos += static_cast<int>(ones) + 1;  // skip ones + the zero
                if (m_bit_pos == 32)
                {
                    LoadNext();
                }
                return count;
            }

            // All remaining bits in this word are ones
            count += static_cast<uint32_t>(available_bits);
            LoadNext();
        }
    }

    size_t BitReader::BitsRemaining() const
    {
        if (m_data.empty())
        {
            return 0;
        }
        const size_t words_left = m_data.size() - m_word_index;
        return (words_left * 32) + static_cast<size_t>(32 - m_bit_pos);
    }

    // ---------------------------------------------------------------------------
    // Delta-basis sorted ID compression
    // ---------------------------------------------------------------------------

    static constexpr int kBasisBitsWidth = 5;
    static constexpr int kCountWidth = 27;

    static int ComputeBasisBits(uint32_t count, uint32_t universe_size)
    {
        if (count == 0)
        {
            return 0;
        }
        // basis_bits = floor(log2(universe_size / count))
        // Avoid division by zero; for count >= universe_size, basis = 0
        if (count >= universe_size)
        {
            return 0;
        }
        const uint32_t ratio = universe_size / count;
        if (ratio <= 1)
        {
            return 0;
        }
        return static_cast<int>(std::bit_width(ratio) - 1);
    }

    std::vector<uint32_t> CompressSortedIds(std::span<const uint32_t> sorted_ids,
                                            uint32_t universe_size)
    {
        const uint32_t count = static_cast<uint32_t>(sorted_ids.size());
        if (count == 0)
        {
            // Header word: basis_bits(5) | count(27)
            BitWriter header_writer;
            header_writer.WriteBits(0, kBasisBitsWidth);  // basis_bits
            header_writer.WriteBits(0, kCountWidth);      // count
            return header_writer.Finish();
        }

        const int basis_bits = ComputeBasisBits(count, universe_size);

        BitWriter writer;
        // Header: kBasisBitsWidth bits for basis_bits, kCountWidth bits for count
        writer.WriteBits(static_cast<uint32_t>(basis_bits), kBasisBitsWidth);
        writer.WriteBits(count, kCountWidth);

        uint32_t prev = 0;
        for (const uint32_t index: sorted_ids)
        {
            assert(index >= prev);
            const uint32_t delta = (index - prev);
            // Split: high part (unary) = delta >> basis_bits
            //        low part (fixed)  = delta & ((1 << basis_bits) - 1)
            const uint32_t high_bits = delta >> basis_bits;
            const uint32_t low_bits =
                (basis_bits > 0) ? (delta & ((uint32_t { 1 } << basis_bits) - 1)) : 0;

            writer.WriteUnary(high_bits);
            if (basis_bits > 0)
            {
                writer.WriteBits(low_bits, basis_bits);
            }
            prev = index;
        }

        return writer.Finish();
    }

    std::vector<uint32_t> DecompressSortedIds(std::span<const uint32_t> packed,
                                              [[maybe_unused]] uint32_t count,
                                              [[maybe_unused]] uint32_t universe_size)
    {
        if (packed.empty())
        {
            return {};
        }

        BitReader reader(packed);

        // Read header: 5-bit basis_bits, 27-bit count
        const int basis_bits = static_cast<int>(reader.ReadBits(5));
        const uint32_t stored_count = reader.ReadBits(27);

        if (stored_count == 0)
        {
            return {};
        }

        std::vector<uint32_t> result;
        result.reserve(stored_count);

        uint32_t prev = 0;
        for (uint32_t i = 0; i < stored_count; ++i)
        {
            const uint32_t high_bits = reader.ReadUnary();
            uint32_t low_bits = 0;
            if (basis_bits > 0)
            {
                low_bits = reader.ReadBits(basis_bits);
            }
            const uint32_t delta = (high_bits << basis_bits) | low_bits;
            prev += delta;
            result.push_back(prev);
        }

        return result;
    }

}  // namespace ftsrch
