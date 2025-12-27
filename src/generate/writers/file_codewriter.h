//////////////////////////////////////////////////////////////////////////
// Purpose:   Class to write code to disk
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>

#include <wx/filename.h>  // wxFileName - encapsulates a file path

#include "wxue_namespace/wxue_view_vector.h"  // ViewVector -- wxue::ViewVector class

#include "write_code.h"

// Use for writing code to disk
namespace code
{
    enum : std::uint8_t
    {
        flag_none = 0,
        flag_test_only = 1 << 0,          // Don't write the file, just return the result
        flag_no_ui = 1 << 1,              // Don't display any UI (cannot create missing folder)
        flag_add_closing_brace = 1 << 2,  // Set when no_closing_brace property is set
    };

    enum : std::int8_t
    {
        write_error = -1,        // File could not be written
        write_cant_create = -2,  // File could not be created
        write_cant_read = -3,    // File can't be read, so no comparison can be made
        write_no_folder =
            -3,  // Folder doesn't exist and flag_no_ui is set or user cancelled folder creation
        write_current = 0,  // File is current, no update needed
        write_success = 1,  // File written, user has not added edits
        write_edited = 2,   // File written with user-edits
        write_needed = 3,   // Returned if flag_test_only is set and file needs updating
    };
};  // namespace code

class Node;  // forward declaration

class FileCodeWriter : public WriteCode
{
public:
    FileCodeWriter(const wxString& file, size_t reserved_amount = static_cast<size_t>(8 * 1024)) :
        m_filename(file)
    {
        m_buffer.clear();
        m_buffer.reserve(reserved_amount);
    }

    void Clear() override { m_buffer.clear(); };
    [[nodiscard]] auto GetString() -> std::string& { return m_buffer; };

    // Returns one of code::write_ enums. Errors are negative values, 0 is current, positive
    // values indicate success or update needed (if testing).
    [[nodiscard]] auto WriteFile(GenLang language, int flags = code::flag_none,
                                 Node* node = nullptr) -> int;

protected:
    void doWrite(std::string_view code) override { m_buffer += std::string(code); };

private:
    // Helper methods

    // Returns the " ************* End of generated code" prefixed with language-specific comment
    // character
    [[nodiscard]] static auto GetCommentLineToFind(GenLang language) -> std::string_view;

    [[nodiscard]] static auto GetBlockLength(GenLang language) -> size_t;
    [[nodiscard]] auto IsOldStyleFile() -> bool;

    // Returns the index of the line after the final comment block, or -1 if not found
    [[nodiscard]] auto FindAdditionalContentIndex() -> std::ptrdiff_t;

    // All Append...() functions append to m_buffer

    // Add the "End of generated code" comment block and fake user content (Ruby 'end',
    // C++ '};', Perl '1;') appropriate for the language. Sets m_fake_content_pos to mark
    // where fake content starts so it can be removed later if needed.
    void AppendEndOfFileBlock();
    void AppendMissingCommentBlockWarning();

    // Preserves user content from the original file. If the original file has content after
    // its comment block, this function removes any fake content we added (at m_fake_content_pos)
    // and appends the original content instead. Also skips duplicate asterisk lines from
    // earlier buggy versions. Returns true if any content was appended.
    [[nodiscard]] auto AppendOriginalUserContent(size_t begin_new_user_content) -> bool;

    // Language-specific end-of-file block handlers
    void AppendCppEndBlock();
    void AppendPerlEndBlock();
    void AppendPythonEndBlock();
    void AppendRubyEndBlock();

    // This reads the original file into m_org_buffer
    [[nodiscard]] auto ReadOriginalFile(bool is_comparing) -> int;

    [[nodiscard]] auto EnsureDirectoryExists(int flags) -> int;
    [[nodiscard]] auto WriteToFile() -> int;

    // Member variables
    wxue::string m_buffer;
    wxFileName m_filename;
    Node* m_node { nullptr };

    // Shared state between helper methods
    GenLang m_language { GEN_LANG_NONE };
    int m_flags { 0 };
    bool m_file_exists { false };
    size_t m_block_length { 0 };

    // Line index in m_org_file where user content begins (after final comment block).
    // -1 means not yet determined or no user content exists.
    // Used to preserve user-added code when regenerating files.
    std::ptrdiff_t m_additional_content { -1 };

    // Position in m_buffer where fake content (Ruby 'end', C++ '};', Perl '1;') starts.
    // 0 means no fake content was added. Used to remove fake content when preserving
    // original file content instead.
    size_t m_fake_content_pos { 0 };

    std::string m_org_buffer;
    wxue::ViewVector m_org_file;
    std::string_view m_comment_line_to_find;

#if defined(_DEBUG)
    bool hasWriteFileBeenCalled { false };
#endif  // _DEBUG
};
