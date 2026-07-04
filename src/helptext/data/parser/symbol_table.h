/////////////////////////////////////////////////////////////////////////////
// Purpose:   Build and query class/symbol cross-reference table
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace docparser
{

    // Pass-1 symbol table built from lightweight regex scanning.
    // Maps class names to files, tracks inheritance, enums, etc.
    class SymbolTable
    {
    public:
        // files  Relative paths to .h files (from ScanDirectory)
        void BuildFromFiles(const std::filesystem::path& root,
                            const std::vector<std::filesystem::path>& files);

        // Resolve a class name to its source file (relative path).
        [[nodiscard]] std::optional<std::filesystem::path>
            ClassToFile(const std::string& name) const;

        // Get the inheritance chain for a class (immediate bases first).
        [[nodiscard]] std::vector<std::string> InheritanceChain(const std::string& name) const;

        // Direct base classes declared by 'name' (empty if none/unknown).
        [[nodiscard]] std::vector<std::string> DirectBases(const std::string& name) const;

        // Direct derived classes — every class that names 'name' as a base.
        // Sorted alphabetically for deterministic output.
        [[nodiscard]] std::vector<std::string> DirectDerived(const std::string& name) const;

        // Get all known classes as (name, relative_path) pairs.
        [[nodiscard]] std::vector<std::pair<std::string, std::filesystem::path>> AllClasses() const;

        // Returns true when 'name' derives from wxEvent anywhere in its inheritance chain.
        bool IsEventClass(const std::string& name) const;

        // Returns true when 'class_name' or any of its ancestors declares a method
        // named 'method_name' with the 'virtual' keyword. wxWidgets interface headers
        // omit 'virtual' on overrides, so this resolves inherited virtuality the way
        // Doxygen does (matching by name across the inheritance graph).
        bool IsMethodVirtual(const std::string& class_name, const std::string& method_name) const;

        // Stats
        [[nodiscard]] std::size_t ClassCount() const { return class_to_file_.size(); }
        [[nodiscard]] std::size_t EnumCount() const { return enum_count_; }

    private:
        // class name -> relative file path
        std::unordered_map<std::string, std::filesystem::path> class_to_file_;

        // class name -> list of base class names
        std::unordered_map<std::string, std::vector<std::string>> class_bases_;

        // class name -> list of directly derived class names (reverse of class_bases_)
        std::unordered_map<std::string, std::vector<std::string>> class_derived_;

        // class name -> set of method names declared 'virtual' directly in that class
        std::unordered_map<std::string, std::unordered_set<std::string>> class_virtual_methods_;

        std::size_t enum_count_ = 0;
    };

}  // namespace docparser
