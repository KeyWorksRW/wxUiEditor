//////////////////////////////////////////////////////////////////////////
// Purpose:   Classs to write code to disk
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "write_code.h"

// Use for writing code to disk
namespace code
{
    enum
    {
        flag_none = 0,
        flag_test_only = 1 << 0,  // Don't write the file, just return the result
        flag_no_ui = 1 << 1,      // Don't display any UI (cannot create missing folder)
    };

    enum
    {
        write_error = -1,        // File could not be written
        write_cant_create = -2,  // File could not be created
        write_cant_read = -3,    // File can't be read, so no comparison can be made
        write_no_folder = -3,    // Folder doesn't exist and flag_no_ui is set or user cancelled folder creation
        write_current = 0,       // File is current, no update needed
        write_success = 1,       // File written, user has not added edits
        write_edited = 2,        // File written with user-edits
        write_needed = 3,        // Returned if flag_test_only is set and file needs updating
    };
};  // namespace code

class FileCodeWriter : public WriteCode
{
public:
    FileCodeWriter(const wxString& file, size_t reserved_amount = 4096) : m_filename(file)
    {
        m_buffer.clear();
        m_buffer.reserve(reserved_amount);
    }

    void Clear() override { m_buffer.clear(); };
    ttlib::cstr& GetString() { return m_buffer; };

    // Returns one of code::write_ enums
    int WriteFile(int language, int flags = code::flag_none);

protected:
    void doWrite(ttlib::sview code) override { m_buffer << code; };

    ttlib::cstr m_buffer;

private:
    ttString m_filename;
    int m_language = GEN_LANG_CPLUSPLUS;

#if defined(_DEBUG)
    bool hasWriteFileBeenCalled { false };
#endif  // _DEBUG
};
