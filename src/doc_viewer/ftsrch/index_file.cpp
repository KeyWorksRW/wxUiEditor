/////////////////////////////////////////////////////////////////////////////
// Purpose:   Index file serialization and deserialization
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "index_file.h"

#include <array>
#include <bit>
#include <cstddef>
#include <cstring>
#include <fstream>

namespace ftsrch
{

    static void WriteU32(std::ostream& output, std::uint32_t value)
    {
        const std::array<char, 4> bytes = { static_cast<char>(value & 0xFFU),
                                            static_cast<char>((value >> 8) & 0xFFU),
                                            static_cast<char>((value >> 16) & 0xFFU),
                                            static_cast<char>((value >> 24) & 0xFFU) };
        output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    }

    static void WriteU64(std::ostream& output, std::uint64_t value)
    {
        WriteU32(output, static_cast<std::uint32_t>(value & 0xFFFF'FFFFULL));
        WriteU32(output, static_cast<std::uint32_t>(value >> 32));
    }

    static void WriteBytes(std::ostream& output, const std::vector<std::uint8_t>& data)
    {
        output.write(std::bit_cast<const char*>(data.data()),
                     static_cast<std::streamsize>(data.size()));
    }

    static std::uint32_t ReadU32(const std::byte* data_ptr)
    {
        return static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data_ptr[0])) |
               (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data_ptr[1])) << 8U) |
               (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data_ptr[2])) << 16U) |
               (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data_ptr[3])) << 24U);
    }

    static std::uint64_t ReadU64(const std::byte* data_ptr)
    {
        const std::uint64_t low_value = static_cast<std::uint64_t>(ReadU32(data_ptr));
        const std::uint64_t high_value = static_cast<std::uint64_t>(ReadU32(data_ptr + 4));
        return low_value | (high_value << 32);
    }

    static std::vector<std::uint8_t> SerializeMeta(const IndexMeta& meta)
    {
        std::vector<std::uint8_t> result;
        const std::uint32_t name_length = static_cast<std::uint32_t>(meta.source_name.size());
        result.push_back(static_cast<std::uint8_t>(name_length & 0xFFU));
        result.push_back(static_cast<std::uint8_t>((name_length >> 8) & 0xFFU));
        result.push_back(static_cast<std::uint8_t>((name_length >> 16) & 0xFFU));
        result.push_back(static_cast<std::uint8_t>((name_length >> 24) & 0xFFU));
        result.insert(result.end(), meta.source_name.begin(), meta.source_name.end());
        std::uint64_t time_value = meta.timestamp;
        for (int i = 0; i < 8; ++i)
        {
            result.push_back(static_cast<std::uint8_t>(time_value & 0xFFU));
            time_value >>= 8;
        }
        return result;
    }

    static IndexMeta DeserializeMeta(std::span<const std::byte> data)
    {
        IndexMeta meta;
        if (data.size() < 4)
        {
            return meta;
        }

        const std::uint32_t name_length = ReadU32(data.data());
        if (data.size() < 4 + name_length + 8)
        {
            return meta;
        }

        const std::span<const std::byte> name_bytes = data.subspan(4, name_length);
        meta.source_name =
            std::string(std::bit_cast<const char*>(name_bytes.data()), name_bytes.size());
        meta.timestamp = ReadU64(data.data() + 4 + name_length);
        return meta;
    }

    static std::vector<std::uint8_t>
        SerializeTitles(const std::vector<std::pair<DocId, std::string>>& titles)
    {
        std::vector<std::uint8_t> result;
        const std::uint32_t title_count = static_cast<std::uint32_t>(titles.size());
        result.push_back(static_cast<std::uint8_t>(title_count & 0xFFU));
        result.push_back(static_cast<std::uint8_t>((title_count >> 8) & 0xFFU));
        result.push_back(static_cast<std::uint8_t>((title_count >> 16) & 0xFFU));
        result.push_back(static_cast<std::uint8_t>((title_count >> 24) & 0xFFU));
        for (const auto& [doc_id, title]: titles)
        {
            const DocId title_doc_id = doc_id;
            result.push_back(static_cast<std::uint8_t>(title_doc_id & 0xFFU));
            result.push_back(static_cast<std::uint8_t>((title_doc_id >> 8) & 0xFFU));
            result.push_back(static_cast<std::uint8_t>((title_doc_id >> 16) & 0xFFU));
            result.push_back(static_cast<std::uint8_t>((title_doc_id >> 24) & 0xFFU));
            const std::uint32_t title_length = static_cast<std::uint32_t>(title.size());
            result.push_back(static_cast<std::uint8_t>(title_length & 0xFFU));
            result.push_back(static_cast<std::uint8_t>((title_length >> 8) & 0xFFU));
            result.push_back(static_cast<std::uint8_t>((title_length >> 16) & 0xFFU));
            result.push_back(static_cast<std::uint8_t>((title_length >> 24) & 0xFFU));
            result.insert(result.end(), title.begin(), title.end());
        }
        return result;
    }

    static std::vector<std::pair<DocId, std::string>>
        DeserializeTitles(std::span<const std::byte> data)
    {
        std::vector<std::pair<DocId, std::string>> titles;
        if (data.size() < 4)
        {
            return titles;
        }

        const std::uint32_t title_count = ReadU32(data.data());
        std::size_t position = 4;
        titles.reserve(title_count);
        for (std::uint32_t i = 0; i < title_count; ++i)
        {
            if (position + 8 > data.size())
            {
                break;
            }

            const DocId doc_id = ReadU32(data.data() + position);
            position += 4;
            const std::uint32_t title_length = ReadU32(data.data() + position);
            position += 4;
            if (position + title_length > data.size())
            {
                break;
            }

            const std::span<const std::byte> title_bytes = data.subspan(position, title_length);
            titles.emplace_back(doc_id, std::string(std::bit_cast<const char*>(title_bytes.data()),
                                                    title_bytes.size()));
            position += title_length;
        }
        return titles;
    }

    static constexpr std::size_t HEADER_SIZE = 32;
    static constexpr std::size_t DIR_ENTRY_SIZE = 20;

    std::expected<void, Error>
        IndexFile::Save(const std::filesystem::path& file, const IndexMeta& meta,
                        const Dictionary& dict, const Collection& coll, const PhraseTable& phrases,
                        const std::vector<std::pair<DocId, std::string>>& titles)
    {
        std::ofstream output(file, std::ios::binary);
        if (!output)
        {
            return std::unexpected(Error::io_error);
        }

        // Write header placeholder (32 bytes)
        output.write(KFTS_SIGNATURE, 4);
        WriteU32(output, KFTS_VERSION);
        WriteU32(output, 0);
        WriteU32(output, 5);
        WriteU64(output, 0);
        WriteU64(output, 0);

        // Serialize all sections
        const std::vector<std::uint8_t> meta_bytes = SerializeMeta(meta);
        const std::vector<std::uint8_t> dict_bytes = dict.Serialize();
        const std::vector<std::uint8_t> coll_bytes = coll.Serialize();
        const std::vector<std::uint8_t> phrase_bytes = phrases.Serialize();
        const std::vector<std::uint8_t> title_bytes = SerializeTitles(titles);

        // Write each section, record directory entries
        struct SectionInfo
        {
            SectionTag tag;
            std::uint64_t offset;
            std::uint64_t size;
        };
        std::vector<SectionInfo> sections;

        auto write_section = [&](SectionTag section_tag, const std::vector<std::uint8_t>& data)
        {
            const std::uint64_t section_offset = static_cast<std::uint64_t>(output.tellp());
            WriteBytes(output, data);
            sections.push_back(SectionInfo { section_tag, section_offset,
                                             static_cast<std::uint64_t>(data.size()) });
        };

        write_section(SectionTag::meta, meta_bytes);
        write_section(SectionTag::dictionary, dict_bytes);
        write_section(SectionTag::collection, coll_bytes);
        write_section(SectionTag::phrase_table, phrase_bytes);
        write_section(SectionTag::titles, title_bytes);

        // Write directory table
        const std::uint64_t dir_offset = static_cast<std::uint64_t>(output.tellp());
        for (const SectionInfo& section: sections)
        {
            WriteU32(output, static_cast<std::uint32_t>(section.tag));
            WriteU64(output, section.offset);
            WriteU64(output, section.size);
        }

        // Patch directory_offset in header (at byte offset 16)
        output.seekp(16);
        WriteU64(output, dir_offset);

        if (!output)
        {
            return std::unexpected(Error::io_error);
        }
        return {};
    }

    std::expected<IndexFile, Error> IndexFile::Open(const std::filesystem::path& file)
    {
        // Read entire file into memory, then delegate to the span-based overload.
        std::ifstream input(file, std::ios::binary | std::ios::ate);
        if (!input)
        {
            return std::unexpected(Error::io_error);
        }

        const std::size_t file_size = static_cast<std::size_t>(input.tellg());
        if (file_size < HEADER_SIZE)
        {
            return std::unexpected(Error::corrupt_index);
        }

        input.seekg(0);
        std::vector<std::uint8_t> buffer(file_size);
        input.read(std::bit_cast<char*>(buffer.data()), static_cast<std::streamsize>(file_size));
        if (!input)
        {
            return std::unexpected(Error::io_error);
        }

        return Open(std::as_bytes(std::span<const std::uint8_t>(buffer)));
    }

    std::expected<IndexFile, Error> IndexFile::Open(std::span<const std::byte> data)
    {
        const std::size_t data_size = data.size();
        if (data_size < HEADER_SIZE)
        {
            return std::unexpected(Error::corrupt_index);
        }

        const std::byte* const base = data.data();

        // Validate header
        if (std::memcmp(base, KFTS_SIGNATURE, 4) != 0)
        {
            return std::unexpected(Error::corrupt_index);
        }
        const std::uint32_t version = ReadU32(base + 4);
        if (version != KFTS_VERSION)
        {
            return std::unexpected(Error::corrupt_index);
        }
        const std::uint32_t section_count = ReadU32(base + 12);
        const std::uint64_t dir_offset = ReadU64(base + 16);

        if (dir_offset + (section_count * DIR_ENTRY_SIZE) > data_size)
        {
            return std::unexpected(Error::corrupt_index);
        }

        // Parse directory and deserialize sections
        IndexFile idx_file;
        const std::byte* dir_ptr = base + dir_offset;

        for (std::uint32_t i = 0; i < section_count; ++i)
        {
            const SectionTag tag = static_cast<SectionTag>(ReadU32(dir_ptr));
            const std::uint64_t offset = ReadU64(dir_ptr + 4);
            const std::uint64_t size = ReadU64(dir_ptr + 12);
            dir_ptr += DIR_ENTRY_SIZE;

            if (offset + size > data_size)
            {
                return std::unexpected(Error::corrupt_index);
            }

            const std::span<const std::byte> section_span(base + offset,
                                                          static_cast<std::size_t>(size));

            switch (tag)
            {
                case SectionTag::meta:
                    idx_file.m_meta = DeserializeMeta(section_span);
                    break;
                case SectionTag::dictionary:
                    idx_file.m_dict = Dictionary::Deserialize(section_span);
                    break;
                case SectionTag::collection:
                    idx_file.m_coll = Collection::Deserialize(section_span);
                    break;
                case SectionTag::phrase_table:
                    idx_file.m_phrases = PhraseTable::Deserialize(section_span);
                    break;
                case SectionTag::titles:
                    idx_file.m_titles = DeserializeTitles(section_span);
                    break;
            }
        }

        return idx_file;
    }

    const Dictionary& IndexFile::GetDictionary()
    {
        return m_dict;
    }

    const Collection& IndexFile::GetCollection()
    {
        return m_coll;
    }

    const PhraseTable& IndexFile::GetPhraseTable()
    {
        return m_phrases;
    }

    std::string_view IndexFile::Title(DocId doc_id)
    {
        for (const auto& [stored_doc_id, title]: m_titles)
        {
            if (stored_doc_id == doc_id)
            {
                return title;
            }
        }
        return {};
    }

    const IndexMeta& IndexFile::Meta()
    {
        return m_meta;
    }

    Dictionary IndexFile::TakeDictionary()
    {
        return std::move(m_dict);
    }

    Collection IndexFile::TakeCollection()
    {
        return std::move(m_coll);
    }

    PhraseTable IndexFile::TakePhraseTable()
    {
        return std::move(m_phrases);
    }

    std::vector<std::pair<DocId, std::string>> IndexFile::TakeTitles()
    {
        return std::move(m_titles);
    }

    IndexMeta IndexFile::TakeMeta()
    {
        return std::move(m_meta);
    }

}  // namespace ftsrch
