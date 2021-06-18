//////////////////////////////////////////////////////////////////////////
// Purpose:   Write code to Scintilla or file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <fstream>

#include <wx/file.h>     // wxFile - encapsulates low-level "file descriptor"
#include <wx/stc/stc.h>  // Scintilla

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "write_code.h"

#include "node_creator.h"  // NodeCreator class

void WriteCode::WriteCodeLine(ttlib::cview code, size_t indentation)
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
        ttlib::multistr lines(code, '\n');
        for (auto& iter: lines)
        {
            WriteCodeLine(iter, indentation);
        }
    }
    else
    {
        code.erase(std::find_if(code.rbegin(), code.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
                   code.end());

        WriteCodeLine(code, indentation);
    }
}

void WriteCode::writeLine(ttlib::cview code, bool auto_indent)
{
    if (code.empty())
    {
        writeLine();
        return;
    }
    if (ttlib::is_found(code.find('\n')))
    {
        ttlib::multistr lines(code, '\n');
        for (auto& iter: lines)
        {
            WriteCodeLine(iter, auto_indent);
        }
    }
    else
    {
        WriteCodeLine(code, auto_indent);
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

void WriteCode::write(ttlib::cview code, bool auto_indent)
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

//////////////////////////////////////////// PanelCodeWriter class /////////////////////////////////////////////

// PanelCodeWriter ctor and methods are implemented here, rather than in header filr to avoid having to include
// wx/stc/stc.h in every app that includes the write_code.h header file.

PanelCodeWriter::PanelCodeWriter(wxStyledTextCtrl* scintilla)
{
    ASSERT(scintilla);
    m_Scintilla = scintilla;
};

void PanelCodeWriter::Clear()
{
    m_Scintilla->ClearAll();
}

void PanelCodeWriter::doWrite(ttlib::cview code)
{
    m_Scintilla->AddTextRaw(code);
}

//////////////////////////////////////////// FileCodeWriter class /////////////////////////////////////////////

int FileCodeWriter::WriteFile(bool test_only)
{
#if defined(_DEBUG)
    hasWriteFileBeenCalled = true;
#endif

    ttlib::cstr filename = m_filename.utf8_str().data();

    std::ifstream fileOriginal(filename, std::ios::binary | std::ios::in);
    if (fileOriginal.is_open())
    {
        std::string buf(std::istreambuf_iterator<char>(fileOriginal), {});
        if (m_buffer.size() == buf.size() && m_buffer == buf)
        {
            return 0;  // origianal file is the same as current file
        }
    }

    // If we get here, the file needs to be written. If we're only testing if it needs writing, then return immediately.
    if (test_only)
        return 1;

    // TODO: [KeyWorks - 05-31-2020] Since we require C++17 compiler, it would make sense to write using std::ifstream
    // instead of wxFile

    wxFile fileOut;
    if (!fileOut.Create(m_filename, true))
    {
        return -1;
    }

    if (fileOut.Write(m_buffer.c_str(), m_buffer.length()) != m_buffer.length())
    {
        return -1;
    }

    return 1;  // file was successfully written
}
