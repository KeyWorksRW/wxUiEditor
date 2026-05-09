// HTML entity decoding (&lt; to <, &#65; to A, etc.)
// Purpose: Converts HTML entities back to characters
// Key functions: houdini_unescape_html(), houdini_unescape_html_f(), houdini_unescape_ent()
// Uses entities.inc: Generated lookup table for named entities (&nbsp;, &copy;, etc.)
// License: MIT (see cppmark/licenses/LICENSE_houdini)
// Status: Legacy code
// Dependencies: buffer.hxx, entities.inc (generated), houdini.hxx, utf8.hxx

#include <cassert>
#include <cstdio>
#include <cstring>

#include "buffer.hxx"
#include "entities.inc"
#include "houdini.hxx"
#include "utf8.hxx"

// Binary tree lookup code for entities added by JGM

struct LookupBounds
{
    int index = 0;
    int low_index = 0;
    int high_index = 0;
};

static const unsigned char* Lookup(LookupBounds bounds, const unsigned char* entity_name,
                                   int entity_length)
{
    int mid_index = 0;
    const int comparison_result =
        strncmp((const char*) entity_name, (const char*) cmark_entities[bounds.index].entity,
                entity_length);
    if (comparison_result == 0 && cmark_entities[bounds.index].entity[entity_length] == 0)
    {
        return (const unsigned char*) cmark_entities[bounds.index].bytes;
    }

    if (comparison_result <= 0 && bounds.index > bounds.low_index)
    {
        mid_index = bounds.index - ((bounds.index - bounds.low_index) / 2);
        if (mid_index == bounds.index)
        {
            mid_index -= 1;
        }
        return Lookup(
            { .index = mid_index, .low_index = bounds.low_index, .high_index = bounds.index - 1 },
            entity_name, entity_length);
    }

    if (comparison_result > 0 && bounds.index < bounds.high_index)
    {
        mid_index = bounds.index + ((bounds.high_index - bounds.index) / 2);
        if (mid_index == bounds.index)
        {
            ++mid_index;
        }
        return Lookup(
            { .index = mid_index, .low_index = bounds.index + 1, .high_index = bounds.high_index },
            entity_name, entity_length);
    }

    return nullptr;
}

static const unsigned char* LookupEntity(const unsigned char* entity_name, int entity_length)
{
    return Lookup({ CMARK_NUM_ENTITIES / 2, 0, CMARK_NUM_ENTITIES - 1 }, entity_name,
                  entity_length);
}

size_t houdini_unescape_ent(cmark_strbuf* output_buffer, const uint8_t* src, size_t size)
{
    size_t i = 0;

    if (size >= 3 && src[0] == '#')
    {
        int codepoint = 0;
        int num_digits = 0;

        if (_isdigit(src[1]))
        {
            for (i = 1; i < size && _isdigit(src[i]); ++i)
            {
                codepoint = (codepoint * 10) + (src[i] - '0');

                if (codepoint >= 0x110000)
                {
                    // Keep counting digits but avoid integer overflow.
                    codepoint = 0x110000;
                }
            }

            num_digits = i - 1;
        }
        else if (src[1] == 'x' || src[1] == 'X')
        {
            for (i = 2; i < size && _isxdigit(src[i]); ++i)
            {
                codepoint = (codepoint * 16) + ((src[i] | 32) % 39 - 9);

                if (codepoint >= 0x110000)
                {
                    // Keep counting digits but avoid integer overflow.
                    codepoint = 0x110000;
                }
            }

            num_digits = i - 2;
        }

        if (num_digits >= 1 && num_digits <= 8 && i < size && src[i] == ';')
        {
            if (codepoint == 0 || (codepoint >= 0xD800 && codepoint < 0xE000) ||
                codepoint >= 0x110000)
            {
                codepoint = 0xFFFD;
            }
            cmark_utf8proc_encode_char(codepoint, output_buffer);
            return i + 1;
        }
    }
    else
    {
        if (size > CMARK_ENTITY_MAX_LENGTH)
        {
            size = CMARK_ENTITY_MAX_LENGTH;
        }

        for (i = CMARK_ENTITY_MIN_LENGTH; i < size; ++i)
        {
            if (src[i] == ' ')
            {
                break;
            }

            if (src[i] == ';')
            {
                const unsigned char* entity = LookupEntity(src, i);

                if (entity != nullptr)
                {
                    output_buffer->Puts((const char*) entity);
                    return i + 1;
                }

                break;
            }
        }
    }

    return 0;
}

int houdini_unescape_html(cmark_strbuf* output_buffer, const uint8_t* src, size_t size)
{
    size_t current_index = 0;
    size_t original_index = 0;
    size_t entity_size = 0;

    while (current_index < size)
    {
        original_index = current_index;
        while (current_index < size && src[current_index] != '&')
        {
            ++current_index;
        }

        if (likely(current_index > original_index))
        {
            if (unlikely(original_index == 0))
            {
                if (current_index >= size)
                {
                    return 0;
                }

                output_buffer->Grow(HOUDINI_UNESCAPED_SIZE(size));
            }

            output_buffer->Put(src + original_index, current_index - original_index);
        }

        // escaping
        if (current_index >= size)
        {
            break;
        }

        ++current_index;

        entity_size =
            houdini_unescape_ent(output_buffer, src + current_index, size - current_index);
        current_index += entity_size;

        // not really an entity
        if (entity_size == 0)
        {
            output_buffer->Putc('&');
        }
    }

    return 1;
}

void houdini_unescape_html_f(cmark_strbuf* output_buffer, const uint8_t* src, size_t size)
{
    if (!houdini_unescape_html(output_buffer, src, size))
    {
        output_buffer->Put(src, size);
    }
}
