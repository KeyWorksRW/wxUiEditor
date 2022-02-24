//////////////////////////////////////////////////////////////////////////
// Purpose:   Write code to Scintilla or file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include <fstream>

#include <wx/file.h>      // wxFile - encapsulates low-level "file descriptor"
#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/stc/stc.h>   // Scintilla

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "write_code.h"

#include "mainapp.h"       // App -- Main application class
#include "node_creator.h"  // NodeCreator class

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
            if (code[0] != '#' ||
                !(code.is_sameprefix("#if") || code.is_sameprefix("#else") || code.is_sameprefix("#endif")))
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

void WriteCode::writeLine(ttlib::sview code, bool auto_indent)
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

//////////////////////////////////////////// FileCodeWriter class /////////////////////////////////////////////

int FileCodeWriter::WriteFile(bool test_only)
{
#if defined(_DEBUG)
    hasWriteFileBeenCalled = true;
#endif

    bool file_exists = m_filename.file_exists();
    if (file_exists)
    {
        wxFile file_original(m_filename, wxFile::read);
        if (file_original.IsOpened())
        {
            auto in_size = file_original.Length();
            if (m_buffer.size() == static_cast<size_t>(in_size))
            {
                auto buffer = std::make_unique<unsigned char[]>(in_size);
                if (file_original.Read(buffer.get(), in_size) == in_size)
                {
                    if (std::memcmp(buffer.get(), m_buffer.data(), in_size) == 0)
                        return 0;  // origianal file is the same as current file
                }
            }
        }
    }

    // If we get here, the file needs to be written. If we're only testing if it needs writing, then return immediately.
    if (test_only)
        return 1;

    if (!file_exists)
    {
        ttString copy(m_filename);
        copy.remove_filename();
        if (!copy.dir_exists() && !wxGetApp().AskedAboutMissingDir(copy))
        {
            if (wxMessageBox(wxString() << "The directory " << copy << " doesn't exist.\n\nWould you like it to be created?",
                             "Generate Files", wxICON_WARNING | wxYES_NO) == wxYES)
            {
                wxFileName fn(copy);
                fn.Mkdir();
            }
            else
            {
                wxGetApp().AddMissingDir(copy);
            }
        }
    }

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
