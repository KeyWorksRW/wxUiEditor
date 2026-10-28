/////////////////////////////////////////////////////////////////////////////
// Purpose:   Parse C++ class, method, enum, and typedef decls
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "decl_parser.h"
#include "comment_parser.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <vector>

// CR: [10-27-2026]

namespace docparser
{

    namespace
    {

        // ---------------------------------------------------------------------------
        // Helpers
        // ---------------------------------------------------------------------------

        // True for every character Trim treats as whitespace. CR and LF are included so
        // CRLF text and wrapped declarations trim the same as single-line text.
        [[nodiscard]] auto IsTrimSpace(char chr) -> bool
        {
            return chr == ' ' || chr == '\t' || chr == '\r' || chr == '\n';
        }

        // Skip whitespace at position pos, return new position
        [[nodiscard]] auto SkipWS(std::string_view text, size_t pos) -> size_t
        {
            while (pos < text.size() && IsTrimSpace(text[pos]))
            {
                ++pos;
            }
            return pos;
        }

        [[nodiscard]] auto Trim(std::string_view text) -> std::string
        {
            size_t start = SkipWS(text, 0);
            size_t endPos = text.size();
            while (endPos > start && IsTrimSpace(text[endPos - 1]))
            {
                --endPos;
            }
            return std::string(text.substr(start, endPos - start));
        }

        // Split text into lines
        [[nodiscard]] auto SplitLines(std::string_view text) -> std::vector<std::string>
        {
            std::vector<std::string> lines;
            size_t start = 0;
            while (start < text.size())
            {
                const size_t new_line = text.find('\n', start);
                if (new_line == std::string_view::npos)
                {
                    lines.emplace_back(text.substr(start));
                    break;
                }
                std::string_view line = text.substr(start, new_line - start);
                if (!line.empty() && line.back() == '\r')
                {
                    line.remove_suffix(1);
                }
                lines.emplace_back(line);
                start = new_line + 1;
            }
            return lines;
        }

        // Check if a string starts with a prefix (case-sensitive).
        // Returns the prefix length on match, 0 on no match.
        [[nodiscard]] auto StartsWith(std::string_view text, std::string_view prefix) -> size_t
        {
            return text.starts_with(prefix) ? prefix.size() : 0;
        }

        // Defined below; declared here so SkipBalanced can delegate to the single
        // nesting-aware implementation.
        auto FindMatchingClose(std::string_view text, size_t pos) -> size_t;

        // Skip a balanced bracket/paren/angle group starting at 'pos', which must point at
        // the opening character. Returns the position just past the closing character.
        [[nodiscard]] auto SkipBalanced(std::string_view text, size_t pos) -> size_t
        {
            if (pos >= text.size())
            {
                return text.size();
            }

            const char open = text[pos];
            if (open != '(' && open != '<' && open != '[' && open != '{')
            {
                return pos + 1;
            }

            const size_t close_pos = FindMatchingClose(text, pos);
            if (close_pos == std::string_view::npos)
            {
                return text.size();
            }
            return close_pos + 1;
        }

        [[nodiscard]] auto IsExportMacroToken(std::string_view token) -> bool
        {
            return token.starts_with("WXDLLIMPEXP") || token.starts_with("WXLLIMPEXP") ||
                   token.starts_with("WXDLLEXPORT") || token.starts_with("WXEXPORT");
        }

        // Remove declaration prefixes that may precede a type or a class keyword:
        // template<...>, __declspec(...), wxDEPRECATED_MSG(...), and export macros.
        [[nodiscard]] auto StripDeclPrefix(std::string_view text) -> std::string
        {
            std::string_view rest = text;
            bool changed = true;
            while (changed)
            {
                changed = false;
                rest = rest.substr(SkipWS(rest, 0));
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
                        rest = rest.substr(SkipBalanced(rest, angle_pos));
                        changed = true;
                        continue;
                    }
                }
                if (rest.starts_with("__declspec("))
                {
                    rest = rest.substr(SkipBalanced(rest, 10));
                    changed = true;
                    continue;
                }
                if (rest.starts_with("wxDEPRECATED_MSG("))
                {
                    rest = rest.substr(SkipBalanced(rest, 16));
                    changed = true;
                    continue;
                }
                if (rest.starts_with("wxDEPRECATED"))
                {
                    rest = rest.substr(std::min<size_t>(12, rest.size()));
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
                    rest = rest.substr(token_end);
                    changed = true;
                    continue;
                }
            }
            return Trim(rest);
        }

        // Collapse "operator !=(...)" to "operator!=(...)" while keeping a single space
        // before word-like operator names ("operator void *", "operator new").
        [[nodiscard]] auto NormalizeOperatorName(std::string_view name) -> std::string
        {
            const size_t op_pos = name.find("operator");
            if (op_pos == std::string_view::npos)
            {
                return std::string(name);
            }

            std::string result = Trim(name.substr(0, op_pos));
            std::string_view rest = name.substr(op_pos + 8);
            rest = rest.substr(SkipWS(rest, 0));

            if (!result.empty())
            {
                result += ' ';
            }
            result += "operator";
            if (rest.empty())
            {
                return result;
            }
            if (std::isalnum(static_cast<unsigned char>(rest[0])) || rest[0] == '_')
            {
                result += ' ';
            }
            result += rest;
            return Trim(result);
        }

        // Find matching closing bracket/paren/angle, handling nesting.
        // pos should point to the opening character.
        // Returns position of closing character, or npos.
        [[nodiscard]] auto FindMatchingClose(std::string_view text, size_t pos) -> size_t
        {
            if (pos >= text.size())
            {
                return std::string_view::npos;
            }

            const char open = text[pos];
            char close = '\0';
            switch (open)
            {
                case '(':
                    close = ')';
                    break;
                case '<':
                    close = '>';
                    break;
                case '{':
                    close = '}';
                    break;
                case '[':
                    close = ']';
                    break;
                default:
                    return std::string_view::npos;
            }

            int depth = 1;
            for (size_t i = pos + 1; i < text.size(); ++i)
            {
                // A bracket inside a comment is text, not nesting.
                if (text[i] == '/' && i + 1 < text.size() && text[i + 1] == '/')
                {
                    const size_t line_end = text.find('\n', i);
                    if (line_end == std::string_view::npos)
                    {
                        break;
                    }
                    i = line_end;
                    continue;
                }
                if (text[i] == '/' && i + 1 < text.size() && text[i + 1] == '*')
                {
                    const size_t block_end = text.find("*/", i + 2);
                    if (block_end == std::string_view::npos)
                    {
                        break;
                    }
                    i = block_end + 1;
                    continue;
                }
                if (text[i] == open)
                {
                    ++depth;
                }
                else if (text[i] == close)
                {
                    --depth;
                    if (depth == 0)
                    {
                        return i;
                    }
                }
                else if (text[i] == '"')
                {
                    // Skip a string literal, but only when it closes on the same line.
                    // An apostrophe inside a prose comment (e.g. "wxImage's palette")
                    // must not swallow the rest of the declaration.
                    const size_t line_end = text.find('\n', i);
                    const size_t limit =
                        line_end == std::string_view::npos ? text.size() : line_end;
                    size_t scan = i + 1;
                    while (scan < limit && text[scan] != '"')
                    {
                        if (text[scan] == '\\')
                        {
                            ++scan;
                        }
                        ++scan;
                    }
                    if (scan < limit)
                    {
                        i = scan;
                    }
                }
                else if (text[i] == '\'')
                {
                    // Skip a char literal only when it closes on the same line.
                    const size_t line_end = text.find('\n', i);
                    const size_t limit =
                        line_end == std::string_view::npos ? text.size() : line_end;
                    size_t scan = i + 1;
                    while (scan < limit && text[scan] != '\'')
                    {
                        if (text[scan] == '\\')
                        {
                            ++scan;
                        }
                        ++scan;
                    }
                    if (scan < limit)
                    {
                        i = scan;
                    }
                }
            }
            return std::string_view::npos;
        }

        // Attempt to split a parameter string (between parentheses) into individual
        // parameters, respecting nested parens and angle brackets and skipping string
        // literals, char literals, and comments.
        // LIFETIME: the returned views point into `params_text` and do NOT own their text.
        // Callers must keep the source string alive for as long as they use the views.
        [[nodiscard]] auto SplitParams(std::string_view params_text)
            -> std::vector<std::string_view>
        {
            std::vector<std::string_view> result;
            if (params_text.empty())
            {
                return result;
            }

            size_t start = 0;
            int paren_depth = 0;
            int angle_depth = 0;

            for (size_t i = 0; i < params_text.size(); ++i)
            {
                const char chr = params_text[i];

                // Comments and literals can hold commas and unmatched angle brackets.
                if (chr == '/' && i + 1 < params_text.size() && params_text[i + 1] == '/')
                {
                    const size_t line_end = params_text.find('\n', i);
                    if (line_end == std::string_view::npos)
                    {
                        break;
                    }
                    i = line_end;
                    continue;
                }
                if (chr == '/' && i + 1 < params_text.size() && params_text[i + 1] == '*')
                {
                    const size_t end_pos = params_text.find("*/", i + 2);
                    if (end_pos == std::string_view::npos)
                    {
                        break;
                    }
                    i = end_pos + 1;
                    continue;
                }
                if (chr == '"' || chr == '\'')
                {
                    const size_t line_end = params_text.find('\n', i);
                    const size_t limit =
                        line_end == std::string_view::npos ? params_text.size() : line_end;
                    size_t scan = i + 1;
                    while (scan < limit && params_text[scan] != chr)
                    {
                        if (params_text[scan] == '\\')
                        {
                            ++scan;
                        }
                        ++scan;
                    }
                    if (scan < limit)
                    {
                        i = scan;
                    }
                    continue;
                }

                if (chr == '(')
                {
                    ++paren_depth;
                }
                else if (chr == ')')
                {
                    // A stray ')' must not make the depth negative, which would stop every
                    // later comma from splitting.
                    if (paren_depth > 0)
                    {
                        --paren_depth;
                    }
                }
                else if (chr == '<')
                {
                    ++angle_depth;
                }
                else if (chr == '>')
                {
                    // Likewise a stray '>' (e.g. from "->").
                    if (angle_depth > 0)
                    {
                        --angle_depth;
                    }
                }
                else if (chr == ',' && paren_depth == 0 && angle_depth == 0)
                {
                    result.push_back(params_text.substr(start, i - start));
                    // A split only happens at depth zero, which resets the counters for the
                    // next parameter.
                    start = i + 1;
                }
            }
            result.push_back(params_text.substr(start));
            return result;
        }

        // Parse a single parameter "Type name = default" into a ParamInfo.
        [[nodiscard]] auto ParseSingleParam(std::string_view text) -> ParamInfo
        {
            ParamInfo param;
            std::string trimmed = Trim(text);
            if (trimmed.empty())
            {
                return param;
            }

            // Check for default value: find '=' not inside angle brackets or parens
            size_t eq_pos = std::string_view::npos;
            int paren_depth = 0;
            int angle_depth = 0;
            for (size_t i = 0; i < trimmed.size(); ++i)
            {
                const char chr = trimmed[i];
                if (chr == '(')
                {
                    ++paren_depth;
                }
                else if (chr == ')')
                {
                    --paren_depth;
                }
                else if (chr == '<')
                {
                    ++angle_depth;
                }
                else if (chr == '>')
                {
                    --angle_depth;
                }
                else if (chr == '=' && paren_depth == 0 && angle_depth == 0)
                {
                    eq_pos = i;
                    break;
                }
            }

            std::string type_and_name;
            if (eq_pos != std::string_view::npos)
            {
                type_and_name = Trim(trimmed.substr(0, eq_pos));
                param.default_value = Trim(trimmed.substr(eq_pos + 1));
            }
            else
            {
                type_and_name = std::string(trimmed);
            }

            // Split type and name: name is the last word-like token
            // Handle cases like "const wxString& label", "wxWindow* parent",
            // "long style", "const wxValidator& validator"
            // Work backwards from end to find name
            if (type_and_name.empty())
            {
                return param;
            }

            // Handle function pointer params: "void (*func)(int)" - rare in interface, just use
            // whole thing
            if (type_and_name.contains("(*"))
            {
                param.type = type_and_name;
                return param;
            }

            // Find the last identifier token
            size_t name_end = type_and_name.size();
            while (name_end > 0 &&
                   (type_and_name[name_end - 1] == ' ' || type_and_name[name_end - 1] == '\t'))
            {
                --name_end;
            }

            // Handle trailing array brackets
            if (name_end > 0 && type_and_name[name_end - 1] == ']')
            {
                size_t const bracket_start = type_and_name.rfind('[');
                if (bracket_start != std::string::npos)
                {
                    name_end = bracket_start;
                    while (name_end > 0 && type_and_name[name_end - 1] == ' ')
                    {
                        --name_end;
                    }
                }
            }

            size_t name_start = name_end;
            while (name_start > 0 &&
                   (std::isalnum(static_cast<unsigned char>(type_and_name[name_start - 1])) ||
                    type_and_name[name_start - 1] == '_'))
            {
                --name_start;
            }

            if (name_start == name_end)
            {
                // Couldn't find name, whole thing is type (e.g., "void")
                param.type = type_and_name;
                return param;
            }

            // A lone builtin keyword is a type, not a name: "SetValue(int)" and
            // "unsigned long" have no parameter name. Only treat the last token as a name
            // when a type prefix is left over and the token is not a builtin type keyword.
            static constexpr std::array<std::string_view, 10> BUILTIN_TYPE_KEYWORDS = {
                "int",      "long",   "short", "char",   "bool",
                "unsigned", "signed", "float", "double", "void"
            };
            const std::string_view last_token =
                std::string_view(type_and_name).substr(name_start, name_end - name_start);
            const std::string_view type_prefix =
                std::string_view(type_and_name).substr(0, name_start);
            if (Trim(type_prefix).empty() ||
                std::ranges::find(BUILTIN_TYPE_KEYWORDS, last_token) != BUILTIN_TYPE_KEYWORDS.end())
            {
                param.type = type_and_name;
                return param;
            }

            param.name = type_and_name.substr(name_start, name_end - name_start);
            param.type = Trim(std::string_view(type_and_name.data(), name_start));

            // Clean up type: remove trailing & or * that got separated
            // Actually they should already be part of the type portion

            return param;
        }

        // True when a (possibly multi-line) initializer ends with a binary operator,
        // meaning the next source line continues the same value.
        [[nodiscard]] auto EndsWithOperator(std::string_view text) -> bool
        {
            const std::string trimmed = Trim(text);
            if (trimmed.empty())
            {
                return false;
            }
            switch (trimmed.back())
            {
                case '|':
                case '&':
                case '+':
                case '-':
                case '*':
                case '/':
                case '%':
                case '^':
                case '~':
                case '(':
                    return true;
                default:
                    return false;
            }
        }

        // Number of unclosed '(' / '[' / '{' in 'text', ignoring comments and string/char
        // literals. Used to keep a multi-line enum initializer in one value.
        [[nodiscard]] auto CountOpenNesting(std::string_view text) -> int
        {
            int nesting = 0;
            for (size_t i = 0; i < text.size(); ++i)
            {
                const char chr = text[i];
                if (chr == '/' && i + 1 < text.size() && text[i + 1] == '/')
                {
                    break;
                }
                if (chr == '/' && i + 1 < text.size() && text[i + 1] == '*')
                {
                    const size_t end_pos = text.find("*/", i + 2);
                    if (end_pos == std::string_view::npos)
                    {
                        break;
                    }
                    i = end_pos + 1;
                    continue;
                }
                if (chr == '"' || chr == '\'')
                {
                    size_t scan = i + 1;
                    while (scan < text.size() && text[scan] != chr)
                    {
                        if (text[scan] == '\\')
                        {
                            ++scan;
                        }
                        ++scan;
                    }
                    i = scan;
                    continue;
                }
                if (chr == '(' || chr == '[' || chr == '{')
                {
                    ++nesting;
                }
                else if ((chr == ')' || chr == ']' || chr == '}') && nesting > 0)
                {
                    --nesting;
                }
            }
            return nesting;
        }

        // Split a comma-separated enum value list at top-level commas, ignoring commas
        // inside parens/brackets/braces, comments, and string/char literals.
        [[nodiscard]] auto SplitEnumValues(std::string_view text) -> std::vector<std::string>
        {
            std::vector<std::string> result;
            size_t start = 0;
            int nesting = 0;
            for (size_t i = 0; i < text.size(); ++i)
            {
                const char chr = text[i];
                if (chr == '/' && i + 1 < text.size() && text[i + 1] == '/')
                {
                    break;
                }
                if (chr == '/' && i + 1 < text.size() && text[i + 1] == '*')
                {
                    const size_t end_pos = text.find("*/", i + 2);
                    if (end_pos == std::string_view::npos)
                    {
                        break;
                    }
                    i = end_pos + 1;
                    continue;
                }
                if (chr == '"' || chr == '\'')
                {
                    size_t scan = i + 1;
                    while (scan < text.size() && text[scan] != chr)
                    {
                        if (text[scan] == '\\')
                        {
                            ++scan;
                        }
                        ++scan;
                    }
                    i = scan;
                    continue;
                }
                if (chr == '(' || chr == '[' || chr == '{')
                {
                    ++nesting;
                }
                else if (chr == ')' || chr == ']' || chr == '}')
                {
                    if (nesting > 0)
                    {
                        --nesting;
                    }
                }
                else if (chr == ',' && nesting == 0)
                {
                    result.push_back(Trim(text.substr(start, i - start)));
                    start = i + 1;
                }
            }
            result.push_back(Trim(text.substr(start)));
            return result;
        }

        // Doxygen trailing markers (/**<, /**!) are punctuation, not doc text.
        [[nodiscard]] auto StripDocMarker(std::string text) -> std::string
        {
            std::string result = Trim(text);
            while (!result.empty() && (result[0] == '<' || result[0] == '!'))
            {
                const std::string rest = Trim(std::string_view(result).substr(1));
                result = rest;
            }
            return result;
        }

    }  // anonymous namespace

    // ---------------------------------------------------------------------------
    // ParseClassDecl
    // ---------------------------------------------------------------------------

    [[nodiscard]] auto ParseClassDecl(std::string_view text) -> ClassInfo
    {
        ClassInfo info;
        std::string trimmed = Trim(text);

        // Remove trailing '{', ';', etc.
        while (!trimmed.empty() &&
               (trimmed.back() == '{' || trimmed.back() == ';' || IsTrimSpace(trimmed.back())))
        {
            trimmed.pop_back();
        }

        // Strip a leading template<...>/export-macro prefix, then the
        // class/struct/union keyword.
        std::string rest_text = StripDeclPrefix(trimmed);
        size_t keyword_len = StartsWith(rest_text, "class ");
        if (keyword_len == 0)
        {
            keyword_len = StartsWith(rest_text, "struct ");
        }
        if (keyword_len == 0)
        {
            keyword_len = StartsWith(rest_text, "union ");
        }
        if (keyword_len == 0)
        {
            return info;
        }
        rest_text = Trim(std::string_view(rest_text).substr(keyword_len));

        // Strip export macros that follow the keyword, e.g.
        // "class WXDLLIMPEXP_CORE wxButton : public wxAnyButton".
        const std::string name_part = StripDeclPrefix(rest_text);

        // Find ':' for inheritance
        if (const size_t colon = name_part.find(':'); colon != std::string_view::npos)
        {
            info.name = Trim(name_part.substr(0, colon));

            // Parse bases: "public Base1, virtual public Base2". SplitParams survives the
            // commas inside template arguments; its views point into bases_str, which
            // outlives this loop.
            const std::string bases_str = Trim(name_part.substr(colon + 1));
            for (const std::string_view base_part: SplitParams(bases_str))
            {
                std::string base = Trim(base_part);
                // Remove access specifiers and 'virtual' in any order.
                bool stripped = true;
                while (stripped)
                {
                    stripped = false;
                    size_t len = StartsWith(base, "public ");
                    if (len == 0)
                    {
                        len = StartsWith(base, "protected ");
                    }
                    if (len == 0)
                    {
                        len = StartsWith(base, "private ");
                    }
                    if (len == 0)
                    {
                        len = StartsWith(base, "virtual ");
                    }
                    if (len > 0)
                    {
                        base = Trim(std::string_view(base).substr(len));
                        stripped = true;
                    }
                }
                if (!base.empty())
                {
                    info.bases.push_back(base);
                }
            }
        }
        else
        {
            info.name = Trim(name_part);
        }

        // Keep only the first identifier of the name: drop template arguments
        // ("wxList<T>" -> "wxList") and trailing specifiers such as "final".
        if (const size_t angle = info.name.find('<'); angle != std::string::npos)
        {
            info.name.resize(angle);
        }
        if (const size_t space = info.name.find_first_of(" \t"); space != std::string::npos)
        {
            info.name.resize(space);
        }
        info.name = Trim(info.name);

        return info;
    }

    // ---------------------------------------------------------------------------
    // ParseMethodDecl
    // ---------------------------------------------------------------------------

    [[nodiscard]] auto ParseMethodDecl(std::string_view text) -> MethodInfo
    {
        MethodInfo info;
        std::string trimmed = Trim(text);

        // Remove trailing semicolon
        while (!trimmed.empty() && (trimmed.back() == ';' || IsTrimSpace(trimmed.back())))
        {
            trimmed.pop_back();
        }

        // Strip a leading template<...>/export-macro prefix: the parens of a macro such as
        // "wxDEPRECATED_MSG(\"...\") void Foo();" would otherwise be taken as the
        // parameter list.
        trimmed = StripDeclPrefix(trimmed);

        // Check for trailing qualifiers after the closing paren
        // Find the parameter list first
        size_t open_paren = std::string_view::npos;
        // An "operator" token moves where the parameter list starts: the operator's own
        // '<'/'>' must not drive the angle tracking, and the empty "()" of "operator()" is
        // part of the name rather than the parameter list.
        const size_t op_pos = trimmed.find("operator");
        const bool has_operator_keyword =
            op_pos != std::string::npos &&
            (op_pos + 8 == trimmed.size() ||
             !(std::isalnum(static_cast<unsigned char>(trimmed[op_pos + 8])) ||
               trimmed[op_pos + 8] == '_'));
        if (has_operator_keyword)
        {
            static constexpr std::string_view OPERATOR_SYMBOLS = "=<>!+-*/%^&|~[],";
            const size_t after_keyword = SkipWS(trimmed, op_pos + 8);
            size_t scan = after_keyword;
            // Consume the operator symbol run (e.g. "==", "<<=", "[]").
            while (scan < trimmed.size() &&
                   OPERATOR_SYMBOLS.find(trimmed[scan]) != std::string_view::npos)
            {
                ++scan;
            }
            // Word-like names: "operator new", "operator delete", "operator void *".
            while (
                scan < trimmed.size() &&
                (std::isalnum(static_cast<unsigned char>(trimmed[scan])) || trimmed[scan] == '_'))
            {
                ++scan;
            }
            scan = SkipWS(trimmed, scan);
            // "operator()": the empty paren pair belongs to the name, so the parameter list
            // is the next paren group.
            if (scan == after_keyword && scan + 1 < trimmed.size() && trimmed[scan] == '(' &&
                trimmed[scan + 1] == ')')
            {
                scan = SkipWS(trimmed, scan + 2);
            }
            // Take the following '(' unconditionally.
            if (const size_t paren = trimmed.find('(', scan); paren != std::string::npos)
            {
                open_paren = paren;
            }
        }
        else
        {
            // Find the FIRST '(' that is at the right nesting level
            int angle_depth = 0;
            for (size_t i = 0; i < trimmed.size(); ++i)
            {
                if (trimmed[i] == '<')
                {
                    ++angle_depth;
                }
                else if (trimmed[i] == '>')
                {
                    --angle_depth;
                }
                else if (trimmed[i] == '(' && angle_depth == 0)
                {
                    open_paren = i;
                    break;
                }
            }
        }

        if (open_paren == std::string_view::npos)
        {
            // No parens — might be a variable or something odd. Only a bare identifier
            // is a usable name; anything else is rejected so the caller discards it.
            std::string name_only = std::string(trimmed);
            if (name_only.contains(' ') || name_only.contains('.') || name_only.contains(';') ||
                name_only.contains(':') || name_only.contains(',') || name_only.contains('#'))
            {
                return info;
            }
            info.name = std::move(name_only);
            return info;
        }

        size_t close_paren = FindMatchingClose(trimmed, open_paren);
        if (close_paren == std::string_view::npos)
        {
            close_paren = trimmed.size() - 1;  // best effort
        }

        // Parse trailing qualifiers after close paren
        std::string trailing = Trim(trimmed.substr(close_paren + 1));
        // May contain: const, override, virtual (trailing), = 0, = delete, = default
        // Check for = 0 / = delete / = default
        if (size_t eq_pos = trailing.find('='); eq_pos != std::string::npos)
        {
            std::string after_eq = Trim(std::string_view(trailing).substr(eq_pos + 1));
            if (after_eq == "0" || after_eq == "0;")
            {
                info.is_pure = true;
                info.is_virtual = true;
            }
            else if (StartsWith(after_eq, "delete"))
            {
                info.is_deleted = true;
            }
            else if (StartsWith(after_eq, "default"))
            {
                info.is_default = true;
            }
        }
        if (trailing.contains("const"))
        {
            info.is_const = true;
        }
        if (trailing.contains("override"))
        {
            info.is_override = true;
        }

        // Parse the prefix (before open paren) for: [virtual] [static] ReturnType Name
        std::string prefix = Trim(StripDeclPrefix(Trim(trimmed.substr(0, open_paren))));

        // Strip leading specifiers from the prefix
        struct PrefixSpec
        {
            std::string_view keyword;
            bool MethodInfo::* flag;  // optional member flag to set
        };

        static constexpr std::array prefix_specs = {
            PrefixSpec { .keyword = "virtual ", .flag = &MethodInfo::is_virtual },
            PrefixSpec { .keyword = "static ", .flag = &MethodInfo::is_static },
            PrefixSpec { .keyword = "explicit ", .flag = nullptr },
            PrefixSpec { .keyword = "constexpr ", .flag = nullptr },
            PrefixSpec { .keyword = "inline ", .flag = nullptr },
        };

        bool spec_matched = true;
        while (spec_matched)
        {
            spec_matched = false;
            for (const auto& [keyword, flag]: prefix_specs)
            {
                if (auto len = StartsWith(prefix, keyword); len > 0)
                {
                    if (flag)
                    {
                        info.*flag = true;
                    }
                    prefix = Trim(std::string_view(prefix).substr(len));
                    spec_matched = true;
                }
            }
        }

        // Now we have "ReturnType Name" or just "Name" (constructor/destructor)
        // Find the name: it's the last identifier before the paren
        // This needs to handle:
        //   "bool Create" -> return="bool", name="Create"
        //   "wxButton" -> name="wxButton" (constructor)
        //   "~wxButton" -> name="~wxButton" (destructor)
        //   "const wxString& GetLabel" -> return="const wxString&", name="GetLabel"
        //   "wxWindow* SetDefault" -> return="wxWindow*", name="SetDefault"
        //   "operator==" -> name="operator=="

        // Handle operator overloads
        if (const size_t op_pos = prefix.find("operator"); op_pos != std::string::npos)
        {
            // Everything from "operator" onwards is the name; normalize the spacing
            // so "operator !=" becomes "operator!=" and "operator []" becomes "operator[]".
            info.name = NormalizeOperatorName(std::string_view(prefix).substr(op_pos));
            if (op_pos > 0)
            {
                info.return_type = Trim(std::string_view(prefix).substr(0, op_pos));
            }
        }
        else
        {
            // Find last identifier: scan backwards
            size_t name_end = prefix.size();
            size_t name_start = name_end;
            while (name_start > 0)
            {
                const char chr = prefix[name_start - 1];
                if (std::isalnum(static_cast<unsigned char>(chr)) || chr == '_' || chr == '~')
                {
                    --name_start;
                }
                else
                {
                    break;
                }
            }
            if (name_start < name_end)
            {
                info.name = prefix.substr(name_start, name_end - name_start);
                if (name_start > 0)
                {
                    info.return_type = Trim(std::string_view(prefix).substr(0, name_start));
                }
            }
            else
            {
                info.name = prefix;
            }
        }

        // Reject bogus names: a name with an embedded space that is not an operator
        // overload, or one containing punctuation that cannot belong to an identifier.
        // This is what keeps "wxDouble m_11, m_12;" from becoming a method.
        // Operator names are exempt: "operator()", "operator[]" and "operator<="
        // legitimately contain punctuation that would fail these identifier tests.
        if (info.name.starts_with("operator"))
        {
            // A valid operator overload name is kept as-is.
        }
        else if (info.name.contains(' '))
        {
            info.name.clear();
        }
        else if (info.name.contains('.') || info.name.contains(';') || info.name.contains(':') ||
                 info.name.contains('(') || info.name.contains(')'))
        {
            info.name.clear();
        }

        // Parse parameters
        std::string params_text = trimmed.substr(open_paren + 1, close_paren - open_paren - 1);
        std::vector<std::string_view> param_parts = SplitParams(params_text);
        for (auto& part: param_parts)
        {
            std::string trimmed_p = Trim(part);
            if (trimmed_p.empty() || trimmed_p == "void")
            {
                continue;
            }
            info.params.push_back(ParseSingleParam(trimmed_p));
        }

        return info;
    }

    // ---------------------------------------------------------------------------
    // ParseEnumDecl
    // ---------------------------------------------------------------------------

    [[nodiscard]] auto ParseEnumDecl(std::string_view text) -> EnumInfo
    {
        EnumInfo info;
        std::vector<std::string> lines = SplitLines(text);
        if (lines.empty())
        {
            return info;
        }

        // First line should be "enum [class] Name" or "enum [class] Name {"
        std::string first_line = Trim(lines[0]);

        // Strip "enum " prefix. An anonymous enum (just "enum") has no name.
        if (first_line == "enum")
        {
            first_line.clear();
        }
        else if (const size_t len = StartsWith(first_line, "enum "); len > 0)
        {
            first_line = first_line.substr(len);
        }
        else
        {
            return info;
        }

        first_line = Trim(first_line);

        // Check for "class" or "struct" (scoped enum)
        size_t scoped_len = StartsWith(first_line, "class ");
        if (scoped_len == 0)
        {
            scoped_len = StartsWith(first_line, "struct ");
        }
        if (scoped_len > 0)
        {
            info.is_scoped = true;
            first_line = Trim(std::string_view(first_line).substr(scoped_len));
        }

        // Extract name (stop at '{', ':', or whitespace)
        size_t name_end = 0;
        while (name_end < first_line.size() && first_line[name_end] != '{' &&
               first_line[name_end] != ';' && first_line[name_end] != ':' &&
               first_line[name_end] != ' ' && first_line[name_end] != '\t')
        {
            ++name_end;
        }
        info.name = first_line.substr(0, name_end);

        // Find the opening '{' and closing '}'
        // Concatenate all lines to find brace content
        std::string full_text;
        for (const auto& line: lines)
        {
            if (!full_text.empty())
            {
                full_text += '\n';
            }
            full_text += line;
        }

        size_t brace_open = full_text.find('{');
        if (brace_open == std::string::npos)
        {
            return info;
        }

        size_t brace_close = FindMatchingClose(full_text, brace_open);
        if (brace_close == std::string::npos || brace_close <= brace_open)
        {
            return info;
        }

        std::string body = full_text.substr(brace_open + 1, brace_close - brace_open - 1);

        // Parse enum values from the body, line by line
        std::vector<std::string> body_lines = SplitLines(body);

        // Accumulate a pending doc comment for the next value
        DocComment pending_doc;
        bool have_pending_block_doc = false;
        std::string pending_block_text;
        bool in_block_comment = false;
        bool in_skip_block = false;    // for non-doxygen /* */ block comments
        bool value_continues = false;  // previous initializer ends with an operator
        int value_nesting = 0;         // unmatched '(' / '[' / '{' in the current value

        for (const auto& raw_line: body_lines)
        {
            std::string line = Trim(raw_line);
            if (line.empty())
            {
                continue;
            }

            // Skip non-doxygen block comments until their closing */
            if (in_skip_block)
            {
                if (line.contains("*/"))
                {
                    in_skip_block = false;
                }
                continue;
            }

            // Handle block comment /** ... */
            if (in_block_comment)
            {
                if (size_t end_pos = line.find("*/"); end_pos != std::string::npos)
                {
                    // End of block comment
                    pending_block_text += " " + Trim(std::string_view(line).substr(0, end_pos));
                    // Strip leading * from each line in the block
                    have_pending_block_doc = true;
                    in_block_comment = false;
                }
                else
                {
                    // Strip leading * if present
                    std::string stripped = Trim(line);
                    if (!stripped.empty() && stripped[0] == '*')
                    {
                        stripped = stripped.substr(1);
                        stripped = Trim(stripped);
                    }
                    if (!pending_block_text.empty())
                    {
                        pending_block_text += ' ';
                    }
                    pending_block_text += stripped;
                }
                continue;
            }

            // Start of block comment
            if (size_t len = StartsWith(line, "/**"); len > 0)
            {
                const std::string rest = line.substr(len);
                const size_t end_pos = rest.find("*/");
                if (end_pos == std::string::npos)
                {
                    pending_block_text = StripDocMarker(rest);
                    in_block_comment = true;
                    continue;
                }

                // Single-line block comment: /** text */. Whatever follows the closing
                // "*/" is the value (or a further comment) on the same line, so keep
                // processing that remainder instead of dropping it.
                pending_block_text = StripDocMarker(rest.substr(0, end_pos));
                have_pending_block_doc = true;
                line = Trim(std::string_view(rest).substr(end_pos + 2));
                if (line.empty())
                {
                    continue;
                }
            }

            // Skip C-style comments that aren't doxygen: /* ... */
            if (StartsWith(line, "/*"))
            {
                if (line.contains("*/"))
                {
                    continue;  // single-line non-doxygen comment
                }
                // Multi-line non-doxygen comment — skip until */
                in_skip_block = true;
                continue;
            }

            // Skip plain // comments (not ///)
            if (StartsWith(line, "//") && !StartsWith(line, "///"))
            {
                continue;
            }

            // Lines with enum values
            // Could be: "wxFOO = 0x1234," or "wxFOO," or "wxFOO = wxBAR | wxBAZ,"
            // May have trailing /// comment

            // Check for a trailing /// or //!< comment
            std::string trailing_comment;
            std::string value_part;
            size_t marker_pos = line.find("///");
            if (marker_pos == std::string::npos)
            {
                marker_pos = line.find("//!");
            }
            if (marker_pos != std::string::npos)
            {
                value_part = Trim(std::string_view(line).substr(0, marker_pos));
                trailing_comment = Trim(std::string_view(line).substr(marker_pos + 3));

                // Doxygen marks a trailing comment with '<' (///<, //!<); the marker is
                // punctuation, not content. A leading '!' can also leak from //!<.
                while (!trailing_comment.empty() &&
                       (trailing_comment[0] == '<' || trailing_comment[0] == '!'))
                {
                    trailing_comment = Trim(std::string_view(trailing_comment).substr(1));
                }

                // A line holding nothing but the doc comment documents the next value.
                if (marker_pos == 0)
                {
                    if (!trailing_comment.empty())
                    {
                        if (!pending_block_text.empty())
                        {
                            pending_block_text += ' ';
                        }
                        pending_block_text += trailing_comment;
                        have_pending_block_doc = true;
                    }
                    continue;
                }
            }
            else
            {
                value_part = std::string(line);
            }

            // A plain // comment after a value is not part of the initializer.
            if (const size_t plain_comment = value_part.find("//");
                plain_comment != std::string::npos)
            {
                value_part = Trim(std::string_view(value_part).substr(0, plain_comment));
            }

            // Likewise a trailing block comment: "wxFOO, /* deprecated */".
            if (const size_t block_comment = value_part.find("/*");
                block_comment != std::string::npos)
            {
                value_part = Trim(std::string_view(value_part).substr(0, block_comment));
            }

            if (value_part.empty())
            {
                continue;
            }

            // A continuation of a multi-line initializer (a flag combination or a call
            // split over several lines) belongs to the previous value. Commas inside the
            // continuation are part of the value, so it is not split here.
            if ((value_continues || value_nesting > 0) && !info.values.empty())
            {
                std::string& initializer = info.values.back().initializer;
                if (!initializer.empty())
                {
                    initializer += ' ';
                }
                initializer += value_part;
                value_nesting = CountOpenNesting(initializer);
                value_continues = (value_nesting == 0) && EndsWithOperator(initializer);
                continue;
            }

            // Remove trailing comma
            if (value_part.back() == ',')
            {
                value_part.pop_back();
                value_part = Trim(value_part);
            }

            if (value_part.empty())
            {
                continue;
            }

            // Skip if it's just a comment or preprocessor
            if (value_part[0] == '#' || value_part[0] == '/')
            {
                continue;
            }

            // A single line can hold several values: "enum Flags { A, B, C };"
            const std::vector<std::string> value_pieces = SplitEnumValues(value_part);
            for (size_t piece_index = 0; piece_index < value_pieces.size(); ++piece_index)
            {
                const std::string& piece = value_pieces[piece_index];
                if (piece.empty())
                {
                    continue;
                }

                EnumValue enum_val;

                // Split on '='
                if (size_t eq_pos = piece.find('='); eq_pos != std::string::npos)
                {
                    enum_val.name = Trim(std::string_view(piece).substr(0, eq_pos));
                    enum_val.initializer = Trim(std::string_view(piece).substr(eq_pos + 1));
                }
                else
                {
                    enum_val.name = Trim(piece);
                }

                if (enum_val.name.empty())
                {
                    continue;
                }

                // Apply doc comment: a pending block comment belongs to the first value on
                // the line, a trailing "///<" comment to the last one.
                if (piece_index == 0 && have_pending_block_doc)
                {
                    // Parse rather than assign raw text so inline commands (@c, @e, ...)
                    // are processed instead of leaking into the value table.
                    enum_val.doc = ParseCommentBlock(pending_block_text);
                    pending_block_text.clear();
                    have_pending_block_doc = false;
                }
                else if (piece_index + 1 == value_pieces.size() && !trailing_comment.empty())
                {
                    // Route the trailing text through the comment parser so inline commands
                    // such as @c and @e are processed instead of leaking raw into the table.
                    enum_val.doc = ParseCommentBlock(trailing_comment);
                }

                info.values.push_back(std::move(enum_val));
                value_nesting = CountOpenNesting(info.values.back().initializer);
                value_continues =
                    (value_nesting == 0) && EndsWithOperator(info.values.back().initializer);
            }
        }

        return info;
    }

    // ---------------------------------------------------------------------------
    // ParseTypedef
    // ---------------------------------------------------------------------------

    [[nodiscard]] auto ParseTypedef(std::string_view text) -> TypedefInfo
    {
        TypedefInfo info;
        std::string trimmed = Trim(text);

        // Remove trailing semicolon
        while (!trimmed.empty() && (trimmed.back() == ';' || IsTrimSpace(trimmed.back())))
        {
            trimmed.pop_back();
        }

        if (size_t len = StartsWith(trimmed, "typedef "); len > 0)
        {
            // "typedef OldType NewName"
            const std::string rest = Trim(std::string_view(trimmed).substr(len));
            if (rest.empty())
            {
                return info;
            }

            // Function-pointer typedef: "int (*wxSortCompareFunction)(int, int)" — the name
            // sits inside "(*...)"; the underlying type is the declaration with the name
            // removed.
            if (const size_t pointer_pos = rest.rfind("(*"); pointer_pos != std::string::npos)
            {
                size_t scan = SkipWS(rest, pointer_pos + 2);
                const size_t name_begin = scan;
                while (scan < rest.size() &&
                       (std::isalnum(static_cast<unsigned char>(rest[scan])) || rest[scan] == '_'))
                {
                    ++scan;
                }
                if (scan > name_begin && scan < rest.size() && rest[scan] == ')')
                {
                    info.name = rest.substr(name_begin, scan - name_begin);
                    info.underlying_type = Trim(rest.substr(0, name_begin)) + rest.substr(scan);
                    return info;
                }
            }

            // Array typedef: "wxChar wxCharBuffer[]" — the name precedes the brackets.
            std::string suffix;
            size_t name_end = rest.size();
            if (rest.back() == ']')
            {
                const size_t bracket_pos = rest.rfind('[');
                if (bracket_pos != std::string::npos && bracket_pos > 0)
                {
                    suffix = rest.substr(bracket_pos);
                    name_end = bracket_pos;
                    while (name_end > 0 && IsTrimSpace(rest[name_end - 1]))
                    {
                        --name_end;
                    }
                }
            }

            // Name is the last identifier
            size_t name_start = name_end;
            while (name_start > 0 &&
                   (std::isalnum(static_cast<unsigned char>(rest[name_start - 1])) ||
                    rest[name_start - 1] == '_'))
            {
                --name_start;
            }
            if (name_start < name_end)
            {
                info.name = rest.substr(name_start, name_end - name_start);
                info.underlying_type = Trim(rest.substr(0, name_start)) + suffix;
            }
        }
        else if (size_t len = StartsWith(trimmed, "using "); len > 0)
        {
            // "using NewName = OldType"
            std::string rest = Trim(std::string_view(trimmed).substr(len));
            if (size_t eq_pos = rest.find('='); eq_pos != std::string::npos)
            {
                info.name = Trim(rest.substr(0, eq_pos));
                info.underlying_type = Trim(rest.substr(eq_pos + 1));
            }
        }

        return info;
    }

    // ---------------------------------------------------------------------------
    // ParseDefine
    // ---------------------------------------------------------------------------

    [[nodiscard]] auto ParseDefine(std::string_view text) -> DefineInfo
    {
        DefineInfo info;
        std::string trimmed = Trim(text);

        // "#define NAME value" or "#define NAME(args) value"
        size_t def_len = StartsWith(trimmed, "#define ");
        if (def_len == 0)
        {
            def_len = StartsWith(trimmed, "#define\t");
        }
        if (def_len == 0)
        {
            return info;
        }

        std::string rest = Trim(std::string_view(trimmed).substr(def_len));

        // Name is the first identifier (may be followed by (args))
        size_t name_end = 0;
        while (name_end < rest.size() &&
               (std::isalnum(static_cast<unsigned char>(rest[name_end])) || rest[name_end] == '_'))
        {
            ++name_end;
        }
        info.name = std::string(rest.substr(0, name_end));

        // Value is everything after the name (and optional macro args)
        std::string after_name = rest.substr(name_end);
        if (!after_name.empty() && after_name[0] == '(')
        {
            // Macro with args — skip past the closing paren
            size_t close = FindMatchingClose(after_name, 0);
            if (close != std::string_view::npos)
            {
                after_name = after_name.substr(close + 1);
            }
        }

        // A trailing comment is not part of the macro value.
        if (const size_t line_comment = after_name.find("//"); line_comment != std::string::npos)
        {
            after_name = after_name.substr(0, line_comment);
        }
        if (const size_t block_comment = after_name.find("/*"); block_comment != std::string::npos)
        {
            after_name = after_name.substr(0, block_comment);
        }

        info.value = Trim(after_name);

        return info;
    }

}  // namespace docparser
