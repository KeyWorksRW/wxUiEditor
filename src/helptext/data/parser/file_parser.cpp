/////////////////////////////////////////////////////////////////////////////
// Purpose:   Parse source files, extract declarations and comments
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "file_parser.h"
#include "comment_parser.h"
#include "decl_parser.h"

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

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
                    lines.emplace_back(text.substr(start));
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

        // Read entire file into a string
        auto ReadFile(const std::filesystem::path& path) -> std::string
        {
            std::ifstream ifs(path, std::ios::binary);
            if (!ifs)
            {
                return {};
            }
            return { (std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>() };
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
            std::string trimmed = Trim(line);
            return StartsWith(trimmed, "/**") > 0;
        }

        // Check if a line is a triple-slash comment
        [[nodiscard]] auto IsTripleSlash(std::string_view line) -> bool
        {
            std::string trimmed = Trim(line);
            return StartsWith(trimmed, "///") > 0 && StartsWith(trimmed, "///}") == 0 &&
                   StartsWith(trimmed, "///{") == 0;
        }

        // Check if a line starts a grouping block: ///@{
        [[nodiscard]] auto IsGroupStart(std::string_view line) -> bool
        {
            std::string trimmed = Trim(line);
            return trimmed == "///@{" || trimmed == "/// @{";
        }

        // Check if a line ends a grouping block: ///@}
        [[nodiscard]] auto IsGroupEnd(std::string_view line) -> bool
        {
            std::string trimmed = Trim(line);
            return trimmed == "///@}" || trimmed == "/// @}";
        }

        // Check if a line is a license/file header comment (plain //, not ///)
        [[nodiscard]] auto IsPlainComment(std::string_view line) -> bool
        {
            std::string trimmed = Trim(line);
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
            Method,  // function/method declaration
            Const,   // const int x = ...;
        };

        auto ClassifyDecl(std::string_view line) -> DeclKind
        {
            if (IsBlank(line))
            {
                return DeclKind::Unknown;
            }
            std::string trimmed = Trim(line);

            static const std::map<std::string_view, DeclKind> PREFIX_MAP = {
                { "#define ", DeclKind::Define }, { "class ", DeclKind::Class },
                { "enum ", DeclKind::Enum },      { "typedef ", DeclKind::Typedef },
                { "using ", DeclKind::Using },
            };

            for (const auto& [prefix, kind]: PREFIX_MAP)
            {
                if (trimmed.starts_with(prefix))
                {
                    return kind;
                }
            }

            bool has_paren = trimmed.find('(') != std::string_view::npos;
            if (trimmed.starts_with("const ") && !has_paren)
            {
                return DeclKind::Const;
            }
            if (has_paren)
            {
                return DeclKind::Method;
            }

            return DeclKind::Unknown;
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

            while (i < lines.size())
            {
                const bool was_open = found_open;
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
                            return i + 1;  // line after closing brace
                        }
                    }
                }
                // Only push lines after the one containing the opening brace
                if (was_open && brace_depth > 0)
                {
                    body_lines.push_back(lines[i]);
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

        // Extract a multi-line method declaration that may span multiple lines up to ';'
        auto GatherMethodDecl(const std::vector<std::string>& lines, size_t start_line,
                              std::string& decl_text) -> size_t
        {
            size_t i = start_line;
            int paren_depth = 0;
            bool found_paren = false;

            while (i < lines.size())
            {
                std::string trimmed = Trim(lines[i]);

                if (!decl_text.empty())
                {
                    decl_text += ' ';
                }
                decl_text += trimmed;

                // Track paren depth
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

                ++i;
            }
            return i;
        }

        // Parse class body lines into methods, enums, typedefs
        auto ParseClassBody(const std::vector<std::string>& body_lines, ClassInfo& cls) -> void
        {
            DocComment pending_doc;
            bool have_doc = false;

            for (size_t i = 0; i < body_lines.size(); ++i)
            {
                if (IsBlank(body_lines[i]))
                {
                    continue;
                }
                std::string trimmed = Trim(body_lines[i]);

                // Skip access specifiers
                if (trimmed == "public:" || trimmed == "protected:" || trimmed == "private:")
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
                    // Find the end of the comment block
                    if (trimmed.find("*/") != std::string::npos && trimmed.size() > 3)
                    {
                        // Single-line: /** text */
                        size_t start = trimmed.find("/**") + 3;
                        size_t end = trimmed.find("*/");
                        comment_block = std::string(trimmed.substr(start, end - start));
                    }
                    else
                    {
                        // Multi-line: collect until */
                        size_t start = trimmed.find("/**") + 3;
                        comment_block = std::string(trimmed.substr(start));
                        ++i;
                        while (i < body_lines.size())
                        {
                            const std::string& line = body_lines[i];
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
                    have_doc = true;
                    continue;
                }

                // Triple-slash comments (accumulate)
                if (IsTripleSlash(trimmed))
                {
                    std::string comment_block;
                    while (i < body_lines.size() && IsTripleSlash(Trim(body_lines[i])))
                    {
                        auto line = Trim(body_lines[i]);
                        auto text = line.substr(3);  // skip "///"
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
                    --i;  // back up so outer loop re-examines current line
                    pending_doc = ParseCommentBlock(comment_block);
                    have_doc = true;
                    continue;
                }

                // Now classify the declaration
                DeclKind kind = ClassifyDecl(trimmed);

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
                    cls.enums.push_back(std::move(enum_info));
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
                    cls.typedefs.push_back(std::move(type_def));
                    continue;
                }

                if (kind == DeclKind::Method || kind == DeclKind::Unknown)
                {
                    // Gather multi-line method declaration
                    std::string decl_text;
                    size_t const new_i = GatherMethodDecl(body_lines, i, decl_text);
                    i = new_i - 1;  // -1 because loop increments

                    MethodInfo method = ParseMethodDecl(decl_text);
                    if (!method.name.empty())
                    {
                        if (have_doc)
                        {
                            method.doc = std::move(pending_doc);
                            have_doc = false;
                        }
                        cls.methods.push_back(std::move(method));
                    }
                    else
                    {
                        have_doc = false;  // discard orphaned doc
                    }
                    continue;
                }

                // Default: skip and discard pending doc
                have_doc = false;
            }
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

        std::string content = ReadFile(path);
        if (content.empty())
        {
            return result;
        }

        std::vector<std::string> lines = SplitLines(content);

        // State
        DocComment pending_doc;
        bool have_doc = false;
        std::string current_group;  // from @addtogroup

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
            // Skip blank lines
            if (IsBlank(lines[i]))
            {
                ++i;
                continue;
            }
            std::string trimmed = Trim(lines[i]);

            // Skip preprocessor directives (pass through)
            if (IsPreprocessor(trimmed))
            {
                // Check for #define at file level
                if (StartsWith(trimmed, "#define "))
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

            // Group start/end
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

            // @addtogroup detection in a comment
            // Pattern: "/** @addtogroup group_name */" or  "/** @addtogroup group_name \n ... */"
            // Also: "/// @addtogroup group_name"

            // Doxygen comment block
            if (IsDoxyCommentStart(trimmed))
            {
                std::string comment_block;

                if (trimmed.find("*/") != std::string::npos && trimmed.size() > 5)
                {
                    // Single-line: /** text */
                    size_t start = trimmed.find("/**") + 3;
                    size_t end = trimmed.find("*/");
                    comment_block = std::string(trimmed.substr(start, end - start));
                }
                else
                {
                    // Multi-line
                    size_t start = trimmed.find("/**") + 3;
                    comment_block = std::string(trimmed.substr(start));
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
                have_doc = true;

                // Check if this comment sets a group
                if (!pending_doc.group_name.empty())
                {
                    current_group = pending_doc.group_name;
                }

                ++i;
                continue;
            }

            // Triple-slash comment accumulation
            if (IsTripleSlash(trimmed))
            {
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
                pending_doc = ParseCommentBlock(comment_block);
                have_doc = true;

                if (!pending_doc.group_name.empty())
                {
                    current_group = pending_doc.group_name;
                }
                continue;  // don't increment i — we're already past the comments
            }

            // Now handle declarations following a doc comment
            DeclKind kind = ClassifyDecl(trimmed);

            if (kind == DeclKind::Class)
            {
                // Parse class declaration
                ClassInfo cls = ParseClassDecl(trimmed);

                if (have_doc)
                {
                    cls.doc = std::move(pending_doc);
                    have_doc = false;
                }

                // Gather and parse class body
                std::vector<std::string> body_lines;
                size_t after = GatherClassBody(lines, i, body_lines);

                ParseClassBody(body_lines, cls);

                result.classes.push_back(std::move(cls));
                i = after;
                continue;
            }

            if (kind == DeclKind::Enum)
            {
                std::string enum_text;
                size_t after = GatherEnumBody(lines, i, enum_text);
                EnumInfo enum_info = ParseEnumDecl(enum_text);
                if (have_doc)
                {
                    enum_info.doc = std::move(pending_doc);
                    have_doc = false;
                }
                result.enums.push_back(std::move(enum_info));
                i = after;
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
                result.typedefs.push_back(std::move(type_def));
                ++i;
                continue;
            }

            if (kind == DeclKind::Define)
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
                ++i;
                continue;
            }

            if (kind == DeclKind::Const)
            {
                // Handle "const int wxInvalidOffset = -1;" etc.
                // Treat as a define-like constant
                DefineInfo def;
                std::string rest = Trim(std::string_view(trimmed));
                // Remove trailing ';'
                if (!rest.empty() && rest.back() == ';')
                {
                    rest.pop_back();
                }
                size_t eq_pos = rest.find('=');
                if (eq_pos != std::string::npos)
                {
                    // Name is the last identifier before '='
                    std::string before_eq = Trim(rest.substr(0, eq_pos));
                    size_t name_end = before_eq.size();
                    size_t name_start = name_end;
                    while (name_start > 0 &&
                           (std::isalnum(static_cast<unsigned char>(before_eq[name_start - 1])) ||
                            before_eq[name_start - 1] == '_'))
                    {
                        --name_start;
                    }
                    def.name = before_eq.substr(name_start, name_end - name_start);
                    def.value = Trim(rest.substr(eq_pos + 1));
                }
                if (!def.name.empty())
                {
                    if (have_doc)
                    {
                        def.doc = std::move(pending_doc);
                        have_doc = false;
                    }
                    result.defines.push_back(std::move(def));
                }
                ++i;
                continue;
            }

            if (kind == DeclKind::Method || kind == DeclKind::Unknown)
            {
                // Free function at file level (in a group context or standalone)
                std::string decl_text;
                size_t after = GatherMethodDecl(lines, i, decl_text);

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

        return result;
    }

}  // namespace docparser
