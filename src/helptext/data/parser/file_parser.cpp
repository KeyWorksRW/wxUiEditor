/////////////////////////////////////////////////////////////////////////////
// Purpose:   Parse source files, extract declarations and comments
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "file_parser.h"
#include "comment_parser.h"
#include "decl_parser.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

// CR: [10-27-2026]

namespace docparser
{

    namespace
    {

        // ---------------------------------------------------------------------------
        // Helpers
        // ---------------------------------------------------------------------------

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

        // Whitespace-trimming view over 'text' (no allocation).
        [[nodiscard]] auto TrimView(std::string_view text) -> std::string_view
        {
            size_t start = 0;
            while (start < text.size() && (text[start] == ' ' || text[start] == '\t'))
            {
                ++start;
            }
            size_t end = text.size();
            while (end > start && (text[end - 1] == ' ' || text[end - 1] == '\t'))
            {
                --end;
            }
            return text.substr(start, end - start);
        }

        // Check if a string starts with a prefix (case-sensitive).
        // Returns the prefix length on match, 0 on no match.
        [[nodiscard]] auto StartsWith(std::string_view str, std::string_view prefix) -> size_t
        {
            return str.starts_with(prefix) ? prefix.size() : 0;
        }

        // Split a file into lines
        auto SplitLines(const std::string& text) -> std::vector<std::string>
        {
            std::vector<std::string> lines;
            size_t start = 0;
            while (start < text.size())
            {
                size_t const newline_pos = text.find('\n', start);
                if (newline_pos == std::string::npos)
                {
                    std::string line = text.substr(start);
                    if (!line.empty() && line.back() == '\r')
                    {
                        line.pop_back();
                    }
                    lines.emplace_back(std::move(line));
                    break;
                }
                std::string line = text.substr(start, newline_pos - start);
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }
                lines.push_back(std::move(line));
                start = newline_pos + 1;
            }
            return lines;
        }

        // Read an entire file into 'out_text'. Returns false when the file cannot be
        // opened or the read fails, so an I/O error is never reported as an empty file.
        [[nodiscard]] auto ReadFile(const std::filesystem::path& path, std::string& out_text)
            -> bool
        {
            std::ifstream ifs(path, std::ios::binary);
            if (!ifs)
            {
                return false;
            }
            out_text.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
            return !ifs.bad();
        }

        // Check if a line is a preprocessor directive (#if, #endif, #ifdef, etc.)
        [[nodiscard]] auto IsPreprocessor(std::string_view line) -> bool
        {
            std::string trimmed = Trim(line);
            return !trimmed.empty() && trimmed[0] == '#';
        }

        // Check if a line contains a doxygen comment block start
        [[nodiscard]] auto IsDoxyCommentStart(std::string_view line) -> bool
        {
            const std::string trimmed = Trim(line);
            if (StartsWith(trimmed, "/**") == 0)
            {
                return false;
            }
            if (trimmed.size() == 3)
            {
                return true;  // bare "/**" opens a real multi-line comment
            }
            // "/***" and "/**/" are decorative separators, not doxygen comments.
            return trimmed[3] != '*' && trimmed[3] != '/';
        }

        // Check if a line starts a plain (non-doxygen) block comment: "/*" but not a
        // real doxygen opener, so decorative separators are skipped as plain comments.
        [[nodiscard]] auto IsPlainBlockCommentStart(std::string_view line) -> bool
        {
            const std::string trimmed = Trim(line);
            return StartsWith(trimmed, "/*") > 0 && !IsDoxyCommentStart(trimmed);
        }

        // wxWidgets member-group marker: "///@..." with no space after "///".
        // These are structural markers, not documentation; "/// @..." is a real comment.
        [[nodiscard]] auto IsGroupMarker(std::string_view line) -> bool
        {
            const std::string trimmed = Trim(line);
            return StartsWith(trimmed, "///@") > 0;
        }

        // Check if a line is a triple-slash comment
        [[nodiscard]] auto IsTripleSlash(std::string_view line) -> bool
        {
            const std::string trimmed = Trim(line);
            return StartsWith(trimmed, "///") > 0 && StartsWith(trimmed, "///}") == 0 &&
                   StartsWith(trimmed, "///{") == 0 && StartsWith(trimmed, "///@") == 0;
        }

        // Check if a line starts a grouping block: ///@{
        [[nodiscard]] auto IsGroupStart(std::string_view line) -> bool
        {
            const std::string trimmed = Trim(line);
            return trimmed == "///@{" || trimmed == "/// @{";
        }

        // Check if a line ends a grouping block: ///@}
        [[nodiscard]] auto IsGroupEnd(std::string_view line) -> bool
        {
            const std::string trimmed = Trim(line);
            return trimmed == "///@}" || trimmed == "/// @}";
        }

        // Check if a line is a license/file header comment (plain //, not ///)
        [[nodiscard]] auto IsPlainComment(std::string_view line) -> bool
        {
            const std::string trimmed = Trim(line);
            return StartsWith(trimmed, "//") > 0 && StartsWith(trimmed, "///") == 0;
        }

        // Check if a line is empty or whitespace-only
        [[nodiscard]] auto IsBlank(std::string_view line) -> bool
        {
            return Trim(line).empty();
        }

        // Determine what kind of C++ declaration follows (class, enum, typedef, etc.)
        enum class DeclKind : std::uint8_t
        {
            Unknown,
            Class,
            Enum,
            Typedef,
            Using,
            Define,
            Method,     // function/method declaration
            Const,      // const int x = ...;
            Constexpr,  // constexpr ... = ...;
            Field,      // data member: type name [= value];
        };

        // True when 'text' begins with the given keyword (a word boundary follows).
        [[nodiscard]] auto HasKeyword(std::string_view text, std::string_view keyword) -> bool
        {
            if (!text.starts_with(keyword))
            {
                return false;
            }
            if (text.size() == keyword.size())
            {
                return true;
            }
            const char next = text[keyword.size()];
            return next == ' ' || next == '\t' || next == '<' || next == '{' || next == ':';
        }

        // Skip a balanced bracket/paren/angle group starting at 'pos' (the opening character).
        // Returns the position just past the closing character.
        [[nodiscard]] auto SkipBalanced(std::string_view text, size_t pos) -> size_t
        {
            if (pos >= text.size())
            {
                return text.size();
            }
            const char open = text[pos];
            char close = '\0';
            switch (open)
            {
                case '<':
                    close = '>';
                    break;
                case '(':
                    close = ')';
                    break;
                case '[':
                    close = ']';
                    break;
                default:
                    return pos + 1;
            }

            int depth = 0;
            for (size_t i = pos; i < text.size(); ++i)
            {
                if (text[i] == open)
                {
                    ++depth;
                }
                else if (text[i] == close)
                {
                    --depth;
                    if (depth == 0)
                    {
                        return i + 1;
                    }
                }
            }
            return text.size();
        }

        [[nodiscard]] auto IsExportMacroToken(std::string_view token) -> bool
        {
            return token.starts_with("WXDLLIMPEXP") || token.starts_with("WXLLIMPEXP") ||
                   token.starts_with("WXDLLEXPORT") || token.starts_with("WXEXPORT");
        }

        // Strip prefixes that may precede the declaration keyword or the declared type:
        // template<...>, __declspec(...), wxDEPRECATED_MSG(...), and export macros.
        [[nodiscard]] auto StripDeclPrefixes(std::string_view text) -> std::string_view
        {
            std::string_view rest = TrimView(text);
            bool changed = true;
            while (changed)
            {
                changed = false;
                if (rest.empty())
                {
                    break;
                }

                if (rest.starts_with("template"))
                {
                    // "template <...>" (with whitespace before '<') is as valid as
                    // "template<...>" -- skip the whitespace, then require '<'.
                    size_t angle_pos = 8;
                    while (angle_pos < rest.size() &&
                           (rest[angle_pos] == ' ' || rest[angle_pos] == '\t'))
                    {
                        ++angle_pos;
                    }
                    if (angle_pos < rest.size() && rest[angle_pos] == '<')
                    {
                        rest = TrimView(rest.substr(SkipBalanced(rest, angle_pos)));
                        changed = true;
                        continue;
                    }
                }
                if (rest.starts_with("__declspec("))
                {
                    rest = TrimView(rest.substr(SkipBalanced(rest, 10)));
                    changed = true;
                    continue;
                }
                if (rest.starts_with("wxDEPRECATED_MSG("))
                {
                    rest = TrimView(rest.substr(SkipBalanced(rest, 16)));
                    changed = true;
                    continue;
                }
                if (rest.starts_with("wxDEPRECATED"))
                {
                    rest = TrimView(rest.substr(std::min<size_t>(12, rest.size())));
                    changed = true;
                    continue;
                }

                size_t token_end = 0;
                while (token_end < rest.size() &&
                       !std::isspace(static_cast<unsigned char>(rest[token_end])) &&
                       rest[token_end] != '<' && rest[token_end] != '(')
                {
                    ++token_end;
                }
                if (token_end > 0 && token_end < rest.size() &&
                    IsExportMacroToken(rest.substr(0, token_end)))
                {
                    rest = TrimView(rest.substr(token_end));
                    changed = true;
                    continue;
                }
            }
            return rest;
        }

        // A line that can never be part of a method-declaration gather.
        [[nodiscard]] auto IsDeclarationBarrier(std::string_view line) -> bool
        {
            const std::string_view trimmed = TrimView(line);
            if (trimmed.empty())
            {
                return false;
            }
            if (trimmed[0] == '}' || trimmed[0] == '#')
            {
                return true;
            }
            static constexpr std::array<std::string_view, 6> BARRIER_KEYWORDS = {
                "class", "struct", "enum", "union", "template", "namespace"
            };
            for (const std::string_view keyword: BARRIER_KEYWORDS)
            {
                if (HasKeyword(trimmed, keyword))
                {
                    return true;
                }
            }
            return false;
        }

        // True when a class-like declaration opens a body whose members default to public.
        [[nodiscard]] auto IsStructLikeDecl(std::string_view line) -> bool
        {
            const std::string_view rest = StripDeclPrefixes(line);
            return HasKeyword(rest, "struct") || HasKeyword(rest, "union");
        }

        auto ClassifyDecl(std::string_view line) -> DeclKind
        {
            const std::string_view rest = StripDeclPrefixes(line);
            if (rest.empty())
            {
                return DeclKind::Unknown;
            }

            if (HasKeyword(rest, "class") || HasKeyword(rest, "struct") ||
                HasKeyword(rest, "union"))
            {
                return DeclKind::Class;
            }
            if (HasKeyword(rest, "enum"))
            {
                return DeclKind::Enum;
            }
            if (HasKeyword(rest, "typedef"))
            {
                return DeclKind::Typedef;
            }
            if (HasKeyword(rest, "using"))
            {
                return rest.starts_with("using namespace") ? DeclKind::Unknown : DeclKind::Using;
            }
            if (rest.starts_with("#define ") || rest.starts_with("#define\t"))
            {
                return DeclKind::Define;
            }
            if (rest[0] == '#' || HasKeyword(rest, "namespace"))
            {
                return DeclKind::Unknown;
            }

            // A paren before any '=' is a parameter list; "const int x = Foo(3);" has
            // the paren in its initializer, so it is not a declaration of a method.
            const size_t paren_pos = rest.find('(');
            const size_t equals_pos = rest.find('=');
            bool has_paren = paren_pos != std::string_view::npos &&
                             (equals_pos == std::string_view::npos || paren_pos < equals_pos);
            // An overloaded operator name may itself contain '=' (operator<=,
            // operator==, operator!=). That '=' is part of the name, not an
            // initializer, so the following parenthesis is still a parameter list.
            if (!has_paren && paren_pos != std::string_view::npos)
            {
                const size_t op_pos = rest.find("operator");
                if (op_pos != std::string_view::npos && op_pos < paren_pos)
                {
                    has_paren = true;
                }
            }
            if (HasKeyword(rest, "constexpr") && !has_paren)
            {
                return DeclKind::Constexpr;
            }
            if (HasKeyword(rest, "const") && !has_paren)
            {
                return DeclKind::Const;
            }
            if (has_paren)
            {
                return DeclKind::Method;
            }
            if (rest.find(';') != std::string_view::npos)
            {
                return DeclKind::Field;
            }

            return DeclKind::Unknown;
        }

        // True when the next non-blank line begins a declaration. A @page comment
        // that is followed by a declaration documents that declaration; a @page
        // comment followed by anything else (typically the include guard) is a
        // standalone topic page.
        [[nodiscard]] bool NextNonBlankLineIsDeclaration(const std::vector<std::string>& lines,
                                                         size_t start_line)
        {
            for (size_t index = start_line; index < lines.size(); ++index)
            {
                if (IsBlank(lines[index]))
                {
                    continue;
                }
                return ClassifyDecl(Trim(lines[index])) != DeclKind::Unknown;
            }
            return false;
        }

        // Gather lines for a class body (between { and })
        // Returns the line index after the closing '}'.
        auto GatherClassBody(const std::vector<std::string>& lines, size_t start_line,
                             std::vector<std::string>& body_lines) -> size_t
        {
            // Find opening brace
            size_t i = start_line;
            int brace_depth = 0;
            bool found_open = false;
            bool in_block_comment = false;

            while (i < lines.size())
            {
                const bool was_open = found_open;
                const std::string& line = lines[i];
                // A brace inside a comment or a string literal does not open or close
                // a body, so those regions are skipped while counting.
                const bool is_comment_line = StartsWith(TrimView(line), "//") > 0;
                char string_quote = '\0';
                size_t body_open_pos = std::string::npos;
                for (size_t chr_index = 0; !is_comment_line && chr_index < line.size(); ++chr_index)
                {
                    const char chr = line[chr_index];
                    if (in_block_comment)
                    {
                        if (chr == '*' && chr_index + 1 < line.size() && line[chr_index + 1] == '/')
                        {
                            in_block_comment = false;
                            ++chr_index;
                        }
                        continue;
                    }
                    if (string_quote != '\0')
                    {
                        if (chr == '\\')
                        {
                            ++chr_index;  // skip the escaped character
                        }
                        else if (chr == string_quote)
                        {
                            string_quote = '\0';
                        }
                        continue;
                    }
                    if (chr == '/' && chr_index + 1 < line.size())
                    {
                        const char next = line[chr_index + 1];
                        if (next == '/')
                        {
                            break;  // rest of the line is a comment
                        }
                        if (next == '*')
                        {
                            in_block_comment = true;
                            ++chr_index;
                            continue;
                        }
                    }
                    if (chr == '"' || chr == '\'')
                    {
                        string_quote = chr;
                        continue;
                    }
                    if (chr == '{')
                    {
                        ++brace_depth;
                        found_open = true;
                        if (!was_open && brace_depth == 1)
                        {
                            body_open_pos = chr_index;
                        }
                    }
                    else if (chr == '}')
                    {
                        --brace_depth;
                        if (found_open && brace_depth == 0)
                        {
                            // A body that opens and closes on one line still has
                            // members ("struct Foo { int m_value; };"); keep them.
                            if (body_open_pos != std::string::npos && chr_index > body_open_pos + 1)
                            {
                                body_lines.push_back(
                                    line.substr(body_open_pos + 1, chr_index - body_open_pos - 1));
                            }
                            return i + 1;  // line after closing brace
                        }
                    }
                }
                // Only push lines after the one containing the opening brace
                if (was_open && brace_depth > 0 && !is_comment_line)
                {
                    body_lines.push_back(line);
                }
                ++i;
            }
            return i;
        }

        // Gather lines for an enum body (from first line through closing };)
        auto GatherEnumBody(const std::vector<std::string>& lines, size_t start_line,
                            std::string& enum_text) -> size_t
        {
            size_t i = start_line;
            int brace_depth = 0;
            bool found_open = false;

            while (i < lines.size())
            {
                const std::string_view current = TrimView(lines[i]);

                // A body-less declaration ("enum Foo;") must not gather the rest of the
                // file looking for a brace that never comes.
                if (i == start_line && !current.contains('{') && current.ends_with(';'))
                {
                    return start_line + 1;
                }
                // Before the body opens, a new declaration ends this enum's gather.
                if (i > start_line && !found_open && IsDeclarationBarrier(current))
                {
                    return i;
                }

                if (!enum_text.empty())
                {
                    enum_text += '\n';
                }
                enum_text += lines[i];

                for (const char chr: lines[i])
                {
                    if (chr == '{')
                    {
                        ++brace_depth;
                        found_open = true;
                    }
                    else if (chr == '}')
                    {
                        --brace_depth;
                        if (found_open && brace_depth == 0)
                        {
                            return i + 1;
                        }
                    }
                }
                ++i;
            }
            return i;
        }

        // Accumulate a possibly multi-line typedef/using declaration up to its
        // terminating ';' at brace depth 0. Returns the line index after it.
        auto GatherTypedefDecl(const std::vector<std::string>& lines, size_t start_line,
                               std::string& decl_text) -> size_t
        {
            static constexpr size_t MAX_TYPEDEF_LINES = 40;

            size_t i = start_line;
            int brace_depth = 0;
            bool first_line = true;

            while (i < lines.size() && i - start_line < MAX_TYPEDEF_LINES)
            {
                const std::string_view current = TrimView(lines[i]);
                // A new declaration before the ';' ends this gather.
                if (!first_line && brace_depth <= 0 && IsDeclarationBarrier(current))
                {
                    break;
                }
                if (!decl_text.empty())
                {
                    decl_text += ' ';
                }
                decl_text += current;

                for (const char chr: lines[i])
                {
                    if (chr == '{')
                    {
                        ++brace_depth;
                    }
                    else if (chr == '}')
                    {
                        --brace_depth;
                    }
                }
                ++i;
                first_line = false;
                if (brace_depth <= 0 && current.find(';') != std::string_view::npos)
                {
                    break;
                }
            }
            return i;
        }

        // Extract a multi-line method declaration that may span multiple lines up to ';'.
        // On abort, 'decl_text' is left empty and the returned index is the line to resume at.
        auto GatherMethodDecl(const std::vector<std::string>& lines, size_t start_line,
                              std::string& decl_text) -> size_t
        {
            static constexpr size_t MAX_GATHER_LINES = 40;

            size_t i = start_line;
            int paren_depth = 0;
            int brace_depth = 0;
            bool found_paren = false;
            bool first_line = true;
            size_t consumed = 0;

            while (i < lines.size())
            {
                const std::string trimmed = Trim(lines[i]);

                // Never gather across a structural boundary at brace depth 0.
                if (brace_depth == 0 && IsDeclarationBarrier(trimmed))
                {
                    decl_text.clear();
                    return first_line ? i + 1 : i;
                }

                if (++consumed > MAX_GATHER_LINES)
                {
                    decl_text.clear();
                    return i;
                }

                if (!decl_text.empty())
                {
                    decl_text += ' ';
                }
                decl_text += trimmed;

                if (first_line)
                {
                    const bool opens_parameters = trimmed.ends_with('(') || trimmed.ends_with(',');
                    if (!trimmed.contains('(') && !opens_parameters)
                    {
                        decl_text.clear();
                        return i + 1;
                    }
                    first_line = false;
                }

                // Track paren and brace depth
                for (const char chr: trimmed)
                {
                    if (chr == '(')
                    {
                        ++paren_depth;
                        found_paren = true;
                    }
                    else if (chr == ')')
                    {
                        --paren_depth;
                    }
                    else if (chr == '{')
                    {
                        ++brace_depth;
                    }
                    else if (chr == '}')
                    {
                        --brace_depth;
                    }
                }

                // Done when we hit a ';' at paren_depth 0
                if (found_paren && paren_depth <= 0 && trimmed.find(';') != std::string_view::npos)
                {
                    return i + 1;
                }

                // Also done for single-line declarations with no parens that end with ';'
                if (!found_paren && trimmed.find(';') != std::string_view::npos)
                {
                    return i + 1;
                }

                // An inline body ("void Foo() { DoSomething(); }") ends the declaration
                // even though it carries no trailing ';'.
                if (!trimmed.empty() && trimmed.back() == '}' && brace_depth <= 0)
                {
                    return i + 1;
                }

                ++i;
            }
            return i;
        }

        // Apply a "public:" / "protected:" / "private:" line. Returns true when matched.
        auto TryUpdateAccess(std::string_view line, Access& access) -> bool
        {
            std::string compact;
            compact.reserve(line.size());
            for (const char character: line)
            {
                if (character != ' ' && character != '\t')
                {
                    compact += character;
                }
            }

            if (compact == "public:")
            {
                access = Access::public_;
                return true;
            }
            if (compact == "protected:")
            {
                access = Access::protected_;
                return true;
            }
            if (compact == "private:")
            {
                access = Access::private_;
                return true;
            }
            return false;
        }

        [[nodiscard]] auto IsIdentifier(std::string_view text) -> bool
        {
            if (text.empty())
            {
                return false;
            }
            if (!std::isalpha(static_cast<unsigned char>(text[0])) && text[0] != '_')
            {
                return false;
            }
            for (const char character: text)
            {
                if (!std::isalnum(static_cast<unsigned char>(character)) && character != '_')
                {
                    return false;
                }
            }
            return true;
        }

        // Split on commas that are not nested in <> () [].
        auto SplitTopLevelCommas(std::string_view text) -> std::vector<std::string_view>
        {
            std::vector<std::string_view> parts;
            size_t start = 0;
            int angle_depth = 0;
            int paren_depth = 0;
            int bracket_depth = 0;

            for (size_t i = 0; i < text.size(); ++i)
            {
                const char character = text[i];
                if (character == '<')
                {
                    ++angle_depth;
                }
                else if (character == '>')
                {
                    --angle_depth;
                }
                else if (character == '(')
                {
                    ++paren_depth;
                }
                else if (character == ')')
                {
                    --paren_depth;
                }
                else if (character == '[')
                {
                    ++bracket_depth;
                }
                else if (character == ']')
                {
                    --bracket_depth;
                }
                else if (character == ',' && angle_depth <= 0 && paren_depth == 0 &&
                         bracket_depth == 0)
                {
                    parts.push_back(text.substr(start, i - start));
                    start = i + 1;
                }
            }
            parts.push_back(text.substr(start));
            return parts;
        }

        // Remove a top-level initializer ("= 0" or "= wxEmptyString") from a declarator.
        [[nodiscard]] auto StripInitializer(std::string_view text) -> std::string_view
        {
            int angle_depth = 0;
            int paren_depth = 0;
            for (size_t i = 0; i < text.size(); ++i)
            {
                const char character = text[i];
                if (character == '<')
                {
                    ++angle_depth;
                }
                else if (character == '>')
                {
                    --angle_depth;
                }
                else if (character == '(')
                {
                    ++paren_depth;
                }
                else if (character == ')')
                {
                    --paren_depth;
                }
                else if (character == '=' && angle_depth <= 0 && paren_depth == 0)
                {
                    return TrimView(text.substr(0, i));
                }
            }
            return text;
        }

        // Split a data-member statement ("wxDouble m_11, m_12;") into FieldInfo entries.
        // Statements that are not plain data members produce nothing.
        auto AppendFields(std::string_view statement, Access access, const DocComment* doc_comment,
                          ClassInfo& cls) -> void
        {
            if (access == Access::private_)
            {
                return;
            }

            std::string text = Trim(statement);
            const size_t semi_pos = text.find(';');
            if (semi_pos == std::string::npos)
            {
                return;
            }
            text = Trim(text.substr(0, semi_pos));
            if (text.empty())
            {
                return;
            }
            if (text.find('(') != std::string::npos || text.find('{') != std::string::npos ||
                text.find('}') != std::string::npos || text.find('#') != std::string::npos)
            {
                return;
            }

            // Statements that declare something other than a data member.
            static constexpr std::array<std::string_view, 12> REJECTED_KEYWORDS = {
                "friend", "typedef",   "using",  "class",    "struct",        "union",
                "enum",   "namespace", "return", "template", "static_assert", "operator"
            };
            for (const std::string_view keyword: REJECTED_KEYWORDS)
            {
                if (HasKeyword(text, keyword))
                {
                    return;
                }
            }

            // A trailing bit-field width ("unsigned m_flags : 3") is not part of the name.
            const size_t colon = text.rfind(':');
            if (colon != std::string::npos)
            {
                const std::string_view width = TrimView(std::string_view(text).substr(colon + 1));
                if (!width.empty() &&
                    std::all_of(width.begin(), width.end(),
                                [](char character)
                                {
                                    return std::isdigit(static_cast<unsigned char>(character)) != 0;
                                }))
                {
                    text = Trim(text.substr(0, colon));
                }
            }

            bool is_static = false;
            if (HasKeyword(text, "static"))
            {
                is_static = true;
                text = Trim(std::string_view(text).substr(6));
            }
            if (text.empty())
            {
                return;
            }

            const std::vector<std::string_view> declarators = SplitTopLevelCommas(text);

            // The first declarator carries the type: "Type name". A trailing array
            // dimension ("unsigned m_data[4]") is not part of the name.
            std::string_view first = StripInitializer(declarators.front());
            const size_t bracket_pos = first.rfind('[');
            if (bracket_pos != std::string_view::npos && first.ends_with(']'))
            {
                first = TrimView(first.substr(0, bracket_pos));
            }
            size_t name_end = first.size();
            while (name_end > 0 && (first[name_end - 1] == ' ' || first[name_end - 1] == '\t'))
            {
                --name_end;
            }
            size_t name_start = name_end;
            while (name_start > 0 &&
                   (std::isalnum(static_cast<unsigned char>(first[name_start - 1])) ||
                    first[name_start - 1] == '_'))
            {
                --name_start;
            }
            if (name_start == name_end)
            {
                return;
            }

            const std::string type = Trim(first.substr(0, name_start));
            const std::string first_name =
                std::string(first.substr(name_start, name_end - name_start));
            if (type.empty() || !IsIdentifier(first_name))
            {
                return;
            }

            const DocComment empty_doc;
            const DocComment& field_doc = doc_comment != nullptr ? *doc_comment : empty_doc;

            FieldInfo field;
            field.type = type;
            field.name = first_name;
            field.is_static = is_static;
            field.access = access;
            field.doc = field_doc;
            cls.fields.push_back(std::move(field));

            for (size_t idx = 1; idx < declarators.size(); ++idx)
            {
                const std::string_view declarator = TrimView(StripInitializer(declarators[idx]));
                const size_t extra_end = declarator.size();
                size_t extra_start = extra_end;
                while (extra_start > 0 &&
                       (std::isalnum(static_cast<unsigned char>(declarator[extra_start - 1])) ||
                        declarator[extra_start - 1] == '_'))
                {
                    --extra_start;
                }
                const std::string extra_name =
                    std::string(declarator.substr(extra_start, extra_end - extra_start));
                if (!IsIdentifier(extra_name))
                {
                    continue;
                }

                FieldInfo extra;
                extra.type = type;
                extra.name = extra_name;
                extra.is_static = is_static;
                extra.access = access;
                extra.doc = field_doc;
                cls.fields.push_back(std::move(extra));
            }
        }

        // Parse class body lines into methods, fields, enums, typedefs
        // 'initial_access' is private_ for a class body and public_ for a struct body.
        void ParseClassBody(const std::vector<std::string>& body_lines, ClassInfo& cls,
                            Access initial_access)
        {
            DocComment pending_doc;
            bool have_doc = false;
            Access current_access = initial_access;
            bool in_plain_block_comment = false;

            for (size_t i = 0; i < body_lines.size(); ++i)
            {
                std::string_view current = body_lines[i];
                if (in_plain_block_comment)
                {
                    const size_t close_pos = current.find("*/");
                    if (close_pos == std::string_view::npos)
                    {
                        continue;
                    }
                    in_plain_block_comment = false;
                    current = current.substr(close_pos + 2);
                }

                std::string trimmed = Trim(current);
                if (trimmed.empty())
                {
                    continue;
                }

                // A plain /* ... */ comment is not a declaration: skip to its end and
                // keep processing whatever follows on the same line.
                if (IsPlainBlockCommentStart(trimmed))
                {
                    const size_t close_pos = trimmed.find("*/", 2);
                    if (close_pos == std::string::npos)
                    {
                        in_plain_block_comment = true;
                        continue;
                    }
                    trimmed = Trim(trimmed.substr(close_pos + 2));
                    if (trimmed.empty())
                    {
                        continue;
                    }
                }

                // wx member-group markers (///@{, ///@}, ///@word{...})
                if (IsGroupMarker(trimmed))
                {
                    continue;
                }

                // Plain // comments are not documentation and never declare anything.
                if (IsPlainComment(trimmed) && !IsTripleSlash(trimmed))
                {
                    continue;
                }

                // Access specifiers control which members are emitted.
                if (TryUpdateAccess(trimmed, current_access))
                {
                    continue;
                }

                // Skip preprocessor guards inside class
                if (IsPreprocessor(trimmed))
                {
                    continue;
                }

                // Doxygen comment block
                if (IsDoxyCommentStart(trimmed))
                {
                    std::string comment_block;
                    // Find the end of the comment block. Finding "*/" is the single-line
                    // test: "/**/" and "/***/" close here instead of consuming the body.
                    const size_t open_pos = trimmed.find("/**");
                    const size_t content_start =
                        (open_pos == std::string::npos) ? trimmed.size() : open_pos + 3;
                    const size_t close_pos = (open_pos == std::string::npos) ?
                                                 std::string::npos :
                                                 trimmed.find("*/", content_start);
                    std::string trailing;
                    if (close_pos != std::string::npos)
                    {
                        // Single-line: /** text */
                        comment_block = (close_pos >= content_start) ?
                                            std::string(trimmed.substr(content_start,
                                                                       close_pos - content_start)) :
                                            std::string();
                        // Code sharing the line with the comment declares the member; it
                        // must not be dropped.
                        trailing = Trim(trimmed.substr(close_pos + 2));
                    }
                    else
                    {
                        // Multi-line: collect until */
                        comment_block = std::string(trimmed.substr(content_start));
                        ++i;
                        while (i < body_lines.size())
                        {
                            const std::string& line = body_lines[i];
                            const size_t end_pos = line.find("*/");
                            if (end_pos != std::string::npos)
                            {
                                comment_block += '\n';
                                comment_block += line.substr(0, end_pos);
                                break;
                            }
                            comment_block += '\n';
                            comment_block += line;
                            ++i;
                        }
                    }
                    pending_doc = ParseCommentBlock(comment_block);
                    have_doc = true;
                    if (trailing.empty())
                    {
                        continue;
                    }
                    // Re-process the tail as the current line instead of dropping it.
                    trimmed = trailing;
                }

                // Triple-slash comments (accumulate)
                if (IsTripleSlash(trimmed))
                {
                    // 'trimmed' may be the tail left by a preceding "/* ... */" comment,
                    // in which case the gather below runs zero iterations. The
                    // non-advancing path is removed: seed from 'trimmed' and step forward.
                    const size_t start_i = i;
                    std::string comment_block;
                    while (i < body_lines.size() && IsTripleSlash(Trim(body_lines[i])))
                    {
                        const std::string line = Trim(body_lines[i]);
                        std::string text = line.substr(3);  // skip "///"
                        if (!text.empty() && text[0] == ' ')
                        {
                            text = text.substr(1);
                        }
                        if (!comment_block.empty())
                        {
                            comment_block += '\n';
                        }
                        comment_block += text;
                        ++i;
                    }

                    if (i == start_i)
                    {
                        // No line starts with "///": 'trimmed' is the remainder left by a
                        // stripped "/* ... */" prefix. Seed the block from it and let the
                        // enclosing for-loop advance exactly one line. Decrementing here
                        // would re-enter this state (the old hang); incrementing would skip
                        // the declaration that follows the comment.
                        std::string text = trimmed.substr(3);
                        if (!text.empty() && text[0] == ' ')
                        {
                            text = text.substr(1);
                        }
                        // "///<" and "///!" mark the comment as a member doc; the marker
                        // itself is punctuation, not documentation.
                        if (!text.empty() && (text[0] == '<' || text[0] == '!'))
                        {
                            text = text.substr(1);
                            if (!text.empty() && text[0] == ' ')
                            {
                                text = text.substr(1);
                            }
                        }
                        comment_block = text;
                    }
                    else
                    {
                        --i;  // back up so outer loop re-examines current line
                    }

                    pending_doc = ParseCommentBlock(comment_block);
                    have_doc = true;
                    continue;
                }

                // Now classify the declaration
                const DeclKind kind = ClassifyDecl(trimmed);

                // A nested class/struct is a compound of its own: gather its header
                // and body, parse them recursively, and record it under
                // "Outer::Inner" so the writer can emit a separate page.
                if (kind == DeclKind::Class)
                {
                    std::string header_text = trimmed;
                    size_t header_line = i;

                    if (header_text.find('{') == std::string::npos &&
                        header_text.find(';') != std::string::npos)
                    {
                        // Forward declaration — no body, so no nested page.
                        have_doc = false;
                        continue;
                    }

                    if (header_text.find('{') == std::string::npos)
                    {
                        size_t probe = i + 1;
                        bool found_brace = false;
                        while (probe < body_lines.size() && probe - i <= 40)
                        {
                            const std::string next = Trim(body_lines[probe]);
                            if (IsDeclarationBarrier(next))
                            {
                                break;
                            }
                            header_text += ' ';
                            header_text += next;
                            if (next.find('{') != std::string::npos)
                            {
                                found_brace = true;
                                header_line = probe;
                                ++probe;
                                break;
                            }
                            if (next.find(';') != std::string::npos)
                            {
                                break;
                            }
                            ++probe;
                        }

                        if (!found_brace)
                        {
                            have_doc = false;
                            continue;
                        }
                    }

                    ClassInfo nested = ParseClassDecl(header_text);
                    if (have_doc && !pending_doc.class_name.empty())
                    {
                        // Honour an explicit qualified @class Outer::Inner.
                        nested.name = pending_doc.class_name;
                    }
                    if (nested.name.empty())
                    {
                        // Skip the body, or its members would be re-parsed as this
                        // class's own members.
                        std::vector<std::string> skipped_body;
                        const size_t skip_after =
                            GatherClassBody(body_lines, header_line, skipped_body);
                        i = std::max(skip_after, i + 1) - 1;
                        have_doc = false;
                        continue;
                    }

                    if (!nested.name.contains("::"))
                    {
                        nested.name = cls.name + "::" + nested.name;
                    }

                    if (have_doc)
                    {
                        nested.doc = std::move(pending_doc);
                        have_doc = false;
                    }

                    std::vector<std::string> nested_body;
                    const size_t after = GatherClassBody(body_lines, header_line, nested_body);
                    const Access nested_access =
                        IsStructLikeDecl(header_text) ? Access::public_ : Access::private_;
                    ParseClassBody(nested_body, nested, nested_access);
                    cls.nested.push_back(std::move(nested));
                    i = after - 1;  // -1 because loop increments
                    continue;
                }

                if (kind == DeclKind::Enum)
                {
                    std::string enum_text;
                    i = GatherEnumBody(body_lines, i, enum_text) - 1;  // -1 because loop increments
                    EnumInfo enum_info = ParseEnumDecl(enum_text);
                    if (have_doc)
                    {
                        enum_info.doc = std::move(pending_doc);
                        have_doc = false;
                    }
                    if (!enum_info.name.empty() || !enum_info.values.empty())
                    {
                        // An anonymous enum is kept so its documented values are not lost.
                        cls.enums.push_back(std::move(enum_info));
                    }
                    continue;
                }

                if (kind == DeclKind::Typedef || kind == DeclKind::Using)
                {
                    TypedefInfo type_def = ParseTypedef(trimmed);
                    if (have_doc)
                    {
                        type_def.doc = std::move(pending_doc);
                        have_doc = false;
                    }
                    if (!type_def.name.empty())
                    {
                        cls.typedefs.push_back(std::move(type_def));
                    }
                    continue;
                }

                if (kind == DeclKind::Field || kind == DeclKind::Const ||
                    kind == DeclKind::Constexpr)
                {
                    AppendFields(trimmed, current_access, have_doc ? &pending_doc : nullptr, cls);
                    have_doc = false;
                    continue;
                }

                if (kind == DeclKind::Method || kind == DeclKind::Unknown)
                {
                    // Gather multi-line method declaration
                    std::string decl_text;
                    size_t const new_i = GatherMethodDecl(body_lines, i, decl_text);
                    i = new_i - 1;  // -1 because loop increments

                    if (decl_text.empty())
                    {
                        // A bare "template <...>" line belongs to the declaration that
                        // follows it, so it must not discard the documentation.
                        if (trimmed.starts_with("template"))
                        {
                            continue;
                        }
                        // The gather was rejected; the line may still be a data member.
                        AppendFields(trimmed, current_access, have_doc ? &pending_doc : nullptr,
                                     cls);
                        have_doc = false;
                        continue;
                    }

                    if (!decl_text.contains('('))
                    {
                        // No parameter list: a data member, not a method.
                        AppendFields(decl_text, current_access, have_doc ? &pending_doc : nullptr,
                                     cls);
                        have_doc = false;
                        continue;
                    }

                    MethodInfo method = ParseMethodDecl(decl_text);
                    if (method.name.empty())
                    {
                        have_doc = false;  // discard orphaned doc
                        continue;
                    }

                    method.access = current_access;
                    if (current_access == Access::private_)
                    {
                        have_doc = false;  // private members are not documented
                        continue;
                    }

                    if (have_doc)
                    {
                        method.doc = std::move(pending_doc);
                        have_doc = false;
                    }
                    cls.methods.push_back(std::move(method));
                    continue;
                }

                // Default: skip and discard pending doc
                have_doc = false;
            }
        }

        // True when 'name' matches a class page at any nesting level.
        [[nodiscard]] bool HasClassPageName(const std::vector<ClassInfo>& classes,
                                            const std::string& name)
        {
            for (const ClassInfo& class_info: classes)
            {
                if (class_info.name == name || HasClassPageName(class_info.nested, name))
                {
                    return true;
                }
            }
            return false;
        }

        // Return the identifier after the last class/struct/union keyword, or an
        // empty string when no keyword is present.
        [[nodiscard]] std::string LastDeclKeywordName(std::string_view text)
        {
            static constexpr std::array<std::string_view, 3> KEYWORDS { "class", "struct",
                                                                        "union" };
            size_t keyword_pos = std::string_view::npos;
            size_t keyword_len = 0;
            for (const std::string_view keyword: KEYWORDS)
            {
                size_t found = text.rfind(keyword);
                // A keyword that is part of a longer identifier ("wxMyStructure") does
                // not name a declaration: step back to the previous occurrence.
                while (found != std::string_view::npos && found > 0 &&
                       (std::isalnum(static_cast<unsigned char>(text[found - 1])) ||
                        text[found - 1] == '_'))
                {
                    found = text.rfind(keyword, found - 1);
                }
                if (found != std::string_view::npos &&
                    (keyword_pos == std::string_view::npos || found > keyword_pos))
                {
                    keyword_pos = found;
                    keyword_len = keyword.size();
                }
            }
            if (keyword_pos == std::string_view::npos)
            {
                return {};
            }

            size_t name_start = keyword_pos + keyword_len;
            while (name_start < text.size() &&
                   std::isspace(static_cast<unsigned char>(text[name_start])))
            {
                ++name_start;
            }
            size_t name_end = name_start;
            while (
                name_end < text.size() &&
                (std::isalnum(static_cast<unsigned char>(text[name_end])) || text[name_end] == '_'))
            {
                ++name_end;
            }
            return std::string(text.substr(name_start, name_end - name_start));
        }

    }  // anonymous namespace

    // ---------------------------------------------------------------------------
    // ParseFile
    // ---------------------------------------------------------------------------

    auto ParseFile(const std::filesystem::path& path, const std::filesystem::path& rel_path)
        -> FileContent
    {
        FileContent result;
        result.filename = rel_path.generic_string();

        std::string content;
        if (!ReadFile(path, content) || content.empty())
        {
            // The file could not be read; there is nothing to report for it.
            return result;
        }

        std::vector<std::string> lines = SplitLines(content);

        // State
        DocComment pending_doc;
        bool have_doc = false;
        std::string current_group;  // from @addtogroup
        bool in_plain_block_comment = false;

        // @class docs are held until a declaration claims them by name. Attaching
        // them by position would document the following enum, typedef, or class.
        std::vector<std::pair<std::string, DocComment>> held_class_docs;

        auto hold_class_doc = [&held_class_docs](DocComment&& doc_comment) -> void
        {
            // A template class documents itself as '@class wxSafeArray<varType>';
            // its page is named for the base identifier.
            std::string held_name = doc_comment.class_name;
            if (const size_t angle = held_name.find('<'); angle != std::string::npos)
            {
                held_name.resize(angle);
            }

            for (auto& entry: held_class_docs)
            {
                if (entry.first == held_name)
                {
                    entry.second = std::move(doc_comment);
                    return;
                }
            }
            held_class_docs.emplace_back(std::move(held_name), std::move(doc_comment));
        };

        auto take_held_class_doc = [&held_class_docs, &pending_doc,
                                    &have_doc](const std::string& name) -> bool
        {
            for (std::vector<std::pair<std::string, DocComment>>::iterator entry =
                     held_class_docs.begin();
                 entry != held_class_docs.end(); ++entry)
            {
                if (entry->first == name)
                {
                    pending_doc = std::move(entry->second);
                    have_doc = true;
                    held_class_docs.erase(entry);
                    return true;
                }
            }
            return false;
        };

        // Move a parsed @page comment into the page list when the following
        // non-blank line does not begin a declaration.
        auto collect_page_comment = [&result, &pending_doc,
                                     &have_doc](size_t next_line,
                                                const std::vector<std::string>& lines) -> void
        {
            if (pending_doc.page_id.empty())
            {
                return;
            }
            if (NextNonBlankLineIsDeclaration(lines, next_line))
            {
                return;
            }
            result.pages.push_back(
                { pending_doc.page_id, pending_doc.page_title, std::move(pending_doc) });
            pending_doc = DocComment {};
            have_doc = false;
        };

        // Skip license header: lines starting with // before first blank or doxygen comment
        size_t i = 0;
        while (i < lines.size())
        {
            if (IsBlank(lines[i]))
            {
                ++i;
                continue;  // skip blank lines in header area
            }
            std::string trimmed = Trim(lines[i]);
            if (IsPlainComment(trimmed) && !IsTripleSlash(trimmed))
            {
                ++i;
                continue;  // skip plain // comments
            }
            break;  // found non-comment content
        }

        // Main parsing loop
        while (i < lines.size())
        {
            std::string_view current = lines[i];
            if (in_plain_block_comment)
            {
                const size_t close_pos = current.find("*/");
                if (close_pos == std::string_view::npos)
                {
                    ++i;
                    continue;
                }
                in_plain_block_comment = false;
                current = current.substr(close_pos + 2);
                if (TrimView(current).empty())
                {
                    ++i;
                    continue;
                }
            }

            std::string trimmed = Trim(current);

            // Skip blank lines
            if (trimmed.empty())
            {
                ++i;
                continue;
            }

            // A plain /* ... */ comment is not a declaration: skip it, then continue
            // with whatever follows the closing */ on the same line.
            if (IsPlainBlockCommentStart(trimmed))
            {
                const size_t close_pos = trimmed.find("*/", 2);
                if (close_pos == std::string::npos)
                {
                    in_plain_block_comment = true;
                    ++i;
                    continue;
                }
                trimmed = Trim(trimmed.substr(close_pos + 2));
                if (trimmed.empty())
                {
                    ++i;
                    continue;
                }
            }

            // Skip preprocessor directives (pass through)
            if (IsPreprocessor(trimmed))
            {
                // Check for #define at file level
                if (StartsWith(trimmed, "#define ") || StartsWith(trimmed, "#define\t"))
                {
                    DefineInfo def = ParseDefine(trimmed);
                    if (!def.name.empty())
                    {
                        if (have_doc)
                        {
                            def.doc = std::move(pending_doc);
                            have_doc = false;
                        }
                        result.defines.push_back(std::move(def));
                    }

                    // A macro continued with a trailing backslash owns the following
                    // lines; they must never be mistaken for declarations.
                    while (trimmed.ends_with('\\') && i + 1 < lines.size())
                    {
                        ++i;
                        trimmed = Trim(lines[i]);
                    }
                }
                ++i;
                continue;
            }

            // Skip plain // comments (non-doxygen)
            if (IsPlainComment(trimmed) && !IsTripleSlash(trimmed))
            {
                ++i;
                continue;
            }

            // Group start/end. "///@{" and "///@}" also match IsGroupMarker, so they
            // must be tested first or they never clear 'current_group'.
            if (IsGroupStart(trimmed))
            {
                ++i;
                continue;
            }
            if (IsGroupEnd(trimmed))
            {
                current_group.clear();
                ++i;
                continue;
            }
            // Any remaining "///@" marker is structural and carries no documentation.
            if (IsGroupMarker(trimmed))
            {
                ++i;
                continue;
            }

            // Namespace blocks and their closing brace carry no file-level documentation.
            if (HasKeyword(TrimView(trimmed), "namespace") || trimmed[0] == '}')
            {
                ++i;
                continue;
            }

            // @addtogroup detection in a comment
            // Pattern: "/** @addtogroup group_name */" or  "/** @addtogroup group_name \n ... */"
            // Also: "/// @addtogroup group_name"

            // Doxygen comment block
            if (IsDoxyCommentStart(trimmed))
            {
                std::string comment_block;
                std::string trailing;  // code sharing the line with a single-line comment

                const size_t open_pos = trimmed.find("/**");
                const size_t content_start =
                    (open_pos == std::string::npos) ? trimmed.size() : open_pos + 3;
                const size_t close_pos = (open_pos == std::string::npos) ?
                                             std::string::npos :
                                             trimmed.find("*/", content_start);

                if (close_pos != std::string::npos)
                {
                    // Single-line: /** text */. Finding "*/" is the test, so "/**/" and
                    // "/***/" also close here instead of swallowing the rest of the file.
                    comment_block = (close_pos >= content_start) ?
                                        trimmed.substr(content_start, close_pos - content_start) :
                                        std::string();
                    trailing = Trim(trimmed.substr(close_pos + 2));
                }
                else
                {
                    // Multi-line
                    comment_block = trimmed.substr(content_start);
                    ++i;
                    while (i < lines.size())
                    {
                        const std::string& line = lines[i];
                        size_t end_pos = line.find("*/");
                        if (end_pos != std::string::npos)
                        {
                            comment_block += '\n';
                            comment_block += line.substr(0, end_pos);
                            break;
                        }
                        comment_block += '\n';
                        comment_block += line;
                        ++i;
                    }
                }

                pending_doc = ParseCommentBlock(comment_block);

                // Check if this comment sets a group
                if (!pending_doc.group_name.empty())
                {
                    current_group = pending_doc.group_name;
                }

                // One comment can carry both '@page' and '@class'. Copy what the page
                // needs before the class doc is moved out, or the page is lost.
                std::string page_id;
                std::string page_title;
                DocComment page_doc;
                if (!pending_doc.page_id.empty() && !pending_doc.class_name.empty())
                {
                    page_id = pending_doc.page_id;
                    page_title = pending_doc.page_title;
                    page_doc = pending_doc;
                }

                if (pending_doc.class_name.empty())
                {
                    have_doc = true;
                }
                else
                {
                    hold_class_doc(std::move(pending_doc));
                    pending_doc = DocComment {};
                    have_doc = false;
                    if (!page_id.empty() && !NextNonBlankLineIsDeclaration(lines, i + 1))
                    {
                        result.pages.push_back({ page_id, page_title, std::move(page_doc) });
                    }
                }

                // Code after a single-line comment is not part of the comment: make it
                // the current line again instead of dropping it.
                if (!trailing.empty())
                {
                    lines[i] = trailing;
                    collect_page_comment(i, lines);
                    continue;
                }

                collect_page_comment(i + 1, lines);

                ++i;
                continue;
            }

            // Triple-slash comment accumulation
            if (IsTripleSlash(trimmed))
            {
                // 'trimmed' may be the remainder after a stripped "/* ... */" prefix, in
                // which case the gather consumes nothing. Consume 'trimmed' and advance
                // so the loop provably moves forward on every path.
                const size_t before = i;
                std::string comment_block;
                while (i < lines.size() && IsTripleSlash(Trim(lines[i])))
                {
                    std::string line = Trim(lines[i]);
                    std::string_view text = std::string_view(line).substr(3);
                    if (!text.empty() && text[0] == ' ')
                    {
                        text = text.substr(1);
                    }
                    if (!comment_block.empty())
                    {
                        comment_block += '\n';
                    }
                    comment_block += text;
                    ++i;
                }

                if (i == before)
                {
                    std::string_view text = std::string_view(trimmed).substr(3);
                    if (!text.empty() && text[0] == ' ')
                    {
                        text = text.substr(1);
                    }
                    comment_block = std::string(text);
                    ++i;
                }

                pending_doc = ParseCommentBlock(comment_block);

                if (!pending_doc.group_name.empty())
                {
                    current_group = pending_doc.group_name;
                }

                // One comment can carry both '@page' and '@class'; keep a copy of the
                // page data so moving the class doc out does not lose the page.
                std::string page_id;
                std::string page_title;
                DocComment page_doc;
                if (!pending_doc.page_id.empty() && !pending_doc.class_name.empty())
                {
                    page_id = pending_doc.page_id;
                    page_title = pending_doc.page_title;
                    page_doc = pending_doc;
                }

                if (pending_doc.class_name.empty())
                {
                    have_doc = true;
                }
                else
                {
                    hold_class_doc(std::move(pending_doc));
                    pending_doc = DocComment {};
                    have_doc = false;
                    if (!page_id.empty() && !NextNonBlankLineIsDeclaration(lines, i))
                    {
                        result.pages.push_back({ page_id, page_title, std::move(page_doc) });
                    }
                }

                collect_page_comment(i, lines);

                continue;  // don't increment i — we're already past the comments
            }

            // Now handle declarations following a doc comment
            const DeclKind kind = ClassifyDecl(trimmed);

            if (kind == DeclKind::Class)
            {
                // A class header may put its '{' on a later line; accumulate up to it.
                std::string header_text = trimmed;
                size_t header_line = i;

                if (header_text.find('{') == std::string::npos &&
                    header_text.find(';') != std::string::npos)
                {
                    // A documented forward declaration still owns a page: Doxygen
                    // emits one, and the documentation has nowhere else to live.
                    ClassInfo declaration = ParseClassDecl(header_text);
                    if (declaration.name.empty())
                    {
                        declaration.name = LastDeclKeywordName(header_text);
                    }
                    if (declaration.name.empty())
                    {
                        declaration.name = pending_doc.class_name;
                    }

                    if (!have_doc)
                    {
                        std::ignore = take_held_class_doc(declaration.name);
                    }
                    if (have_doc)
                    {
                        declaration.doc = std::move(pending_doc);
                        have_doc = false;
                    }

                    if (!declaration.name.empty() && !declaration.doc.blocks.empty())
                    {
                        result.classes.push_back(std::move(declaration));
                    }
                    ++i;
                    continue;
                }

                if (header_text.find('{') == std::string::npos)
                {
                    size_t probe = i + 1;
                    bool found_brace = false;
                    while (probe < lines.size() && probe - i <= 40)
                    {
                        const std::string next = Trim(lines[probe]);
                        if (IsDeclarationBarrier(next))
                        {
                            break;
                        }
                        header_text += ' ';
                        header_text += next;
                        if (next.find('{') != std::string::npos)
                        {
                            found_brace = true;
                            header_line = probe;
                            ++probe;
                            break;
                        }
                        if (next.find(';') != std::string::npos)
                        {
                            break;
                        }
                        ++probe;
                    }

                    if (!found_brace)
                    {
                        // Forward declaration or unparseable header — nothing to emit.
                        have_doc = false;
                        i = probe > i ? probe : i + 1;
                        continue;
                    }
                }

                ClassInfo cls = ParseClassDecl(header_text);

                if (have_doc)
                {
                    cls.doc = std::move(pending_doc);
                    have_doc = false;
                }
                else
                {
                    // A preceding @class doc was held for the class its name
                    // identifies. Attach it only on an exact name match: some wx
                    // headers carry a stale @class value (e.g. htmlcell.h names
                    // wxHtmlSelection with @class wxHtmlRenderingStyle), and
                    // attaching by position would misdocument the page.
                    std::ignore = take_held_class_doc(cls.name);
                    if (have_doc)
                    {
                        cls.doc = std::move(pending_doc);
                        have_doc = false;
                    }
                }

                if (cls.name.empty())
                {
                    // Skip the body, or its members are re-parsed as file-level content.
                    std::vector<std::string> skipped_body;
                    i = std::max(GatherClassBody(lines, header_line, skipped_body), i + 1);
                    continue;
                }

                // Gather and parse class body
                std::vector<std::string> body_lines;
                const size_t after = GatherClassBody(lines, header_line, body_lines);

                const Access initial_access =
                    IsStructLikeDecl(header_text) ? Access::public_ : Access::private_;
                ParseClassBody(body_lines, cls, initial_access);

                result.classes.push_back(std::move(cls));
                i = after;
                continue;
            }

            if (kind == DeclKind::Enum)
            {
                std::string enum_text;
                const size_t after = GatherEnumBody(lines, i, enum_text);
                EnumInfo enum_info = ParseEnumDecl(enum_text);
                if (have_doc)
                {
                    enum_info.doc = std::move(pending_doc);
                    have_doc = false;
                }
                if (!enum_info.name.empty() || !enum_info.values.empty())
                {
                    // An anonymous enum is kept so its documented values are not lost.
                    result.enums.push_back(std::move(enum_info));
                }
                i = after;
                continue;
            }

            if (kind == DeclKind::Typedef || kind == DeclKind::Using)
            {
                // A typedef or using alias may span several lines up to its ';'.
                std::string typedef_text;
                i = GatherTypedefDecl(lines, i, typedef_text);
                TypedefInfo type_def = ParseTypedef(typedef_text);
                if (have_doc)
                {
                    type_def.doc = std::move(pending_doc);
                    have_doc = false;
                }
                result.typedefs.push_back(std::move(type_def));
                continue;
            }

            if (kind == DeclKind::Field)
            {
                // A file-level data member declares nothing this converter emits.
                have_doc = false;
                ++i;
                continue;
            }

            if (kind == DeclKind::Const || kind == DeclKind::Constexpr)
            {
                // A file-scope constant such as "const int wxInvalidOffset = -1;" or
                // "constexpr wxUint32 wxPG_COLOUR_WEB_BASE = 0x10000;" is documented
                // under "## Constants", not as a #define.
                std::string rest = Trim(std::string_view(trimmed));
                if (!rest.empty() && rest.back() == ';')
                {
                    rest.pop_back();
                }
                const size_t eq_pos = rest.find('=');
                ConstantInfo constant;
                constant.declaration =
                    eq_pos == std::string::npos ? Trim(rest) : Trim(rest.substr(0, eq_pos));
                if (eq_pos != std::string::npos)
                {
                    constant.value = Trim(rest.substr(eq_pos + 1));
                }

                // Name is the trailing identifier of the declaration, ignoring a
                // trailing array subscript ("const char wxFoo[] = ..." -> wxFoo).
                std::string_view head = TrimView(constant.declaration);
                if (head.ends_with(']'))
                {
                    const size_t bracket = head.rfind('[');
                    if (bracket != std::string_view::npos)
                    {
                        head = TrimView(head.substr(0, bracket));
                    }
                }
                size_t name_start = head.size();
                while (name_start > 0 &&
                       (std::isalnum(static_cast<unsigned char>(head[name_start - 1])) ||
                        head[name_start - 1] == '_'))
                {
                    --name_start;
                }
                constant.name = std::string(head.substr(name_start));
                if (!constant.name.empty())
                {
                    if (have_doc)
                    {
                        constant.doc = std::move(pending_doc);
                        have_doc = false;
                    }
                    result.constants.push_back(std::move(constant));
                }
                else
                {
                    // Nothing was declared, so do not leak the doc onto the next one.
                    pending_doc = DocComment {};
                    have_doc = false;
                }
                ++i;
                continue;
            }

            if (kind == DeclKind::Method || kind == DeclKind::Unknown)
            {
                // Structural lines (namespace braces, preprocessor, template headers) are
                // not free-function declarations and must not start a gather.
                if (kind == DeclKind::Unknown && IsDeclarationBarrier(trimmed))
                {
                    // A bare 'template <...>' line belongs to the declaration that follows
                    // it, so it must not discard the documentation that precedes the pair.
                    if (!StartsWith(trimmed, "template"))
                    {
                        have_doc = false;
                    }
                    ++i;
                    continue;
                }

                // Free function at file level (in a group context or standalone)
                std::string decl_text;
                const size_t after = GatherMethodDecl(lines, i, decl_text);

                // Check if this looks like a function (has parens)
                if (decl_text.find('(') != std::string::npos)
                {
                    MethodInfo func = ParseMethodDecl(decl_text);
                    if (!func.name.empty())
                    {
                        if (have_doc)
                        {
                            func.doc = std::move(pending_doc);
                            have_doc = false;
                        }
                        func.group_name = current_group;
                        result.free_functions.push_back(std::move(func));
                    }
                }

                i = after;
                have_doc = false;
                continue;
            }

            // Unrecognized line — skip
            have_doc = false;
            ++i;
        }

        // Flush a trailing @page block: overview headers contain only the page
        // comment and no declaration for it to attach to.
        collect_page_comment(lines.size(), lines);

        // A @class doc still held names no class page in this header. Emit it as a
        // doc-only alias page so its documentation is not lost.
        for (auto& [held_name, held_doc]: held_class_docs)
        {
            if (held_name.empty() || held_doc.blocks.empty() ||
                HasClassPageName(result.classes, held_name))
            {
                continue;
            }

            ClassInfo alias_page;
            alias_page.name = held_name;
            alias_page.doc = std::move(held_doc);
            result.alias_pages.push_back(std::move(alias_page));
        }

        return result;
    }

}  // namespace docparser
