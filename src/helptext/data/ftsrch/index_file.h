/////////////////////////////////////////////////////////////////////////////
// Purpose:   Index file serialization and deserialization
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "collection.h"
#include "dictionary.h"
#include "phrase_table.h"
#include "types.h"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ftsrch
{

    struct IndexMeta
    {
        std::string source_name;
        std::uint64_t timestamp = 0;
    };

    enum class SectionTag : std::uint32_t
    {
        meta = 1,
        dictionary = 2,
        collection = 3,
        phrase_table = 4,
        titles = 5
    };

    struct IndexHeader
    {
        char signature[4];
        std::uint32_t version;
        std::uint32_t flags;
        std::uint32_t section_count;
        std::uint64_t directory_offset;
        std::uint64_t reserved;
    };

    struct DirectoryEntry
    {
        std::uint32_t tag;
        std::uint64_t offset;
        std::uint64_t size;
    };

    class IndexFile
    {
    public:
        static std::expected<void, Error>
            Save(const std::filesystem::path& file, const IndexMeta& meta, const Dictionary& dict,
                 const Collection& coll, const PhraseTable& phrases,
                 const std::vector<std::pair<DocId, std::string>>& titles);

        static std::expected<IndexFile, Error> Open(const std::filesystem::path& file);
        static std::expected<IndexFile, Error> Open(std::span<const std::byte> data);

        const Dictionary& GetDictionary();
        const Collection& GetCollection();
        const PhraseTable& GetPhraseTable();
        std::string_view Title(DocId doc_id);
        const IndexMeta& Meta();

        // Transfer ownership (move-out) after Open
        Dictionary TakeDictionary();
        Collection TakeCollection();
        PhraseTable TakePhraseTable();
        std::vector<std::pair<DocId, std::string>> TakeTitles();
        IndexMeta TakeMeta();

    private:
        IndexMeta m_meta;
        Dictionary m_dict;
        Collection m_coll;
        PhraseTable m_phrases;
        std::vector<std::pair<DocId, std::string>> m_titles;
    };

}  // namespace ftsrch
