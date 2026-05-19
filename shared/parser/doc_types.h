/////////////////////////////////////////////////////////////////////////////
// Purpose:   Data structures for parsed documentation elements
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

namespace docparser
{

    // ---------------------------------------------------------------------------
    // Parsed doxygen comment structures
    // ---------------------------------------------------------------------------

    struct ParamDoc
    {
        std::string name;
        std::string description;
    };

    // Row in a @beginStyleTable
    struct StyleEntry
    {
        std::string name;  // e.g. "wxBU_LEFT"
        std::string description;
    };

    // Row in a @beginEventTable / @beginEventEmissionTable
    struct EventEntry
    {
        std::string name;  // e.g. "EVT_BUTTON(id, func)"
        std::string description;
        std::string event_type;  // the type argument from @beginEventEmissionTable{type}
    };

    // Generic table row (2 or 3 columns from @row2col / @row3col)
    struct TableRow
    {
        std::vector<std::string> columns;
    };

    // Definition list item from @itemdef
    struct DefListItem
    {
        std::string term;
        std::string definition;
    };

    // A code block from @code{.lang} ... @endcode
    struct CodeBlock
    {
        std::string language;  // e.g. "cpp", empty if unspecified
        std::string code;
    };

    struct DocComment
    {
        std::string brief;
        std::string detailed;

        std::vector<ParamDoc> params;
        std::string returns;
        std::vector<std::string> see_also;
        std::string since;
        std::string deprecated;
        std::vector<std::string> notes;
        std::vector<std::string> warnings;
        std::vector<std::string> todos;
        std::vector<CodeBlock> code_blocks;

        std::vector<StyleEntry> styles;
        std::vector<EventEntry> events;
        std::vector<TableRow> tables;
        std::vector<DefListItem> def_list;

        // Metadata
        std::string library;     // @library{name}
        std::string category;    // @category{name}
        std::string appearance;  // @appearance{name}

        // Class-level
        std::string class_name;   // @class Name
        std::string header_file;  // @header{file}

        // Grouping
        std::string group_name;  // @addtogroup
    };

    // ---------------------------------------------------------------------------
    // Parsed C++ declaration structures
    // ---------------------------------------------------------------------------

    struct ParamInfo
    {
        std::string type;
        std::string name;
        std::string default_value;  // empty if none
    };

    // Method or free function
    struct MethodInfo
    {
        std::string name;
        std::string return_type;
        std::vector<ParamInfo> params;
        bool is_const = false;
        bool is_virtual = false;
        bool is_static = false;
        bool is_override = false;
        bool is_pure = false;
        bool is_deleted = false;
        bool is_default = false;
        DocComment doc;
        std::string group_name;  // for free functions via @addtogroup
    };

    struct EnumValue
    {
        std::string name;
        std::string initializer;  // empty if none
        DocComment doc;           // from trailing /// comment
    };

    // Enum or enum class
    struct EnumInfo
    {
        std::string name;        // empty for anonymous enums
        bool is_scoped = false;  // enum class
        std::vector<EnumValue> values;
        DocComment doc;
    };

    // Typedef or using alias
    struct TypedefInfo
    {
        std::string name;
        std::string underlying_type;
        DocComment doc;
    };

    struct DefineInfo
    {
        std::string name;
        std::string value;
        DocComment doc;
    };

    // A free function (same as MethodInfo but grouped by @addtogroup)
    using FreeFunction = MethodInfo;

    struct ClassInfo
    {
        std::string name;
        std::vector<std::string> bases;
        std::vector<MethodInfo> methods;
        std::vector<EnumInfo> enums;
        std::vector<TypedefInfo> typedefs;
        DocComment doc;
    };

    struct FileContent
    {
        std::string filename;  // relative path from input root
        std::vector<ClassInfo> classes;
        std::vector<EnumInfo> enums;        // file-level enums
        std::vector<TypedefInfo> typedefs;  // file-level typedefs
        std::vector<DefineInfo> defines;
        std::vector<FreeFunction> free_functions;
    };

}  // namespace docparser
