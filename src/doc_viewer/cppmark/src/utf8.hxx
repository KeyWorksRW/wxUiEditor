// UTF-8 validation and Unicode character properties
// Purpose: Parse UTF-8 multi-byte sequences, detect spaces/punctuation, case folding
// Key functions: cmark_utf8proc_iterate() (decode UTF-8), cmark_utf8proc_is_space/punctuation()
// Why kept: Required for correct markdown parsing - emphasis, word boundaries are Unicode-aware
// NOT for encoding conversion: Used for parsing logic, not converting between encodings
// License: MIT (see cppmark/licenses/LICENSE_utf8proc)
// Status: Legacy code
// Dependencies: buffer.hxx

#pragma once

#include "buffer.hxx"

void cmark_utf8proc_case_fold(CMarkStringBuffer* dest, const uint8_t* input, size_t length);

void cmark_utf8proc_encode_char(int32_t code_point, CMarkStringBuffer* buffer);

int cmark_utf8proc_iterate(const uint8_t* input, size_t input_length, int32_t* decoded_code_point);

int cmark_utf8proc_check(const uint8_t* line, size_t size);

void cmark_utf8proc_check(CMarkStringBuffer* dest, const uint8_t* line, size_t size);

int cmark_utf8proc_is_space(int32_t code_point);

int cmark_utf8proc_is_punctuation(int32_t code_point);
