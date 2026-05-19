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

        for (const fs::path& rel_path: files)
        {
            const fs::path full_path = root / rel_path;
            std::ifstream file(full_path);
            if (!file.is_open())
            {
                continue;
            }

            std::string line;
            while (std::getline(file, line))
            {
                std::smatch match;

                // Check for class declaration
                if (std::regex_search(line, match, class_re))
                {
                    std::string const class_name = match[1].str();

                    // Skip forward declarations (lines ending with ;)
                    if (line.find(';') != std::string::npos)
                    {
                        continue;
                    }

                    // Skip WXDLLIMPEXP-type macros being matched as class names
                    if (class_name.starts_with("WXDLLIMPEXP"))
                    {
                        continue;
                    }

                    class_to_file_[class_name] = rel_path;

                    // Extract base classes
                    if (match[2].matched)
                    {
                        const std::string bases_str = match[2].str();
                        std::vector<std::string> bases;
                        const std::sregex_iterator bases_begin =
                            std::sregex_iterator(bases_str.begin(), bases_str.end(), base_re);
                        for (std::sregex_iterator it = bases_begin; it != std::sregex_iterator {};
                             ++it)
                        {
                            bases.push_back((*it)[1].str());
                        }
                        if (!bases.empty())
                        {
                            class_bases_[class_name] = std::move(bases);
                        }
                    }
                }

                // Check for enum declaration
                if (std::regex_search(line, match, enum_re))
                {
                    ++enum_count_;
                }
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

}  // namespace docparser
