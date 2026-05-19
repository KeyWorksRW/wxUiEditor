/////////////////////////////////////////////////////////////////////////////
// Purpose:   Parse doxygen comment blocks into DocComment
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "comment_parser.h"
#include "doxy_commands.h"

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace docparser
{

    namespace
    {

        // ---------------------------------------------------------------------------
        // Helpers
        // ---------------------------------------------------------------------------

        // Strip leading '*', whitespace, and comment markers from a Javadoc line.
        auto StripCommentLine(std::string_view line) -> std::string
        {
            // Skip leading whitespace
            size_t pos = 0;
            while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t'))
            {
                ++pos;
            }

            // Skip optional leading '*'
            if (pos < line.size() && line[pos] == '*')
            {
                ++pos;
                // Skip one space after '*' if present
                if (pos < line.size() && line[pos] == ' ')
                {
                    ++pos;
                }
            }

            return std::string(line.substr(pos));
        }

        // Split text into lines
        auto SplitLines(std::string_view text) -> std::vector<std::string>
        {
            std::vector<std::string> lines;
            size_t start = 0;
            while (start < text.size())
            {
                const size_t newline_pos = text.find('\n', start);
                if (newline_pos == std::string_view::npos)
                {
                    lines.emplace_back(text.substr(start));
                    break;
                }
                std::string_view line = text.substr(start, newline_pos - start);
                // Strip trailing \r
                if (!line.empty() && line.back() == '\r')
                {
                    line.remove_suffix(1);
                }
                lines.emplace_back(line);
                start = newline_pos + 1;
            }
            return lines;
        }

        // Trim leading and trailing whitespace
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

        // Extract braced argument: "{foo}" -> "foo", advancing pos past '}'.
        // Returns empty if no braces found.
        auto ExtractBraced(std::string_view text) -> std::string
        {
            if (text.empty() || text[0] != '{')
            {
                return {};
            }
            const size_t close = text.find('}', 1);
            if (close == std::string_view::npos)
            {
                return {};
            }
            return std::string(text.substr(1, close - 1));
        }

        // Get position after braced argument (past the '}')
        auto AfterBraced(std::string_view text) -> size_t
        {
            if (text.empty() || text[0] != '{')
            {
                return 0;
            }
            const size_t close = text.find('}', 1);
            if (close == std::string_view::npos)
            {
                return 0;
            }
            return close + 1;
        }

        // Process inline formatting commands in a line of text.
        // Converts @a/@e -> italic, @b -> bold, @c/@p -> code, @ref -> link placeholder,
        // @true/@false/@NULL -> expanded aliases.
        auto ProcessInlineCommands(std::string_view text) -> std::string
        {
            std::string result;
            result.reserve(text.size());

            size_t offset = 0;
            while (offset < text.size())
            {
                if ((text[offset] == '@' || text[offset] == '\\') && offset + 1 < text.size() &&
                    std::isalpha(static_cast<unsigned char>(text[offset + 1])))
                {
                    const auto [cmd, rest] = ParseCommand(text.substr(offset));

                    switch (cmd)
                    {
                        case DoxyCmd::a:
                        case DoxyCmd::e:
                            {
                                // Next word becomes italic
                                size_t word_end = 0;
                                while (word_end < rest.size() &&
                                       !std::isspace(static_cast<unsigned char>(rest[word_end])) &&
                                       rest[word_end] != ',' && rest[word_end] != '.' &&
                                       rest[word_end] != ')' && rest[word_end] != ';')
                                {
                                    ++word_end;
                                }
                                if (word_end > 0)
                                {
                                    result += '*';
                                    result += rest.substr(0, word_end);
                                    result += '*';
                                    offset =
                                        static_cast<size_t>((rest.data() + word_end) - text.data());
                                }
                                else
                                {
                                    result += text[offset];
                                    ++offset;
                                }
                                break;
                            }
                        case DoxyCmd::b:
                            {
                                // Next word becomes bold
                                size_t word_end = 0;
                                while (word_end < rest.size() &&
                                       !std::isspace(static_cast<unsigned char>(rest[word_end])) &&
                                       rest[word_end] != ',' && rest[word_end] != '.' &&
                                       rest[word_end] != ')' && rest[word_end] != ';')
                                {
                                    ++word_end;
                                }
                                if (word_end > 0)
                                {
                                    result += "**";
                                    result += rest.substr(0, word_end);
                                    result += "**";
                                    offset =
                                        static_cast<size_t>((rest.data() + word_end) - text.data());
                                }
                                else
                                {
                                    result += text[offset];
                                    ++offset;
                                }
                                break;
                            }
                        case DoxyCmd::c:
                        case DoxyCmd::p:
                            {
                                // Next word becomes code
                                size_t word_end = 0;
                                while (word_end < rest.size() &&
                                       !std::isspace(static_cast<unsigned char>(rest[word_end])) &&
                                       rest[word_end] != ',' && rest[word_end] != '.' &&
                                       rest[word_end] != ')' && rest[word_end] != ';')
                                {
                                    ++word_end;
                                }
                                if (word_end > 0)
                                {
                                    result += '`';
                                    result += rest.substr(0, word_end);
                                    result += '`';
                                    offset =
                                        static_cast<size_t>((rest.data() + word_end) - text.data());
                                }
                                else
                                {
                                    result += text[offset];
                                    ++offset;
                                }
                                break;
                            }
                        case DoxyCmd::ref:
                            {
                                // @ref Name "display" or @ref Name
                                size_t word_end = 0;
                                while (word_end < rest.size() &&
                                       !std::isspace(static_cast<unsigned char>(rest[word_end])) &&
                                       rest[word_end] != ',' && rest[word_end] != '.' &&
                                       rest[word_end] != ')' && rest[word_end] != ';')
                                {
                                    ++word_end;
                                }
                                if (word_end > 0)
                                {
                                    const std::string_view target = rest.substr(0, word_end);
                                    // Check for optional quoted display name
                                    const std::string_view after = rest.substr(word_end);
                                    size_t skip_ws = 0;
                                    while (skip_ws < after.size() && after[skip_ws] == ' ')
                                    {
                                        ++skip_ws;
                                    }
                                    if (skip_ws < after.size() && after[skip_ws] == '"')
                                    {
                                        const size_t quote_end = after.find('"', skip_ws + 1);
                                        if (quote_end != std::string_view::npos)
                                        {
                                            const std::string_view display =
                                                after.substr(skip_ws + 1, quote_end - skip_ws - 1);
                                            result += '[';
                                            result += display;
                                            result += "](";
                                            result += target;
                                            result += ')';
                                            offset = static_cast<size_t>(
                                                (after.data() + quote_end + 1) - text.data());
                                            break;
                                        }
                                    }
                                    // No display name — use target as text
                                    result += '[';
                                    result += target;
                                    result += "](";
                                    result += target;
                                    result += ')';
                                    offset =
                                        static_cast<size_t>((rest.data() + word_end) - text.data());
                                }
                                else
                                {
                                    result += text[offset];
                                    ++offset;
                                }
                                break;
                            }
                        case DoxyCmd::true_:
                        case DoxyCmd::false_:
                        case DoxyCmd::null:
                            {
                                result += ExpandAlias(cmd);
                                // ParseCommand strips leading whitespace from rest;
                                // re-insert a space so "@true if" becomes "**true** if" not
                                // "**true**if".
                                if (!rest.empty())
                                {
                                    result += ' ';
                                }
                                offset = static_cast<size_t>(rest.data() - text.data());
                                break;
                            }
                        default:
                            {
                                // Unknown inline command — pass through as-is
                                result += text[offset];
                                ++offset;
                                break;
                            }
                    }
                }
                else
                {
                    result += text[offset];
                    ++offset;
                }
            }

            return result;
        }

        // Check if a line starts with a doxygen command (@ or \)
        [[nodiscard]] auto StartsWithCommand(std::string_view line) -> bool
        {
            const std::string trimmed = Trim(line);
            return !trimmed.empty() && (trimmed[0] == '@' || trimmed[0] == '\\');
        }

        // Parse a command from the beginning of a line (skips leading whitespace in-place).
        // Returns views into the original line's backing memory — caller must ensure it stays
        // alive.
        auto ParseLineCommand(std::string_view line) -> std::pair<DoxyCmd, std::string_view>
        {
            // Skip leading whitespace IN the view (no temporary string)
            while (!line.empty() && (line[0] == ' ' || line[0] == '\t'))
            {
                line.remove_prefix(1);
            }
            if (line.empty() || (line[0] != '@' && line[0] != '\\'))
            {
                return { DoxyCmd::unknown, line };
            }
            return ParseCommand(line);
        }

    }  // anonymous namespace

    // ---------------------------------------------------------------------------
    // Main entry point
    // ---------------------------------------------------------------------------

    auto ParseCommentBlock(std::string_view text) -> DocComment
    {
        DocComment result;

        std::vector<std::string> lines = SplitLines(text);

        // Strip Javadoc comment markers
        std::vector<std::string> cleaned;
        cleaned.reserve(lines.size());
        for (std::string& line: lines)
        {
            cleaned.push_back(StripCommentLine(line));
        }

        // State machine
        enum class State : std::uint8_t
        {
            Normal,
            InCode,
            InStyleTable,
            InEventTable,
            InTable,
            InDefList,
            InSkipBlock,  // Perl/Python only blocks
        };

        State state = State::Normal;
        bool have_brief = false;
        std::string current_text;      // accumulator for the current block of text
        std::string code_lang;         // language for current code block
        std::string code_content;      // accumulated code block content
        std::string event_table_type;  // type from @beginEventEmissionTable{type}

        // Accumulators for multi-line @param, @note, @warning, etc.
        std::string current_param_name;
        std::string current_param_desc;
        bool in_param = false;
        bool in_return = false;
        bool in_since = false;
        bool in_deprecated = false;
        bool in_note = false;
        bool in_warning = false;
        bool in_todo = false;
        bool in_remark = false;
        bool in_ordered_list = false;

        // Flush current param if any
        auto FlushParam = [&]() -> void
        {
            if (in_param && !current_param_name.empty())
            {
                result.params.push_back({ current_param_name, Trim(current_param_desc) });
            }
            in_param = false;
            current_param_name.clear();
            current_param_desc.clear();
        };

        // Flush current multi-line block (note, warning, etc.)
        auto FlushBlock = [&]() -> void
        {
            FlushParam();
            in_ordered_list = false;
            if (in_return)
            {
                result.returns = Trim(current_text);
                in_return = false;
            }
            if (in_since)
            {
                result.since = Trim(current_text);
                in_since = false;
            }
            if (in_deprecated)
            {
                result.deprecated = Trim(current_text);
                in_deprecated = false;
            }
            if (in_note)
            {
                std::string block_text = Trim(current_text);
                if (!block_text.empty())
                {
                    result.notes.push_back(block_text);
                }
                in_note = false;
            }
            if (in_warning)
            {
                std::string block_text = Trim(current_text);
                if (!block_text.empty())
                {
                    result.warnings.push_back(block_text);
                }
                in_warning = false;
            }
            if (in_todo)
            {
                std::string block_text = Trim(current_text);
                if (!block_text.empty())
                {
                    result.todos.push_back(block_text);
                }
                in_todo = false;
            }
            if (in_remark)
            {
                // Treat remarks like notes
                std::string block_text = Trim(current_text);
                if (!block_text.empty())
                {
                    result.notes.push_back(block_text);
                }
                in_remark = false;
            }
            current_text.clear();
        };

        for (size_t line_idx = 0; line_idx < cleaned.size(); ++line_idx)
        {
            const std::string& line = cleaned[line_idx];
            const std::string trimmed = Trim(line);

            // --- Skip block handling ---
            if (state == State::InSkipBlock)
            {
                const auto [cmd, rest] = ParseLineCommand(trimmed);
                if (IsSkipBlockEnd(cmd))
                {
                    state = State::Normal;
                }
                continue;
            }

            // --- Code block handling ---
            if (state == State::InCode)
            {
                const auto [cmd, rest] = ParseLineCommand(trimmed);
                if (cmd == DoxyCmd::end_code)
                {
                    result.code_blocks.push_back({ code_lang, code_content });
                    code_lang.clear();
                    code_content.clear();
                    state = State::Normal;
                }
                else
                {
                    if (!code_content.empty())
                    {
                        code_content += '\n';
                    }
                    code_content += line;
                }
                continue;
            }

            // --- Style table handling ---
            if (state == State::InStyleTable)
            {
                const auto [cmd, rest] = ParseLineCommand(trimmed);
                if (cmd == DoxyCmd::end_style_table)
                {
                    state = State::Normal;
                    continue;
                }
                if (cmd == DoxyCmd::style)
                {
                    // @style{name} description on this + following lines until next @style or
                    // @endStyleTable
                    const std::string name = ExtractBraced(rest);
                    const size_t after = AfterBraced(rest);
                    std::string desc;
                    if (after < rest.size())
                    {
                        desc = Trim(rest.substr(after));
                    }
                    // Read continuation lines
                    while (line_idx + 1 < cleaned.size())
                    {
                        const std::string next_trimmed = Trim(cleaned[line_idx + 1]);
                        const auto [next_cmd, ignored] = ParseLineCommand(next_trimmed);
                        if (next_cmd == DoxyCmd::style || next_cmd == DoxyCmd::end_style_table)
                        {
                            break;
                        }
                        if (!desc.empty())
                        {
                            desc += ' ';
                        }
                        desc += ProcessInlineCommands(next_trimmed);
                        ++line_idx;
                    }
                    result.styles.push_back({ name, Trim(desc) });
                }
                continue;
            }

            // --- Event table handling ---
            if (state == State::InEventTable)
            {
                const auto [cmd, rest] = ParseLineCommand(trimmed);
                if (cmd == DoxyCmd::end_event_table)
                {
                    state = State::Normal;
                    continue;
                }
                if (cmd == DoxyCmd::event)
                {
                    const std::string name = ExtractBraced(rest);
                    const size_t after = AfterBraced(rest);
                    std::string desc;
                    if (after < rest.size())
                    {
                        desc = Trim(rest.substr(after));
                    }
                    // Read continuation lines
                    while (line_idx + 1 < cleaned.size())
                    {
                        const std::string next_trimmed = Trim(cleaned[line_idx + 1]);
                        const auto [next_cmd, ignored] = ParseLineCommand(next_trimmed);
                        if (next_cmd == DoxyCmd::event || next_cmd == DoxyCmd::end_event_table)
                        {
                            break;
                        }
                        if (!desc.empty())
                        {
                            desc += ' ';
                        }
                        desc += ProcessInlineCommands(next_trimmed);
                        ++line_idx;
                    }
                    EventEntry entry;
                    entry.name = name;
                    entry.description = Trim(desc);
                    entry.event_type = event_table_type;
                    result.events.push_back(std::move(entry));
                }
                continue;
            }

            // --- Generic table handling ---
            if (state == State::InTable)
            {
                const auto [cmd, rest] = ParseLineCommand(trimmed);
                if (cmd == DoxyCmd::end_table)
                {
                    state = State::Normal;
                    continue;
                }
                if (cmd == DoxyCmd::row_2_col || cmd == DoxyCmd::row_3_col)
                {
                    // @row2col{col1, col2} or @row3col{col1, col2, col3}
                    // The braced content contains comma-separated columns
                    std::string braced = ExtractBraced(rest);
                    TableRow row;
                    // Simple split by first comma (or first two commas for row3col)
                    // These are often multi-line, so also gather continuation lines
                    const size_t after = AfterBraced(rest);
                    if (after < rest.size())
                    {
                        std::string extra = Trim(rest.substr(after));
                        if (!extra.empty())
                        {
                            if (!braced.empty())
                            {
                                braced += ' ';
                            }
                            braced += extra;
                        }
                    }
                    // Read continuation lines until next @row or @endTable
                    while (line_idx + 1 < cleaned.size())
                    {
                        const std::string next_trimmed = Trim(cleaned[line_idx + 1]);
                        const auto [next_cmd, ignored] = ParseLineCommand(next_trimmed);
                        if (next_cmd == DoxyCmd::row_2_col || next_cmd == DoxyCmd::row_3_col ||
                            next_cmd == DoxyCmd::end_table)
                        {
                            break;
                        }
                        if (!braced.empty())
                        {
                            braced += ' ';
                        }
                        braced += next_trimmed;
                        ++line_idx;
                    }
                    // For now, store the whole braced content as a single column
                    // TODO: improve column splitting for complex cases
                    row.columns.push_back(ProcessInlineCommands(braced));
                    result.tables.push_back(std::move(row));
                }
                continue;
            }

            // --- Definition list handling ---
            if (state == State::InDefList)
            {
                const auto [cmd, rest] = ParseLineCommand(trimmed);
                if (cmd == DoxyCmd::end_def_list)
                {
                    state = State::Normal;
                    continue;
                }
                if (cmd == DoxyCmd::item_def)
                {
                    const std::string braced = ExtractBraced(rest);
                    const size_t after = AfterBraced(rest);
                    std::string def;
                    if (after < rest.size())
                    {
                        def = Trim(rest.substr(after));
                    }
                    // Read continuation lines
                    while (line_idx + 1 < cleaned.size())
                    {
                        const std::string next_trimmed = Trim(cleaned[line_idx + 1]);
                        const auto [next_cmd, ignored] = ParseLineCommand(next_trimmed);
                        if (next_cmd == DoxyCmd::item_def || next_cmd == DoxyCmd::end_def_list)
                        {
                            break;
                        }
                        if (!def.empty())
                        {
                            def += ' ';
                        }
                        def += ProcessInlineCommands(next_trimmed);
                        ++line_idx;
                    }
                    result.def_list.push_back({ ProcessInlineCommands(braced), Trim(def) });
                }
                continue;
            }

            // --- Normal state ---
            const auto [cmd, rest] = ParseLineCommand(trimmed);

            switch (cmd)
            {
                case DoxyCmd::class_:
                    {
                        FlushBlock();
                        // @class ClassName
                        size_t word_end = 0;
                        while (word_end < rest.size() &&
                               !std::isspace(static_cast<unsigned char>(rest[word_end])))
                        {
                            ++word_end;
                        }
                        result.class_name = std::string(rest.substr(0, word_end));
                        continue;
                    }
                case DoxyCmd::brief:
                    {
                        FlushBlock();
                        have_brief = true;
                        result.brief = ProcessInlineCommands(rest);
                        // Read continuation lines until blank or next command
                        while (line_idx + 1 < cleaned.size())
                        {
                            const std::string next_trimmed = Trim(cleaned[line_idx + 1]);
                            if (next_trimmed.empty())
                            {
                                break;
                            }
                            if (StartsWithCommand(next_trimmed))
                            {
                                break;
                            }
                            result.brief += ' ';
                            result.brief += ProcessInlineCommands(next_trimmed);
                            ++line_idx;
                        }
                        continue;
                    }
                case DoxyCmd::param:
                    {
                        FlushBlock();
                        in_param = true;
                        // @param [dir] name description — skip optional direction qualifier
                        std::string_view param_rest = rest;
                        if (!param_rest.empty() && param_rest[0] == '[')
                        {
                            const size_t close = param_rest.find(']');
                            if (close != std::string_view::npos)
                            {
                                param_rest.remove_prefix(close + 1);
                                while (!param_rest.empty() && param_rest[0] == ' ')
                                {
                                    param_rest.remove_prefix(1);
                                }
                            }
                        }
                        size_t word_end = 0;
                        while (word_end < param_rest.size() &&
                               !std::isspace(static_cast<unsigned char>(param_rest[word_end])))
                        {
                            ++word_end;
                        }
                        current_param_name = std::string(param_rest.substr(0, word_end));
                        const auto desc_start = param_rest.substr(word_end);
                        current_param_desc = ProcessInlineCommands(Trim(desc_start));
                        // Read continuation lines
                        while (line_idx + 1 < cleaned.size())
                        {
                            const std::string next_trimmed = Trim(cleaned[line_idx + 1]);
                            if (next_trimmed.empty())
                            {
                                break;
                            }
                            if (StartsWithCommand(next_trimmed))
                            {
                                break;
                            }
                            current_param_desc += ' ';
                            current_param_desc += ProcessInlineCommands(next_trimmed);
                            ++line_idx;
                        }
                        FlushParam();
                        continue;
                    }
                case DoxyCmd::return_:
                case DoxyCmd::returns:
                    {
                        FlushBlock();
                        in_return = true;
                        current_text = ProcessInlineCommands(rest);
                        // Read continuation lines
                        while (line_idx + 1 < cleaned.size())
                        {
                            const std::string next_trimmed = Trim(cleaned[line_idx + 1]);
                            if (next_trimmed.empty())
                            {
                                break;
                            }
                            if (StartsWithCommand(next_trimmed))
                            {
                                break;
                            }
                            current_text += ' ';
                            current_text += ProcessInlineCommands(next_trimmed);
                            ++line_idx;
                        }
                        FlushBlock();
                        continue;
                    }
                case DoxyCmd::see:
                case DoxyCmd::sa:
                    {
                        FlushBlock();
                        // @see Ref1, Ref2, ...
                        std::string refs_text(rest);
                        // Read continuation lines
                        while (line_idx + 1 < cleaned.size())
                        {
                            const std::string next_trimmed = Trim(cleaned[line_idx + 1]);
                            if (next_trimmed.empty())
                            {
                                break;
                            }
                            if (StartsWithCommand(next_trimmed))
                            {
                                break;
                            }
                            refs_text += ' ';
                            refs_text += next_trimmed;
                            ++line_idx;
                        }
                        // Split by commas and/or spaces
                        std::istringstream iss(refs_text);
                        std::string token;
                        while (iss >> token)
                        {
                            // Remove trailing commas
                            while (!token.empty() && token.back() == ',')
                            {
                                token.pop_back();
                            }
                            if (!token.empty())
                            {
                                result.see_also.push_back(token);
                            }
                        }
                        continue;
                    }
                case DoxyCmd::since:
                    {
                        FlushBlock();
                        in_since = true;
                        current_text = ProcessInlineCommands(rest);
                        // Read continuation lines
                        while (line_idx + 1 < cleaned.size())
                        {
                            const std::string next_trimmed = Trim(cleaned[line_idx + 1]);
                            if (next_trimmed.empty())
                            {
                                break;
                            }
                            if (StartsWithCommand(next_trimmed))
                            {
                                break;
                            }
                            current_text += ' ';
                            current_text += next_trimmed;
                            ++line_idx;
                        }
                        FlushBlock();
                        continue;
                    }
                case DoxyCmd::deprecated:
                    {
                        FlushBlock();
                        in_deprecated = true;
                        current_text = ProcessInlineCommands(rest);
                        while (line_idx + 1 < cleaned.size())
                        {
                            const std::string next_trimmed = Trim(cleaned[line_idx + 1]);
                            if (next_trimmed.empty())
                            {
                                break;
                            }
                            if (StartsWithCommand(next_trimmed))
                            {
                                break;
                            }
                            current_text += ' ';
                            current_text += ProcessInlineCommands(next_trimmed);
                            ++line_idx;
                        }
                        FlushBlock();
                        continue;
                    }
                case DoxyCmd::note:
                    {
                        FlushBlock();
                        in_note = true;
                        current_text = ProcessInlineCommands(rest);
                        while (line_idx + 1 < cleaned.size())
                        {
                            const std::string next_trimmed = Trim(cleaned[line_idx + 1]);
                            if (next_trimmed.empty())
                            {
                                break;
                            }
                            if (StartsWithCommand(next_trimmed))
                            {
                                break;
                            }
                            current_text += ' ';
                            current_text += ProcessInlineCommands(next_trimmed);
                            ++line_idx;
                        }
                        FlushBlock();
                        continue;
                    }
                case DoxyCmd::warning:
                    {
                        FlushBlock();
                        in_warning = true;
                        current_text = ProcessInlineCommands(rest);
                        while (line_idx + 1 < cleaned.size())
                        {
                            const std::string next_trimmed = Trim(cleaned[line_idx + 1]);
                            if (next_trimmed.empty())
                            {
                                break;
                            }
                            if (StartsWithCommand(next_trimmed))
                            {
                                break;
                            }
                            current_text += ' ';
                            current_text += ProcessInlineCommands(next_trimmed);
                            ++line_idx;
                        }
                        FlushBlock();
                        continue;
                    }
                case DoxyCmd::todo:
                    {
                        FlushBlock();
                        in_todo = true;
                        current_text = ProcessInlineCommands(rest);
                        while (line_idx + 1 < cleaned.size())
                        {
                            const std::string next_trimmed = Trim(cleaned[line_idx + 1]);
                            if (next_trimmed.empty())
                            {
                                break;
                            }
                            if (StartsWithCommand(next_trimmed))
                            {
                                break;
                            }
                            current_text += ' ';
                            current_text += ProcessInlineCommands(next_trimmed);
                            ++line_idx;
                        }
                        FlushBlock();
                        continue;
                    }
                case DoxyCmd::remark:
                case DoxyCmd::remarks:
                    {
                        FlushBlock();
                        in_remark = true;
                        current_text = ProcessInlineCommands(rest);
                        while (line_idx + 1 < cleaned.size())
                        {
                            const std::string next_trimmed = Trim(cleaned[line_idx + 1]);
                            if (next_trimmed.empty())
                            {
                                break;
                            }
                            if (StartsWithCommand(next_trimmed))
                            {
                                break;
                            }
                            current_text += ' ';
                            current_text += ProcessInlineCommands(next_trimmed);
                            ++line_idx;
                        }
                        FlushBlock();
                        continue;
                    }
                case DoxyCmd::code:
                    {
                        FlushBlock();
                        state = State::InCode;
                        // Check for language spec: @code{.cpp}
                        code_lang.clear();
                        if (!rest.empty() && rest[0] == '{')
                        {
                            std::string lang = ExtractBraced(rest);
                            if (!lang.empty() && lang[0] == '.')
                            {
                                code_lang = lang.substr(1);
                            }
                            else
                            {
                                code_lang = lang;
                            }
                        }
                        code_content.clear();
                        continue;
                    }
                case DoxyCmd::begin_style_table:
                    {
                        FlushBlock();
                        state = State::InStyleTable;
                        continue;
                    }
                case DoxyCmd::begin_event_table:
                case DoxyCmd::begin_event_emission_table:
                    {
                        FlushBlock();
                        state = State::InEventTable;
                        event_table_type = ExtractBraced(rest);
                        continue;
                    }
                case DoxyCmd::begin_table:
                    {
                        FlushBlock();
                        state = State::InTable;
                        continue;
                    }
                case DoxyCmd::begin_def_list:
                    {
                        FlushBlock();
                        state = State::InDefList;
                        continue;
                    }
                case DoxyCmd::library:
                    {
                        result.library = ExtractBraced(rest);
                        continue;
                    }
                case DoxyCmd::category:
                    {
                        result.category = ExtractBraced(rest);
                        continue;
                    }
                case DoxyCmd::appearance:
                    {
                        result.appearance = ExtractBraced(rest);
                        continue;
                    }
                case DoxyCmd::header:
                    {
                        result.header_file = ExtractBraced(rest);
                        continue;
                    }
                case DoxyCmd::add_to_group:
                    {
                        size_t word_end = 0;
                        while (word_end < rest.size() &&
                               !std::isspace(static_cast<unsigned char>(rest[word_end])))
                        {
                            ++word_end;
                        }
                        result.group_name = std::string(rest.substr(0, word_end));
                        continue;
                    }
                case DoxyCmd::li:
                    {
                        // @li content -> append to detailed with bullet marker
                        if (!result.detailed.empty())
                        {
                            result.detailed += '\n';
                        }
                        result.detailed += "- ";
                        result.detailed += ProcessInlineCommands(rest);
                        // Read continuation lines
                        while (line_idx + 1 < cleaned.size())
                        {
                            const std::string next_trimmed = Trim(cleaned[line_idx + 1]);
                            if (next_trimmed.empty())
                            {
                                break;
                            }
                            if (StartsWithCommand(next_trimmed))
                            {
                                break;
                            }
                            result.detailed += ' ';
                            result.detailed += ProcessInlineCommands(next_trimmed);
                            ++line_idx;
                        }
                        continue;
                    }
                case DoxyCmd::section:
                case DoxyCmd::subsection:
                    {
                        // Section headings — add as markdown heading in detailed text
                        if (!result.detailed.empty())
                        {
                            result.detailed += "\n\n";
                        }
                        result.detailed += (cmd == DoxyCmd::section) ? "## " : "### ";
                        // @section id Title text
                        // skip the ID word, get the title
                        size_t word_end = 0;
                        while (word_end < rest.size() &&
                               !std::isspace(static_cast<unsigned char>(rest[word_end])))
                        {
                            ++word_end;
                        }
                        const auto title_part = Trim(rest.substr(word_end));
                        result.detailed += ProcessInlineCommands(title_part);
                        continue;
                    }
                case DoxyCmd::only_for:
                case DoxyCmd::native_impl:
                case DoxyCmd::name:
                    {
                        // Metadata we note but don't act on right now
                        continue;
                    }
                default:
                    {
                        // Check for skip block starts
                        if (IsSkipBlockStart(cmd))
                        {
                            FlushBlock();
                            state = State::InSkipBlock;
                            continue;
                        }
                        break;
                    }
            }

            // --- Doxygen ordered list items: -# text ---
            if (trimmed.starts_with("-#"))
            {
                std::string_view item_text = std::string_view(trimmed).substr(2);
                while (!item_text.empty() && item_text[0] == ' ')
                {
                    item_text.remove_prefix(1);
                }
                if (!result.detailed.empty())
                {
                    result.detailed += in_ordered_list ? "\n" : "\n\n";
                }
                result.detailed += "1. ";
                result.detailed += ProcessInlineCommands(item_text);
                // Read continuation lines
                while (line_idx + 1 < cleaned.size())
                {
                    const std::string next_trimmed = Trim(cleaned[line_idx + 1]);
                    if (next_trimmed.empty())
                    {
                        break;
                    }
                    if (StartsWithCommand(next_trimmed) || next_trimmed.starts_with("-#"))
                    {
                        break;
                    }
                    result.detailed += ' ';
                    result.detailed += ProcessInlineCommands(next_trimmed);
                    ++line_idx;
                }
                in_ordered_list = true;
                continue;
            }
            in_ordered_list = false;

            // --- Normal text processing ---
            // If we haven't extracted a brief yet, the first non-empty paragraph becomes the brief
            if (trimmed.empty())
            {
                FlushBlock();
                if (!have_brief && !result.detailed.empty())
                {
                    // First paragraph becomes brief if @brief wasn't used
                    result.brief = result.detailed;
                    result.detailed.clear();
                    have_brief = true;
                }
                else if (have_brief && !result.detailed.empty() &&
                         !result.detailed.ends_with("\n\n"))
                {
                    result.detailed += "\n\n";
                }
                continue;
            }

            // Append to detailed text
            const std::string processed = ProcessInlineCommands(trimmed);
            if (result.detailed.empty() || result.detailed.ends_with("\n\n") ||
                result.detailed.ends_with("\n"))
            {
                result.detailed += processed;
            }
            else
            {
                result.detailed += ' ';
                result.detailed += processed;
            }
        }

        // Final flush
        FlushBlock();

        // If no explicit @brief, extract first sentence from detailed
        if (!have_brief && !result.detailed.empty())
        {
            result.brief = result.detailed;
            result.detailed.clear();
        }

        return result;
    }

}  // namespace docparser
