// Locale-independent character classification implementation
// Purpose: ASCII-only ctype functions - same behavior regardless of locale
// Key functions: cmark_isspace(), cmark_ispunct(), cmark_isalnum(), cmark_isdigit(),
// cmark_isalpha() Why needed: Markdown spec requires ASCII behavior, system locale can change
// std::isspace() results.
// Dependencies: cmark_ctype.hxx

#include <cstdint>

#include "cmark_ctype.hxx"

static constexpr std::uint8_t cmark_ctype_class[256] = {
    /*      0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f */
    /* 0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,
    /* 1 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 2 */ 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* 3 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2,
    /* 4 */ 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    /* 5 */ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 2,
    /* 6 */ 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    /* 7 */ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 0,
    /* 8 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 9 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* a */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* b */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* c */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* d */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* e */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* f */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Returns true if c is a "whitespace" character as defined by the spec.
bool cmark_isspace(char character)
{
    return cmark_ctype_class[static_cast<std::uint8_t>(character)] == 1;
}

// Returns true if c is an ascii punctuation character.
bool cmark_ispunct(char character)
{
    return cmark_ctype_class[static_cast<std::uint8_t>(character)] == 2;
}

bool cmark_isalnum(char character)
{
    const std::uint8_t result = cmark_ctype_class[static_cast<std::uint8_t>(character)];
    return (result == 3 || result == 4);
}

bool cmark_isdigit(char character)
{
    return cmark_ctype_class[static_cast<std::uint8_t>(character)] == 3;
}

bool cmark_isalpha(char character)
{
    return cmark_ctype_class[static_cast<std::uint8_t>(character)] == 4;
}
