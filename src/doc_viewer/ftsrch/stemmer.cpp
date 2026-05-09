/////////////////////////////////////////////////////////////////////////////
// Purpose:   Snowball stemmer wrapper for full-text search
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "stemmer.h"

#include <libstemmer.h>

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ftsrch
{

    SnowballStemmer::SnowballStemmer(std::string_view language)
    {
        const std::string language_name(language);
        m_stemmer = sb_stemmer_new(language_name.c_str(), "UTF_8");
        if (!m_stemmer)
        {
            throw std::runtime_error("SnowballStemmer: unsupported language '" + language_name +
                                     "'");
        }
    }

    SnowballStemmer::~SnowballStemmer()
    {
        if (m_stemmer)
        {
            sb_stemmer_delete(m_stemmer);
        }
    }

    SnowballStemmer::SnowballStemmer(SnowballStemmer&& other) noexcept :
        m_stemmer(std::exchange(other.m_stemmer, nullptr))
    {
    }

    SnowballStemmer& SnowballStemmer::operator=(SnowballStemmer&& other) noexcept
    {
        if (this != &other)
        {
            if (m_stemmer)
            {
                sb_stemmer_delete(m_stemmer);
            }
            m_stemmer = std::exchange(other.m_stemmer, nullptr);
        }
        return *this;
    }

    std::string SnowballStemmer::Stem(std::string_view word)
    {
        if (!m_stemmer || word.empty())
        {
            return std::string(word);
        }

        const std::vector<sb_symbol> input_word(word.begin(), word.end());
        const sb_symbol* result =
            sb_stemmer_stem(m_stemmer, input_word.data(), static_cast<int>(word.size()));
        if (!result)
        {
            return std::string(word);
        }

        const int result_length = sb_stemmer_length(m_stemmer);
        std::string stemmed_word(static_cast<size_t>(result_length), '\0');
        std::ranges::copy_n(result, result_length, stemmed_word.begin());
        return stemmed_word;
    }

    StemmerFn SnowballStemmer::AsFunction()
    {
        return [this](std::string_view word) -> std::string
        {
            return Stem(word);
        };
    }

}  // namespace ftsrch
