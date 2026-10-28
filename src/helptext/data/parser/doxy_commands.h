/////////////////////////////////////////////////////////////////////////////
// Purpose:   Map and process doxygen command strings
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace docparser
{

    // All recognized doxygen commands.
    // Synonym spellings each get their own enumerator (e.g. return_/returns, see/sa,
    // remark/remarks) -- consumers must handle all synonyms of a command together.
    enum class DoxyCmd : std::uint8_t
    {
        unknown,

        // Class/file-level metadata
        class_,
        page,
        brief,
        param,
        return_,
        returns,
        see,
        sa,
        ref,
        since,
        code,
        end_code,
        library,
        category,
        appearance,
        header,
        note,
        warning,
        deprecated,
        todo,
        table_of_contents,

        // Block content and cross references
        verbatim,
        end_verbatim,
        anchor,
        image,
        copydoc,
        ref_member_group,
        member_group_name,

        // Inline formatting
        a,
        b,
        c,
        p,
        e,
        em,

        // Lists
        li,

        // Style/Event tables
        begin_style_table,
        end_style_table,
        style,
        begin_event_table,
        begin_event_emission_table,
        end_event_table,
        event,

        // Generic tables
        begin_table,
        end_table,
        row_2_col,
        row_3_col,

        // Definition lists
        begin_def_list,
        end_def_list,
        item_def,

        // Grouping
        name,
        add_to_group,

        // Language-specific skips
        begin_wx_perl_only,
        end_wx_perl_only,
        begin_wx_python_only,
        end_wx_python_only,

        // Aliases that expand to formatted text
        true_,
        false_,
        null,

        // Section headings
        section,
        subsection,
        subsubsection,

        // Platform-specific
        only_for,
        native_impl,

        // Remarks
        remark,
        remarks,

        // Directives that affect how a value is displayed and never produce output
        hide_initializer,
    };

    // Parse a doxygen command from text starting at '@' or '\'.
    // Returns the command enum and the remaining text after the command word.
    // On an unknown command, returns { DoxyCmd::unknown, <the command text as passed in> }.
    // The returned string_view refers to the caller's buffer -- `text` must outlive the result.
    [[nodiscard]] std::pair<DoxyCmd, std::string_view> ParseCommand(std::string_view text);

    // Expand simple alias commands to their markdown equivalents.
    // Returns empty string if the command is not an alias.
    std::string ExpandAlias(DoxyCmd command);

    // Check if a command starts a block that should be skipped entirely
    // (e.g., Python-only blocks).
    [[nodiscard]] bool IsSkipBlockStart(DoxyCmd command);

    [[nodiscard]] bool IsSkipBlockEnd(DoxyCmd command);

}  // namespace docparser
