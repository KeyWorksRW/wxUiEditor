/////////////////////////////////////////////////////////////////////////////
// Purpose:   Snowball stemmer wrapper for full-text search
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "types.h"

#include <string>
#include <string_view>

struct sb_stemmer;  // forward-declare C type

namespace ftsrch
{

    class SnowballStemmer
    {
    public:
        explicit SnowballStemmer(std::string_view language = "english");
        ~SnowballStemmer();

        SnowballStemmer(const SnowballStemmer&) = delete;
        SnowballStemmer& operator=(const SnowballStemmer&) = delete;
        SnowballStemmer(SnowballStemmer&& other) noexcept;
        SnowballStemmer& operator=(SnowballStemmer&& other) noexcept;

        std::string Stem(std::string_view word);
        StemmerFn AsFunction();

    private:
        struct sb_stemmer* m_stemmer = nullptr;
    };

}  // namespace ftsrch
