/////////////////////////////////////////////////////////////////////////////
// Purpose:   Map and process doxygen command strings
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "doxy_commands.h"

#include <unordered_map>

namespace docparser
{

    // Map command strings to DoxyCmd enum values
    static const std::unordered_map<std::string_view, DoxyCmd>& CommandMap()
    {
        static const std::unordered_map<std::string_view, DoxyCmd> cmdMap = {
            // Class/file-level metadata
            { "class", DoxyCmd::class_ },
            { "brief", DoxyCmd::brief },
            { "param", DoxyCmd::param },
            { "return", DoxyCmd::return_ },
            { "returns", DoxyCmd::returns },
            { "see", DoxyCmd::see },
            { "sa", DoxyCmd::sa },
            { "ref", DoxyCmd::ref },
            { "since", DoxyCmd::since },
            { "code", DoxyCmd::code },
            { "endcode", DoxyCmd::end_code },
            { "library", DoxyCmd::library },
            { "category", DoxyCmd::category },
            { "appearance", DoxyCmd::appearance },
            { "header", DoxyCmd::header },
            { "note", DoxyCmd::note },
            { "warning", DoxyCmd::warning },
            { "deprecated", DoxyCmd::deprecated },
            { "todo", DoxyCmd::todo },

            // Inline formatting
            { "a", DoxyCmd::a },
            { "b", DoxyCmd::b },
            { "c", DoxyCmd::c },
            { "p", DoxyCmd::p },
            { "e", DoxyCmd::e },

            // Lists
            { "li", DoxyCmd::li },

            // Style/Event tables
            { "beginStyleTable", DoxyCmd::begin_style_table },
            { "endStyleTable", DoxyCmd::end_style_table },
            { "style", DoxyCmd::style },
            { "beginEventTable", DoxyCmd::begin_event_table },
            { "beginEventEmissionTable", DoxyCmd::begin_event_emission_table },
            { "endEventTable", DoxyCmd::end_event_table },
            { "event", DoxyCmd::event },

            // Generic tables
            { "beginTable", DoxyCmd::begin_table },
            { "endTable", DoxyCmd::end_table },
            { "row2col", DoxyCmd::row_2_col },
            { "row3col", DoxyCmd::row_3_col },

            // Definition lists
            { "beginDefList", DoxyCmd::begin_def_list },
            { "endDefList", DoxyCmd::end_def_list },
            { "itemdef", DoxyCmd::item_def },

            // Grouping
            { "name", DoxyCmd::name },
            { "addtogroup", DoxyCmd::add_to_group },

            // Language-specific skips
            { "beginWxPerlOnly", DoxyCmd::begin_wx_perl_only },
            { "endWxPerlOnly", DoxyCmd::end_wx_perl_only },
            { "beginWxPythonOnly", DoxyCmd::begin_wx_python_only },
            { "endWxPythonOnly", DoxyCmd::end_wx_python_only },

            // Aliases
            { "true", DoxyCmd::true_ },
            { "false", DoxyCmd::false_ },
            { "NULL", DoxyCmd::null },

            // Section headings
            { "section", DoxyCmd::section },
            { "subsection", DoxyCmd::subsection },

            // Platform-specific
            { "onlyfor", DoxyCmd::only_for },
            { "nativeimpl", DoxyCmd::native_impl },

            // Remarks
            { "remark", DoxyCmd::remark },
            { "remarks", DoxyCmd::remarks },
        };
        return cmdMap;
    }

    std::pair<DoxyCmd, std::string_view> ParseCommand(std::string_view text)
    {
        // text should start with '@' or '\'
        if (text.empty() || (text[0] != '@' && text[0] != '\\'))
        {
            return { DoxyCmd::unknown, text };
        }

        // Find end of command word
        constexpr size_t start = 1;
        size_t endPos = start;
        while (endPos < text.size() &&
               (std::isalnum(static_cast<unsigned char>(text[endPos])) || text[endPos] == '_'))
        {
            ++endPos;
        }

        if (endPos == start)
        {
            return { DoxyCmd::unknown, text };
        }

        const std::string_view cmd_name = text.substr(start, endPos - start);
        std::string_view rest = text.substr(endPos);

        // Strip leading whitespace from rest
        while (!rest.empty() && rest[0] == ' ')
        {
            rest.remove_prefix(1);
        }

        const std::unordered_map<std::string_view, DoxyCmd>& cmdMap = CommandMap();
        const std::unordered_map<std::string_view, DoxyCmd>::const_iterator iter =
            cmdMap.find(cmd_name);
        if (iter != cmdMap.end())
        {
            return { iter->second, rest };
        }

        return { DoxyCmd::unknown, rest };
    }

    std::string ExpandAlias(DoxyCmd command)
    {
        switch (command)
        {
            case DoxyCmd::true_:
                return "**true**";
            case DoxyCmd::false_:
                return "**false**";
            case DoxyCmd::null:
                return "**NULL**";
            default:
                return {};
        }
    }

    [[nodiscard]] bool IsSkipBlockStart(DoxyCmd command)
    {
        return command == DoxyCmd::begin_wx_perl_only || command == DoxyCmd::begin_wx_python_only;
    }

    [[nodiscard]] bool IsSkipBlockEnd(DoxyCmd command)
    {
        return command == DoxyCmd::end_wx_perl_only || command == DoxyCmd::end_wx_python_only;
    }

}  // namespace docparser
