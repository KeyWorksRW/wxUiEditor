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

    // All recognized doxygen commands
    enum class DoxyCmd : std::uint8_t
    {
        unknown,

        // Class/file-level metadata
        class_,
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

        // Inline formatting
        a,
        b,
        c,
        p,
        e,

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

        // Platform-specific
        only_for,
        native_impl,

        // Remarks
        remark,
        remarks,
    };

    // Parse a doxygen command from text starting at '@' or '\'.
    // Returns the command enum and the remaining text after the command word.
    // If the command is not recognized, returns DoxyCmd::unknown.
    [[nodiscard]] std::pair<DoxyCmd, std::string_view> ParseCommand(std::string_view text);

    // Expand simple alias commands to their markdown equivalents.
    // Returns empty string if the command is not an alias.
    std::string ExpandAlias(DoxyCmd command);

    // Check if a command starts a block that should be skipped entirely
    // (e.g., Python-only blocks).
    [[nodiscard]] bool IsSkipBlockStart(DoxyCmd command);

    [[nodiscard]] bool IsSkipBlockEnd(DoxyCmd command);

}  // namespace docparser
