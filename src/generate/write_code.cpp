//////////////////////////////////////////////////////////////////////////
// Purpose:   Write code to Scintilla
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/stc/stc.h>   // Scintilla

#include "write_code.h"

void WriteCode::WriteCodeLine(ttlib::sview code, size_t indentation)
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
                    doWrite("    ");
                }
            }
        }
        m_isLineWriting = true;
    }

    if (ttlib::is_found(code.find('\t')))
    {
        std::string tab_code;
        tab_code.reserve(code.size() + 16);
        for (auto ch: code)
        {
            if (ch == '\t')
            {
                tab_code += "    ";
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
    if (ttlib::is_found(code.find('\n')))
    {
        ttlib::multiview lines(code, '\n');
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

void WriteCode::writeLine(ttlib::sview code, size_t indentation)
{
    if (code.empty())
    {
        writeLine();
        return;
    }
    if (ttlib::is_found(code.find('\n')))
    {
        ttlib::multiview lines(code, '\n');
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

void WriteCode::write(ttlib::sview code, bool auto_indent)
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
                doWrite("    ");
            }
        }
        m_isLineWriting = true;
    }

    if (ttlib::is_found(code.find('\t')))
    {
        std::string tab_code;
        tab_code.reserve(code.size() + 16);
        for (auto ch: code)
        {
            if (ch == '\t')
            {
                tab_code += "    ";
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
