/////////////////////////////////////////////////////////////////////////////
// Purpose:   Build and query class/symbol cross-reference table
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "symbol_table.h"

#include <algorithm>
#include <fstream>
#include <regex>
#include <unordered_set>

namespace fs = std::filesystem;

namespace docparser
{

    // Strip C/C++ comments from a single line, carrying multi-line block-comment
    // state across calls via 'in_block_comment'. Returns only the code portion so
    // braces and keywords inside doxygen comments (e.g. '@style{...}', or prose
    // mentioning "virtual") don't confuse brace tracking or method detection.
    static std::string StripComments(const std::string& line, bool& in_block_comment)
    {
        std::string code;
        size_t pos = 0;
        while (pos < line.size())
        {
            if (in_block_comment)
            {
                const size_t close = line.find("*/", pos);
                if (close == std::string::npos)
                {
                    return code;
                }
                in_block_comment = false;
                pos = close + 2;
                continue;
            }
            if (line[pos] == '/' && pos + 1 < line.size() && line[pos + 1] == '/')
            {
                break;
            }
            if (line[pos] == '/' && pos + 1 < line.size() && line[pos + 1] == '*')
            {
                in_block_comment = true;
                pos += 2;
                continue;
            }
            code += line[pos];
            ++pos;
        }
        return code;
    }

    void SymbolTable::BuildFromFiles(const fs::path& root, const std::vector<fs::path>& files)
    {
        // Regex patterns for lightweight pass-1 scanning
        // Match: class ClassName : public Base1, public Base2 { ... }
        // Also matches: class WXDLLIMPEXP_FOO ClassName : ...
        static const std::regex class_re(
            R"(^\s*class\s+(?:\w+\s+)?(\w+)\s*(?::\s*(.+?))?\s*(?:\{|$))", std::regex::optimize);

        // Match: enum [class] Name
        static const std::regex enum_re(R"(^\s*enum\s+(?:class\s+)?(\w+))", std::regex::optimize);

        // Match base classes from inheritance list
        static const std::regex base_re(R"((?:public|protected|private)\s+(\w+))",
                                        std::regex::optimize);

        // Match a method declared with the 'virtual' keyword, capturing the method
        // name immediately before the opening parenthesis.
        static const std::regex virtual_method_re(R"(\bvirtual\b.*?(~?\w+)\s*\()",
                                                  std::regex::optimize);

        for (const fs::path& rel_path: files)
        {
            const fs::path full_path = root / rel_path;
            std::ifstream file(full_path);
            if (!file.is_open())
            {
                continue;
            }

            // Per-file state for associating virtual methods with their class body.
            bool in_block_comment = false;
            int brace_depth = 0;
            std::string pending_class;  // class declared but awaiting its opening '{'
            std::vector<std::pair<std::string, int>> class_stack;  // (name, depth at open)

            std::string line;
            while (std::getline(file, line))
            {
                const std::string code = StripComments(line, in_block_comment);

                std::smatch match;

                // Check for class declaration
                if (std::regex_search(line, match, class_re))
                {
                    std::string const class_name = match[1].str();

                    // Skip forward declarations (lines ending with ;) and WXDLLIMPEXP-type
                    // macros mistaken for class names.
                    if (line.find(';') == std::string::npos &&
                        !class_name.starts_with("WXDLLIMPEXP"))
                    {
                        class_to_file_[class_name] = rel_path;

                        // Extract base classes
                        if (match[2].matched)
                        {
                            const std::string bases_str = match[2].str();
                            std::vector<std::string> bases;
                            const std::sregex_iterator bases_begin =
                                std::sregex_iterator(bases_str.begin(), bases_str.end(), base_re);
                            for (std::sregex_iterator it = bases_begin;
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

                // Check for enum declaration
                if (std::regex_search(line, match, enum_re))
                {
                    ++enum_count_;
                }

                // Record a virtual method declared directly in the current class body.
                if (!class_stack.empty())
                {
                    std::smatch virtual_match;
                    if (std::regex_search(code, virtual_match, virtual_method_re))
                    {
                        class_virtual_methods_[class_stack.back().first].insert(
                            virtual_match[1].str());
                    }
                }

                // Track brace nesting (comment-stripped) to know which class is open.
                for (const char character: code)
                {
                    if (character == '{')
                    {
                        if (!pending_class.empty())
                        {
                            class_stack.emplace_back(pending_class, brace_depth);
                            pending_class.clear();
                        }
                        ++brace_depth;
                    }
                    else if (character == '}')
                    {
                        if (brace_depth > 0)
                        {
                            --brace_depth;
                        }
                        if (!class_stack.empty() && class_stack.back().second == brace_depth)
                        {
                            class_stack.pop_back();
                        }
                    }
                }
            }
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

    std::vector<std::string> SymbolTable::InheritanceChain(const std::string& name) const
    {
        std::vector<std::string> chain;
        std::string current = name;

        // Walk up the inheritance chain (follow first base only for linear chain)
        while (true)
        {
            std::unordered_map<std::string, std::vector<std::string>>::const_iterator const found =
                class_bases_.find(current);
            if (found == class_bases_.end() || found->second.empty())
            {
                break;
            }

            // Add all immediate bases
            for (const std::string& base: found->second)
            {
                chain.push_back(base);
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
