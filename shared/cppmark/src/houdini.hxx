// HTML/URL escaping library (Houdini - MIT license)
// Purpose: Escape/unescape HTML entities (&lt;, &quot;) and URL-encode href attributes
// Key functions: houdini_escape_html(), houdini_unescape_html(), houdini_escape_href()
// Use case: Renderer calls these to safely output user content in HTML
// License: MIT (see cppmark/licenses/LICENSE_houdini)
// Status: Legacy code
// Dependencies: buffer.hxx

#pragma once

#include <cstdint>

#include "buffer.hxx"

#if defined(HAVE___BUILTIN_EXPECT)
    #define likely(x)   __builtin_expect((x), 1)
    #define unlikely(x) __builtin_expect((x), 0)
#else
    #define likely(x)   (x)
    #define unlikely(x) (x)
#endif

#if defined(HOUDINI_USE_LOCALE)
    #define _isxdigit(c) isxdigit(c)
    #define _isdigit(c)  isdigit(c)
#else
    // Helper _isdigit methods -- do not trust the current locale */
    #define _isxdigit(c) (strchr("0123456789ABCDEFabcdef", (c)) != nullptr)
    #define _isdigit(c)  ((c) >= '0' && (c) <= '9')
#endif

#define HOUDINI_ESCAPED_SIZE(x)   (((x) * 12) / 10)
#define HOUDINI_UNESCAPED_SIZE(x) (x)

size_t houdini_unescape_ent(CMarkStringBuffer* output_buffer, const uint8_t* source_text,
                            size_t source_size);
int houdini_escape_html(CMarkStringBuffer* output_buffer, const uint8_t* source_text,
                        size_t source_size);
int houdini_escape_html0(CMarkStringBuffer* output_buffer, const uint8_t* source_text,
                         size_t source_size, int secure);
int houdini_unescape_html(CMarkStringBuffer* output_buffer, const uint8_t* source_text,
                          size_t source_size);
void houdini_unescape_html_f(CMarkStringBuffer* output_buffer, const uint8_t* source_text,
                             size_t source_size);
int houdini_escape_href(CMarkStringBuffer* output_buffer, const uint8_t* source_text,
                        size_t source_size);
