/////////////////////////////////////////////////////////////////////////////
// Purpose:   Write parsed documentation as Markdown files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "markdown_writer.h"
#include "parser_reporter.h"

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#else
    #include <sys/wait.h>
    #include <unistd.h>
#endif

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

namespace docparser
{

    // ---------------------------------------------------------------------------
    // Helpers
    // ---------------------------------------------------------------------------

    // Convert a class name to its output .md filename, preserving case.
    // A nested compound keeps its qualification as a dot:
    // "wxButton" -> "wxButton.md", "wxImage::HSVValue" -> "wxImage.HSVValue.md".
    std::string ClassNameToFile(const std::string& name)
    {
        std::string filename = name;
        size_t separator = 0;
        while ((separator = filename.find("::", separator)) != std::string::npos)
        {
            filename.replace(separator, 2, ".");
            ++separator;
        }
        return filename + ".md";
    }

    // Generate a stable anchor id from heading text.
    // Mirrors tests/scripts/audit_markdown.ps1 ConvertTo-Anchor and the viewer's
    // HeadingTextToId: lowercase, markdown syntax stripped, runs outside [a-z0-9_-]
    // collapsed to '-', leading/trailing hyphens trimmed.
    static std::string AnchorForHeading(std::string_view heading_text)
    {
        std::string plain;
        plain.reserve(heading_text.size());
        for (size_t index = 0; index < heading_text.size(); ++index)
        {
            const char character = heading_text[index];
            if (character == '[')
            {
                // Keep the link text but drop the (...) destination.
                const size_t close_bracket = heading_text.find(']', index + 1);
                if (close_bracket != std::string_view::npos)
                {
                    plain.append(heading_text.substr(index + 1, close_bracket - index - 1));
                    index = close_bracket;
                    if (index + 1 < heading_text.size() && heading_text[index + 1] == '(')
                    {
                        const size_t close_paren = heading_text.find(')', index + 2);
                        if (close_paren != std::string_view::npos)
                        {
                            index = close_paren;
                        }
                    }
                    continue;
                }
            }
            if (character == '`' || character == '*' || character == '_')
            {
                continue;
            }
            plain += character;
        }

        std::string result;
        result.reserve(plain.size());
        for (const char character: plain)
        {
            const unsigned char value = static_cast<unsigned char>(character);
            if ((value >= 'a' && value <= 'z') || (value >= '0' && value <= '9') ||
                character == '-' || character == '_')
            {
                result += character;
            }
            else if (value >= 'A' && value <= 'Z')
            {
                result += static_cast<char>(value - 'A' + 'a');
            }
            else if (!result.empty() && result.back() != '-')
            {
                result += '-';
            }
        }
        while (!result.empty() && result.front() == '-')
        {
            result.erase(0, 1);
        }
        while (!result.empty() && result.back() == '-')
        {
            result.pop_back();
        }
        if (result.empty())
        {
            result = "section";
        }
        return result;
    }

    // Remove a template argument list from a class name (wxList<T> -> wxList).
    static std::string StripTemplateArgs(const std::string& name)
    {
        const size_t open = name.find('<');
        if (open == std::string::npos)
        {
            return name;
        }
        const size_t close = name.find('>', open);
        if (close == std::string::npos)
        {
            return name.substr(0, open);
        }
        return name.substr(0, open) + name.substr(close + 1);
    }

    static std::string EscapeProse(std::string_view text);

    // Resolve a cross-reference target to a markdown link destination.
    // Returns an empty string when the target is unknown. A Class::Member target only
    // receives an anchor when the member is documented on the current page; otherwise
    // it links to the class page so that no dead anchor is emitted.
    static std::string ResolveDestination(const std::string& target, const SymbolTable& symbols,
                                          const std::string& current_class,
                                          const std::map<std::string, std::string>* method_anchors)
    {
        if (target.empty())
        {
            return {};
        }
        if (target[0] == '#')
        {
            return target;
        }

        // A nested compound is itself a page, so try the whole target before
        // interpreting a '::' as a class/member separator.
        const std::string whole_class = StripTemplateArgs(target);
        if (symbols.ClassToFile(whole_class).has_value())
        {
            return ClassNameToFile(whole_class);
        }

        // For Class::Nested::Member, prefer the longest registered class prefix
        // (nested member documentation lives on the nested page).
        const size_t first_separator = target.find("::");
        const size_t last_separator = target.rfind("::");
        if (last_separator != std::string::npos && last_separator != first_separator)
        {
            size_t separator = last_separator;
            while (separator != std::string::npos && separator > 0)
            {
                const std::string prefix = StripTemplateArgs(target.substr(0, separator));
                if (symbols.ClassToFile(prefix).has_value())
                {
                    return ClassNameToFile(prefix);
                }
                separator = target.rfind("::", separator - 1);
            }
        }

        const size_t double_colon = target.find("::");
        if (double_colon != std::string::npos)
        {
            const std::string class_name = StripTemplateArgs(target.substr(0, double_colon));
            std::string member_name = target.substr(double_colon + 2);
            if (const size_t nested = member_name.rfind("::"); nested != std::string::npos)
            {
                member_name = member_name.substr(nested + 2);
            }
            if (const size_t paren = member_name.find('('); paren != std::string::npos)
            {
                member_name.resize(paren);
            }
            if (symbols.ClassToFile(class_name).has_value())
            {
                const bool same_page = class_name == current_class;
                const bool known_member =
                    method_anchors != nullptr && method_anchors->contains(member_name);
                if (same_page && known_member)
                {
                    return ClassNameToFile(class_name) + "#" + method_anchors->at(member_name);
                }
                return ClassNameToFile(class_name);
            }
            return {};
        }

        std::string class_name = StripTemplateArgs(target);
        if (class_name.ends_with("()"))
        {
            class_name.resize(class_name.size() - 2);
        }
        if (symbols.ClassToFile(class_name).has_value())
        {
            return ClassNameToFile(class_name);
        }
        if (symbols.IsPageName(target))
        {
            return target + ".md";
        }
        return {};
    }

    // Resolve a cross-reference target to a markdown link (or a code span).
    static std::string ResolveTarget(const std::string& target, const SymbolTable& symbols,
                                     const std::string& current_class,
                                     const std::map<std::string, std::string>* method_anchors)
    {
        const std::string destination =
            ResolveDestination(target, symbols, current_class, method_anchors);
        if (destination.empty())
        {
            return "`" + target + "`";
        }
        return "[" + EscapeProse(target) + "](" + destination + ")";
    }

    // Allocates unique anchors in document order.
    class AnchorAllocator
    {
    public:
        std::string Allocate(const std::string& base)
        {
            const std::string stem = base.empty() ? std::string("section") : base;
            std::string candidate = stem;
            int suffix = 2;
            while (!used_.insert(candidate).second)
            {
                candidate = stem + "-" + std::to_string(suffix);
                ++suffix;
            }
            return candidate;
        }

    private:
        std::set<std::string> used_;
    };

    // Escape prose characters that would otherwise be interpreted as HTML.
    static std::string EscapeProse(std::string_view text)
    {
        std::string result;
        result.reserve(text.size());
        for (const char character: text)
        {
            if (character == '&')
            {
                result += "&amp;";
            }
            else if (character == '<')
            {
                result += "&lt;";
            }
            else if (character == '>')
            {
                result += "&gt;";
            }
            else
            {
                result += character;
            }
        }
        return result;
    }

    // Escape pipe characters in table cell text. Use the HTML entity rather than a
    // backslash escape: the markdown linter counts every '|' character, escaped or not.
    static std::string EscapePipe(const std::string& text)
    {
        std::string result;
        result.reserve(text.size());
        for (const char character: text)
        {
            if (character == '|')
            {
                result += "&#124;";
            }
            else if (character == '\n' || character == '\r')
            {
                // A newline inside a cell would split the table row.
                result += ' ';
            }
            else
            {
                result += character;
            }
        }
        return result;
    }

    // Concatenate inline spans into plain text (no markdown markers).
    static std::string SpansPlainText(const std::vector<InlineSpan>& spans)
    {
        std::string result;
        for (const InlineSpan& span: spans)
        {
            if (span.kind == InlineSpan::Kind::ref)
            {
                result += span.text.empty() ? span.target : span.text;
            }
            else
            {
                result += span.text;
            }
        }
        return result;
    }

    // One-line summary for the summary tables: the brief paragraph if marked,
    // otherwise the first paragraph.
    static std::string BriefText(const DocComment& doc_comment)
    {
        for (const DocBlock& block: doc_comment.blocks)
        {
            if (block.kind == BlockKind::paragraph && block.is_brief)
            {
                return SpansPlainText(block.spans);
            }
        }
        for (const DocBlock& block: doc_comment.blocks)
        {
            if (block.kind == BlockKind::paragraph)
            {
                return SpansPlainText(block.spans);
            }
        }
        return {};
    }

    [[nodiscard]] static std::string FormatCodeBlock(const std::string& code)
    {
        return code;
    }

    // RenderContext carries what block rendering needs to resolve links.
    struct RenderContext
    {
        const SymbolTable& symbols;
        std::string class_name;
        const std::set<std::string>* method_names = nullptr;
        const std::map<std::string, std::string>* method_anchors = nullptr;
        AnchorAllocator* anchors = nullptr;
    };

    // Link only "Name()" at word boundaries. Backtick spans and existing markdown
    // links are left untouched. Never called with heading text.
    [[nodiscard]] static std::string LinkifyMethodCalls(const std::string& text,
                                                        const RenderContext& context)
    {
        if (context.method_names == nullptr || context.method_names->empty() ||
            context.method_anchors == nullptr)
        {
            return text;
        }

        std::string result;
        result.reserve(text.size());

        size_t offset = 0;
        while (offset < text.size())
        {
            if (text[offset] == '`')
            {
                const size_t close = text.find('`', offset + 1);
                if (close != std::string::npos)
                {
                    result.append(text, offset, close - offset + 1);
                    offset = close + 1;
                }
                else
                {
                    result += text[offset];
                    ++offset;
                }
                continue;
            }

            if (text[offset] == '[')
            {
                const size_t close_bracket = text.find(']', offset + 1);
                if (close_bracket != std::string::npos && close_bracket + 1 < text.size() &&
                    text[close_bracket + 1] == '(')
                {
                    const size_t close_paren = text.find(')', close_bracket + 2);
                    if (close_paren != std::string::npos)
                    {
                        result.append(text, offset, close_paren - offset + 1);
                        offset = close_paren + 1;
                        continue;
                    }
                }
                result += text[offset];
                ++offset;
                continue;
            }

            bool matched = false;
            const bool at_boundary =
                (offset == 0) ||
                (!std::isalnum(static_cast<unsigned char>(text[offset - 1])) &&
                 text[offset - 1] != '_' && text[offset - 1] != ':' && text[offset - 1] != '.');
            if (at_boundary)
            {
                for (const std::string& method_name: *context.method_names)
                {
                    const std::string call = method_name + "()";
                    if (text.compare(offset, call.size(), call) != 0)
                    {
                        continue;
                    }
                    const size_t after_pos = offset + call.size();
                    if (after_pos < text.size() &&
                        (std::isalnum(static_cast<unsigned char>(text[after_pos])) ||
                         text[after_pos] == '_'))
                    {
                        continue;
                    }
                    const std::map<std::string, std::string>::const_iterator anchor_iter =
                        context.method_anchors->find(method_name);
                    if (anchor_iter == context.method_anchors->end())
                    {
                        continue;
                    }
                    result += "[" + call + "](#" + anchor_iter->second + ")";
                    offset = after_pos;
                    matched = true;
                    break;
                }
            }
            if (!matched)
            {
                result += text[offset];
                ++offset;
            }
        }
        return result;
    }

    // Render inline spans to a string (without a surrounding paragraph break).
    static std::string RenderSpans(const std::vector<InlineSpan>& spans,
                                   const RenderContext& context, bool linkify)
    {
        std::string result;
        for (const InlineSpan& span: spans)
        {
            if (span.kind == InlineSpan::Kind::text)
            {
                std::string text = EscapeProse(span.text);
                if (linkify)
                {
                    text = LinkifyMethodCalls(text, context);
                }
                result += text;
            }
            else if (span.kind == InlineSpan::Kind::code)
            {
                result += '`';
                result += span.text;
                result += '`';
            }
            else if (span.kind == InlineSpan::Kind::emphasis)
            {
                result += '*';
                result += EscapeProse(span.text);
                result += '*';
            }
            else if (span.kind == InlineSpan::Kind::strong)
            {
                result += "**";
                result += EscapeProse(span.text);
                result += "**";
            }
            else
            {
                const std::string display = span.text.empty() ? span.target : span.text;
                const std::string destination = ResolveDestination(
                    span.target, context.symbols, context.class_name, context.method_anchors);
                if (destination.empty())
                {
                    result += EscapeProse(display);
                }
                else
                {
                    result += "[" + EscapeProse(display) + "](" + destination + ")";
                }
            }
        }
        return result;
    }

    static void WriteSpans(std::ostream& output, const std::vector<InlineSpan>& spans,
                           const RenderContext& context, bool linkify)
    {
        output << RenderSpans(spans, context, linkify);
    }

    // Emit an explicit anchor followed by the blank line that precedes its heading.
    static void WriteAnchor(std::ostream& output, const std::string& id)
    {
        if (id.empty())
        {
            return;
        }
        output << "<a name=\"" << id << "\"></a>\n\n";
    }

    // Render one blockquote value block, prefixing continuation lines with '>'.
    static void WriteQuotedBlock(std::ostream& output, std::string_view label,
                                 const std::vector<InlineSpan>& spans, const RenderContext& context)
    {
        const std::string rendered = RenderSpans(spans, context, true);
        output << "> **" << label << ":** ";
        for (size_t index = 0; index < rendered.size(); ++index)
        {
            if (rendered[index] == '\n')
            {
                output << "\n> ";
            }
            else
            {
                output << rendered[index];
            }
        }
        output << "\n\n";
    }

    // Render the ordered documentation blocks.
    static void WriteBlocks(std::ostream& output, const std::vector<DocBlock>& blocks,
                            const RenderContext& context)
    {
        for (const DocBlock& block: blocks)
        {
            switch (block.kind)
            {
                case BlockKind::paragraph:
                    {
                        WriteSpans(output, block.spans, context, true);
                        output << "\n\n";
                        break;
                    }
                case BlockKind::bullet_list:
                case BlockKind::ordered_list:
                    {
                        output << "\n";
                        for (const std::vector<InlineSpan>& item: block.items)
                        {
                            if (block.kind == BlockKind::bullet_list)
                            {
                                output << "- ";
                            }
                            else
                            {
                                output << "1. ";
                            }
                            WriteSpans(output, item, context, true);
                            output << "\n";
                        }
                        output << "\n";
                        break;
                    }
                case BlockKind::code:
                case BlockKind::verbatim:
                    {
                        output << "```";
                        if (block.kind == BlockKind::verbatim)
                        {
                            output << "text";
                        }
                        else
                        {
                            output << block.language;
                        }
                        output << "\n";
                        output << FormatCodeBlock(block.code);
                        if (!block.code.empty() && !block.code.ends_with('\n'))
                        {
                            output << "\n";
                        }
                        output << "```\n\n";
                        break;
                    }
                case BlockKind::table:
                    {
                        if (block.rows.empty())
                        {
                            break;
                        }
                        size_t column_count = 0;
                        for (const std::vector<std::vector<InlineSpan>>& row: block.rows)
                        {
                            column_count = std::max(column_count, row.size());
                        }
                        if (column_count == 0)
                        {
                            break;
                        }
                        output << "\n";
                        for (size_t row_index = 0; row_index < block.rows.size(); ++row_index)
                        {
                            const std::vector<std::vector<InlineSpan>>& row = block.rows[row_index];
                            output << "|";
                            for (size_t column_index = 0; column_index < column_count;
                                 ++column_index)
                            {
                                std::string cell;
                                if (column_index < row.size())
                                {
                                    cell = RenderSpans(row[column_index], context, false);
                                }
                                std::string single_line;
                                for (const char character: cell)
                                {
                                    if (character == '\n')
                                    {
                                        single_line += ' ';
                                    }
                                    else
                                    {
                                        single_line += character;
                                    }
                                }
                                output << " " << EscapePipe(single_line) << " |";
                            }
                            output << "\n";
                            if (row_index == 0)
                            {
                                output << "|";
                                for (size_t column_index = 0; column_index < column_count;
                                     ++column_index)
                                {
                                    output << "---|";
                                }
                                output << "\n";
                            }
                        }
                        output << "\n";
                        break;
                    }
                case BlockKind::def_list:
                case BlockKind::params:
                    {
                        for (const auto& item: block.def_items)
                        {
                            output << "**" << RenderSpans(item.first, context, false) << "**\n";
                            output << RenderSpans(item.second, context, true) << "\n\n";
                        }
                        break;
                    }
                case BlockKind::returns:
                    {
                        output << "**Returns:** " << RenderSpans(block.spans, context, true)
                               << "\n\n";
                        break;
                    }
                case BlockKind::since:
                    {
                        output << "**Since:** " << RenderSpans(block.spans, context, true)
                               << "\n\n";
                        break;
                    }
                case BlockKind::deprecated:
                    {
                        output << "**Deprecated:** " << RenderSpans(block.spans, context, true)
                               << "\n\n";
                        break;
                    }
                case BlockKind::note:
                    {
                        WriteQuotedBlock(output, "Note", block.spans, context);
                        break;
                    }
                case BlockKind::warning:
                    {
                        WriteQuotedBlock(output, "Warning", block.spans, context);
                        break;
                    }
                case BlockKind::todo:
                    {
                        WriteQuotedBlock(output, "Todo", block.spans, context);
                        break;
                    }
                case BlockKind::see_also:
                    {
                        output << "**See also:** ";
                        bool first_item = true;
                        for (const std::vector<InlineSpan>& item: block.items)
                        {
                            if (!first_item)
                            {
                                output << ", ";
                            }
                            first_item = false;
                            output << RenderSpans(item, context, false);
                        }
                        output << "\n\n";
                        break;
                    }
                case BlockKind::section:
                    {
                        std::string anchor = block.id;
                        if (anchor.empty())
                        {
                            anchor = AnchorForHeading(RenderSpans(block.spans, context, false));
                        }
                        if (context.anchors != nullptr)
                        {
                            anchor = context.anchors->Allocate(anchor);
                        }
                        WriteAnchor(output, anchor);
                        if (block.level <= 0)
                        {
                            output << "## ";
                        }
                        else if (block.level == 1)
                        {
                            output << "### ";
                        }
                        else
                        {
                            output << "#### ";
                        }
                        output << RenderSpans(block.spans, context, false) << "\n\n";
                        break;
                    }
                case BlockKind::anchor:
                    {
                        WriteAnchor(output, block.id);
                        break;
                    }
                case BlockKind::image:
                    {
                        output << "*[Image: " << block.id << "]*\n\n";
                        break;
                    }
                case BlockKind::availability:
                    {
                        output << "**Availability:** " << RenderSpans(block.spans, context, true)
                               << "\n\n";
                        break;
                    }
            }
        }
    }

    // Write style table
    static void WriteStyleTable(std::ostream& output, const std::vector<StyleEntry>& styles,
                                const std::string& anchor)
    {
        if (styles.empty())
        {
            return;
        }
        WriteAnchor(output, anchor);
        output << "## Styles\n\n";
        output << "| Style | Description |\n";
        output << "|-------|-------------|\n";
        for (const StyleEntry& style: styles)
        {
            output << "| `" << style.name << "` | " << EscapePipe(style.description) << " |\n";
        }
        output << "\n";
    }

    // Write event table
    static void WriteEventTable(std::ostream& output, const std::vector<EventEntry>& events,
                                const std::string& anchor)
    {
        if (events.empty())
        {
            return;
        }
        WriteAnchor(output, anchor);
        output << "## Events\n\n";
        output << "| Event | Description |\n";
        output << "|-------|-------------|\n";
        for (const EventEntry& entry: events)
        {
            output << "| `" << entry.name << "` | " << EscapePipe(entry.description) << " |\n";
        }
        output << "\n";
    }

    // Format a method signature as a C++ code block
    static std::string FormatMethodSignature(const MethodInfo& method)
    {
        std::string signature;
        if (method.is_static)
        {
            signature += "static ";
        }
        if (method.is_virtual)
        {
            signature += "virtual ";
        }
        if (!method.return_type.empty())
        {
            // If return_type contains a template prefix (e.g. "template<typename T> void"),
            // split it so the template clause is on its own line.
            const std::string& rtype = method.return_type;
            if (rtype.starts_with("template"))
            {
                // Find the closing '>' of the template parameter list
                size_t angle_depth = 0;
                size_t split_pos = std::string::npos;
                for (size_t i = 0; i < rtype.size(); ++i)
                {
                    if (rtype[i] == '<')
                    {
                        ++angle_depth;
                    }
                    else if (rtype[i] == '>')
                    {
                        --angle_depth;
                        if (angle_depth == 0)
                        {
                            split_pos = i + 1;
                            break;
                        }
                    }
                }
                if (split_pos != std::string::npos)
                {
                    // template<...> on its own line, then the actual return type
                    signature += rtype.substr(0, split_pos);
                    signature += '\n';
                    std::string_view remainder(rtype);
                    remainder.remove_prefix(split_pos);
                    // Skip leading whitespace after '>'
                    while (!remainder.empty() && remainder.front() == ' ')
                    {
                        remainder.remove_prefix(1);
                    }
                    if (!remainder.empty())
                    {
                        signature += remainder;
                        signature += ' ';
                    }
                }
                else
                {
                    // Malformed template — fall back to single-line
                    signature += rtype;
                    signature += ' ';
                }
            }
            else
            {
                signature += rtype;
                signature += ' ';
            }
        }
        signature += method.name;
        signature += '(';
        bool first = true;
        for (const ParamInfo& param: method.params)
        {
            if (!first)
            {
                signature += ", ";
            }
            first = false;
            if (!param.type.empty())
            {
                signature += param.type;
                if (!param.name.empty())
                {
                    signature += ' ';
                }
            }
            signature += param.name;
            if (!param.default_value.empty())
            {
                signature += " = ";
                signature += param.default_value;
            }
        }
        signature += ')';
        if (method.is_const)
        {
            signature += " const";
        }
        if (method.is_override)
        {
            signature += " override";
        }
        if (method.is_pure)
        {
            signature += " = 0";
        }
        if (method.is_deleted)
        {
            signature += " = delete";
        }
        if (method.is_default)
        {
            signature += " = default";
        }
        return signature;
    }

    struct MethodHeading
    {
        std::string heading;
        std::string anchor;
    };

    // Compute heading text and a page-unique anchor for every method in source order.
    // The first occurrence of a name keeps the plain "name()" heading; later overloads
    // list parameter names so the table and the sections cannot disagree.
    static std::vector<MethodHeading>
        BuildMethodHeadings(const std::vector<MethodInfo>& methods, AnchorAllocator& anchors,
                            std::map<std::string, std::string>& method_anchors)
    {
        std::vector<MethodHeading> result;
        result.reserve(methods.size());
        std::set<std::string> seen_names;
        for (const MethodInfo& method: methods)
        {
            const bool is_overload = !seen_names.insert(method.name).second;
            std::string heading = method.name + "()";
            if (is_overload && !method.params.empty())
            {
                std::string args;
                bool first_arg = true;
                for (const ParamInfo& param: method.params)
                {
                    if (!first_arg)
                    {
                        args += ", ";
                    }
                    first_arg = false;
                    args += param.name.empty() ? param.type : param.name;
                }
                // A trailing ']' (operator[]) would otherwise form "[](i)", which the
                // markdown linter reads as a link with target 'i'.
                heading = method.name.ends_with(']') ? method.name + " (" + args + ")" :
                                                       method.name + "(" + args + ")";
            }

            MethodHeading heading_info;
            heading_info.heading = heading;
            heading_info.anchor = anchors.Allocate(AnchorForHeading(heading));
            if (!method_anchors.contains(method.name))
            {
                method_anchors[method.name] = heading_info.anchor;
            }
            result.push_back(std::move(heading_info));
        }
        return result;
    }

    // Write a single method section using a heading and anchor computed by
    // BuildMethodHeadings(). Documentation blocks render in source order.
    static void WriteMethod(std::ostream& output, const MethodInfo& method,
                            const std::string& heading, const std::string& anchor,
                            const RenderContext& context)
    {
        output << "---\n\n";
        WriteAnchor(output, anchor);
        output << "### " << heading << "\n\n";
        output << "```cpp\n";
        output << FormatMethodSignature(method) << "\n";
        output << "```\n\n";

        WriteBlocks(output, method.doc.blocks, context);
    }

    // Write a single enum section
    static void WriteEnum(std::ostream& output, const EnumInfo& enum_info,
                          const RenderContext& context)
    {
        const std::string heading =
            enum_info.name.empty() ? std::string("Anonymous Enum") : enum_info.name;
        std::string anchor = AnchorForHeading(heading);
        if (context.anchors != nullptr)
        {
            anchor = context.anchors->Allocate(anchor);
        }
        WriteAnchor(output, anchor);
        output << "### " << heading << "\n\n";

        WriteBlocks(output, enum_info.doc.blocks, context);

        if (!enum_info.values.empty())
        {
            output << "| Value | Description |\n";
            output << "|-------|-------------|\n";
            for (const EnumValue& value: enum_info.values)
            {
                output << "| `" << EscapePipe(value.name) << "`";
                if (!value.initializer.empty() && !value.doc.hide_initializer)
                {
                    output << " = `" << EscapePipe(value.initializer) << "`";
                }
                output << " | " << EscapePipe(EscapeProse(BriefText(value.doc))) << " |\n";
            }
            output << "\n";
        }
    }

    // Write a class section. 'include_hint' is the path used for the '#include'
    // directive (e.g. "wx/button.h"); an explicit @header value overrides it.
    static void WriteClass(std::ostream& output, const ClassInfo& class_info,
                           const SymbolTable& symbols, const std::string& include_hint)
    {
        std::set<std::string> method_names;
        for (const MethodInfo& method: class_info.methods)
        {
            method_names.insert(method.name);
        }

        AnchorAllocator anchors;
        std::map<std::string, std::string> method_anchors;

        // Reserve page-level headings first so method anchors never collide with them.
        const std::string title_anchor = anchors.Allocate(AnchorForHeading(class_info.name));
        const std::string methods_anchor = anchors.Allocate("methods");
        const std::string enums_anchor = anchors.Allocate("enums");
        const std::string aliases_anchor = anchors.Allocate("type-aliases");
        const std::string attributes_anchor = anchors.Allocate("public-attributes");
        const std::string styles_anchor = anchors.Allocate("styles");
        const std::string events_anchor = anchors.Allocate("events");
        const std::string nested_anchor = anchors.Allocate("nested-classes");

        const std::vector<MethodHeading> headings =
            BuildMethodHeadings(class_info.methods, anchors, method_anchors);

        const RenderContext context { symbols, class_info.name, &method_names, &method_anchors,
                                      &anchors };

        WriteAnchor(output, title_anchor);
        output << "# " << class_info.name << "\n\n";

        // Include directive (Doxygen lists this prominently near the top). Prefer an
        // explicit @header value; otherwise use the derived include hint.
        const std::string include_path =
            class_info.doc.header_file.empty() ? include_hint : class_info.doc.header_file;
        if (!include_path.empty())
        {
            output << "```cpp\n#include <" << include_path << ">\n```\n\n";
        }

        // Metadata line
        bool have_meta = false;
        if (!class_info.doc.library.empty())
        {
            output << "**Library:** " << class_info.doc.library;
            have_meta = true;
        }
        if (!class_info.doc.category.empty())
        {
            if (have_meta)
            {
                output << " | ";
            }
            output << "**Category:** " << class_info.doc.category;
            have_meta = true;
        }
        if (!class_info.doc.header_file.empty())
        {
            if (have_meta)
            {
                output << " | ";
            }
            output << "**Header:** `<" << class_info.doc.header_file << ">`";
            have_meta = true;
        }
        if (have_meta)
        {
            output << "\n\n";
        }

        // Inheritance
        if (!class_info.bases.empty())
        {
            output << "**Inherits from:** ";
            bool first_base = true;
            for (const std::string& base: class_info.bases)
            {
                if (!first_base)
                {
                    output << ", ";
                }
                first_base = false;
                output << ResolveTarget(base, symbols, class_info.name, &method_anchors);
            }

            // Walk inheritance chain for the first base
            const std::vector<std::string> chain = symbols.InheritanceChain(class_info.bases[0]);
            for (const std::string& ancestor: chain)
            {
                output << " → "
                       << ResolveTarget(ancestor, symbols, class_info.name, &method_anchors);
            }
            output << "\n\n";
        }

        // Description — blocks render in source order, including sections and code.
        WriteBlocks(output, class_info.doc.blocks, context);

        // Style table
        WriteStyleTable(output, class_info.doc.styles, styles_anchor);

        // Event table
        WriteEventTable(output, class_info.doc.events, events_anchor);

        // Nested classes/structs have their own pages; link to them from here.
        if (!class_info.nested.empty())
        {
            WriteAnchor(output, nested_anchor);
            output << "## Nested Classes\n\n";
            for (const ClassInfo& nested: class_info.nested)
            {
                if (nested.name.empty())
                {
                    continue;
                }
                output << "- [" << EscapeProse(nested.name) << "](" << ClassNameToFile(nested.name)
                       << ")";
                const std::string nested_brief = BriefText(nested.doc);
                if (!nested_brief.empty())
                {
                    output << " — " << EscapeProse(nested_brief);
                }
                output << "\n";
            }
            output << "\n";
        }

        // Enums inside class
        if (!class_info.enums.empty())
        {
            WriteAnchor(output, enums_anchor);
            output << "## Enums\n\n";
            for (const EnumInfo& enum_entry: class_info.enums)
            {
                WriteEnum(output, enum_entry, context);
            }
        }

        // Typedefs inside class
        if (!class_info.typedefs.empty())
        {
            WriteAnchor(output, aliases_anchor);
            output << "## Type Aliases\n\n";
            for (const TypedefInfo& alias: class_info.typedefs)
            {
                WriteAnchor(output, anchors.Allocate(AnchorForHeading(alias.name)));
                output << "### " << alias.name << "\n\n";
                if (!alias.underlying_type.empty())
                {
                    output << "```cpp\n";
                    output << "using " << alias.name << " = " << alias.underlying_type << ";\n";
                    output << "```\n\n";
                }
                WriteBlocks(output, alias.doc.blocks, context);
            }
        }

        // Public data members
        if (!class_info.fields.empty())
        {
            WriteAnchor(output, attributes_anchor);
            output << "## Public Attributes\n\n";
            output << "| Type | Name | Description |\n";
            output << "|------|------|-------------|\n";
            for (const FieldInfo& field: class_info.fields)
            {
                output << "| `" << field.type << "` | `" << field.name << "` | "
                       << EscapePipe(EscapeProse(BriefText(field.doc))) << " |\n";
            }
            output << "\n";
        }

        // Methods
        if (!class_info.methods.empty())
        {
            WriteAnchor(output, methods_anchor);
            output << "## Methods\n\n";

            // Emit a compact summary table of the methods with truncated briefs.
            static constexpr size_t BRIEF_MAX_LEN = 120;
            output << "| Method | Description |\n";
            output << "|--------|-------------|\n";
            for (size_t method_index = 0; method_index < class_info.methods.size(); ++method_index)
            {
                const MethodInfo& method = class_info.methods[method_index];
                std::string brief = BriefText(method.doc);
                if (brief.size() > BRIEF_MAX_LEN)
                {
                    // Truncate at the last space before the limit
                    size_t chop_pos = BRIEF_MAX_LEN;
                    while (chop_pos > 0 && brief[chop_pos] != ' ')
                    {
                        --chop_pos;
                    }
                    if (chop_pos == 0)
                    {
                        chop_pos = BRIEF_MAX_LEN;
                    }
                    brief.resize(chop_pos);
                    brief += "…";
                }
                output << "| [" << method.name << "()"
                       << "](#" << headings[method_index].anchor << ") | "
                       << EscapePipe(EscapeProse(brief)) << " |\n";
            }
            output << "\n";

            for (size_t method_index = 0; method_index < class_info.methods.size(); ++method_index)
            {
                MethodInfo effective = class_info.methods[method_index];
                // wxWidgets interface headers omit 'virtual' on overrides; restore it
                // when an ancestor declares the method virtual (matches Doxygen output).
                if (!effective.is_virtual && !effective.is_static &&
                    symbols.IsMethodVirtual(class_info.name, effective.name))
                {
                    effective.is_virtual = true;
                }

                WriteMethod(output, effective, headings[method_index].heading,
                            headings[method_index].anchor, context);
            }
        }

        // Inherited members note — references the first known direct base class
        if (!class_info.bases.empty() && !class_info.methods.empty())
        {
            for (const std::string& base: class_info.bases)
            {
                if (symbols.ClassToFile(base).has_value())
                {
                    output << "> See [" << base << "](" << ClassNameToFile(base)
                           << ") for inherited members.\n\n";
                    break;
                }
            }
        }
    }

    // ---------------------------------------------------------------------------
    // WriteNonClassContent  (internal helper)
    // ---------------------------------------------------------------------------

    static void WriteNonClassContent(std::ostream& output, const FileContent& content,
                                     const SymbolTable& symbols)
    {
        AnchorAllocator anchors;
        std::map<std::string, std::string> method_anchors;

        // The file title is written by WriteFile; reserve its computed anchor so no
        // member heading can reuse it.
        anchors.Allocate(AnchorForHeading(content.filename));
        const std::string enumerations_anchor = anchors.Allocate("enumerations");
        const std::string aliases_anchor = anchors.Allocate("type-aliases");
        const std::string constants_anchor = anchors.Allocate("constants");
        const std::string defines_anchor = anchors.Allocate("defines");
        const std::string functions_anchor = anchors.Allocate("functions");

        std::set<std::string> method_names;
        for (const MethodInfo& function: content.free_functions)
        {
            method_names.insert(function.name);
        }

        const std::vector<MethodHeading> headings =
            BuildMethodHeadings(content.free_functions, anchors, method_anchors);

        const RenderContext context { symbols, {}, &method_names, &method_anchors, &anchors };

        // File-level enums
        if (!content.enums.empty())
        {
            WriteAnchor(output, enumerations_anchor);
            output << "## Enumerations\n\n";
            for (const EnumInfo& enum_entry: content.enums)
            {
                WriteEnum(output, enum_entry, context);
            }
        }

        // File-level typedefs
        if (!content.typedefs.empty())
        {
            WriteAnchor(output, aliases_anchor);
            output << "## Type Aliases\n\n";
            for (const TypedefInfo& alias: content.typedefs)
            {
                WriteAnchor(output, anchors.Allocate(AnchorForHeading(alias.name)));
                output << "### " << alias.name << "\n\n";
                if (!alias.underlying_type.empty())
                {
                    output << "```cpp\n";
                    output << "typedef " << alias.underlying_type << " " << alias.name << ";\n";
                    output << "```\n\n";
                }
                WriteBlocks(output, alias.doc.blocks, context);
            }
        }

        // Constants (file-scope const / constexpr declarations)
        if (!content.constants.empty())
        {
            WriteAnchor(output, constants_anchor);
            output << "## Constants\n\n";
            for (const ConstantInfo& constant: content.constants)
            {
                WriteAnchor(output, anchors.Allocate(AnchorForHeading(constant.name)));
                output << "### " << constant.name << "\n\n";
                output << "```cpp\n";
                output << constant.declaration;
                if (!constant.value.empty())
                {
                    output << " = " << constant.value;
                }
                output << ";\n```\n\n";
                WriteBlocks(output, constant.doc.blocks, context);
            }
        }

        // Defines
        if (!content.defines.empty())
        {
            WriteAnchor(output, defines_anchor);
            output << "## Defines\n\n";
            for (const DefineInfo& define: content.defines)
            {
                WriteAnchor(output, anchors.Allocate(AnchorForHeading(define.name)));
                output << "### " << define.name << "\n\n";
                output << "```cpp\n";
                output << "#define " << define.name;
                if (!define.value.empty())
                {
                    output << " " << define.value;
                }
                output << "\n```\n\n";
                WriteBlocks(output, define.doc.blocks, context);
            }
        }

        // Free functions, grouped by group_name
        if (!content.free_functions.empty())
        {
            std::string current_group;
            bool functions_header_written = false;

            for (size_t function_index = 0; function_index < content.free_functions.size();
                 ++function_index)
            {
                const MethodInfo& function = content.free_functions[function_index];
                if (function_index == 0 || function.group_name != current_group)
                {
                    current_group = function.group_name;
                    if (!current_group.empty())
                    {
                        WriteAnchor(output, anchors.Allocate(AnchorForHeading(current_group)));
                        output << "## " << current_group << "\n\n";
                    }
                    else if (!functions_header_written)
                    {
                        WriteAnchor(output, functions_anchor);
                        output << "## Functions\n\n";
                        functions_header_written = true;
                    }
                }

                WriteMethod(output, function, headings[function_index].heading,
                            headings[function_index].anchor, context);
            }
        }
    }

    // ---------------------------------------------------------------------------
    // WritePage  (internal helper)
    // ---------------------------------------------------------------------------

    // Write one @page topic page: the title as the document heading, the page body,
    // then the standard trailer (written by WriteFile).
    static void WritePage(std::ostream& output, const PageInfo& page, const SymbolTable& symbols)
    {
        AnchorAllocator anchors;
        const std::string title_anchor = anchors.Allocate(AnchorForHeading(page.title));
        WriteAnchor(output, title_anchor);
        output << "# " << page.title << "\n\n";

        const RenderContext context { symbols, {}, nullptr, nullptr, &anchors };
        WriteBlocks(output, page.doc.blocks, context);
    }

    // ---------------------------------------------------------------------------
    // WriteFile
    // ---------------------------------------------------------------------------

    // Collect a class page followed by the pages of its nested compounds, in
    // declaration order. Nested classes are written flat as '<Outer>.<Inner>.md'.
    static void CollectClassPages(const ClassInfo& class_info,
                                  std::vector<const ClassInfo*>& class_pages)
    {
        class_pages.push_back(&class_info);
        for (const ClassInfo& nested: class_info.nested)
        {
            CollectClassPages(nested, class_pages);
        }
    }

    std::vector<fs::path> WriteFile(const FileContent& content, const SymbolTable& symbols,
                                    const fs::path& output_dir, const fs::path& source_rel_path,
                                    const fs::path& input_dir, const std::string& non_class_stem)
    {
        // Guard: only header files should be written here. Assert to catch any
        // unexpected source or markdown file routed through this function.
        assert(source_rel_path.extension() == ".h");

        // Derive the '#include' path. The parser is run with the wxWidgets 'wx'
        // directory as its input root, so prefixing the scan root's leaf name
        // reconstructs the canonical include (e.g. "wx/button.h").
        const fs::path include_leaf = input_dir.filename();
        const std::string include_hint = include_leaf.empty() ?
                                             source_rel_path.generic_string() :
                                             (include_leaf / source_rel_path).generic_string();

        // Ensure the output directory exists
        {
            std::error_code error_code;
            std::ignore = fs::create_directories(output_dir, error_code);
            if (error_code)
            {
                parser::AddErrorMessage("Error creating directory " + output_dir.string() + ": " +
                                        error_code.message());
                return {};
            }
        }

        std::vector<fs::path> generated_files;

        // Write one .md file per class. A header can declare the same class name twice
        // (e.g. scopedptr.h declares both the macro-based wxScopedPtr and the template
        // specialization wxScopedPtr<T>). The first declaration owns the page and keeps
        // its documentation; members that appear only in a later declaration are merged
        // into it so no member heading is lost.
        std::vector<ClassInfo> class_pages;
        std::map<std::string, size_t> class_page_index;
        for (const ClassInfo& class_info: content.classes)
        {
            const std::map<std::string, size_t>::iterator existing =
                class_page_index.find(class_info.name);
            if (existing == class_page_index.end())
            {
                class_page_index[class_info.name] = class_pages.size();
                class_pages.push_back(class_info);
                continue;
            }

            ClassInfo& target = class_pages[existing->second];
            for (const MethodInfo& method: class_info.methods)
            {
                bool present = false;
                for (const MethodInfo& known: target.methods)
                {
                    if (known.name == method.name)
                    {
                        present = true;
                        break;
                    }
                }
                if (!present)
                {
                    target.methods.push_back(method);
                }
            }
            for (const FieldInfo& field: class_info.fields)
            {
                bool present = false;
                for (const FieldInfo& known: target.fields)
                {
                    if (known.name == field.name)
                    {
                        present = true;
                        break;
                    }
                }
                if (!present)
                {
                    target.fields.push_back(field);
                }
            }
            for (const EnumInfo& enum_info: class_info.enums)
            {
                bool present = false;
                for (const EnumInfo& known: target.enums)
                {
                    if (known.name == enum_info.name)
                    {
                        present = true;
                        break;
                    }
                }
                if (!present)
                {
                    target.enums.push_back(enum_info);
                }
            }
            for (const TypedefInfo& alias: class_info.typedefs)
            {
                bool present = false;
                for (const TypedefInfo& known: target.typedefs)
                {
                    if (known.name == alias.name)
                    {
                        present = true;
                        break;
                    }
                }
                if (!present)
                {
                    target.typedefs.push_back(alias);
                }
            }
            if (target.doc.blocks.empty())
            {
                target.doc = class_info.doc;
            }
        }

        std::vector<const ClassInfo*> class_page_ptrs;
        for (const ClassInfo& class_info: class_pages)
        {
            CollectClassPages(class_info, class_page_ptrs);
        }

        for (const ClassInfo* class_page: class_page_ptrs)
        {
            const ClassInfo& class_info = *class_page;
            const fs::path class_rel = fs::path(ClassNameToFile(class_info.name));
            const fs::path class_abs = output_dir / class_rel;

            std::ofstream output_stream(class_abs);
            if (!output_stream)
            {
                parser::AddErrorMessage("Error: could not write to " + class_abs.string());
                continue;
            }

            WriteClass(output_stream, class_info, symbols, include_hint);
            output_stream << "---\n*Generated from `" << content.filename << "`*\n";

            generated_files.emplace_back(class_rel);
        }

        // Write one .md file per @page topic. Pages are independent documents; they
        // are not routed through the non-class content branch below.
        for (const PageInfo& page: content.pages)
        {
            if (page.id.empty())
            {
                continue;
            }

            const fs::path page_rel = fs::path(page.id + ".md");
            const fs::path page_abs = output_dir / page_rel;

            std::ofstream output_stream(page_abs);
            if (!output_stream)
            {
                parser::AddErrorMessage("Error: could not write to " + page_abs.string());
                continue;
            }

            WritePage(output_stream, page, symbols);
            output_stream << "---\n*Generated from `" << content.filename << "`*\n";

            generated_files.emplace_back(page_rel);
        }

        // Write non-class content (enums, typedefs, defines, free functions) to a
        // file named after the source header, written flat in the output root.
        const bool has_non_class_content = !content.enums.empty() || !content.typedefs.empty() ||
                                           !content.defines.empty() || !content.constants.empty() ||
                                           !content.free_functions.empty();
        if (has_non_class_content)
        {
            // Use only the filename, discarding any subdirectory from the header
            // path — all converted markdown files are written flat in the output
            // root. Subdirectories (e.g. data/) are reserved for support files
            // such as the FTS index, not for header conversions.
            fs::path non_class_rel;
            if (non_class_stem.empty())
            {
                non_class_rel = source_rel_path.filename();
                non_class_rel.replace_extension(".md");
            }
            else
            {
                non_class_rel = fs::path(non_class_stem + ".md");
            }
            const fs::path non_class_abs = output_dir / non_class_rel;

            std::ofstream output_stream(non_class_abs);
            if (!output_stream)
            {
                parser::AddErrorMessage("Error: could not write to " + non_class_abs.string());
                return generated_files;
            }

            // Title: use source filename when there are no classes in this file
            if (content.classes.empty())
            {
                output_stream << "# " << content.filename << "\n\n";
            }

            WriteNonClassContent(output_stream, content, symbols);
            output_stream << "---\n*Generated from `" << content.filename << "`*\n";

            generated_files.emplace_back(non_class_rel);
        }

        return generated_files;
    }

}  // namespace docparser
