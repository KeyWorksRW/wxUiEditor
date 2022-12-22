//////////////////////////////////////////////////////////////////////////
// Purpose:   Classs to write code to disk
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "write_code.h"

class FileCodeWriter : public WriteCode
{
public:
    FileCodeWriter(const wxString& file) : m_filename(file) { m_buffer.clear(); }

    void Clear() override { m_buffer.clear(); };
    ttlib::cstr& GetString() { return m_buffer; };

    // Returns -1 if an error occurred, 0 if no update is needed, 1 on success
    int WriteFile(bool test_only = false);

protected:
    void doWrite(ttlib::sview code) override { m_buffer << code; };

    ttlib::cstr m_buffer;

private:
    ttString m_filename;

#if defined(_DEBUG)
    bool hasWriteFileBeenCalled { false };
#endif  // _DEBUG
};
