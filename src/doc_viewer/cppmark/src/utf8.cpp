// UTF-8 validation and Unicode character properties
// Purpose: Parse UTF-8 multi-byte sequences, detect spaces/punctuation, case folding
// Key functions: cmark_utf8proc_iterate(), cmark_utf8proc_is_space/punctuation(),
// cmark_utf8proc_case_fold() Why kept: Required for Unicode-aware markdown parsing (emphasis rules,
// word boundaries) Uses case_fold_switch.inc: Generated Unicode case folding lookup table License:
// MIT (see cppmark/licenses/LICENSE_utf8proc) Status: Legacy code Dependencies: cmark_ctype.hxx,
// utf8.hxx, case_fold_switch.inc (generated)

#include <cassert>
#include <cstdint>

#include "cmark_ctype.hxx"

#include "utf8.hxx"

static const int8_t utf8proc_utf8class[256] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0
};

static void encode_unknown(CMarkStringBuffer* buffer)
{
    static const uint8_t repl[] = { 239, 191, 189 };
    buffer->Put(repl, 3);
}

static int utf8proc_charlen(const uint8_t* input, size_t input_length)
{
    int length = 0;
    int index = 0;

    if (!input_length)
    {
        return 0;
    }

    length = utf8proc_utf8class[input[0]];

    if (!length)
    {
        return -1;
    }

    if (input_length >= 0 && static_cast<size_t>(length) > input_length)
    {
        return -input_length;
    }

    for (index = 1; index < length; ++index)
    {
        if ((input[index] & 0xC0) != 0x80)
        {
            return -index;
        }
    }

    return length;
}

// Validate a single UTF-8 character according to RFC 3629.
static int utf8proc_valid(const uint8_t* input, size_t input_length)
{
    const int length = utf8proc_utf8class[input[0]];

    if (!length)
    {
        return -1;
    }

    if (static_cast<size_t>(length) > input_length)
    {
        return -input_length;
    }

    switch (length)
    {
        case 2:
            if ((input[1] & 0xC0) != 0x80)
            {
                return -1;
            }
            if (input[0] < 0xC2)
            {
                // Overlong
                return -length;
            }
            break;

        case 3:
            if ((input[1] & 0xC0) != 0x80)
            {
                return -1;
            }
            if ((input[2] & 0xC0) != 0x80)
            {
                return -2;
            }
            if (input[0] == 0xE0)
            {
                if (input[1] < 0xA0)
                {
                    // Overlong
                    return -length;
                }
            }
            else if (input[0] == 0xED)
            {
                if (input[1] >= 0xA0)
                {
                    // Surrogate
                    return -length;
                }
            }
            break;

        case 4:
            if ((input[1] & 0xC0) != 0x80)
            {
                return -1;
            }
            if ((input[2] & 0xC0) != 0x80)
            {
                return -2;
            }
            if ((input[3] & 0xC0) != 0x80)
            {
                return -3;
            }
            if (input[0] == 0xF0)
            {
                if (input[1] < 0x90)
                {
                    // Overlong
                    return -length;
                }
            }
            else if (input[0] >= 0xF4)
            {
                if (input[0] > 0xF4 || input[1] >= 0x90)
                {
                    // Above 0x10FFFF
                    return -length;
                }
            }
            break;
    }

    return length;
}

void cmark_utf8proc_check(CMarkStringBuffer* output_buffer, const uint8_t* line, size_t size)
{
    size_t index = 0;

    while (index < size)
    {
        const size_t original_index = index;
        int char_length = 0;

        while (index < size)
        {
            if (line[index] < 0x80 && line[index] != 0)
            {
                ++index;
            }
            else if (line[index] >= 0x80)
            {
                char_length = utf8proc_valid(line + index, size - index);
                if (char_length < 0)
                {
                    char_length = -char_length;
                    break;
                }
                index += char_length;
            }
            else if (line[index] == 0)
            {
                // ASCII NUL is technically valid but rejected
                // for security reasons.
                char_length = 1;
                break;
            }
        }

        if (index > original_index)
        {
            output_buffer->Put(line + original_index, index - original_index);
        }

        if (index >= size)
        {
            break;
        }

        // Invalid UTF-8
        encode_unknown(output_buffer);
        index += char_length;
    }
}

int cmark_utf8proc_iterate(const uint8_t* input, size_t input_length, int32_t* decoded_code_point)
{
    int length = 0;
    int32_t code_point = -1;

    *decoded_code_point = -1;
    length = utf8proc_charlen(input, input_length);
    if (length < 0)
    {
        return -1;
    }

    switch (length)
    {
        case 1:
            code_point = input[0];
            break;
        case 2:
            code_point = ((input[0] & 0x1F) << 6) + (input[1] & 0x3F);
            if (code_point < 0x80)
            {
                code_point = -1;
            }
            break;
        case 3:
            code_point = ((input[0] & 0x0F) << 12) + ((input[1] & 0x3F) << 6) + (input[2] & 0x3F);
            if (code_point < 0x800 || (code_point >= 0xD800 && code_point < 0xE000))
            {
                code_point = -1;
            }
            break;
        case 4:
            code_point = ((input[0] & 0x07) << 18) + ((input[1] & 0x3F) << 12) +
                         ((input[2] & 0x3F) << 6) + (input[3] & 0x3F);
            if (code_point < 0x10000 || code_point >= 0x110000)
            {
                code_point = -1;
            }
            break;
    }

    if (code_point < 0)
    {
        return -1;
    }

    *decoded_code_point = code_point;
    return length;
}

void cmark_utf8proc_encode_char(int32_t code_point, CMarkStringBuffer* buffer)
{
    uint8_t encoded_bytes[4];
    size_t encoded_length = 0;

    assert(code_point >= 0);

    if (code_point < 0x80)
    {
        encoded_bytes[0] = static_cast<uint8_t>(code_point);
        encoded_length = 1;
    }
    else if (code_point < 0x800)
    {
        encoded_bytes[0] = static_cast<uint8_t>(0xC0 + (code_point >> 6));
        encoded_bytes[1] = 0x80 + (code_point & 0x3F);
        encoded_length = 2;
    }
    else if (code_point == 0xFFFF)
    {
        encoded_bytes[0] = 0xFF;
        encoded_length = 1;
    }
    else if (code_point == 0xFFFE)
    {
        encoded_bytes[0] = 0xFE;
        encoded_length = 1;
    }
    else if (code_point < 0x10000)
    {
        encoded_bytes[0] = static_cast<uint8_t>(0xE0 + (code_point >> 12));
        encoded_bytes[1] = 0x80 + ((code_point >> 6) & 0x3F);
        encoded_bytes[2] = 0x80 + (code_point & 0x3F);
        encoded_length = 3;
    }
    else if (code_point < 0x110000)
    {
        encoded_bytes[0] = static_cast<uint8_t>(0xF0 + (code_point >> 18));
        encoded_bytes[1] = 0x80 + ((code_point >> 12) & 0x3F);
        encoded_bytes[2] = 0x80 + ((code_point >> 6) & 0x3F);
        encoded_bytes[3] = 0x80 + (code_point & 0x3F);
        encoded_length = 4;
    }
    else
    {
        encode_unknown(buffer);
        return;
    }

    buffer->Put(encoded_bytes, encoded_length);
}

void cmark_utf8proc_case_fold(CMarkStringBuffer* dest, const uint8_t* input, size_t length)
{
    int32_t code_point = 0;

#define c          code_point
#define bufpush(x) cmark_utf8proc_encode_char(x, dest)

    while (length > 0)
    {
        size_t char_len = cmark_utf8proc_iterate(input, length, &code_point);

        if (char_len >= 0)
        {
#include "case_fold_switch.inc"
        }
        else
        {
            encode_unknown(dest);
            char_len = -char_len;
        }

        input += char_len;
        length -= char_len;
    }

#undef bufpush
#undef c
}

// matches anything in the Zs class, plus LF, CR, TAB, FF.
int cmark_utf8proc_is_space(int32_t code_point)
{
    return (code_point == 9 || code_point == 10 || code_point == 12 || code_point == 13 ||
            code_point == 32 || code_point == 160 || code_point == 5760 ||
            (code_point >= 8192 && code_point <= 8202) || code_point == 8239 ||
            code_point == 8287 || code_point == 12288);
}

// matches anything in the P[cdefios] classes.
int cmark_utf8proc_is_punctuation(int32_t code_point)
{
    return (
        (code_point < 128 && cmark_ispunct((char) code_point)) || code_point == 161 ||
        code_point == 167 || code_point == 171 || code_point == 182 || code_point == 183 ||
        code_point == 187 || code_point == 191 || code_point == 894 || code_point == 903 ||
        (code_point >= 1370 && code_point <= 1375) || code_point == 1417 || code_point == 1418 ||
        code_point == 1470 || code_point == 1472 || code_point == 1475 || code_point == 1478 ||
        code_point == 1523 || code_point == 1524 || code_point == 1545 || code_point == 1546 ||
        code_point == 1548 || code_point == 1549 || code_point == 1563 || code_point == 1566 ||
        code_point == 1567 || (code_point >= 1642 && code_point <= 1645) || code_point == 1748 ||
        (code_point >= 1792 && code_point <= 1805) || (code_point >= 2039 && code_point <= 2041) ||
        (code_point >= 2096 && code_point <= 2110) || code_point == 2142 || code_point == 2404 ||
        code_point == 2405 || code_point == 2416 || code_point == 2800 || code_point == 3572 ||
        code_point == 3663 || code_point == 3674 || code_point == 3675 ||
        (code_point >= 3844 && code_point <= 3858) || code_point == 3860 ||
        (code_point >= 3898 && code_point <= 3901) || code_point == 3973 ||
        (code_point >= 4048 && code_point <= 4052) || code_point == 4057 || code_point == 4058 ||
        (code_point >= 4170 && code_point <= 4175) || code_point == 4347 ||
        (code_point >= 4960 && code_point <= 4968) || code_point == 5120 || code_point == 5741 ||
        code_point == 5742 || code_point == 5787 || code_point == 5788 ||
        (code_point >= 5867 && code_point <= 5869) || code_point == 5941 || code_point == 5942 ||
        (code_point >= 6100 && code_point <= 6102) || (code_point >= 6104 && code_point <= 6106) ||
        (code_point >= 6144 && code_point <= 6154) || code_point == 6468 || code_point == 6469 ||
        code_point == 6686 || code_point == 6687 || (code_point >= 6816 && code_point <= 6822) ||
        (code_point >= 6824 && code_point <= 6829) || (code_point >= 7002 && code_point <= 7008) ||
        (code_point >= 7164 && code_point <= 7167) || (code_point >= 7227 && code_point <= 7231) ||
        code_point == 7294 || code_point == 7295 || (code_point >= 7360 && code_point <= 7367) ||
        code_point == 7379 || (code_point >= 8208 && code_point <= 8231) ||
        (code_point >= 8240 && code_point <= 8259) || (code_point >= 8261 && code_point <= 8273) ||
        (code_point >= 8275 && code_point <= 8286) || code_point == 8317 || code_point == 8318 ||
        code_point == 8333 || code_point == 8334 || (code_point >= 8968 && code_point <= 8971) ||
        code_point == 9001 || code_point == 9002 || (code_point >= 10088 && code_point <= 10101) ||
        code_point == 10181 || code_point == 10182 ||
        (code_point >= 10214 && code_point <= 10223) ||
        (code_point >= 10627 && code_point <= 10648) ||
        (code_point >= 10712 && code_point <= 10715) || code_point == 10748 ||
        code_point == 10749 || (code_point >= 11513 && code_point <= 11516) ||
        code_point == 11518 || code_point == 11519 || code_point == 11632 ||
        (code_point >= 11776 && code_point <= 11822) ||
        (code_point >= 11824 && code_point <= 11842) ||
        (code_point >= 12289 && code_point <= 12291) ||
        (code_point >= 12296 && code_point <= 12305) ||
        (code_point >= 12308 && code_point <= 12319) || code_point == 12336 ||
        code_point == 12349 || code_point == 12448 || code_point == 12539 || code_point == 42238 ||
        code_point == 42239 || (code_point >= 42509 && code_point <= 42511) ||
        code_point == 42611 || code_point == 42622 ||
        (code_point >= 42738 && code_point <= 42743) ||
        (code_point >= 43124 && code_point <= 43127) || code_point == 43214 ||
        code_point == 43215 || (code_point >= 43256 && code_point <= 43258) ||
        code_point == 43310 || code_point == 43311 || code_point == 43359 ||
        (code_point >= 43457 && code_point <= 43469) || code_point == 43486 ||
        code_point == 43487 || (code_point >= 43612 && code_point <= 43615) ||
        code_point == 43742 || code_point == 43743 || code_point == 43760 || code_point == 43761 ||
        code_point == 44011 || code_point == 64830 || code_point == 64831 ||
        (code_point >= 65040 && code_point <= 65049) ||
        (code_point >= 65072 && code_point <= 65106) ||
        (code_point >= 65108 && code_point <= 65121) || code_point == 65123 ||
        code_point == 65128 || code_point == 65130 || code_point == 65131 ||
        (code_point >= 65281 && code_point <= 65283) ||
        (code_point >= 65285 && code_point <= 65290) ||
        (code_point >= 65292 && code_point <= 65295) || code_point == 65306 ||
        code_point == 65307 || code_point == 65311 || code_point == 65312 ||
        (code_point >= 65339 && code_point <= 65341) || code_point == 65343 ||
        code_point == 65371 || code_point == 65373 ||
        (code_point >= 65375 && code_point <= 65381) ||
        (code_point >= 65792 && code_point <= 65794) || code_point == 66463 ||
        code_point == 66512 || code_point == 66927 || code_point == 67671 || code_point == 67871 ||
        code_point == 67903 || (code_point >= 68176 && code_point <= 68184) ||
        code_point == 68223 || (code_point >= 68336 && code_point <= 68342) ||
        (code_point >= 68409 && code_point <= 68415) ||
        (code_point >= 68505 && code_point <= 68508) ||
        (code_point >= 69703 && code_point <= 69709) || code_point == 69819 ||
        code_point == 69820 || (code_point >= 69822 && code_point <= 69825) ||
        (code_point >= 69952 && code_point <= 69955) || code_point == 70004 ||
        code_point == 70005 || (code_point >= 70085 && code_point <= 70088) ||
        code_point == 70093 || (code_point >= 70200 && code_point <= 70205) ||
        code_point == 70854 || (code_point >= 71105 && code_point <= 71113) ||
        (code_point >= 71233 && code_point <= 71235) ||
        (code_point >= 74864 && code_point <= 74868) || code_point == 92782 ||
        code_point == 92783 || code_point == 92917 ||
        (code_point >= 92983 && code_point <= 92987) || code_point == 92996 ||
        code_point == 113823);
}
