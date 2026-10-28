/////////////////////////////////////////////////////////////////////////////
// Purpose:   Data structures for parsed documentation elements
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <string>
#include <utility>
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

    // Ordered documentation blocks. Rendering walks this list in source order so
    // that prose, code, tables, lists, and sections never drift apart.
    enum class BlockKind : std::uint8_t
    {
        paragraph,
        bullet_list,
        ordered_list,
        code,
        table,
        def_list,
        params,
        returns,
        since,
        deprecated,
        note,
        warning,
        todo,
        see_also,
        section,
        verbatim,
        image,
        availability,
        anchor
    };

    // One run of inline text. 'target' is only used by the ref kind.
    struct InlineSpan
    {
        enum class Kind : std::uint8_t
        {
            text,
            code,
            emphasis,
            strong,
            ref
        } kind = Kind::text;
        std::string text;
        std::string target;
    };

    struct DocBlock
    {
        BlockKind kind = BlockKind::paragraph;
        std::vector<InlineSpan> spans;               // paragraph and single-value blocks
        std::vector<std::vector<InlineSpan>> items;  // bullet_list / ordered_list / see_also
        std::vector<std::vector<std::vector<InlineSpan>>> rows;  // table: rows -> cells -> spans
        std::vector<std::pair<std::vector<InlineSpan>, std::vector<InlineSpan>>>
            def_items;          // def_list / params
        std::string language;   // code block language
        std::string code;       // code block body, emitted verbatim
        std::string id;         // section / anchor id
        int level = 0;          // section level: 0 = ##, 1 = ###, 2 = ####
        bool is_brief = false;  // paragraph promoted to the one-line summary
    };

    struct DocComment
    {
        std::vector<DocBlock> blocks;

        std::vector<StyleEntry> styles;
        std::vector<EventEntry> events;

        // Metadata
        std::string library;     // @library{name}
        std::string category;    // @category{name}
        std::string appearance;  // @appearance{name}

        // Class-level
        std::string class_name;   // @class Name
        std::string header_file;  // @header{file}

        // Page-level (@page id title)
        std::string page_id;
        std::string page_title;

        // Grouping
        std::string group_name;  // @addtogroup

        // @hideinitializer was present: suppress the initializer when the
        // declaration is rendered as a value row.
        bool hide_initializer = false;
    };

    // A topic page declared with @page. The body lives in 'doc'.
    struct PageInfo
    {
        std::string id;
        std::string title;
        DocComment doc;
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

    // Accessibility of a class member as declared in the class body.
    enum class Access : std::uint8_t
    {
        public_,
        protected_,
        private_
    };

    // Data member declared inside a class/struct body.
    struct FieldInfo
    {
        std::string type;
        std::string name;
        bool is_static = false;
        Access access = Access::public_;
        DocComment doc;
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
        Access access = Access::public_;
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

    // File-scope constant: "const"/"constexpr" declaration with an optional
    // initializer (e.g. "constexpr wxUint32 wxPG_COLOUR_WEB_BASE = 0x10000").
    struct ConstantInfo
    {
        std::string name;  // declared identifier
        std::string
            declaration;    // everything before '=', e.g. "constexpr wxUint32 wxPG_COLOUR_WEB_BASE"
        std::string value;  // initializer text; empty if none
        DocComment doc;
    };

    // A free function (same as MethodInfo but grouped by @addtogroup)
    using FreeFunction = MethodInfo;

    struct ClassInfo
    {
        std::string name;
        std::vector<std::string> bases;
        std::vector<MethodInfo> methods;
        std::vector<FieldInfo> fields;
        std::vector<EnumInfo> enums;
        std::vector<TypedefInfo> typedefs;
        std::vector<ClassInfo> nested;  // class/struct declared inside this body
        DocComment doc;
    };

    struct FileContent
    {
        std::string filename;  // relative path from input root
        std::vector<ClassInfo> classes;
        std::vector<EnumInfo> enums;        // file-level enums
        std::vector<TypedefInfo> typedefs;  // file-level typedefs
        std::vector<DefineInfo> defines;
        std::vector<ConstantInfo> constants;  // file-scope const/constexpr declarations
        std::vector<FreeFunction> free_functions;
        std::vector<PageInfo> pages;  // topic pages declared with @page
        // @class documentation whose name matches no class page in this header.
        std::vector<ClassInfo> alias_pages;
    };

}  // namespace docparser
