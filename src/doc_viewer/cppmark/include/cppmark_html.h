/////////////////////////////////////////////////////////////////////////////
// Purpose:   CommonMark parsing, manipulating, and rendering
// Authors:   John MacFarlane, Vicent Marti, Kārlis Gaņģis, Nick Wellnhofer
// License:   FreeBSD -- see ../licenses/LICENSE_cmark
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <string_view>

// Convert UTF-8 Markdown to HTML and return it as a UTF-8 std::string.
std::string cmark_markdown_to_html(std::string_view text, int options);

// Option flags retained for compatibility with existing code.
constexpr int CMARK_OPT_DEFAULT = 0;
constexpr int CMARK_OPT_SOURCEPOS = (1 << 1);
constexpr int CMARK_OPT_HARDBREAKS = (1 << 2);
constexpr int CMARK_OPT_SAFE = (1 << 3);
constexpr int CMARK_OPT_NOBREAKS = (1 << 4);
constexpr int CMARK_OPT_NORMALIZE = (1 << 8);
constexpr int CMARK_OPT_VALIDATE_UTF8 = (1 << 9);
constexpr int CMARK_OPT_SMART = (1 << 10);
constexpr int CMARK_OPT_GITHUB_PRE_LANG = (1 << 11);
constexpr int CMARK_OPT_LIBERAL_HTML_TAG = (1 << 12);
constexpr int CMARK_OPT_FOOTNOTES = (1 << 13);
constexpr int CMARK_OPT_STRIKETHROUGH_DOUBLE_TILDE = (1 << 14);
constexpr int CMARK_OPT_TABLE_PREFER_STYLE_ATTRIBUTES = (1 << 15);
constexpr int CMARK_OPT_FULL_INFO_STRING = (1 << 16);
constexpr int CMARK_OPT_UNSAFE = (1 << 17);
