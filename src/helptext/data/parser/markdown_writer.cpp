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
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace docparser
{

    // ---------------------------------------------------------------------------
    // Helpers
    // ---------------------------------------------------------------------------

    // Convert a class name to its output .md filename, preserving case.
    // Example: "wxButton" -> "wxButton.md"
    std::string ClassNameToFile(const std::string& name)
    {
        return name + ".md";
    }

    // Convert a method name to a lowercase anchor (e.g. "Create" -> "create")
    static std::string NameToAnchor(const std::string& name)
    {
        std::string result;
        for (const char character: name)
        {
            if (std::isalnum(static_cast<unsigned char>(character)) || character == '_' ||
                character == '-')
            {
                result += static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
            }
        }
        return result;
    }

    // Resolve a cross-reference target to a markdown link.
    // Handles "wxFoo", "wxFoo::Bar", and non-class refs (leaves as-is).
    static std::string ResolveRef(const std::string& reference, const SymbolTable& symbols)
    {
        // Check for Class::Member pattern
        const std::string::size_type double_colon = reference.find("::");
        if (double_colon != std::string::npos)
        {
            const std::string class_name = reference.substr(0, double_colon);
            const std::string member_name = reference.substr(double_colon + 2);

            if (symbols.ClassToFile(class_name).has_value())
            {
                return "[" + reference + "](" + ClassNameToFile(class_name) + "#" +
                       NameToAnchor(member_name) + ")";
            }
            return "`" + reference + "`";
        }

        // Check for plain class reference
        if (symbols.ClassToFile(reference).has_value())
        {
            return "[" + reference + "](" + ClassNameToFile(reference) + ")";
        }

        // Not a known class — return as code
        return "`" + reference + "`";
    }

    // Escape pipe characters in table cell text
    static std::string EscapePipe(const std::string& text)
    {
        std::string result;
        result.reserve(text.size());
        for (const char character: text)
        {
            if (character == '|')
            {
                result += "\\|";
            }
            else
            {
                result += character;
            }
        }
        return result;
    }

    // Classify a method name for the summary table kind column.
    [[nodiscard]] static std::string ClassifyMethodKind(const std::string& method_name,
                                                        const std::string& class_name)
    {
        if (method_name == class_name)
        {
            return "🏗️";
        }
        if (method_name.starts_with('~'))
        {
            return "💣";
        }
        if (method_name.starts_with("operator"))
        {
            return "⚙️";
        }
        // Group getters by common prefixes
        if (method_name.starts_with("Get") || method_name.starts_with("Is") ||
            method_name.starts_with("Has") || method_name.starts_with("Can") ||
            method_name.starts_with("Should"))
        {
            return "🔍";
        }
        if (method_name.starts_with("Set"))
        {
            return "✏️";
        }
        return "";
    }

    // Strip markdown formatting from text for use in the summary table.
    // Removes **bold**, *italic*, `code`, [links](url), and collapses whitespace.
    [[nodiscard]] static std::string StripInlineMarkdown(const std::string& text)
    {
        std::string result;
        result.reserve(text.size());
        bool in_bold = false;
        bool in_italic = false;
        bool in_code = false;
        bool in_link_text = false;

        for (size_t idx = 0; idx < text.size(); ++idx)
        {
            const char character = text[idx];

            if (character == '`')
            {
                in_code = !in_code;
                continue;
            }
            if (in_code)
            {
                continue;
            }
            if (character == '*' && idx + 1 < text.size() && text[idx + 1] == '*')
            {
                in_bold = !in_bold;
                ++idx;
                continue;
            }
            if (character == '*' && !in_bold)
            {
                in_italic = !in_italic;
                continue;
            }
            if (character == '[')
            {
                in_link_text = true;
                continue;
            }
            if (in_link_text && character == ']')
            {
                in_link_text = false;
                // Skip the URL portion: ](url)
                if (idx + 1 < text.size() && text[idx + 1] == '(')
                {
                    const size_t close_paren = text.find(')', idx + 2);
                    if (close_paren != std::string::npos)
                    {
                        idx = close_paren;
                    }
                }
                continue;
            }
            if (in_link_text)
            {
                result += character;
                continue;
            }
            result += character;
        }

        // Collapse multiple spaces and trim
        std::string cleaned;
        cleaned.reserve(result.size());
        bool last_was_space = false;
        for (const char character: result)
        {
            if (character == ' ' || character == '\n' || character == '\t')
            {
                if (!last_was_space)
                {
                    cleaned += ' ';
                    last_was_space = true;
                }
            }
            else
            {
                cleaned += character;
                last_was_space = false;
            }
        }
        while (!cleaned.empty() && cleaned.back() == ' ')
        {
            cleaned.pop_back();
        }
        while (!cleaned.empty() && cleaned.front() == ' ')
        {
            cleaned.erase(0, 1);
        }
        return cleaned;
    }

    // Fix indentation of code blocks based on brace nesting depth.
    // Strips existing leading whitespace and re-indents with 4 spaces per brace level.
    [[nodiscard]] static std::string FixBraceIndentation(const std::string& code)
    {
        static constexpr int INDENT_WIDTH = 4;
        std::string result;
        static constexpr size_t RESERVE_FACTOR = 4;
        result.reserve(code.size() + (code.size() / RESERVE_FACTOR));

        int depth = 0;
        size_t line_start = 0;

        while (line_start <= code.size())
        {
            // Find end of current line
            const size_t newline_pos = code.find('\n', line_start);
            const size_t line_end = (newline_pos == std::string::npos) ? code.size() : newline_pos;

            // Extract line and strip leading whitespace
            const std::string_view raw_line(code.data() + line_start, line_end - line_start);
            size_t leading = 0;
            while (leading < raw_line.size() &&
                   (raw_line[leading] == ' ' || raw_line[leading] == '\t'))
            {
                ++leading;
            }
            const std::string_view stripped = raw_line.substr(leading);

            // Count braces in the stripped line (simple count, ignores strings/comments)
            const auto opens = static_cast<int>(std::ranges::count(stripped, '{'));
            const auto closes = static_cast<int>(std::ranges::count(stripped, '}'));

            // Closing braces reduce depth before this line is output
            depth -= closes;
            if (depth < 0)
            {
                depth = 0;
            }

            // Output indented line (blank lines stay blank)
            if (!stripped.empty())
            {
                const int indent = depth * INDENT_WIDTH;
                result.append(static_cast<size_t>(indent), ' ');
                result.append(stripped);
            }
            result += '\n';

            // Opening braces increase depth for subsequent lines
            depth += opens;

            if (newline_pos == std::string::npos)
            {
                break;
            }
            line_start = newline_pos + 1;
        }

        // Remove trailing newline if original didn't have one
        if (!code.empty() && code.back() != '\n' && !result.empty() && result.back() == '\n')
        {
            result.pop_back();
        }

        return result;
    }

    // Format a code block using clang-format via stdin/stdout pipe.
    // Falls back to FixBraceIndentation() if clang-format is unavailable or returns an error.
    [[nodiscard]] static std::string FormatCodeBlock(const std::string& code)
    {
        if (code.empty())
        {
            return code;
        }

        std::string result;
        bool success = false;

#if defined(_WIN32)
        // --- Windows: CreateProcess with redirected stdin/stdout ---

        SECURITY_ATTRIBUTES security_attr {};
        security_attr.nLength = sizeof(security_attr);
        security_attr.bInheritHandle = TRUE;

        HANDLE child_stdin_read = nullptr;
        HANDLE child_stdin_write = nullptr;
        HANDLE child_stdout_read = nullptr;
        HANDLE child_stdout_write = nullptr;

        if (CreatePipe(&child_stdout_read, &child_stdout_write, &security_attr, 0) &&
            CreatePipe(&child_stdin_read, &child_stdin_write, &security_attr, 0))
        {
            // Parent keeps the write-end of stdin and read-end of stdout
            std::ignore = SetHandleInformation(child_stdin_write, HANDLE_FLAG_INHERIT, 0);
            std::ignore = SetHandleInformation(child_stdout_read, HANDLE_FLAG_INHERIT, 0);

            STARTUPINFOA startup_info {};
            startup_info.cb = sizeof(startup_info);
            startup_info.dwFlags = STARTF_USESTDHANDLES;
            startup_info.hStdInput = child_stdin_read;
            startup_info.hStdOutput = child_stdout_write;
            startup_info.hStdError = GetStdHandle(STD_ERROR_HANDLE);

            PROCESS_INFORMATION proc_info {};

            // --assume-filename lets clang-format detect C++ language
            // ColumnLimit: 0 avoids reflowing documentation snippet lines
            std::string cmdline = R"(clang-format --assume-filename=snippet.cpp )" +
                                  std::string(R"(--style="{IndentWidth: 4, ColumnLimit: 0}")");

            if (CreateProcessA(nullptr, cmdline.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
                               nullptr, nullptr, &startup_info, &proc_info))
            {
                std::ignore = CloseHandle(child_stdin_read);
                std::ignore = CloseHandle(child_stdout_write);

                // Write code to child's stdin, then close to signal EOF
                DWORD bytes_written = 0;
                std::ignore = ::WriteFile(child_stdin_write, code.c_str(),
                                          static_cast<DWORD>(code.size()), &bytes_written, nullptr);
                std::ignore = CloseHandle(child_stdin_write);

                // Read formatted output from child's stdout
                static constexpr DWORD BUF_SIZE = 4096;
                std::array<char, BUF_SIZE> buffer {};
                DWORD bytes_read = 0;
                while (ReadFile(child_stdout_read, buffer.data(), BUF_SIZE, &bytes_read, nullptr) &&
                       bytes_read > 0)
                {
                    result.append(buffer.data(), bytes_read);
                }
                std::ignore = CloseHandle(child_stdout_read);

                std::ignore = WaitForSingleObject(proc_info.hProcess, INFINITE);
                DWORD exit_code = 1;
                std::ignore = GetExitCodeProcess(proc_info.hProcess, &exit_code);
                std::ignore = CloseHandle(proc_info.hProcess);
                std::ignore = CloseHandle(proc_info.hThread);

                success = (exit_code == 0 && !result.empty());
            }
            else
            {
                std::ignore = CloseHandle(child_stdin_read);
                std::ignore = CloseHandle(child_stdin_write);
                std::ignore = CloseHandle(child_stdout_read);
                std::ignore = CloseHandle(child_stdout_write);
            }
        }

#else
        // --- POSIX: fork/exec with pipe redirection ---

        // pipe_in:  parent writes to [1], child reads from [0]
        // pipe_out: child writes to [1], parent reads from [0]
        std::array<int, 2> pipe_in { -1, -1 };
        std::array<int, 2> pipe_out { -1, -1 };

        if (pipe(pipe_in.data()) == 0 && pipe(pipe_out.data()) == 0)
        {
            const pid_t child_pid = fork();
            if (child_pid == 0)
            {
                // Child process: redirect stdin/stdout and exec clang-format
                close(pipe_in[1]);
                close(pipe_out[0]);
                dup2(pipe_in[0], STDIN_FILENO);
                dup2(pipe_out[1], STDOUT_FILENO);
                close(pipe_in[0]);
                close(pipe_out[1]);

                execlp("clang-format", "clang-format", "--assume-filename=snippet.cpp",
                       "--style={IndentWidth: 4, ColumnLimit: 0}", nullptr);
                _exit(127);  // exec failed
            }

            if (child_pid > 0)
            {
                // Parent process
                close(pipe_in[0]);
                close(pipe_out[1]);

                // Write code to child's stdin, then close to signal EOF
                std::ignore = write(pipe_in[1], code.c_str(), code.size());
                close(pipe_in[1]);

                // Read formatted output from child's stdout
                static constexpr size_t BUF_SIZE = 4096;
                std::array<char, BUF_SIZE> buffer {};
                ssize_t bytes_read = 0;
                while ((bytes_read = read(pipe_out[0], buffer.data(), buffer.size())) > 0)
                {
                    result.append(buffer.data(), static_cast<size_t>(bytes_read));
                }
                close(pipe_out[0]);

                int status = 0;
                waitpid(child_pid, &status, 0);
                success = (WIFEXITED(status) && WEXITSTATUS(status) == 0 && !result.empty());
            }
            else
            {
                // fork() failed — close all pipe ends
                close(pipe_in[0]);
                close(pipe_in[1]);
                close(pipe_out[0]);
                close(pipe_out[1]);
            }
        }

#endif

        if (!success)
        {
            return FixBraceIndentation(code);
        }

        // Remove trailing newline if original didn't have one
        if (!code.empty() && code.back() != '\n' && !result.empty() && result.back() == '\n')
        {
            result.pop_back();
        }

        return result;
    }

    // Replace occurrences of known method names (e.g. "GetKeyCode()") in text
    // with markdown links to their section anchors within the same file.
    // Skips occurrences already inside markdown links or backtick code spans.
    // When class_name is non-empty, bare constructor names (method == class)
    // are rendered as bold text without () rather than linked as methods.
    [[nodiscard]] static std::string LinkifyMethods(const std::string& text,
                                                    const std::set<std::string>& method_names,
                                                    const std::string& class_name = {})
    {
        if (method_names.empty())
        {
            return text;
        }

        std::string result;
        result.reserve(text.size());

        size_t offset = 0;
        while (offset < text.size())
        {
            // Skip backtick code spans
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

            // Skip existing markdown links [text](url)
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

            // Try to match a method name at the current position.
            // Handles: MethodName(), MethodName (without parens), and
            // ClassName::MethodName / ClassName::MethodName() patterns.
            bool matched = false;
            for (const std::string& method_name: method_names)
            {
                const std::string with_parens = method_name + "()";
                bool has_parens = false;
                size_t match_len = 0;

                if (text.compare(offset, with_parens.size(), with_parens) == 0)
                {
                    match_len = with_parens.size();
                    has_parens = true;
                }
                else if (text.compare(offset, method_name.size(), method_name) == 0)
                {
                    // Must be followed by a word boundary (not alphanumeric, '_', '(', or ':')
                    const size_t after_pos = offset + method_name.size();
                    if (after_pos < text.size() &&
                        (std::isalnum(static_cast<unsigned char>(text[after_pos])) ||
                         text[after_pos] == '_' || text[after_pos] == '(' ||
                         text[after_pos] == ':'))
                    {
                        continue;
                    }
                    match_len = method_name.size();
                }

                if (match_len == 0)
                {
                    continue;
                }

                // Check for optional ClassName:: prefix
                size_t prefix_len = 0;
                if (offset >= 2 && text[offset - 1] == ':' && text[offset - 2] == ':')
                {
                    // Scan backwards past the class name
                    size_t class_start = offset - 2;
                    while (class_start > 0 &&
                           (std::isalnum(static_cast<unsigned char>(text[class_start - 1])) ||
                            text[class_start - 1] == '_'))
                    {
                        --class_start;
                    }
                    // Verify word boundary before class name
                    if (class_start < offset - 2 &&
                        (class_start == 0 ||
                         (!std::isalnum(static_cast<unsigned char>(text[class_start - 1])) &&
                          text[class_start - 1] != '_' && text[class_start - 1] != '.')))
                    {
                        prefix_len = offset - class_start;
                    }
                    else
                    {
                        continue;  // Nested in a larger qualified name
                    }
                }
                else if (offset > 0 &&
                         (std::isalnum(static_cast<unsigned char>(text[offset - 1])) ||
                          text[offset - 1] == ':' || text[offset - 1] == '_' ||
                          text[offset - 1] == '.'))
                {
                    continue;  // Part of a larger identifier
                }

                // Build the display text, removing any already-appended prefix from result
                std::string display;
                if (prefix_len > 0)
                {
                    result.erase(result.size() - prefix_len);
                    display = std::string(text, offset - prefix_len, prefix_len + match_len);
                }
                else
                {
                    display = std::string(text, offset, match_len);
                }
                // Constructor names (method == class) are rendered as bold
                // rather than linked with () — they refer to the class, not the
                // constructor call. Destructors (~Name) still link normally.
                if (!has_parens && !class_name.empty() && method_name == class_name)
                {
                    result += "**" + display + "**";
                    offset += match_len;
                    matched = true;
                    break;
                }

                if (!has_parens)
                {
                    display += "()";
                }

                result += "[" + display + "](#" + NameToAnchor(method_name) + ")";
                offset += match_len;
                matched = true;
                break;
            }
            if (!matched)
            {
                result += text[offset];
                ++offset;
            }
        }

        return result;
    }

    // Write a DocComment's brief and detailed text, optionally linkifying method names.
    // class_name is used to detect constructors (method == class) and render them as
    // bold rather than method links with ().
    static void WriteDocText(std::ostream& output, const DocComment& doc_comment,
                             const std::set<std::string>& method_names = {},
                             const std::string& class_name = {})
    {
        if (!doc_comment.brief.empty())
        {
            std::string text(doc_comment.brief);
            while (text.ends_with('\n'))
            {
                text.pop_back();
            }
            output << LinkifyMethods(text, method_names, class_name) << "\n\n";
        }
        if (!doc_comment.detailed.empty() && doc_comment.detailed != doc_comment.brief)
        {
            std::string text(doc_comment.detailed);
            while (text.ends_with('\n'))
            {
                text.pop_back();
            }
            output << LinkifyMethods(text, method_names, class_name) << "\n\n";
        }
    }

    // Write parameters section
    static void WriteParams(std::ostream& output, const std::vector<ParamDoc>& params)
    {
        if (params.empty())
        {
            return;
        }
        output << "**Parameters:**\n";
        for (const ParamDoc& param: params)
        {
            output << "- `" << param.name << "` — " << param.description << "\n";
        }
        output << "\n";
    }

    // Write style table
    static void WriteStyleTable(std::ostream& output, const std::vector<StyleEntry>& styles)
    {
        if (styles.empty())
        {
            return;
        }
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
    static void WriteEventTable(std::ostream& output, const std::vector<EventEntry>& events)
    {
        if (events.empty())
        {
            return;
        }
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

    // Write a single method section. When 'disambiguate' is true and the method has
    // parameters, the heading lists the parameter names so overloads can be told apart
    // (e.g. "### wxButton(parent, id, label, ...)" instead of a second "### wxButton()").
    // class_name is the owning class, used to suppress () on constructor-name references.
    static void WriteMethod(std::ostream& output, const MethodInfo& method,
                            const SymbolTable& symbols,
                            const std::set<std::string>& method_names = {},
                            bool disambiguate = false, const std::string& class_name = {})
    {
        output << "---\n\n";

        std::string heading = method.name + "()";
        if (disambiguate && !method.params.empty())
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
            heading = method.name + "(" + args + ")";
        }
        output << "### " << heading << "\n\n";
        output << "```cpp\n";
        output << FormatMethodSignature(method) << "\n";
        output << "```\n\n";

        WriteDocText(output, method.doc, method_names, class_name);

        // Code blocks from method doc
        for (const CodeBlock& code: method.doc.code_blocks)
        {
            output << "```";
            if (!code.language.empty())
            {
                output << code.language;
            }
            output << "\n" << FormatCodeBlock(code.code) << "\n```\n\n";
        }

        WriteParams(output, method.doc.params);

        if (!method.doc.returns.empty())
        {
            output << "**Returns:** " << method.doc.returns << "\n\n";
        }
        if (!method.doc.since.empty())
        {
            output << "**Since:** " << method.doc.since << "\n\n";
        }
        if (!method.doc.deprecated.empty())
        {
            output << "**Deprecated:** " << method.doc.deprecated << "\n\n";
        }
        for (const std::string& note: method.doc.notes)
        {
            output << "> **Note:** " << note << "\n\n";
        }
        for (const std::string& warn: method.doc.warnings)
        {
            output << "> **Warning:** " << warn << "\n\n";
        }

        if (!method.doc.see_also.empty())
        {
            output << "**See also:** ";
            bool first_ref = true;
            for (const std::string& see_ref: method.doc.see_also)
            {
                if (!first_ref)
                {
                    output << ", ";
                }
                first_ref = false;
                output << ResolveRef(see_ref, symbols);
            }
            output << "\n\n";
        }
    }

    // Write a single enum section
    static void WriteEnum(std::ostream& output, const EnumInfo& enum_info)
    {
        const std::string heading =
            enum_info.name.empty() ? std::string("Anonymous Enum") : enum_info.name;
        output << "### " << heading << "\n\n";

        WriteDocText(output, enum_info.doc);

        if (!enum_info.values.empty())
        {
            output << "| Value | Description |\n";
            output << "|-------|-------------|\n";
            for (const EnumValue& value: enum_info.values)
            {
                output << "| `" << value.name << "`";
                if (!value.initializer.empty())
                {
                    output << " = `" << value.initializer << "`";
                }
                output << " | " << EscapePipe(value.doc.brief) << " |\n";
            }
            output << "\n";
        }
    }

    // Write a class section. 'include_hint' is the path used for the '#include'
    // directive (e.g. "wx/button.h"); an explicit @header value overrides it.
    static void WriteClass(std::ostream& output, const ClassInfo& class_info,
                           const SymbolTable& symbols, const std::string& include_hint)
    {
        // Collect all method names for cross-reference linkification
        std::set<std::string> method_names;
        for (const MethodInfo& method: class_info.methods)
        {
            method_names.insert(method.name);
        }

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
                output << ResolveRef(base, symbols);
            }

            // Walk inheritance chain for the first base
            const std::vector<std::string> chain = symbols.InheritanceChain(class_info.bases[0]);
            for (const std::string& ancestor: chain)
            {
                output << " → " << ResolveRef(ancestor, symbols);
            }
            output << "\n\n";
        }

        // Description
        WriteDocText(output, class_info.doc, method_names, class_info.name);

        // Code blocks from class doc
        for (const CodeBlock& code: class_info.doc.code_blocks)
        {
            output << "```";
            if (!code.language.empty())
            {
                output << code.language;
            }
            output << "\n" << FormatCodeBlock(code.code) << "\n```\n\n";
        }

        // Style table
        WriteStyleTable(output, class_info.doc.styles);

        // Event table
        WriteEventTable(output, class_info.doc.events);

        // Enums inside class
        if (!class_info.enums.empty())
        {
            output << "## Enums\n\n";
            for (const EnumInfo& enum_entry: class_info.enums)
            {
                WriteEnum(output, enum_entry);
            }
        }

        // Typedefs inside class
        if (!class_info.typedefs.empty())
        {
            output << "## Type Aliases\n\n";
            for (const TypedefInfo& alias: class_info.typedefs)
            {
                output << "### " << alias.name << "\n\n";
                if (!alias.underlying_type.empty())
                {
                    output << "```cpp\n";
                    output << "using " << alias.name << " = " << alias.underlying_type << ";\n";
                    output << "```\n\n";
                }
                WriteDocText(output, alias.doc);
            }
        }

        // Methods
        if (!class_info.methods.empty())
        {
            output << "## Methods\n\n";

            // Emit a compact summary table grouping methods by kind
            static constexpr size_t BRIEF_MAX_LEN = 120;
            output << "| | Method | Description |\n";
            output << "|---|--------|-------------|\n";
            for (const MethodInfo& method: class_info.methods)
            {
                const std::string kind_icon = ClassifyMethodKind(method.name, class_info.name);
                std::string brief = StripInlineMarkdown(method.doc.brief);
                if (brief.size() > BRIEF_MAX_LEN)
                {
                    // Truncate at last space before the limit
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
                const std::string anchor = NameToAnchor(method.name);
                output << "| " << kind_icon << " | [" << method.name << "()"
                       << "](#" << anchor << ") | " << EscapePipe(brief) << " |\n";
            }
            output << "\n";

            std::set<std::string> seen_names;
            for (const MethodInfo& method: class_info.methods)
            {
                // First occurrence of a name keeps the plain "name()" heading (and its
                // anchor); later overloads get parameter names for disambiguation.
                const bool is_overload = !seen_names.insert(method.name).second;

                MethodInfo effective = method;
                // wxWidgets interface headers omit 'virtual' on overrides; restore it
                // when an ancestor declares the method virtual (matches Doxygen output).
                if (!effective.is_virtual && !effective.is_static &&
                    symbols.IsMethodVirtual(class_info.name, effective.name))
                {
                    effective.is_virtual = true;
                }

                WriteMethod(output, effective, symbols, method_names, is_overload, class_info.name);
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

        // See also (class-level)
        if (!class_info.doc.see_also.empty())
        {
            output << "**See also:** ";
            bool first_ref = true;
            for (const std::string& see_ref: class_info.doc.see_also)
            {
                if (!first_ref)
                {
                    output << ", ";
                }
                first_ref = false;
                output << ResolveRef(see_ref, symbols);
            }
            output << "\n\n";
        }
    }

    // ---------------------------------------------------------------------------
    // WriteNonClassContent  (internal helper)
    // ---------------------------------------------------------------------------

    static void WriteNonClassContent(std::ostream& output, const FileContent& content,
                                     const SymbolTable& symbols)
    {
        // File-level enums
        if (!content.enums.empty())
        {
            output << "## Enumerations\n\n";
            for (const EnumInfo& enum_entry: content.enums)
            {
                WriteEnum(output, enum_entry);
            }
        }

        // File-level typedefs
        if (!content.typedefs.empty())
        {
            output << "## Type Aliases\n\n";
            for (const TypedefInfo& alias: content.typedefs)
            {
                output << "### " << alias.name << "\n\n";
                if (!alias.underlying_type.empty())
                {
                    output << "```cpp\n";
                    output << "typedef " << alias.underlying_type << " " << alias.name << ";\n";
                    output << "```\n\n";
                }
                WriteDocText(output, alias.doc);
            }
        }

        // Defines
        if (!content.defines.empty())
        {
            output << "## Defines\n\n";
            for (const DefineInfo& define: content.defines)
            {
                output << "### " << define.name << "\n\n";
                output << "```cpp\n";
                output << "#define " << define.name;
                if (!define.value.empty())
                {
                    output << " " << define.value;
                }
                output << "\n```\n\n";
                WriteDocText(output, define.doc);
            }
        }

        // Free functions, grouped by group_name
        if (!content.free_functions.empty())
        {
            std::string current_group;
            bool need_header = true;
            std::set<std::string> seen_func_names;

            for (const MethodInfo& func: content.free_functions)
            {
                if (func.group_name != current_group)
                {
                    current_group = func.group_name;
                    if (!current_group.empty())
                    {
                        output << "## " << current_group << "\n\n";
                    }
                    else if (need_header)
                    {
                        output << "## Functions\n\n";
                    }
                    need_header = false;
                }
                else if (need_header)
                {
                    output << "## Functions\n\n";
                    need_header = false;
                }
                const bool is_overload = !seen_func_names.insert(func.name).second;
                WriteMethod(output, func, symbols, {}, is_overload);
            }
        }
    }

    // ---------------------------------------------------------------------------
    // WriteFile
    // ---------------------------------------------------------------------------

    std::vector<fs::path> WriteFile(const FileContent& content, const SymbolTable& symbols,
                                    const fs::path& output_dir, const fs::path& source_rel_path,
                                    const fs::path& input_dir)
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

        // Write one .md file per class
        for (const ClassInfo& class_info: content.classes)
        {
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

        // Write non-class content (enums, typedefs, defines, free functions) to a
        // file named after the source header, written flat in the output root.
        const bool has_non_class_content = !content.enums.empty() || !content.typedefs.empty() ||
                                           !content.defines.empty() ||
                                           !content.free_functions.empty();
        if (has_non_class_content)
        {
            // Use only the filename, discarding any subdirectory from the header
            // path — all converted markdown files are written flat in the output
            // root. Subdirectories (e.g. data/) are reserved for support files
            // such as the FTS index, not for header conversions.
            fs::path non_class_rel = source_rel_path.filename();
            non_class_rel.replace_extension(".md");
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
