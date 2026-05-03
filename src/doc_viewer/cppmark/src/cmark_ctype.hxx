// Locale-independent character classification
// Purpose: Portable ctype.h replacements - same behavior regardless of system locale
// Key functions: cmark_isspace(), cmark_ispunct(), cmark_isalnum(), cmark_isdigit(),
// cmark_isalpha() Why needed: Markdown spec requires ASCII-only behavior, system locale can change
// ctype results Status: Legacy code Dependencies: None

#pragma once

// Locale-independent versions of functions from ctype.h. We want cmark to behave the same no matter
// what the system locale.

bool cmark_isspace(char character);
bool cmark_ispunct(char character);
bool cmark_isalnum(char character);
bool cmark_isdigit(char character);
bool cmark_isalpha(char character);
