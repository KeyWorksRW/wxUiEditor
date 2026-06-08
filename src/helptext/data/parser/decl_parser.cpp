/////////////////////////////////////////////////////////////////////////////
// Purpose:   Parse C++ class, method, enum, and typedef decls
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "decl_parser.h"

#include <array>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace docparser
{

    namespace
    {

        // ---------------------------------------------------------------------------
        // Helpers
        // ---------------------------------------------------------------------------

        // Skip whitespace at position pos, return new position
        [[nodiscard]] auto SkipWS(std::string_view text, size_t pos) -> size_t
        {
            while (pos < text.size() && (text[pos] == ' ' || text[pos] == '\t'))
            {
                ++pos;
            }
            return pos;
        }

        [[nodiscard]] auto Trim(std::string_view text) -> std::string
        {
            size_t start = SkipWS(text, 0);
            size_t endPos = text.size();
            while (endPos > start && (text[endPos - 1] == ' ' || text[endPos - 1] == '\t'))
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
                    // Skip string literals
                    ++i;
                    while (i < text.size() && text[i] != '"')
                    {
                        if (text[i] == '\\')
                        {
                            ++i;
                        }
                        ++i;
                    }
                }
                else if (text[i] == '\'')
                {
                    // Skip char literals
                    ++i;
                    while (i < text.size() && text[i] != '\'')
                    {
                        if (text[i] == '\\')
                        {
                            ++i;
                        }
                        ++i;
                    }
                }
            }
            return std::string_view::npos;
        }

        // Attempt to split a parameter string (between parentheses) into individual
        // parameters, respecting nested angle brackets, parens, etc.
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
                else if (chr == ',' && paren_depth == 0 && angle_depth == 0)
                {
                    result.push_back(params_text.substr(start, i - start));
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

            param.name = type_and_name.substr(name_start, name_end - name_start);
            param.type = Trim(std::string_view(type_and_name.data(), name_start));

            // Clean up type: remove trailing & or * that got separated
            // Actually they should already be part of the type portion

            return param;
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
        while (!trimmed.empty() && (trimmed.back() == '{' || trimmed.back() == ';' ||
                                    trimmed.back() == ' ' || trimmed.back() == '\t'))
        {
            trimmed.pop_back();
        }

        // Skip "class " prefix
        if (size_t len = StartsWith(trimmed, "class "); len > 0)
        {
            trimmed = trimmed.substr(len);
        }
        else
        {
            return info;
        }

        trimmed = Trim(trimmed);

        // Check for WXDLLIMPEXP_* macro between "class" and the name
        // e.g., "class WXDLLIMPEXP_CORE wxButton : public wxAnyButton"
        std::string name_part = trimmed;
        if (!trimmed.empty() && std::isupper(static_cast<unsigned char>(trimmed[0])))
        {
            // Could be a macro like WXDLLIMPEXP_CORE
            if (size_t space = trimmed.find(' '); space != std::string_view::npos)
            {
                std::string potential_macro = trimmed.substr(0, space);
                // Check if it looks like WXDLLIMPEXP_XXX or similar all-caps macro
                bool is_macro = true;
                for (const char chr: potential_macro)
                {
                    if (chr != '_' && !std::isupper(static_cast<unsigned char>(chr)) &&
                        !std::isdigit(static_cast<unsigned char>(chr)))
                    {
                        is_macro = false;
                        break;
                    }
                }
                if (is_macro && potential_macro.size() > 2)
                {
                    name_part = Trim(trimmed.substr(space + 1));
                }
            }
        }

        // Find ':' for inheritance
        if (size_t colon = name_part.find(':'); colon != std::string_view::npos)
        {
            info.name = Trim(name_part.substr(0, colon));

            // Parse bases: "public Base1, public Base2"
            std::string bases_str = Trim(name_part.substr(colon + 1));
            std::istringstream stream { bases_str };
            std::string token;
            while (std::getline(stream, token, ','))
            {
                std::string base = Trim(token);
                // Remove access specifier
                if (size_t len = StartsWith(base, "public "); len > 0)
                {
                    base = base.substr(len);
                }
                else if (size_t len = StartsWith(base, "protected "); len > 0)
                {
                    base = base.substr(len);
                }
                else if (size_t len = StartsWith(base, "private "); len > 0)
                {
                    base = base.substr(len);
                }
                base = Trim(base);
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
        while (!trimmed.empty() && (trimmed.back() == ';' || trimmed.back() == ' '))
        {
            trimmed.pop_back();
        }

        // Check for trailing qualifiers after the closing paren
        // Find the parameter list first
        size_t open_paren = std::string_view::npos;
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
            // No parens — might be a variable or something odd; treat as name-only
            info.name = std::string(trimmed);
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
        std::string prefix = Trim(trimmed.substr(0, open_paren));

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
        if (size_t op_pos = prefix.find("operator"); op_pos != std::string::npos)
        {
            // Everything from "operator" onwards is the name
            info.name = Trim(std::string_view(prefix).substr(op_pos));
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

        // Strip "enum " prefix
        if (size_t len = StartsWith(first_line, "enum "); len > 0)
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
               first_line[name_end] != ':' && first_line[name_end] != ' ' &&
               first_line[name_end] != '\t')
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
        bool in_skip_block = false;  // for non-doxygen /* */ block comments

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
                std::string rest = line.substr(len);
                if (auto end_pos = rest.find("*/"); end_pos != std::string::npos)
                {
                    // Single-line block comment: /** text */
                    pending_block_text = Trim(rest.substr(0, end_pos));
                    have_pending_block_doc = true;
                }
                else
                {
                    pending_block_text = Trim(rest);
                    in_block_comment = true;
                }
                continue;
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

            // Check for trailing /// comment
            std::string trailing_comment;
            std::string value_part;
            if (size_t triple_slash = line.find("///"); triple_slash != std::string::npos)
            {
                value_part = Trim(std::string_view(line).substr(0, triple_slash));
                trailing_comment = Trim(std::string_view(line).substr(triple_slash + 3));

                // Handle "/// @c wxBOTH." or "/// A synonym for ..." forms by just using as-is
            }
            else
            {
                value_part = std::string(line);
            }

            if (value_part.empty())
            {
                continue;
            }

            // Remove trailing comma
            if (!value_part.empty() && value_part.back() == ',')
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

            EnumValue enum_val;

            // Split on '='
            if (size_t eq_pos = value_part.find('='); eq_pos != std::string::npos)
            {
                enum_val.name = Trim(std::string_view(value_part).substr(0, eq_pos));
                enum_val.initializer = Trim(std::string_view(value_part).substr(eq_pos + 1));
            }
            else
            {
                enum_val.name = Trim(value_part);
            }

            if (enum_val.name.empty())
            {
                continue;
            }

            // Apply doc comment
            if (have_pending_block_doc)
            {
                enum_val.doc.brief = pending_block_text;
                pending_block_text.clear();
                have_pending_block_doc = false;
            }
            else if (!trailing_comment.empty())
            {
                enum_val.doc.brief = trailing_comment;
            }

            info.values.push_back(std::move(enum_val));
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
        while (!trimmed.empty() && (trimmed.back() == ';' || trimmed.back() == ' '))
        {
            trimmed.pop_back();
        }

        if (size_t len = StartsWith(trimmed, "typedef "); len > 0)
        {
            // "typedef OldType NewName"
            std::string rest = Trim(std::string_view(trimmed).substr(len));
            // Name is the last identifier
            size_t name_end = rest.size();
            size_t name_start = name_end;
            while (name_start > 0 &&
                   (std::isalnum(static_cast<unsigned char>(rest[name_start - 1])) ||
                    rest[name_start - 1] == '_'))
            {
                --name_start;
            }
            if (name_start < name_end)
            {
                info.name = std::string(rest.substr(name_start, name_end - name_start));
                info.underlying_type = Trim(rest.substr(0, name_start));
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

        info.value = Trim(after_name);

        return info;
    }

}  // namespace docparser
