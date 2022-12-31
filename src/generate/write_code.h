//////////////////////////////////////////////////////////////////////////
// Purpose:   Write code to Scintilla
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace indent
{
    enum size_t
    {
        none,
        auto_no_whitespace,
        auto_keep_whitespace,
    };
};

class Code;

class WriteCode
{
public:
    WriteCode() {};
    virtual ~WriteCode() {};

    void Indent() { ++m_indent; }
    void Unindent()
    {
        if (m_indent > 0)
            --m_indent;
    }
    void ResetIndent() { m_indent = 0; }

    // Write one or more lines, adding a trailing \n to the final line. Multiple lines
    // are indicated if the supplied string contains one or more \n characters.
    void writeLine(const Code& code);

    // Write one or more lines, adding a trailing \n to the final line. Multiple lines
    // are indicated if the supplied string contains one or more \n characters.
    void writeLine(std::string& lines, size_t indentation = indent::auto_no_whitespace);

    // This will NOT right trim a single line
    void writeLine(ttlib::sview, size_t indentation = indent::auto_no_whitespace);

    // Write an empty line (unless the previous line was also empty)
    void writeLine();

    // Write the code without adding a trailing \n.
    void write(ttlib::sview code, bool auto_indent = true);

    // Call this to prevent any further blank lines from being written until the next non-blank line is
    // written
    void SetLastLineBlank() { m_IsLastLineBlank = true; }

    virtual void Clear() = 0;

    // Derived class provides this to write text to whatever output device is being used
    virtual void doWrite(ttlib::sview code) = 0;

protected:
    void WriteCodeLine(ttlib::sview code, size_t indentation);

private:
    int m_indent { 0 };
    bool m_isLineWriting { false };
    bool m_IsLastLineBlank { false };
};
