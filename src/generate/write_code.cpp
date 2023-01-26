//////////////////////////////////////////////////////////////////////////
// Purpose:   Write code to Scintilla
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/stc/stc.h>  // Scintilla

#include "write_code.h"

#include "code.h"  // Code -- Helper class for generating code

// String to write whenever a tab is encountered at the beginning of a line
constexpr const char* TabSpaces = "    ";

void WriteCode::writeLine(const Code& code)
{
    if (!code.size())
    {
        writeLine();
        return;
    }

    // tt_view_vector only creates a vector of std::string_views, so it's cheap to create
    // even for a single line.

    tt_view_vector lines(code.GetView(), '\n');
    for (auto& line: lines)
    {
        // Remove any trailing tabs -- this occurs when Code::Eol() is called when an indent
        // is active.
        while (line.size() && line.back() == '\t')
        {
            line.remove_suffix(1);
        }

        if (line.size())
        {
            // Don't indent #if, #else or #endif
            if (line[0] != '#' || !(line.starts_with("#if") || line.starts_with("#else") || line.starts_with("#endif")))
            {
                for (int i = 0; i < m_indent; ++i)
                {
                    doWrite(TabSpaces);
                }
            }

            if (line[0] == '\t')
            {
                do
                {
                    doWrite(TabSpaces);
                    line.remove_prefix(1);
                    ASSERT_MSG(line.size(), "Line ended with nothing but tabs.");
                } while (line.size() && line[0] == '\t');
            }

            doWrite(line);
        }
        doWrite("\n");
    }

    m_IsLastLineBlank = (lines.back().empty() ? true : false);
}

void WriteCode::writeLine(std::vector<std::string>& lines)
{
    for (auto& line: lines)
    {
        // Remove any trailing tabs -- this occurs when Code::Eol() is called when an indent
        // is active.
        while (line.size() && line.back() == '\t')
        {
            line.pop_back();
        }

        if (line.size())
        {
            // Don't indent #if, #else or #endif
            if (line[0] != '#' || !(line.starts_with("#if") || line.starts_with("#else") || line.starts_with("#endif")))
            {
                for (int i = 0; i < m_indent; ++i)
                {
                    doWrite(TabSpaces);
                }
            }

            if (line[0] == '\t')
            {
                size_t idx = 0;
                for (; idx < line.size(); ++idx)
                {
                    if (line[idx] == '\t')
                    {
                        doWrite(TabSpaces);
                    }
                    else
                    {
                        break;
                    }
                }

                doWrite(line.substr(idx));
            }
            else
            {
                doWrite(line);
            }
        }
        doWrite("\n");
    }

    m_IsLastLineBlank = (lines.back().empty() ? true : false);
}

void WriteCode::WriteCodeLine(tt_string_view code, size_t indentation)
{
    if (indentation == indent::auto_no_whitespace)
    {
        code.moveto_nonspace();
    }

    if (code.empty())
    {
        writeLine();
        return;
    }

    if (!m_isLineWriting)
    {
        if (indentation != indent::none)
        {
            // Don't indent #if, #else or #endif
            if (code[0] != '#' || !(code.starts_with("#if") || code.starts_with("#else") || code.starts_with("#endif")))
            {
                for (int i = 0; i < m_indent; ++i)
                {
                    doWrite(TabSpaces);
                }
            }
        }
        m_isLineWriting = true;
    }

    if (tt::is_found(code.find('\t')))
    {
        std::string tab_code;
        tab_code.reserve(code.size() + 16);
        for (auto ch: code)
        {
            if (ch == '\t')
            {
                tab_code += TabSpaces;
            }
            else
            {
                tab_code.push_back(ch);
            }
        }
        doWrite(tab_code);
    }
    else
    {
        doWrite(code);
    }

    doWrite("\n");

    m_IsLastLineBlank = false;
    m_isLineWriting = false;
}

void WriteCode::writeLine(std::string& code, size_t indentation)
{
    if (code.empty())
    {
        writeLine();
        return;
    }
    if (tt::is_found(code.find('\n')))
    {
        tt_view_vector lines(code, '\n');
        for (auto& iter: lines)
        {
            WriteCodeLine(iter, indentation);
        }
    }
    else
    {
        WriteCodeLine(code, indentation);
    }
}

void WriteCode::writeLine(tt_string_view code, size_t indentation)
{
    if (code.empty())
    {
        writeLine();
        return;
    }
    if (tt::is_found(code.find('\n')))
    {
        tt_view_vector lines(code, '\n');
        for (auto& iter: lines)
        {
            WriteCodeLine(iter, indentation);
        }
    }
    else
    {
        WriteCodeLine(code, indentation);
    }
}

void WriteCode::writeLine()
{
    m_isLineWriting = false;
    if (m_IsLastLineBlank)
        return;
    doWrite("\n");
    m_IsLastLineBlank = true;
}

void WriteCode::write(tt_string_view code, bool auto_indent)
{
    // Early abort to not produce lines with trailing whitespace
    if (code.empty())
    {
        return;
    }

    if (!m_isLineWriting)
    {
        if (auto_indent)
        {
            for (int i = 0; i < m_indent; ++i)
            {
                doWrite(TabSpaces);
            }
        }
        m_isLineWriting = true;
    }

    if (tt::is_found(code.find('\t')))
    {
        std::string tab_code;
        tab_code.reserve(code.size() + 16);
        for (auto ch: code)
        {
            if (ch == '\t')
            {
                tab_code += TabSpaces;
            }
            else
            {
                tab_code.push_back(ch);
            }
        }
        doWrite(tab_code);
    }
    else
    {
        doWrite(code);
    }
}
