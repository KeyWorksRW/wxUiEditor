// HTML entity encoding (&lt;, &amp;, &quot;, etc.)
// Purpose: Escapes HTML special characters to prevent XSS
// Key functions: houdini_escape_html(), houdini_escape_html0()
// License: MIT (see cppmark/licenses/LICENSE_houdini)
// Status: Legacy code
// Dependencies: houdini.hxx

#include <cassert>

#include "houdini.hxx"

/**
 * According to the OWASP rules:
 *
 * & --> &amp;
 * < --> &lt;
 * > --> &gt;
 * " --> &quot;
 * ' --> &#x27;     &apos; is not recommended
 * / --> &#x2F;     forward slash is included as it helps end an HTML entity
 *
 */
static const char HTML_ESCAPE_TABLE[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 2, 3, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 6, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const char* HTML_ESCAPES[] = { "", "&quot;", "&amp;", "&#39;", "&#47;", "&lt;", "&gt;" };

int houdini_escape_html0(cmark_strbuf* output_buffer, const uint8_t* src, size_t size, int secure)
{
    size_t i = 0;
    size_t original_index = 0;
    size_t escape_index = 0;

    while (i < size)
    {
        original_index = i;
        while (i < size && (escape_index = HTML_ESCAPE_TABLE[src[i]]) == 0)
        {
            i++;
        }

        if (i > original_index)
        {
            output_buffer->Put(src + original_index, i - original_index);
        }

        /* escaping */
        if (unlikely(i >= size))
        {
            break;
        }

        /* The forward slash and single quote are only escaped in secure mode */
        if ((src[i] == '/' || src[i] == '\'') && !secure)
        {
            output_buffer->Putc(src[i]);
        }
        else
        {
            output_buffer->Puts(HTML_ESCAPES[escape_index]);
        }

        i++;
    }

    return 1;
}

int houdini_escape_html(cmark_strbuf* output_buffer, const uint8_t* src, size_t size)
{
    return houdini_escape_html0(output_buffer, src, size, 1);
}
