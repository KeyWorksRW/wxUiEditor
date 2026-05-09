// URL percent-encoding for href attributes
// Purpose: Escapes unsafe characters in URLs (%20 for space, etc.)
// Key function: houdini_escape_href()
// License: MIT (see cppmark/licenses/LICENSE_houdini)
// Status: Legacy code
// Dependencies: houdini.hxx

#include <cassert>

#include "houdini.hxx"

/*
 * The following characters will not be escaped:
 *
 *		-_.+!*'(),%#@?=;:/,+&$~ alphanum
 *
 * Note that this character set is the addition of:
 *
 *	- The characters which are safe to be in an URL
 *	- The characters which are *not* safe to be in
 *	an URL because they are RESERVED characters.
 *
 * We assume (lazily) that any RESERVED char that
 * appears inside an URL is actually meant to
 * have its native function (i.e. as an URL
 * component/separator) and hence needs no escaping.
 *
 * There are two exceptions: the chacters & (amp)
 * and ' (single quote) do not appear in the table.
 * They are meant to appear in the URL as components,
 * yet they require special HTML-entity escaping
 * to generate valid HTML markup.
 *
 * All other characters will be escaped to %XX.
 *
 */
static constexpr char HREF_SAFE[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

int houdini_escape_href(CMarkStringBuffer* output_buffer, const uint8_t* source_text,
                        size_t source_size)
{
    static constexpr uint8_t HEX_CHARS[] = "0123456789ABCDEF";
    size_t current_index = 0;
    uint8_t hex_buffer[3] {};

    hex_buffer[0] = '%';

    while (current_index < source_size)
    {
        const size_t original_index = current_index;
        while (current_index < source_size && HREF_SAFE[source_text[current_index]] != 0)
        {
            ++current_index;
        }

        if (likely(current_index > original_index))
        {
            output_buffer->Put(source_text + original_index, current_index - original_index);
        }

        /* escaping */
        if (current_index >= source_size)
        {
            break;
        }

        switch (source_text[current_index])
        {
            /* amp appears all the time in URLs, but needs
             * HTML-entity escaping to be inside an href */
            case '&':
                output_buffer->Puts("&amp;");
                break;

            /* the single quote is a valid URL character
             * according to the standard; it needs HTML
             * entity escaping too */
            case '\'':
                output_buffer->Puts("&#x27;");
                break;

            /* every other character goes with a %XX escaping */
            default:
                hex_buffer[1] = HEX_CHARS[(source_text[current_index] >> 4) & 0xF];
                hex_buffer[2] = HEX_CHARS[source_text[current_index] & 0xF];
                output_buffer->Put(hex_buffer, 3);
        }

        ++current_index;
    }

    return 1;
}
