//////////////////////////////////////////////////////////////////////////
// Purpose:   Class to write code to disk
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>

#include <wx/filename.h>  // wxFileName - encapsulates a file path

#include "ttwx_view_vector.h"  // ViewVector -- ttwx::ViewVector class

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

extern const std::string_view cpp_end_cmt_line;  // "// ************* End of generated code"

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
    [[nodiscard]] static auto GetCommentLineToFind(GenLang language) -> std::string_view;
    [[nodiscard]] static auto GetBlockLength(GenLang language) -> size_t;
    [[nodiscard]] static auto GetCommentCharacter(GenLang language) -> std::string_view;
    [[nodiscard]] auto IsOldStyleFile() -> bool;

    // Returns the index of the line after the final comment block, or -1 if not found
    [[nodiscard]] auto FindAdditionalContentIndex() -> std::ptrdiff_t;

    // All Append...() functions append to m_buffer

    void AppendEndOfFileBlock();
    void AppendMissingCommentBlockWarning();

    // If fake user content was added to m_buffer, the offset to it is in begin_new_user_content.
    // This function will set m_additional_content if the file has not been read before.
    // Returns true if any user content was appended.
    auto AppendOriginalUserContent(size_t begin_new_user_content) -> bool;

    // Language-specific end-of-file block handlers
    void AppendCppEndBlock();
    void AppendPerlEndBlock();
    void AppendPythonEndBlock();
    void AppendRubyEndBlock();

    // Ruby classes require an 'end' statement for the class.
    // C++ header files with no closing brace require a closing brace.
    // In both situations, fake user content is added after the final comment block.
    // Returns the size of the appended content, zero if nothing was appended.
    auto AppendFakeUserContent() -> size_t;

    // This reads the original file into m_org_buffer
    [[nodiscard]] auto ReadOriginalFile(bool is_comparing) -> int;

    [[nodiscard]] auto EnsureDirectoryExists(int flags) -> int;
    [[nodiscard]] auto WriteToFile() -> int;

    // Helper methods for WriteFile complexity reduction
    [[nodiscard]] auto HandleEqualSizeBuffers() -> int;
    [[nodiscard]] auto HandleLargerOriginalFile() -> int;
    [[nodiscard]] auto ProcessDifferentSizeFiles() -> int;

    // Member variables
    std::string m_buffer;
    wxFileName m_filename;
    Node* m_node { nullptr };

    // Shared state between helper methods
    GenLang m_language { GEN_LANG_NONE };
    int m_flags { 0 };
    bool m_file_exists { false };
    size_t m_block_length { 0 };
    std::ptrdiff_t m_additional_content { -1 };
    std::string m_org_buffer;
    ttwx::ViewVector m_org_file;
    std::string_view m_comment_line_to_find;

#if defined(_DEBUG)
    bool hasWriteFileBeenCalled { false };
#endif  // _DEBUG
};
