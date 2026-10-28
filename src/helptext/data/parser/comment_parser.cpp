/////////////////////////////////////////////////////////////////////////////
// Purpose:   Parse doxygen comment blocks into DocComment
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "comment_parser.h"
#include "doxy_commands.h"

#include <array>
#include <cctype>
#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// CR: [10-27-2026]

namespace docparser
{

    namespace
    {

        // ---------------------------------------------------------------------------
        // Helpers
        // ---------------------------------------------------------------------------

        // Strip the Javadoc '*' marker from a line. Only javadoc-prefixed blocks are
        // stripped; a plain block keeps its text verbatim, so a leading '*' in prose or
        // code is not mistaken for a comment marker. Inside @code/@verbatim the marker
        // is removed only when it is followed by a space or the end of the line, so an
        // asterisk that belongs to the code (for example "*ptr = 0;") survives.
        auto StripCommentLine(std::string_view line, bool javadoc, bool in_code) -> std::string
        {
            if (!javadoc)
            {
                return std::string(line);
            }

            size_t pos = 0;
            while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t'))
            {
                ++pos;
            }

            if (pos < line.size() && line[pos] == '*' &&
                (!in_code || pos + 1 == line.size() || line[pos + 1] == ' '))
            {
                ++pos;
                // Skip one space after '*' if present
                if (pos < line.size() && line[pos] == ' ')
                {
                    ++pos;
                }
                return std::string(line.substr(pos));
            }

            return std::string(line);
        }

        // Split text into lines
        auto SplitLines(std::string_view text) -> std::vector<std::string>
        {
            std::vector<std::string> lines;
            size_t start = 0;
            while (start < text.size())
            {
                const size_t newline_pos = text.find('\n', start);
                if (newline_pos == std::string_view::npos)
                {
                    std::string_view last_line = text.substr(start);
                    // Strip trailing \r
                    if (!last_line.empty() && last_line.back() == '\r')
                    {
                        last_line.remove_suffix(1);
                    }
                    lines.emplace_back(last_line);
                    break;
                }
                std::string_view line = text.substr(start, newline_pos - start);
                // Strip trailing \r
                if (!line.empty() && line.back() == '\r')
                {
                    line.remove_suffix(1);
                }
                lines.emplace_back(line);
                start = newline_pos + 1;
            }
            return lines;
        }

        // Trim leading and trailing whitespace
        auto Trim(std::string_view str) -> std::string
        {
            size_t start = 0;
            while (start < str.size() && (str[start] == ' ' || str[start] == '\t'))
            {
                ++start;
            }
            size_t end = str.size();
            while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t'))
            {
                --end;
            }
            return std::string(str.substr(start, end - start));
        }

        // True when the block uses the Javadoc "/** ... * ... */" layout: the first
        // non-blank line carries the marker.
        [[nodiscard]] auto IsJavadocBlock(const std::vector<std::string>& lines) -> bool
        {
            for (const std::string& line: lines)
            {
                const std::string trimmed = Trim(line);
                if (trimmed.empty())
                {
                    continue;
                }
                return trimmed.starts_with('*') || trimmed.starts_with("/*");
            }
            return false;
        }

        // Extract braced argument: "{foo}" -> "foo". Use AfterBraced() for the offset past '}'.
        // Returns empty if no braces found.
        auto ExtractBraced(std::string_view text) -> std::string
        {
            if (text.empty() || text[0] != '{')
            {
                return {};
            }
            const size_t close = text.find('}', 1);
            if (close == std::string_view::npos)
            {
                return {};
            }
            return std::string(text.substr(1, close - 1));
        }

        // Get position after braced argument (past the '}')
        auto AfterBraced(std::string_view text) -> size_t
        {
            if (text.empty() || text[0] != '{')
            {
                return 0;
            }
            const size_t close = text.find('}', 1);
            if (close == std::string_view::npos)
            {
                return 0;
            }
            return close + 1;
        }

        // Find the first unescaped occurrence of 'needle' in text.
        auto FindUnescaped(std::string_view text, char needle) -> size_t
        {
            for (size_t index = 0; index < text.size(); ++index)
            {
                if (text[index] == '\\' && index + 1 < text.size())
                {
                    ++index;
                    continue;
                }
                if (text[index] == needle)
                {
                    return index;
                }
            }
            return std::string_view::npos;
        }

        // Remove the backslash used to escape a comma in a doxygen table cell.
        auto UnescapeCommas(std::string_view text) -> std::string
        {
            std::string result;
            result.reserve(text.size());
            for (size_t index = 0; index < text.size(); ++index)
            {
                if (text[index] == '\\' && index + 1 < text.size() && text[index + 1] == ',')
                {
                    result += ',';
                    ++index;
                }
                else
                {
                    result += text[index];
                }
            }
            return result;
        }

        // Escape prose characters that would otherwise be interpreted as HTML.
        auto EscapeInlineText(std::string_view text) -> std::string
        {
            std::string result;
            result.reserve(text.size());
            for (const char character: text)
            {
                if (character == '&')
                {
                    result += "&amp;";
                }
                else if (character == '<')
                {
                    result += "&lt;";
                }
                else if (character == '>')
                {
                    result += "&gt;";
                }
                else
                {
                    result += character;
                }
            }
            return result;
        }

        // Append a span, merging adjacent runs of the same non-ref kind.
        void AppendSpan(std::vector<InlineSpan>& spans, InlineSpan::Kind kind,
                        std::string_view text, std::string_view target = {})
        {
            if (text.empty() && target.empty())
            {
                return;
            }

            if (kind != InlineSpan::Kind::ref && !spans.empty() && spans.back().kind == kind &&
                spans.back().target.empty())
            {
                spans.back().text.append(text);
                return;
            }

            InlineSpan span;
            span.kind = kind;
            span.text.assign(text);
            span.target.assign(target);
            spans.push_back(std::move(span));
        }

        // Append spans, optionally inserting a single separating space.
        void AppendSpans(std::vector<InlineSpan>& target, const std::vector<InlineSpan>& source,
                         bool add_space)
        {
            if (source.empty())
            {
                return;
            }
            if (add_space && !target.empty())
            {
                AppendSpan(target, InlineSpan::Kind::text, " ");
            }
            for (const InlineSpan& span: source)
            {
                target.push_back(span);
            }
        }

        // Return the next inline word. Parentheses stay with the word so that
        // @c IsNull() renders as one code span; a nested command marker ends it so
        // that "@b NAME(@e arg" does not swallow the @e command.
        auto NextWord(std::string_view text) -> std::string_view
        {
            size_t end_pos = 0;
            while (end_pos < text.size())
            {
                const char character = text[end_pos];
                // Only whitespace terminates a word, so "@c a/b" keeps the slash and
                // "@c x,y" keeps the comma.
                if (std::isspace(static_cast<unsigned char>(character)))
                {
                    break;
                }
                // A period ends the word only at a sentence boundary ("foo.").
                if (character == '.' &&
                    (end_pos + 1 == text.size() ||
                     std::isspace(static_cast<unsigned char>(text[end_pos + 1]))))
                {
                    break;
                }
                // A nested command marker ends the word only when it starts a command;
                // otherwise it is part of an email address or similar literal.
                if ((character == '@' || character == '\\') && end_pos + 1 < text.size() &&
                    ParseCommand(text.substr(end_pos)).first != DoxyCmd::unknown)
                {
                    break;
                }
                ++end_pos;
            }
            return text.substr(0, end_pos);
        }

        // Return the first whitespace-delimited word.
        auto FirstWord(std::string_view text) -> std::string_view
        {
            size_t end_pos = 0;
            while (end_pos < text.size() &&
                   !std::isspace(static_cast<unsigned char>(text[end_pos])))
            {
                ++end_pos;
            }
            return text.substr(0, end_pos);
        }

        // Forward declaration: strips inline command markers from a display string.
        auto PlainInlineText(std::string_view text) -> std::string;

        // Parse inline text into spans. HTML tags in the doxygen whitelist become
        // formatting spans; every other '<' stays literal text (escaped by the writer).
        void ParseInlineInto(std::string_view text, InlineSpan::Kind kind,
                             std::vector<InlineSpan>& spans, bool* hide_initializer = nullptr)
        {
            struct TagSpec
            {
                std::string_view open_tag;
                std::string_view close_tag;
                InlineSpan::Kind kind;
            };
            static constexpr std::array<TagSpec, 5> INLINE_TAGS = {
                { { "<b>", "</b>", InlineSpan::Kind::strong },
                  { "<i>", "</i>", InlineSpan::Kind::emphasis },
                  { "<em>", "</em>", InlineSpan::Kind::emphasis },
                  { "<tt>", "</tt>", InlineSpan::Kind::code },
                  { "<code>", "</code>", InlineSpan::Kind::code } }
            };

            size_t offset = 0;
            while (offset < text.size())
            {
                const char character = text[offset];

                if (character == '<')
                {
                    bool handled = false;
                    for (const TagSpec& spec: INLINE_TAGS)
                    {
                        if (!text.substr(offset).starts_with(spec.open_tag))
                        {
                            continue;
                        }
                        const std::string_view content = text.substr(offset + spec.open_tag.size());
                        const size_t close_pos = content.find(spec.close_tag);
                        if (close_pos == std::string_view::npos)
                        {
                            continue;
                        }
                        ParseInlineInto(content.substr(0, close_pos), spec.kind, spans);
                        offset += spec.open_tag.size() + close_pos + spec.close_tag.size();
                        handled = true;
                        break;
                    }
                    if (handled)
                    {
                        continue;
                    }

                    if (text.substr(offset).starts_with("<br>"))
                    {
                        AppendSpan(spans, kind, "  \n");
                        offset += 4;
                        continue;
                    }
                    if (text.substr(offset).starts_with("<br/>"))
                    {
                        AppendSpan(spans, kind, "  \n");
                        offset += 5;
                        continue;
                    }
                    if (text.substr(offset).starts_with("<br />"))
                    {
                        AppendSpan(spans, kind, "  \n");
                        offset += 6;
                        continue;
                    }

                    AppendSpan(spans, kind, "<");
                    ++offset;
                    continue;
                }

                // A command marker must not be embedded in a word (an email address
                // such as user@host stays literal); any non-word predecessor allows it.
                const bool is_command_start =
                    (character == '@' || character == '\\') && offset + 1 < text.size() &&
                    std::isalpha(static_cast<unsigned char>(text[offset + 1])) &&
                    (offset == 0 || (!std::isalnum(static_cast<unsigned char>(text[offset - 1])) &&
                                     text[offset - 1] != '_'));

                if (!is_command_start)
                {
                    AppendSpan(spans, kind, text.substr(offset, 1));
                    ++offset;
                    continue;
                }

                const std::string_view command_text = text.substr(offset);
                const std::pair<DoxyCmd, std::string_view> parsed = ParseCommand(command_text);
                const DoxyCmd command = parsed.first;
                const std::string_view rest = parsed.second;

                if (rest.data() == text.data() + offset)
                {
                    // No command word after the marker.
                    AppendSpan(spans, kind, text.substr(offset, 1));
                    ++offset;
                    continue;
                }

                if (command == DoxyCmd::unknown)
                {
                    // Drop the unknown @word token but keep its argument text.
                    offset = static_cast<size_t>(rest.data() - text.data());
                    continue;
                }

                switch (command)
                {
                    case DoxyCmd::a:
                    case DoxyCmd::e:
                        {
                            const std::string_view word = NextWord(rest);
                            AppendSpan(spans, InlineSpan::Kind::emphasis, word);
                            offset =
                                word.empty() ?
                                    static_cast<size_t>(rest.data() - text.data()) :
                                    static_cast<size_t>(rest.data() + word.size() - text.data());
                            break;
                        }
                    case DoxyCmd::b:
                    case DoxyCmd::em:
                        {
                            const std::string_view word = NextWord(rest);
                            AppendSpan(spans, InlineSpan::Kind::strong, word);
                            offset =
                                word.empty() ?
                                    static_cast<size_t>(rest.data() - text.data()) :
                                    static_cast<size_t>(rest.data() + word.size() - text.data());
                            break;
                        }
                    case DoxyCmd::c:
                    case DoxyCmd::p:
                        {
                            const std::string_view word = NextWord(rest);
                            AppendSpan(spans, InlineSpan::Kind::code, word);
                            offset =
                                word.empty() ?
                                    static_cast<size_t>(rest.data() - text.data()) :
                                    static_cast<size_t>(rest.data() + word.size() - text.data());
                            break;
                        }
                    case DoxyCmd::ref:
                        {
                            const std::string_view word = NextWord(rest);
                            if (word.empty())
                            {
                                offset = static_cast<size_t>(rest.data() - text.data());
                                break;
                            }

                            std::string display(word);
                            size_t consumed = word.size();
                            const std::string_view after = rest.substr(word.size());
                            size_t skip_space = 0;
                            while (skip_space < after.size() && after[skip_space] == ' ')
                            {
                                ++skip_space;
                            }
                            if (skip_space < after.size() && after[skip_space] == '"')
                            {
                                const size_t quote_end = after.find('"', skip_space + 1);
                                if (quote_end != std::string_view::npos)
                                {
                                    display.assign(
                                        after.substr(skip_space + 1, quote_end - skip_space - 1));
                                    consumed = word.size() + quote_end + 1;
                                }
                            }
                            display = PlainInlineText(display);
                            AppendSpan(spans, InlineSpan::Kind::ref, display, word);
                            offset = static_cast<size_t>(rest.data() + consumed - text.data());
                            break;
                        }
                    case DoxyCmd::true_:
                    case DoxyCmd::false_:
                    case DoxyCmd::null:
                        {
                            std::string_view literal = "NULL";
                            if (command == DoxyCmd::true_)
                            {
                                literal = "true";
                            }
                            else if (command == DoxyCmd::false_)
                            {
                                literal = "false";
                            }
                            AppendSpan(spans, InlineSpan::Kind::strong, literal);
                            if (!rest.empty() &&
                                (std::isalnum(static_cast<unsigned char>(rest[0])) ||
                                 rest[0] == '(' || rest[0] == '['))
                            {
                                AppendSpan(spans, kind, " ");
                            }
                            offset = static_cast<size_t>(rest.data() - text.data());
                            break;
                        }
                    case DoxyCmd::ref_member_group:
                        {
                            const std::string content = ExtractBraced(rest);
                            const size_t after = AfterBraced(rest);
                            std::string title = content;
                            const size_t comma = content.find(',');
                            if (comma != std::string::npos)
                            {
                                title = content.substr(comma + 1);
                            }
                            const std::string trimmed_title = Trim(title);
                            AppendSpan(spans, kind, trimmed_title);
                            offset = after > 0 ?
                                         static_cast<size_t>(rest.data() + after - text.data()) :
                                         static_cast<size_t>(rest.data() - text.data());
                            break;
                        }
                    case DoxyCmd::member_group_name:
                        {
                            const size_t after = AfterBraced(rest);
                            offset = after > 0 ?
                                         static_cast<size_t>(rest.data() + after - text.data()) :
                                         static_cast<size_t>(rest.data() - text.data());
                            break;
                        }
                    case DoxyCmd::only_for:
                    case DoxyCmd::native_impl:
                        {
                            const std::string platforms = Trim(ExtractBraced(rest));
                            const size_t after = AfterBraced(rest);
                            if (!platforms.empty())
                            {
                                std::string rendered = command == DoxyCmd::only_for ?
                                                           "only available for " :
                                                           "native implementation for ";
                                rendered += platforms;
                                AppendSpan(spans, kind, rendered);
                            }
                            offset = after > 0 ?
                                         static_cast<size_t>(rest.data() + after - text.data()) :
                                         static_cast<size_t>(rest.data() - text.data());
                            break;
                        }
                    case DoxyCmd::hide_initializer:
                        if (hide_initializer != nullptr)
                        {
                            *hide_initializer = true;
                        }
                        offset = static_cast<size_t>(rest.data() - text.data());
                        break;
                    default:
                        {
                            // Commands that have no inline rendering: drop the token.
                            offset = static_cast<size_t>(rest.data() - text.data());
                            break;
                        }
                }
            }
        }

        // Check if a line starts with a doxygen command (@ or \\)
        [[nodiscard]] auto StartsWithCommand(std::string_view line) -> bool
        {
            const std::string trimmed = Trim(line);
            return !trimmed.empty() && (trimmed[0] == '@' || trimmed[0] == '\\');
        }

        // Parse a command from the beginning of a line (skips leading whitespace).
        auto ParseLineCommand(std::string_view line) -> std::pair<DoxyCmd, std::string_view>
        {
            while (!line.empty() && (line[0] == ' ' || line[0] == '\t'))
            {
                line.remove_prefix(1);
            }
            if (line.empty() || (line[0] != '@' && line[0] != '\\'))
            {
                return { DoxyCmd::unknown, line };
            }
            return ParseCommand(line);
        }

        // True when a line starts with a block-level (non-inline) command.
        [[nodiscard]] auto StartsBlockCommand(std::string_view line) -> bool
        {
            if (!StartsWithCommand(line))
            {
                return false;
            }
            const std::pair<DoxyCmd, std::string_view> parsed = ParseLineCommand(line);
            switch (parsed.first)
            {
                case DoxyCmd::a:
                case DoxyCmd::b:
                case DoxyCmd::c:
                case DoxyCmd::p:
                case DoxyCmd::e:
                case DoxyCmd::em:
                case DoxyCmd::ref:
                case DoxyCmd::ref_member_group:
                case DoxyCmd::member_group_name:
                case DoxyCmd::true_:
                case DoxyCmd::false_:
                case DoxyCmd::null:
                    return false;
                default:
                    return true;
            }
        }

        // True when a line starts a bullet/ordered list item or a table row, which a
        // multi-line value block must not absorb as continuation text.
        [[nodiscard]] auto StartsListOrTable(std::string_view line) -> bool
        {
            const std::string trimmed = Trim(line);
            if (trimmed.empty())
            {
                return false;
            }
            return trimmed[0] == '-' || trimmed[0] == '|';
        }

        // Parse inline text into spans. When 'hide_initializer' is non-null, an
        // @hideinitializer encountered in the text sets it.
        auto ParseInline(std::string_view text, bool* hide_initializer = nullptr)
            -> std::vector<InlineSpan>
        {
            std::vector<InlineSpan> spans;
            ParseInlineInto(text, InlineSpan::Kind::text, spans, hide_initializer);
            return spans;
        }

        // Strip inline command markers and formatting from a display string,
        // keeping the argument text (e.g. "@c WXTRACE variable" -> "WXTRACE variable").
        auto PlainInlineText(std::string_view text) -> std::string
        {
            const std::vector<InlineSpan> spans = ParseInline(text);
            std::string result;
            for (const InlineSpan& span: spans)
            {
                if (span.kind == InlineSpan::Kind::ref)
                {
                    result += span.text.empty() ? span.target : span.text;
                }
                else
                {
                    result += span.text;
                }
            }
            return result;
        }

        // Append table-cell text: '|' is escaped so it cannot end the row, and a line
        // break from '<br>' becomes a literal '<br>' so it survives inside a cell.
        void AppendTableCellText(std::string& target, std::string_view text)
        {
            const std::string escaped = EscapeInlineText(text);
            for (const char character: escaped)
            {
                if (character == '|')
                {
                    target += "\\|";
                }
                else if (character == '\n')
                {
                    target += "<br>";
                }
                else
                {
                    target += character;
                }
            }
        }

        // Render a markdown code span whose backtick delimiter cannot be closed early
        // by the backticks in 'text'; '|' is escaped so the span stays inside a cell.
        auto CodeSpanMarkdown(std::string_view text) -> std::string
        {
            size_t longest_run = 0;
            size_t current_run = 0;
            for (const char character: text)
            {
                current_run = character == '`' ? current_run + 1 : 0;
                if (current_run > longest_run)
                {
                    longest_run = current_run;
                }
            }

            std::string escaped;
            escaped.reserve(text.size());
            for (const char character: text)
            {
                if (character == '|')
                {
                    escaped += "\\|";
                }
                else
                {
                    escaped += character;
                }
            }

            const std::string delimiter(longest_run + 1, '`');
            const bool needs_padding =
                !escaped.empty() && (escaped.front() == '`' || escaped.back() == '`');
            std::string result = delimiter;
            if (needs_padding)
            {
                result += ' ';
            }
            result += escaped;
            if (needs_padding)
            {
                result += ' ';
            }
            result += delimiter;
            return result;
        }

        // Render inline spans to a markdown string for fixed-position tables.
        auto InlineToMarkdown(std::string_view text) -> std::string
        {
            const std::vector<InlineSpan> spans = ParseInline(text);
            std::string result;
            for (const InlineSpan& span: spans)
            {
                if (span.kind == InlineSpan::Kind::code)
                {
                    result += CodeSpanMarkdown(span.text);
                }
                else if (span.kind == InlineSpan::Kind::emphasis)
                {
                    result += '*';
                    AppendTableCellText(result, span.text);
                    result += '*';
                }
                else if (span.kind == InlineSpan::Kind::strong)
                {
                    result += "**";
                    AppendTableCellText(result, span.text);
                    result += "**";
                }
                else if (span.kind == InlineSpan::Kind::ref)
                {
                    AppendTableCellText(result, span.text.empty() ? span.target : span.text);
                }
                else
                {
                    AppendTableCellText(result, span.text);
                }
            }
            return result;
        }

        // Append the continuation lines of a command block to 'spans'.
        void ReadContinuation(const std::vector<std::string>& cleaned, size_t& line_index,
                              std::vector<InlineSpan>& spans)
        {
            while (line_index + 1 < cleaned.size())
            {
                const std::string next_trimmed = Trim(cleaned[line_index + 1]);
                if (next_trimmed.empty() || StartsBlockCommand(next_trimmed) ||
                    StartsListOrTable(next_trimmed))
                {
                    break;
                }
                ++line_index;
                AppendSpans(spans, ParseInline(next_trimmed), true);
            }
        }

        // Split one @see argument list into reference items.
        void AppendSeeItems(std::vector<std::vector<InlineSpan>>& items, std::string_view segment)
        {
            if (segment.empty())
            {
                return;
            }

            if (segment.find('@') != std::string_view::npos ||
                segment.find('<') != std::string_view::npos)
            {
                std::vector<InlineSpan> spans = ParseInline(segment);
                if (!spans.empty())
                {
                    items.push_back(std::move(spans));
                }
                return;
            }

            size_t start = 0;
            while (start < segment.size())
            {
                while (start < segment.size() &&
                       std::isspace(static_cast<unsigned char>(segment[start])))
                {
                    ++start;
                }
                size_t end_pos = start;
                while (end_pos < segment.size() &&
                       !std::isspace(static_cast<unsigned char>(segment[end_pos])))
                {
                    ++end_pos;
                }
                if (end_pos > start)
                {
                    std::vector<InlineSpan> item;
                    const std::string token(segment.substr(start, end_pos - start));
                    AppendSpan(item, InlineSpan::Kind::ref, token, token);
                    items.push_back(std::move(item));
                }
                start = end_pos;
            }
        }

        // Append a block, merging adjacent list/table/definition blocks.
        void PushBlock(std::vector<DocBlock>& blocks, DocBlock block)
        {
            if (!blocks.empty() && blocks.back().kind == block.kind)
            {
                DocBlock& last = blocks.back();
                if (block.kind == BlockKind::bullet_list || block.kind == BlockKind::ordered_list ||
                    block.kind == BlockKind::see_also)
                {
                    for (std::vector<InlineSpan>& item: block.items)
                    {
                        last.items.push_back(std::move(item));
                    }
                    return;
                }
                if (block.kind == BlockKind::params || block.kind == BlockKind::def_list)
                {
                    for (auto& item: block.def_items)
                    {
                        last.def_items.push_back(std::move(item));
                    }
                    return;
                }
                if (block.kind == BlockKind::table && !block.rows.empty() && !last.rows.empty() &&
                    last.rows[0].size() == block.rows[0].size())
                {
                    for (std::vector<std::vector<InlineSpan>>& row: block.rows)
                    {
                        last.rows.push_back(std::move(row));
                    }
                    return;
                }
            }
            blocks.push_back(std::move(block));
        }

        // Split braced table content into the requested number of cells at unescaped commas.
        auto SplitTableCells(std::string_view content, size_t column_count)
            -> std::vector<std::string>
        {
            std::vector<std::string> cells;
            size_t start = 0;
            size_t remaining = column_count > 0 ? column_count - 1 : 0;
            while (remaining > 0)
            {
                const std::string_view tail = content.substr(start);
                const size_t comma = FindUnescaped(tail, ',');
                if (comma == std::string_view::npos)
                {
                    break;
                }
                cells.push_back(UnescapeCommas(tail.substr(0, comma)));
                start += comma + 1;
                --remaining;
            }
            cells.push_back(UnescapeCommas(content.substr(start)));
            for (std::string& cell: cells)
            {
                cell = Trim(cell);
            }
            return cells;
        }

    }  // anonymous namespace

    // ---------------------------------------------------------------------------
    // Main entry point
    // ---------------------------------------------------------------------------

    auto ParseCommentBlock(std::string_view text) -> DocComment
    {
        DocComment result;

        const std::vector<std::string> lines = SplitLines(text);
        const bool javadoc = IsJavadocBlock(lines);

        // Strip Javadoc comment markers, but keep the leading '*' of a @code or
        // @verbatim body so that code such as "*ptr = 0;" is not mangled.
        std::vector<std::string> cleaned;
        cleaned.reserve(lines.size());
        bool inside_code_block = false;
        for (const std::string& line: lines)
        {
            // Detect @code/@verbatim toggles on the marker-stripped view so that the
            // Javadoc prefix does not hide the command.
            const std::string command_view = StripCommentLine(line, javadoc, false);
            const std::string command_trimmed = Trim(command_view);
            const std::pair<DoxyCmd, std::string_view> parsed = ParseLineCommand(command_trimmed);

            std::string cleaned_line;
            if (inside_code_block && parsed.first != DoxyCmd::end_code &&
                parsed.first != DoxyCmd::end_verbatim)
            {
                cleaned_line = StripCommentLine(line, javadoc, true);
            }
            else
            {
                cleaned_line = command_view;
            }

            if (parsed.first == DoxyCmd::code || parsed.first == DoxyCmd::verbatim)
            {
                inside_code_block = true;
            }
            else if (parsed.first == DoxyCmd::end_code || parsed.first == DoxyCmd::end_verbatim)
            {
                inside_code_block = false;
            }
            cleaned.push_back(std::move(cleaned_line));
        }

        enum class State : std::uint8_t
        {
            Normal,
            InCode,
            InVerbatim,
            InStyleTable,
            InEventTable,
            InTable,
            InDefList,
            InSkipBlock,  // Python only blocks
        };

        State state = State::Normal;
        bool have_brief = false;
        bool hide_initializer = false;
        bool paragraph_open = false;
        std::vector<InlineSpan> paragraph_spans;
        std::vector<DocBlock> blocks;

        std::string code_lang;
        std::string code_content;
        bool code_started = false;
        std::string event_table_type;

        DocBlock table_block;
        DocBlock def_list_block;

        // Flush the pending paragraph into the block list.
        auto flush_paragraph = [&]() -> void
        {
            if (!paragraph_open)
            {
                return;
            }
            paragraph_open = false;
            if (paragraph_spans.empty())
            {
                return;
            }
            DocBlock block;
            block.kind = BlockKind::paragraph;
            block.spans = std::move(paragraph_spans);
            paragraph_spans.clear();
            blocks.push_back(std::move(block));
        };

        auto append_paragraph = [&](std::string_view plain_text) -> void
        {
            if (plain_text.empty())
            {
                return;
            }
            const std::vector<InlineSpan> spans = ParseInline(plain_text, &hide_initializer);
            if (spans.empty())
            {
                return;
            }
            AppendSpans(paragraph_spans, spans, paragraph_open);
            paragraph_open = true;
        };

        auto push_value_block = [&](BlockKind kind, std::vector<InlineSpan> spans) -> void
        {
            if (spans.empty())
            {
                return;
            }
            DocBlock block;
            block.kind = kind;
            block.spans = std::move(spans);
            PushBlock(blocks, std::move(block));
        };

        auto ensure_list_block = [&](BlockKind kind) -> void
        {
            if (blocks.empty() || blocks.back().kind != kind)
            {
                DocBlock block;
                block.kind = kind;
                blocks.push_back(std::move(block));
            }
        };

        for (size_t line_index = 0; line_index < cleaned.size(); ++line_index)
        {
            const std::string& line = cleaned[line_index];
            const std::string trimmed = Trim(line);

            if (state == State::InSkipBlock)
            {
                const std::pair<DoxyCmd, std::string_view> parsed = ParseLineCommand(trimmed);
                if (IsSkipBlockEnd(parsed.first))
                {
                    state = State::Normal;
                }
                continue;
            }

            if (state == State::InCode)
            {
                const std::pair<DoxyCmd, std::string_view> parsed = ParseLineCommand(trimmed);
                if (parsed.first == DoxyCmd::end_code)
                {
                    DocBlock block;
                    block.kind = BlockKind::code;
                    block.language = code_lang;
                    block.code = code_content;
                    PushBlock(blocks, std::move(block));
                    code_lang.clear();
                    code_content.clear();
                    state = State::Normal;
                }
                else
                {
                    if (code_started)
                    {
                        code_content += '\n';
                    }
                    code_started = true;
                    code_content += line;
                }
                continue;
            }

            if (state == State::InVerbatim)
            {
                const std::pair<DoxyCmd, std::string_view> parsed = ParseLineCommand(trimmed);
                if (parsed.first == DoxyCmd::end_verbatim)
                {
                    DocBlock block;
                    block.kind = BlockKind::verbatim;
                    block.code = code_content;
                    PushBlock(blocks, std::move(block));
                    code_content.clear();
                    state = State::Normal;
                }
                else
                {
                    if (code_started)
                    {
                        code_content += '\n';
                    }
                    code_started = true;
                    code_content += line;
                }
                continue;
            }

            if (state == State::InStyleTable)
            {
                const std::pair<DoxyCmd, std::string_view> parsed = ParseLineCommand(trimmed);
                if (parsed.first == DoxyCmd::end_style_table)
                {
                    flush_paragraph();
                    state = State::Normal;
                    continue;
                }
                if (parsed.first == DoxyCmd::style)
                {
                    flush_paragraph();
                    const std::string name = Trim(ExtractBraced(parsed.second));
                    const size_t after = AfterBraced(parsed.second);
                    std::string description;
                    if (after < parsed.second.size())
                    {
                        description = InlineToMarkdown(Trim(parsed.second.substr(after)));
                    }
                    while (line_index + 1 < cleaned.size())
                    {
                        const std::string next_trimmed = Trim(cleaned[line_index + 1]);
                        const std::pair<DoxyCmd, std::string_view> next_parsed =
                            ParseLineCommand(next_trimmed);
                        if (next_parsed.first == DoxyCmd::style ||
                            next_parsed.first == DoxyCmd::end_style_table ||
                            StartsBlockCommand(next_trimmed))
                        {
                            break;
                        }
                        if (!description.empty())
                        {
                            description += ' ';
                        }
                        description += InlineToMarkdown(next_trimmed);
                        ++line_index;
                    }
                    result.styles.push_back({ name, Trim(description) });
                }
                else if (StartsBlockCommand(trimmed))
                {
                    // Any other block command ends the style table; reprocess it in the
                    // normal state so that it is not dropped.
                    flush_paragraph();
                    state = State::Normal;
                    --line_index;
                    continue;
                }
                else if (!trimmed.empty() && trimmed[0] != '@' && trimmed[0] != '\\')
                {
                    // A plain line inside the table is prose (for example 'See
                    // wxDataViewCtrl for the list of supported styles.'); keep it.
                    append_paragraph(trimmed);
                }
                continue;
            }

            if (state == State::InEventTable)
            {
                const std::pair<DoxyCmd, std::string_view> parsed = ParseLineCommand(trimmed);
                if (parsed.first == DoxyCmd::end_event_table)
                {
                    flush_paragraph();
                    state = State::Normal;
                    continue;
                }
                if (parsed.first == DoxyCmd::event)
                {
                    flush_paragraph();
                    const std::string name = Trim(ExtractBraced(parsed.second));
                    const size_t after = AfterBraced(parsed.second);
                    std::string description;
                    if (after < parsed.second.size())
                    {
                        description = InlineToMarkdown(Trim(parsed.second.substr(after)));
                    }
                    while (line_index + 1 < cleaned.size())
                    {
                        const std::string next_trimmed = Trim(cleaned[line_index + 1]);
                        const std::pair<DoxyCmd, std::string_view> next_parsed =
                            ParseLineCommand(next_trimmed);
                        if (next_parsed.first == DoxyCmd::event ||
                            next_parsed.first == DoxyCmd::end_event_table ||
                            StartsBlockCommand(next_trimmed))
                        {
                            break;
                        }
                        if (!description.empty())
                        {
                            description += ' ';
                        }
                        description += InlineToMarkdown(next_trimmed);
                        ++line_index;
                    }
                    EventEntry entry;
                    entry.name = name;
                    entry.description = Trim(description);
                    entry.event_type = event_table_type;
                    result.events.push_back(std::move(entry));
                }
                else if (StartsBlockCommand(trimmed))
                {
                    // Any other block command ends the event table; reprocess it in the
                    // normal state so that it is not dropped.
                    flush_paragraph();
                    state = State::Normal;
                    --line_index;
                    continue;
                }
                else if (!trimmed.empty() && trimmed[0] != '@' && trimmed[0] != '\\')
                {
                    // A plain line inside the table is prose (for example 'See
                    // wxDataViewCtrl for the list of events emitted by this class.').
                    append_paragraph(trimmed);
                }
                continue;
            }

            if (state == State::InTable)
            {
                const std::pair<DoxyCmd, std::string_view> parsed = ParseLineCommand(trimmed);
                if (parsed.first == DoxyCmd::end_table)
                {
                    if (!table_block.rows.empty())
                    {
                        PushBlock(blocks, std::move(table_block));
                    }
                    table_block = DocBlock {};
                    state = State::Normal;
                    continue;
                }
                if (parsed.first != DoxyCmd::row_2_col && parsed.first != DoxyCmd::row_3_col)
                {
                    if (trimmed.empty())
                    {
                        continue;
                    }
                    // A non-row line ends the table; reprocess it in the normal state so
                    // that prose and block commands after a table are not dropped.
                    if (!table_block.rows.empty())
                    {
                        PushBlock(blocks, std::move(table_block));
                    }
                    table_block = DocBlock {};
                    state = State::Normal;
                    --line_index;
                    continue;
                }

                const size_t column_count = parsed.first == DoxyCmd::row_3_col ? 3U : 2U;
                std::string content;
                std::string_view rest = parsed.second;
                if (!rest.empty() && rest[0] == '{')
                {
                    rest.remove_prefix(1);
                }

                bool closed = false;
                size_t close_pos = FindUnescaped(rest, '}');
                if (close_pos != std::string_view::npos)
                {
                    content.append(rest.substr(0, close_pos));
                    const std::string remainder = Trim(rest.substr(close_pos + 1));
                    if (!remainder.empty())
                    {
                        content += ' ';
                        content += remainder;
                    }
                    closed = true;
                }
                else
                {
                    content.append(rest);
                }

                while (!closed && line_index + 1 < cleaned.size())
                {
                    const std::string next_trimmed = Trim(cleaned[line_index + 1]);
                    const std::pair<DoxyCmd, std::string_view> next_parsed =
                        ParseLineCommand(next_trimmed);
                    // An unclosed row must not swallow the next row or the end of the
                    // table, so leave those lines for the main loop.
                    if (next_parsed.first == DoxyCmd::row_2_col ||
                        next_parsed.first == DoxyCmd::row_3_col ||
                        next_parsed.first == DoxyCmd::end_table || StartsBlockCommand(next_trimmed))
                    {
                        break;
                    }
                    ++line_index;
                    close_pos = FindUnescaped(next_trimmed, '}');
                    if (close_pos != std::string::npos)
                    {
                        content += ' ';
                        content.append(next_trimmed, 0, close_pos);
                        const std::string remainder = Trim(next_trimmed.substr(close_pos + 1));
                        if (!remainder.empty())
                        {
                            content += ' ';
                            content += remainder;
                        }
                        closed = true;
                    }
                    else
                    {
                        content += ' ';
                        content += next_trimmed;
                    }
                }

                while (line_index + 1 < cleaned.size())
                {
                    const std::string next_trimmed = Trim(cleaned[line_index + 1]);
                    const std::pair<DoxyCmd, std::string_view> next_parsed =
                        ParseLineCommand(next_trimmed);
                    if (next_parsed.first == DoxyCmd::row_2_col ||
                        next_parsed.first == DoxyCmd::row_3_col ||
                        next_parsed.first == DoxyCmd::end_table)
                    {
                        break;
                    }
                    if (next_trimmed.empty())
                    {
                        ++line_index;
                        continue;
                    }
                    content += ' ';
                    content += next_trimmed;
                    ++line_index;
                }

                const std::vector<std::string> cells = SplitTableCells(content, column_count);
                std::vector<std::vector<InlineSpan>> row;
                row.reserve(cells.size());
                for (const std::string& cell: cells)
                {
                    row.push_back(ParseInline(cell));
                }
                table_block.rows.push_back(std::move(row));
                continue;
            }

            if (state == State::InDefList)
            {
                const std::pair<DoxyCmd, std::string_view> parsed = ParseLineCommand(trimmed);
                if (parsed.first == DoxyCmd::end_def_list)
                {
                    if (!def_list_block.def_items.empty())
                    {
                        PushBlock(blocks, std::move(def_list_block));
                    }
                    def_list_block = DocBlock {};
                    state = State::Normal;
                    continue;
                }
                if (parsed.first == DoxyCmd::item_def)
                {
                    const std::string term = Trim(ExtractBraced(parsed.second));
                    const size_t after = AfterBraced(parsed.second);
                    std::vector<InlineSpan> definition;
                    if (after < parsed.second.size())
                    {
                        AppendSpans(definition, ParseInline(Trim(parsed.second.substr(after))),
                                    false);
                    }
                    ReadContinuation(cleaned, line_index, definition);
                    def_list_block.def_items.emplace_back(ParseInline(term), std::move(definition));
                }
                continue;
            }

            const std::pair<DoxyCmd, std::string_view> parsed = ParseLineCommand(trimmed);
            const DoxyCmd command = parsed.first;
            const std::string_view rest = parsed.second;

            switch (command)
            {
                case DoxyCmd::class_:
                    {
                        flush_paragraph();
                        result.class_name = std::string(FirstWord(rest));
                        continue;
                    }
                case DoxyCmd::page:
                    {
                        flush_paragraph();
                        result.page_id = std::string(FirstWord(rest));
                        std::string title = Trim(rest.substr(FirstWord(rest).size()));
                        while (line_index + 1 < cleaned.size())
                        {
                            const std::string next_trimmed = Trim(cleaned[line_index + 1]);
                            if (next_trimmed.empty() || StartsWithCommand(next_trimmed))
                            {
                                break;
                            }
                            title += ' ';
                            title += next_trimmed;
                            ++line_index;
                        }
                        result.page_title = Trim(title);
                        continue;
                    }
                case DoxyCmd::brief:
                    {
                        flush_paragraph();
                        std::vector<InlineSpan> spans = ParseInline(rest);
                        ReadContinuation(cleaned, line_index, spans);
                        if (!spans.empty())
                        {
                            // Only a non-empty @brief may suppress the first-paragraph
                            // fallback.
                            have_brief = true;
                            DocBlock block;
                            block.kind = BlockKind::paragraph;
                            block.is_brief = true;
                            block.spans = std::move(spans);
                            blocks.push_back(std::move(block));
                        }
                        continue;
                    }
                case DoxyCmd::param:
                    {
                        flush_paragraph();
                        std::string_view param_rest = rest;
                        if (!param_rest.empty() && param_rest[0] == '[')
                        {
                            const size_t close_pos = param_rest.find(']');
                            if (close_pos != std::string_view::npos)
                            {
                                param_rest.remove_prefix(close_pos + 1);
                                while (!param_rest.empty() && param_rest[0] == ' ')
                                {
                                    param_rest.remove_prefix(1);
                                }
                            }
                        }
                        const std::string_view name = FirstWord(param_rest);
                        if (name.empty())
                        {
                            continue;
                        }
                        std::vector<InlineSpan> description;
                        const std::string description_text = Trim(param_rest.substr(name.size()));
                        if (!description_text.empty())
                        {
                            description = ParseInline(description_text);
                        }
                        ReadContinuation(cleaned, line_index, description);
                        DocBlock block;
                        block.kind = BlockKind::params;
                        block.def_items.emplace_back(ParseInline(name), std::move(description));
                        PushBlock(blocks, std::move(block));
                        continue;
                    }
                case DoxyCmd::return_:
                case DoxyCmd::returns:
                case DoxyCmd::since:
                case DoxyCmd::deprecated:
                case DoxyCmd::note:
                case DoxyCmd::warning:
                case DoxyCmd::todo:
                case DoxyCmd::remark:
                case DoxyCmd::remarks:
                    {
                        flush_paragraph();
                        std::vector<InlineSpan> spans = ParseInline(rest);
                        ReadContinuation(cleaned, line_index, spans);

                        BlockKind kind = BlockKind::note;
                        if (command == DoxyCmd::return_ || command == DoxyCmd::returns)
                        {
                            kind = BlockKind::returns;
                        }
                        else if (command == DoxyCmd::since)
                        {
                            kind = BlockKind::since;
                        }
                        else if (command == DoxyCmd::deprecated)
                        {
                            kind = BlockKind::deprecated;
                        }
                        else if (command == DoxyCmd::warning)
                        {
                            kind = BlockKind::warning;
                        }
                        else if (command == DoxyCmd::todo)
                        {
                            kind = BlockKind::todo;
                        }
                        push_value_block(kind, std::move(spans));
                        continue;
                    }
                case DoxyCmd::see:
                case DoxyCmd::sa:
                    {
                        flush_paragraph();
                        std::string refs_text(rest);
                        while (line_index + 1 < cleaned.size())
                        {
                            const std::string next_trimmed = Trim(cleaned[line_index + 1]);
                            if (next_trimmed.empty() || StartsBlockCommand(next_trimmed))
                            {
                                break;
                            }
                            refs_text += ' ';
                            refs_text += next_trimmed;
                            ++line_index;
                        }

                        DocBlock block;
                        block.kind = BlockKind::see_also;
                        size_t start = 0;
                        while (start <= refs_text.size())
                        {
                            const size_t comma = refs_text.find(',', start);
                            const std::string segment = Trim(std::string_view(refs_text).substr(
                                start,
                                comma == std::string::npos ? std::string::npos : comma - start));
                            AppendSeeItems(block.items, segment);
                            if (comma == std::string::npos)
                            {
                                break;
                            }
                            start = comma + 1;
                        }
                        if (!block.items.empty())
                        {
                            PushBlock(blocks, std::move(block));
                        }
                        continue;
                    }
                case DoxyCmd::code:
                    {
                        flush_paragraph();
                        state = State::InCode;
                        code_lang.clear();
                        if (!rest.empty() && rest[0] == '{')
                        {
                            std::string language = Trim(ExtractBraced(rest));
                            if (!language.empty() && language[0] == '.')
                            {
                                language.erase(0, 1);
                            }
                            code_lang = std::move(language);
                        }
                        code_content.clear();
                        code_started = false;
                        continue;
                    }
                case DoxyCmd::begin_style_table:
                    {
                        flush_paragraph();
                        state = State::InStyleTable;
                        continue;
                    }
                case DoxyCmd::begin_event_table:
                case DoxyCmd::begin_event_emission_table:
                    {
                        flush_paragraph();
                        state = State::InEventTable;
                        event_table_type = Trim(ExtractBraced(rest));
                        continue;
                    }
                case DoxyCmd::begin_table:
                    {
                        flush_paragraph();
                        table_block = DocBlock {};
                        table_block.kind = BlockKind::table;
                        state = State::InTable;
                        continue;
                    }
                case DoxyCmd::begin_def_list:
                    {
                        flush_paragraph();
                        def_list_block = DocBlock {};
                        def_list_block.kind = BlockKind::def_list;
                        state = State::InDefList;
                        continue;
                    }
                case DoxyCmd::library:
                    {
                        flush_paragraph();
                        result.library = Trim(ExtractBraced(rest));
                        continue;
                    }
                case DoxyCmd::category:
                    {
                        flush_paragraph();
                        result.category = Trim(ExtractBraced(rest));
                        continue;
                    }
                case DoxyCmd::appearance:
                    {
                        flush_paragraph();
                        result.appearance = Trim(ExtractBraced(rest));
                        continue;
                    }
                case DoxyCmd::header:
                    {
                        flush_paragraph();
                        result.header_file = Trim(ExtractBraced(rest));
                        continue;
                    }
                case DoxyCmd::add_to_group:
                    {
                        flush_paragraph();
                        result.group_name = std::string(FirstWord(rest));
                        continue;
                    }
                case DoxyCmd::li:
                    {
                        flush_paragraph();
                        std::vector<InlineSpan> item = ParseInline(rest);
                        ReadContinuation(cleaned, line_index, item);
                        if (!item.empty())
                        {
                            ensure_list_block(BlockKind::bullet_list);
                            blocks.back().items.push_back(std::move(item));
                        }
                        continue;
                    }
                case DoxyCmd::section:
                case DoxyCmd::subsection:
                case DoxyCmd::subsubsection:
                    {
                        flush_paragraph();
                        const std::string_view id = FirstWord(rest);
                        const std::string title = Trim(rest.substr(id.size()));
                        DocBlock block;
                        block.kind = BlockKind::section;
                        block.id = std::string(id);
                        block.spans = ParseInline(title);
                        if (command == DoxyCmd::section)
                        {
                            block.level = 0;
                        }
                        else if (command == DoxyCmd::subsection)
                        {
                            block.level = 1;
                        }
                        else
                        {
                            block.level = 2;
                        }
                        PushBlock(blocks, std::move(block));
                        continue;
                    }
                case DoxyCmd::verbatim:
                    {
                        flush_paragraph();
                        state = State::InVerbatim;
                        code_content.clear();
                        code_started = false;
                        continue;
                    }
                case DoxyCmd::anchor:
                    {
                        flush_paragraph();
                        const std::string_view anchor_id = FirstWord(rest);
                        if (!anchor_id.empty())
                        {
                            DocBlock block;
                            block.kind = BlockKind::anchor;
                            block.id = std::string(anchor_id);
                            PushBlock(blocks, std::move(block));
                        }
                        continue;
                    }
                case DoxyCmd::image:
                    {
                        flush_paragraph();
                        std::string_view image_rest = rest;
                        const std::string_view format = FirstWord(image_rest);
                        if (!format.empty())
                        {
                            image_rest.remove_prefix(format.size());
                            while (!image_rest.empty() && image_rest[0] == ' ')
                            {
                                image_rest.remove_prefix(1);
                            }
                        }
                        const std::string_view image_name = FirstWord(image_rest);
                        if (!image_name.empty())
                        {
                            DocBlock block;
                            block.kind = BlockKind::image;
                            block.id = std::string(image_name);
                            PushBlock(blocks, std::move(block));
                        }
                        continue;
                    }
                case DoxyCmd::copydoc:
                    {
                        flush_paragraph();
                        const std::string_view target = FirstWord(rest);
                        if (!target.empty())
                        {
                            DocBlock block;
                            block.kind = BlockKind::paragraph;
                            AppendSpan(block.spans, InlineSpan::Kind::emphasis, "See");
                            AppendSpan(block.spans, InlineSpan::Kind::text, " ");
                            AppendSpan(block.spans, InlineSpan::Kind::ref, target, target);
                            PushBlock(blocks, std::move(block));
                        }
                        continue;
                    }
                case DoxyCmd::only_for:
                case DoxyCmd::native_impl:
                    {
                        flush_paragraph();
                        const std::string platforms = ExtractBraced(rest);
                        if (!platforms.empty())
                        {
                            std::string formatted;
                            size_t start = 0;
                            while (start <= platforms.size())
                            {
                                const size_t comma = platforms.find(',', start);
                                const size_t length =
                                    comma == std::string::npos ? std::string::npos : comma - start;
                                const std::string item =
                                    Trim(std::string_view(platforms).substr(start, length));
                                if (!item.empty())
                                {
                                    if (!formatted.empty())
                                    {
                                        formatted += ", ";
                                    }
                                    formatted += item;
                                }
                                if (comma == std::string::npos)
                                {
                                    break;
                                }
                                start = comma + 1;
                            }

                            std::string text = command == DoxyCmd::only_for ?
                                                   "only available for " :
                                                   "native implementation for ";
                            text += formatted;
                            // Text after the closing brace ("@onlyfor{wxmsw} 3.1.0")
                            // belongs to the availability note.
                            const std::string trailing = Trim(rest.substr(AfterBraced(rest)));
                            if (!trailing.empty())
                            {
                                text += ' ';
                                text += trailing;
                            }
                            push_value_block(BlockKind::availability, ParseInline(text));
                        }
                        continue;
                    }
                case DoxyCmd::ref_member_group:
                    {
                        flush_paragraph();
                        const std::string content = ExtractBraced(rest);
                        if (!content.empty())
                        {
                            std::string title = content;
                            const size_t comma = content.find(',');
                            if (comma != std::string::npos)
                            {
                                title = content.substr(comma + 1);
                            }
                            DocBlock block;
                            block.kind = BlockKind::section;
                            block.level = 0;
                            block.spans = ParseInline(Trim(title));
                            PushBlock(blocks, std::move(block));
                        }
                        continue;
                    }
                case DoxyCmd::member_group_name:
                case DoxyCmd::table_of_contents:
                case DoxyCmd::name:
                case DoxyCmd::end_verbatim:
                case DoxyCmd::end_code:
                case DoxyCmd::end_table:
                case DoxyCmd::end_def_list:
                case DoxyCmd::end_style_table:
                case DoxyCmd::end_event_table:
                case DoxyCmd::begin_wx_perl_only:
                case DoxyCmd::end_wx_perl_only:
                case DoxyCmd::begin_wx_python_only:
                case DoxyCmd::end_wx_python_only:
                    {
                        // The Perl/Python-only markers are conditions, not content: drop
                        // the marker and keep the prose and code between them.
                        continue;
                    }
                case DoxyCmd::hide_initializer:
                    // A display directive with no prose of its own: record it so the
                    // enum value row omits the initializer.
                    hide_initializer = true;
                    continue;
                default:
                    {
                        if (IsSkipBlockStart(command))
                        {
                            flush_paragraph();
                            state = State::InSkipBlock;
                            continue;
                        }
                        if (command == DoxyCmd::unknown &&
                            (trimmed.starts_with('@') || trimmed.starts_with('\\')))
                        {
                            // Drop the unknown command token but keep its argument text.
                            const std::string argument = Trim(rest);
                            if (!argument.empty())
                            {
                                append_paragraph(argument);
                            }
                            continue;
                        }
                        break;
                    }
            }

            if (trimmed.starts_with("-#"))
            {
                flush_paragraph();
                std::string_view item_text = std::string_view(trimmed).substr(2);
                while (!item_text.empty() && item_text[0] == ' ')
                {
                    item_text.remove_prefix(1);
                }
                std::vector<InlineSpan> item = ParseInline(item_text);
                ReadContinuation(cleaned, line_index, item);
                if (!item.empty())
                {
                    ensure_list_block(BlockKind::ordered_list);
                    blocks.back().items.push_back(std::move(item));
                }
                continue;
            }

            if (trimmed.empty())
            {
                flush_paragraph();
                continue;
            }

            append_paragraph(trimmed);
        }

        flush_paragraph();

        // A missing @endcode / @endTable / @endDefList must still produce output.
        if (state == State::InCode && !code_content.empty())
        {
            DocBlock block;
            block.kind = BlockKind::code;
            block.language = code_lang;
            block.code = code_content;
            PushBlock(blocks, std::move(block));
        }
        if (state == State::InVerbatim && !code_content.empty())
        {
            DocBlock block;
            block.kind = BlockKind::verbatim;
            block.code = code_content;
            PushBlock(blocks, std::move(block));
        }
        if (state == State::InTable && !table_block.rows.empty())
        {
            PushBlock(blocks, std::move(table_block));
        }
        if (state == State::InDefList && !def_list_block.def_items.empty())
        {
            PushBlock(blocks, std::move(def_list_block));
        }

        // Without an explicit @brief, the first paragraph is the summary.
        if (!have_brief)
        {
            for (DocBlock& block: blocks)
            {
                if (block.kind == BlockKind::paragraph && !block.spans.empty())
                {
                    block.is_brief = true;
                    break;
                }
            }
        }

        result.hide_initializer = hide_initializer;
        result.blocks = std::move(blocks);
        return result;
    }

}  // namespace docparser
