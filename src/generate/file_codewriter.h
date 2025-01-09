//////////////////////////////////////////////////////////////////////////
// Purpose:   Classs to write code to disk
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
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
        flag_test_only = 1 << 0,          // Don't write the file, just return the result
        flag_no_ui = 1 << 1,              // Don't display any UI (cannot create missing folder)
        flag_add_closing_brace = 1 << 2,  // Set when no_closing_brace property is set
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

class Node;  // forward declaration

class FileCodeWriter : public WriteCode
{
public:
    FileCodeWriter(const wxString& file, size_t reserved_amount = 8 * 1024) : m_filename(file.utf8_string())
    {
        m_buffer.clear();
        m_buffer.reserve(reserved_amount);
    }
    FileCodeWriter(const tt_string& file, size_t reserved_amount = 8 * 1024)
    {
        // REVIEW: [Randalphwa - 06-18-2023] Both of these *should* work! However, we end up
        // with m_filename being empty.

        // m_filename.FromUTF8(file);
        // m_filename.FromUTF8(file.data(), file.size());

        m_filename = file;
        m_buffer.clear();
        m_buffer.reserve(reserved_amount);
    }

    void Clear() override { m_buffer.clear(); };
    tt_string& GetString() { return m_buffer; };

    // Returns one of code::write_ enums. Errors are negative values, 0 is current, positive
    // values indicate success or update needed (if testing).
    int WriteFile(GenLang language, int flags = code::flag_none, Node* node = nullptr);

protected:
    void doWrite(tt_string_view code) override { m_buffer += code; };

    tt_string m_buffer;

private:
    tt_string m_filename;
    Node* m_node { nullptr };

#if defined(_DEBUG)
    bool hasWriteFileBeenCalled { false };
#endif  // _DEBUG
};
