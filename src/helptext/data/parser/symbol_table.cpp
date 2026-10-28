/////////////////////////////////////////////////////////////////////////////
// Purpose:   Build and query class/symbol cross-reference table
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "symbol_table.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <regex>
#include <string_view>
#include <unordered_set>

// CR: [10-27-2026]

namespace fs = std::filesystem;

namespace docparser
{

    // Strip C/C++ comments from a single line, carrying multi-line block-comment
    // state across calls via 'in_block_comment'. Returns only the code portion so
    // braces and keywords inside doxygen comments (e.g. '@style{...}', or prose
    // mentioning "virtual") don't confuse brace tracking or method detection. When
    // 'comment_text' is supplied, the removed comment text is appended to it.
    static std::string StripComments(const std::string& line, bool& in_block_comment,
                                     std::string* comment_text = nullptr)
    {
        std::string code;
        size_t pos = 0;
        // Quote state, so comment markers inside string/char literals are ignored.
        char quote = '\0';

        while (pos < line.size())
        {
            if (in_block_comment)
            {
                const size_t close = line.find("*/", pos);
                const size_t end = close == std::string::npos ? line.size() : close + 2;
                if (comment_text != nullptr)
                {
                    comment_text->append(line, pos, end - pos);
                }
                if (close == std::string::npos)
                {
                    return code;
                }
                in_block_comment = false;
                pos = end;
                continue;
            }

            const char current = line[pos];
            if (quote != '\0')
            {
                code += current;
                if (current == '\\' && pos + 1 < line.size())
                {
                    code += line[pos + 1];
                    pos += 2;
                    continue;
                }
                if (current == quote)
                {
                    quote = '\0';
                }
                ++pos;
                continue;
            }
            if (current == '/' && pos + 1 < line.size() && line[pos + 1] == '/')
            {
                if (comment_text != nullptr)
                {
                    comment_text->append(line, pos, std::string::npos);
                }
                break;
            }
            if (current == '/' && pos + 1 < line.size() && line[pos + 1] == '*')
            {
                in_block_comment = true;
                pos += 2;
                if (comment_text != nullptr)
                {
                    comment_text->append("/*");
                }
                continue;
            }
            if (current == '"' || current == '\'')
            {
                quote = current;
            }
            code += current;
            ++pos;
        }
        return code;
    }

    // Replace the contents of string and character literals with spaces so braces
    // and keywords inside literals are not mistaken for code.
    static std::string BlankLiterals(std::string_view code)
    {
        std::string result(code);
        char quote = '\0';
        for (size_t i = 0; i < result.size(); ++i)
        {
            const char current = result[i];
            if (quote != '\0')
            {
                if (current == '\\' && i + 1 < result.size())
                {
                    result[i] = ' ';
                    result[i + 1] = ' ';
                    ++i;
                    continue;
                }
                if (current == quote)
                {
                    quote = '\0';
                }
                result[i] = ' ';
                continue;
            }
            if (current == '"' || current == '\'')
            {
                quote = current;
                result[i] = ' ';
            }
        }
        return result;
    }

    static constexpr bool IsTrimSpace(char value)
    {
        return value == ' ' || value == '\t' || value == '\r' || value == '\n';
    }

    // Trim leading/trailing whitespace from a view (no allocation). A leading UTF-8
    // BOM is treated as whitespace so the first line parses normally.
    static std::string_view TrimView(std::string_view text)
    {
        if (text.starts_with("\xEF\xBB\xBF"))
        {
            text.remove_prefix(3);
        }
        size_t start = 0;
        while (start < text.size() && IsTrimSpace(text[start]))
        {
            ++start;
        }
        size_t end = text.size();
        while (end > start && IsTrimSpace(text[end - 1]))
        {
            --end;
        }
        return text.substr(start, end - start);
    }

    // True when 'text' begins with the given keyword (a word boundary follows).
    static bool HasKeyword(std::string_view text, std::string_view keyword)
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

    // Skip a balanced angle/paren group starting at 'pos' (the opening character).
    static size_t SkipBalanced(std::string_view text, size_t pos)
    {
        if (pos >= text.size())
        {
            return text.size();
        }
        const char open = text[pos];
        const char close = open == '<' ? '>' : open == '(' ? ')' : '\0';
        if (close == '\0')
        {
            return pos + 1;
        }

        int depth = 0;
        for (size_t i = pos; i < text.size(); ++i)
        {
            // A ';', '{' or '(' at depth zero means the '<' was a comparison, not a
            // template argument list, so fall back to the original position.
            if (depth == 0 && open == '<' && (text[i] == ';' || text[i] == '{' || text[i] == '('))
            {
                return pos;
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
                    return i + 1;
                }
            }
        }
        return text.size();
    }

    static bool IsExportMacroToken(std::string_view token)
    {
        return token.starts_with("WXDLLIMPEXP") || token.starts_with("WXLLIMPEXP") ||
               token.starts_with("WXDLLEXPORT") || token.starts_with("WXEXPORT");
    }

    // Strip prefixes that may precede the class/struct/union keyword: a template<...>
    // clause, __declspec(...), wxDEPRECATED_MSG(...), and export macros.
    static std::string_view StripDeclPrefixes(std::string_view text)
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

            size_t token_end = 0;
            while (token_end < rest.size() &&
                   !std::isspace(static_cast<unsigned char>(rest[token_end])) &&
                   rest[token_end] != '<' && rest[token_end] != '(')
            {
                ++token_end;
            }
            if (token_end > 0 && IsExportMacroToken(rest.substr(0, token_end)))
            {
                rest = TrimView(rest.substr(token_end));
                changed = true;
                continue;
            }
        }
        return rest;
    }

    void SymbolTable::BuildFromFiles(const fs::path& root, const std::vector<fs::path>& files)
    {
        // Match base classes from an inheritance list. The access specifier and the
        // 'virtual' keyword are optional; qualified names are captured whole.
        static const std::regex base_re(
            R"((?:(?:public|protected|private)\s+)?(?:virtual\s+)?((?:::)?\w+(?:::\w+)*))",
            std::regex::optimize);

        // Match: enum [class] Name
        static const std::regex enum_re(R"(^\s*enum\s+(?:class\s+)?(\w+))", std::regex::optimize);

        // Match a @page topic id (only scanned on comment lines).
        static const std::regex page_re(R"(@page\s+(\w+))", std::regex::optimize);

        // Match a method declared with the 'virtual' keyword, anchored at the start
        // of the line, capturing the method name before the opening parenthesis.
        static const std::regex virtual_method_re(R"(^\s*(?:\w+\s+)*virtual\b.*?(~?\w+)\s*\()",
                                                  std::regex::optimize);

        // Scope kinds tracked alongside brace_depth, so a compound declared inside a
        // function body can be recognized and skipped.
        enum class ScopeKind
        {
            other,
            class_body,
            function_body
        };

        // A rebuild must produce the same table, so start from a clean slate.
        class_to_file_.clear();
        class_bases_.clear();
        class_derived_.clear();
        class_virtual_methods_.clear();
        page_names_.clear();
        class_page_stems_.clear();
        unreadable_files_.clear();
        enum_count_ = 0;

        for (const fs::path& rel_path: files)
        {
            const fs::path full_path = root / rel_path;
            std::ifstream file(full_path);
            if (!file.is_open())
            {
                unreadable_files_.push_back(rel_path);
                continue;
            }

            // Per-file state for associating virtual methods with their class body.
            bool in_block_comment = false;
            int brace_depth = 0;
            std::string pending_class;  // class declared but awaiting its opening '{'
            std::vector<std::pair<std::string, int>> class_stack;  // (name, depth at open)
            std::vector<ScopeKind> scope_stack;                    // kind of each open brace

            // A class header can span lines (base clauses are commonly wrapped), so
            // accumulate from the keyword line until '{' or ';' appears.
            std::string header_buffer;

            // '#if 0' suppression state.
            bool in_dead_branch = false;
            int dead_branch_nesting = 0;

            std::string line;
            while (std::getline(file, line))
            {
                // Join explicit line continuations before any analysis.
                while (!line.empty() && line.back() == '\\')
                {
                    line.pop_back();
                    std::string continuation;
                    if (!std::getline(file, continuation))
                    {
                        break;
                    }
                    line += continuation;
                }

                // Preprocessor directives are not code. Skip them, but track '#if 0'
                // so brace changes inside a dead branch are ignored.
                const std::string_view directive = TrimView(line);
                if (directive.starts_with('#'))
                {
                    if (directive.starts_with("#if"))
                    {
                        if (in_dead_branch)
                        {
                            ++dead_branch_nesting;
                        }
                        else if (TrimView(directive.substr(3)) == "0")
                        {
                            in_dead_branch = true;
                            dead_branch_nesting = 0;
                        }
                    }
                    else if (directive.starts_with("#endif"))
                    {
                        if (in_dead_branch)
                        {
                            if (dead_branch_nesting > 0)
                            {
                                --dead_branch_nesting;
                            }
                            else
                            {
                                in_dead_branch = false;
                            }
                        }
                    }
                    else if (directive.starts_with("#else") || directive.starts_with("#elif"))
                    {
                        if (in_dead_branch && dead_branch_nesting == 0)
                        {
                            in_dead_branch = false;
                        }
                    }
                    continue;
                }
                if (in_dead_branch)
                {
                    continue;
                }

                std::string comment_text;
                const std::string code = StripComments(line, in_block_comment, &comment_text);
                const std::string literals_removed = BlankLiterals(code);
                const bool has_open_brace = literals_removed.find('{') != std::string::npos;
                const bool has_semicolon = literals_removed.find(';') != std::string::npos;

                // '@page' is comment text, so scan only the comment portion. This is
                // quote-aware and carries block-comment state across lines.
                if (!comment_text.empty())
                {
                    std::smatch page_match;
                    if (std::regex_search(comment_text, page_match, page_re))
                    {
                        RegisterPage(page_match[1].str());
                    }
                }

                // Join this line onto any header already being accumulated.
                if (!header_buffer.empty())
                {
                    const std::string_view addition = TrimView(code);
                    if (!addition.empty())
                    {
                        header_buffer += ' ';
                        header_buffer += addition;
                    }
                }
                else
                {
                    const std::string_view line_head = StripDeclPrefixes(TrimView(code));
                    if (HasKeyword(line_head, "class") || HasKeyword(line_head, "struct") ||
                        HasKeyword(line_head, "union"))
                    {
                        header_buffer = TrimView(code);
                    }
                }

                // The header is complete once its body opens or it ends with ';'.
                const bool header_complete =
                    !header_buffer.empty() && (has_open_brace || has_semicolon);

                // Check for a class/struct/union definition header. A template clause,
                // export macro, or attribute macro may precede the keyword.
                const std::string_view head = header_complete ?
                                                  StripDeclPrefixes(TrimView(header_buffer)) :
                                                  std::string_view {};
                size_t keyword_len = 0;
                if (header_complete)
                {
                    if (HasKeyword(head, "class"))
                    {
                        keyword_len = 5;
                    }
                    else if (HasKeyword(head, "struct"))
                    {
                        keyword_len = 6;
                    }
                    else if (HasKeyword(head, "union"))
                    {
                        keyword_len = 5;
                    }
                }

                if (keyword_len > 0)
                {
                    std::string_view rest = TrimView(head.substr(keyword_len));
                    // An export/attribute macro may follow the keyword:
                    // 'class WXDLLIMPEXP_BASE TimeZone'.
                    rest = StripDeclPrefixes(rest);
                    // Accept a qualified name (Outer::Inner) as one key; a lone ':'
                    // starts the base clause rather than the name.
                    size_t name_end = 0;
                    while (name_end < rest.size())
                    {
                        const char character = rest[name_end];
                        if (std::isalnum(static_cast<unsigned char>(character)) != 0 ||
                            character == '_')
                        {
                            ++name_end;
                            continue;
                        }
                        if (character == ':' && name_end + 1 < rest.size() &&
                            rest[name_end + 1] == ':')
                        {
                            name_end += 2;
                            continue;
                        }
                        break;
                    }
                    const std::string simple_name(rest.substr(0, name_end));

                    // A '{' before any ';' is a definition; a lone ';' is a forward
                    // declaration. Compounds opened inside a function body are local.
                    const size_t open_brace = header_buffer.find('{');
                    const size_t terminator = header_buffer.find(';');
                    const bool is_definition =
                        open_brace != std::string::npos &&
                        (terminator == std::string::npos || open_brace < terminator);
                    const bool is_function_local =
                        !scope_stack.empty() && scope_stack.back() == ScopeKind::function_body;
                    if (!simple_name.empty() && is_definition && !is_function_local)
                    {
                        // A compound declared inside another class is registered
                        // under its qualified name so links can resolve it.
                        const std::string class_name =
                            class_stack.empty() ? simple_name :
                                                  class_stack.back().first + "::" + simple_name;
                        // Duplicate class names across files keep the first mapping.
                        if (!class_to_file_.contains(class_name))
                        {
                            class_to_file_[class_name] = rel_path;
                        }

                        std::string page_stem = class_name;
                        size_t separator = 0;
                        while ((separator = page_stem.find("::", separator)) != std::string::npos)
                        {
                            page_stem.replace(separator, 2, ".");
                            ++separator;
                        }
                        class_page_stems_.insert(std::move(page_stem));

                        // Extract base classes from the inheritance list, stopping at
                        // the opening brace so body text is never scanned.
                        const size_t brace_pos = rest.find('{');
                        const std::string_view header_part =
                            brace_pos == std::string_view::npos ? rest : rest.substr(0, brace_pos);
                        const size_t colon = header_part.find(':');
                        if (colon != std::string_view::npos)
                        {
                            const std::string bases_str(header_part.substr(colon + 1));
                            std::vector<std::string> bases;
                            for (std::sregex_iterator it(bases_str.begin(), bases_str.end(),
                                                         base_re);
                                 it != std::sregex_iterator {}; ++it)
                            {
                                bases.push_back((*it)[1].str());
                            }
                            if (!bases.empty())
                            {
                                class_bases_[class_name] = std::move(bases);
                            }
                        }

                        pending_class = class_name;
                    }
                }

                if (header_complete)
                {
                    header_buffer.clear();
                }

                // Count named enum definitions. Anonymous enums ('enum { ... };') and
                // typedef enums are not counted; a brace on a later line still counts
                // as a definition when the line is not a forward declaration.
                std::smatch match;
                if (std::regex_search(code, match, enum_re) && (has_open_brace || !has_semicolon))
                {
                    ++enum_count_;
                }

                // Track brace nesting (comment-stripped, literal-blanked) to know
                // which class is open.
                for (size_t index = 0; index < literals_removed.size(); ++index)
                {
                    const char character = literals_removed[index];
                    if (character == '{')
                    {
                        if (!pending_class.empty())
                        {
                            class_stack.emplace_back(pending_class, brace_depth);
                            pending_class.clear();
                            scope_stack.push_back(ScopeKind::class_body);
                        }
                        else
                        {
                            size_t previous = index;
                            while (previous > 0)
                            {
                                const char before = literals_removed[previous - 1];
                                if (std::isspace(static_cast<unsigned char>(before)) == 0)
                                {
                                    break;
                                }
                                --previous;
                            }
                            // A '{' that follows a ')' opens a function (or control
                            // flow) body; anything else is a namespace/other scope.
                            const bool follows_paren =
                                previous > 0 && literals_removed[previous - 1] == ')';
                            scope_stack.push_back(follows_paren ? ScopeKind::function_body :
                                                                  ScopeKind::other);
                        }
                        ++brace_depth;
                    }
                    else if (character == '}')
                    {
                        if (brace_depth > 0)
                        {
                            --brace_depth;
                        }
                        if (!scope_stack.empty())
                        {
                            scope_stack.pop_back();
                        }
                        if (!class_stack.empty() && class_stack.back().second == brace_depth)
                        {
                            class_stack.pop_back();
                        }
                    }
                }

                // Record every virtual method declared directly in the current class
                // body. This runs after brace tracking so the correct class is open.
                if (!class_stack.empty())
                {
                    for (std::sregex_iterator it(code.begin(), code.end(), virtual_method_re);
                         it != std::sregex_iterator {}; ++it)
                    {
                        class_virtual_methods_[class_stack.back().first].insert((*it)[1].str());
                    }
                }

                // A pending class that never opened a body is stale.
                if (!pending_class.empty() && !has_open_brace && has_semicolon)
                {
                    pending_class.clear();
                }
            }
            pending_class.clear();
            header_buffer.clear();
        }

        // Build the reverse (derived) map now that every class's bases are known.
        for (const auto& [derived, bases]: class_bases_)
        {
            for (const std::string& base: bases)
            {
                class_derived_[base].push_back(derived);
            }
        }
    }

    std::optional<fs::path> SymbolTable::ClassToFile(const std::string& name) const
    {
        std::unordered_map<std::string, fs::path>::const_iterator const found =
            class_to_file_.find(name);
        if (found != class_to_file_.end())
        {
            return found->second;
        }
        return std::nullopt;
    }

    bool SymbolTable::IsClassPageStem(const std::string& stem) const
    {
        return class_page_stems_.contains(stem);
    }

    void SymbolTable::RegisterClassPageStem(const std::string& stem)
    {
        if (!stem.empty())
        {
            class_page_stems_.insert(stem);
        }
    }

    void SymbolTable::RegisterPage(const std::string& id)
    {
        if (!id.empty())
        {
            page_names_.insert(id);
        }
    }

    bool SymbolTable::IsPageName(const std::string& id) const
    {
        return page_names_.contains(id);
    }

    std::vector<std::string> SymbolTable::InheritanceChain(const std::string& name) const
    {
        std::vector<std::string> chain;
        std::string current = name;

        // Walk up the inheritance chain (follow first base only for linear chain).
        // A visited set makes the walk terminate on a cyclic class graph.
        std::unordered_set<std::string> visited;
        std::unordered_set<std::string> chain_members;
        while (true)
        {
            if (!visited.insert(current).second)
            {
                break;
            }

            std::unordered_map<std::string, std::vector<std::string>>::const_iterator const found =
                class_bases_.find(current);
            if (found == class_bases_.end() || found->second.empty())
            {
                break;
            }

            // Add all immediate bases, skipping any already recorded.
            for (const std::string& base: found->second)
            {
                if (chain_members.insert(base).second)
                {
                    chain.push_back(base);
                }
            }

            // Follow primary base for transitive chain
            current = found->second[0];
        }

        return chain;
    }

    std::vector<std::string> SymbolTable::DirectBases(const std::string& name) const
    {
        const std::unordered_map<std::string, std::vector<std::string>>::const_iterator found =
            class_bases_.find(name);
        if (found != class_bases_.end())
        {
            return found->second;
        }
        return {};
    }

    std::vector<std::string> SymbolTable::DirectDerived(const std::string& name) const
    {
        const std::unordered_map<std::string, std::vector<std::string>>::const_iterator found =
            class_derived_.find(name);
        if (found == class_derived_.end())
        {
            return {};
        }
        std::vector<std::string> result = found->second;
        std::ranges::sort(result);
        return result;
    }

    std::vector<std::pair<std::string, fs::path>> SymbolTable::AllClasses() const
    {
        std::vector<std::pair<std::string, fs::path>> result;
        result.reserve(class_to_file_.size());
        for (const auto& [name, path]: class_to_file_)
        {
            result.emplace_back(name, path);
        }
        std::ranges::sort(result, {}, &std::pair<std::string, fs::path>::first);
        return result;
    }

    bool SymbolTable::IsEventClass(const std::string& name) const
    {
        // BFS through the inheritance graph looking for wxEvent.
        std::unordered_set<std::string> visited;
        std::vector<std::string> work_queue { name };

        while (!work_queue.empty())
        {
            const std::string current = work_queue.back();
            work_queue.pop_back();

            if (current == "wxEvent")
            {
                return true;
            }

            if (visited.contains(current))
            {
                continue;
            }
            visited.insert(current);

            const std::unordered_map<std::string, std::vector<std::string>>::const_iterator found =
                class_bases_.find(current);
            if (found != class_bases_.end())
            {
                for (const std::string& base: found->second)
                {
                    work_queue.push_back(base);
                }
            }
        }

        return false;
    }

    bool SymbolTable::IsMethodVirtual(const std::string& class_name,
                                      const std::string& method_name) const
    {
        // BFS through the inheritance graph: a method is virtual in 'class_name' if it
        // or any ancestor declared it virtual.
        std::unordered_set<std::string> visited;
        std::vector<std::string> work_queue { class_name };

        while (!work_queue.empty())
        {
            const std::string current = work_queue.back();
            work_queue.pop_back();

            if (visited.contains(current))
            {
                continue;
            }
            visited.insert(current);

            const std::unordered_map<std::string, std::unordered_set<std::string>>::const_iterator
                virtual_found = class_virtual_methods_.find(current);
            if (virtual_found != class_virtual_methods_.end() &&
                virtual_found->second.contains(method_name))
            {
                return true;
            }

            const std::unordered_map<std::string, std::vector<std::string>>::const_iterator found =
                class_bases_.find(current);
            if (found != class_bases_.end())
            {
                for (const std::string& base: found->second)
                {
                    work_queue.push_back(base);
                }
            }
        }

        return false;
    }

}  // namespace docparser
