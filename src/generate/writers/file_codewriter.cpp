//////////////////////////////////////////////////////////////////////////
// Purpose:   Class to write code to disk
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/file.h>      // wxFile - encapsulates low-level "file descriptor"
#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/msgdlg.h>    // common header and base class for wxMessageDialog

#include "file_codewriter.h"

#include "code.h"            // Code -- Helper class for generating code
#include "comment_blocks.h"  // Shared generated comment blocks
#include "common_strings.h"  // Common strings used in code generation
#include "gen_enums.h"
#include "mainapp.h"             // App -- Main application class
#include "wxue_string_vector.h"  // wxue::StringVector class
#include "wxue_view_vector.h"    // wxue::ViewVector class

using namespace code;

// WriteFile() handles writing generated code to disk while preserving any user-added content.
//
// The approach is:
// 1. Generate code and append end-of-file comment block with "fake" user content
//    (Ruby 'end', C++ '};', Perl '1;') - this ensures new files are complete.
// 2. If file exists, compare buffers to detect changes.
// 3. If original file has content after its comment block, remove the fake content
//    we added and replace it with the original file's content (preserving user edits).
// 4. Also handles cleanup of duplicate asterisk lines from earlier buggy versions.

auto FileCodeWriter::WriteFile(GenLang language, int flags,
                               Node* node)  // NOLINT (cppcheck-suppress)
    -> int                                  // NOLINT (cppcheck-suppress)
{
    ASSERT_MSG(!m_filename.GetFullPath().IsEmpty(),
               "Filename must be set before calling WriteFile()");

    if (int dir_result = EnsureDirectoryExists(flags); dir_result != 0)
    {
        return dir_result;
    }

    m_node = node;
    m_language = language;
    m_flags = flags;

    // Note that AppendEndOfFileBlock() may change m_block_length, so be sure to set it first so it
    // can be updated if needed.
    m_block_length = GetBlockLength(language);

    AppendEndOfFileBlock();

    m_file_exists = m_filename.FileExists();
    if (!m_file_exists)
    {
        // File doesn't exist. The end-of-file block and fake user content (Ruby 'end',
        // C++ '};', Perl '1;') were already added by AppendEndOfFileBlock().
        // For new files, we keep this fake content to make them syntactically complete.
        return (flags & flag_test_only) ? write_needed : WriteToFile();
    }

    m_additional_content = -1;
    m_comment_line_to_find = GetCommentLineToFind(language);

    // TODO: [Randalphwa - 11-28-2025] Do we ever create temporary files with names starting with
    // '~' anymore? If not, this needs to go away.
    bool is_comparing = (!m_filename.GetName().IsEmpty() && m_filename.GetName()[0] == '~');
    if (int result = ReadOriginalFile(is_comparing); result != 0)
    {
        // TODO: [Randalphwa - 11-06-2025] Need to be sure caller handles this properly
        return result;
    }

    // At this point, m_buffer contains newly generated code WITH fake user content already
    // appended (Ruby 'end', C++ '};', Perl '1;'). m_org_buffer contains the original file.
    // m_fake_content_pos marks where we can erase the fake content if we need to preserve
    // original content instead.
    //
    // The comparison logic below has three main branches:
    //
    // 1. SAME SIZE: Buffers are exactly the same size
    //    - If content matches exactly → file is current, no write needed
    //    - If content differs → try appending original user content and compare
    //
    // 2. ORIGINAL LARGER: Original file has more content than new buffer
    //    - If new buffer matches the prefix of original → file is current
    //    - Otherwise → find comment block, preserve user content, and write
    //
    // 3. SIZES DIFFER: Files differ significantly in size or content
    //    - Search for final comment block in original file
    //    - Remove fake content, extract and preserve any user content after the block
    //    - Write new file with preserved user content

    // ========== BRANCH 1: Handle case where buffers are the same size ==========
    if (m_buffer.size() == m_org_buffer.size())
    {
        if (std::equal(m_buffer.begin(), m_buffer.end(), m_org_buffer.begin()))
        {
            // Content matches exactly - file is current, no write needed.
            return write_current;
        }

        // Same size but different content. For Ruby, user may have modified/removed
        // the comment after 'end', which doesn't require an update. Fall through to
        // Branch 2 logic which handles this case.
    }

    // ========== BRANCH 2: Handle Ruby files with special 'end' statement handling ==========
    // For Ruby files: We only care that buffer up to </auto-generated> matches, and that
    // there's an 'end' statement in the original. User may have modified or removed the
    // comment, added blank lines, or added their own content - none require an update.
    // This handles all size comparisons (same size, larger, or smaller original) in one place.
    if (m_language == GEN_LANG_RUBY && m_fake_content_pos > 0 &&
        m_org_buffer.size() >= m_fake_content_pos)
    {
        // Compare up to the end of </auto-generated> line (before fake content)
        if (std::equal(m_buffer.begin(), m_buffer.begin() + m_fake_content_pos,
                       m_org_buffer.begin()))
        {
            // Generated code matches. Now verify there's an 'end' statement somewhere
            // after the </auto-generated> marker in the original file.
            auto remaining = std::string_view(m_org_buffer).substr(m_fake_content_pos);

            // Find first non-whitespace character
            auto first_non_ws = remaining.find_first_not_of(" \t\r\n");
            if (first_non_ws != std::string_view::npos)
            {
                // There's content after </auto-generated> - check if it starts with 'end'
                auto content_start = remaining.substr(first_non_ws);
                if (content_start.starts_with("end"))
                {
                    // Original file has 'end' statement - file is current, no write needed.
                    return write_current;
                }
            }
            // No 'end' found - fall through to write the file with proper ending
        }
        // Content before </auto-generated> differs - fall through to Branch 3
    }

    // ========== BRANCH 2b: Handle non-Ruby files where original is larger ==========
    if (m_language != GEN_LANG_RUBY && m_org_buffer.size() > m_buffer.size())
    {
        // Non-Ruby languages: Original file has more content than our buffer.
        // Check if our new buffer matches the beginning portion of the original.
        if (std::equal(m_buffer.begin(), m_buffer.end(), m_org_buffer.begin()))
        {
            // New generated code matches the prefix of the original file exactly.
            // The extra content in the original is user code that should be preserved.
            // No write needed - file is current.
            return write_current;
        }
        // New buffer doesn't match prefix - fall through to Branch 3 to handle differences
    }

    // ========== BRANCH 2c: Handle non-Ruby files where buffers are same size ==========
    if (m_language != GEN_LANG_RUBY && m_org_buffer.size() == m_buffer.size())
    {
        // Non-Ruby, same size but different content - try to append original user content.
        auto begin_user_content = m_buffer.size();
        bool files_are_different = AppendOriginalUserContent(begin_user_content);

        if (!files_are_different)
        {
            // After appending, files match exactly - no write needed
            return write_current;
        }

        // Files still differ after appending user content - write is needed
        return (m_flags & flag_test_only) ? write_needed : WriteToFile();
    }

    // ========== BRANCH 3: Files differ in size or content significantly ==========
    // Files are different in size or content. We need to find where the end of the final comment
    // block is in the original file, and preserve anything after that as user content.

    // We could try searching for the final comment block in the original string buffer, however
    // we don't know if the line endings are still the same, so the *safe* way to do it is to
    // create a vector of std::string_views that we can use to search for the comment line.

    m_org_file.reserve(m_org_buffer.size() /
                       150);  // Pre-allocate assuming average line length of 80 chars
    m_org_file.ReadString(std::string_view(m_org_buffer));
    auto line = FindAdditionalContentIndex();
    if (line == -1)
    {
        // The original file is missing the final comment block marker.
        // This is unexpected - copy everything from the original file as user content
        // and add a warning comment. The fake user content (Ruby 'end' or C++ '};')
        // was already added by AppendEndOfFileBlock(), so we don't need to add it again.
        AppendMissingCommentBlockWarning();
        return (m_flags & flag_test_only) ? write_needed : WriteToFile();
    }

    if (m_org_file.size() > static_cast<size_t>(line))
    {
        // Real user content exists after the final comment block in the original file.
        // Set m_additional_content to mark where user content begins, then append it.
        m_additional_content = line;
        (void) AppendOriginalUserContent(0);
        return (m_flags & flag_test_only) ? write_needed : WriteToFile();
    }

    // Original file had no user content after the comment block, but files still differ.
    // This means the generated code itself has changed.
    // The end-of-file block (including fake user content) was already added by
    // AppendEndOfFileBlock(), so just write the file.
    return (m_flags & flag_test_only) ? write_needed : WriteToFile();
}

[[nodiscard]] auto FileCodeWriter::GetCommentLineToFind(GenLang language) -> std::string_view
{
    if (language == GEN_LANG_CPLUSPLUS)
    {
        return GetCppEndCommentLine();
    }
    if (language == GEN_LANG_PYTHON)
    {
        return GetPythonEndCommentLine();
    }
    if (language == GEN_LANG_RUBY)
    {
        return GetRubyEndCommentLine();
    }
    if (language == GEN_LANG_PERL)
    {
        return GetPerlEndCommentLine();
    }
    return {};
}

[[nodiscard]] auto FileCodeWriter::GetBlockLength(GenLang language) -> size_t
{
    if (language == GEN_LANG_CPLUSPLUS)
    {
        return GetCppEndBlockLength();
    }
    if (language == GEN_LANG_RUBY)
    {
        return GetRubyEndBlockLength();
    }
    if (language == GEN_LANG_PYTHON)
    {
        return GetPythonEndBlockLength();
    }
    if (language == GEN_LANG_PERL)
    {
        return GetPerlEndBlockLength();
    }
    return 0;
}

[[nodiscard]] auto FileCodeWriter::IsOldStyleFile() -> bool
{
    constexpr auto npos = std::string::npos;
    return m_org_file.size() > 3 && m_org_file[1].find("Code generated by wxUiEditor") != npos &&
           m_org_file[3].find(
               "DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!") != npos;
}

// Searches m_org_file for the final comment block and returns the line index where
// user content begins (the line after the comment block ends).
// Returns -1 if the comment block marker cannot be found.
//
// Handles both 1.2-style files (no </auto-generated> marker) and 1.3-style files
// (with </auto-generated> marker). Returns the first line after the comment block
// without skipping any content - the caller (AppendOriginalUserContent) is responsible
// for handling duplicate lines and preserving the original file's structure.
[[nodiscard]] auto FileCodeWriter::FindAdditionalContentIndex() -> std::ptrdiff_t
{
    // Step 1: Find the "End of generated code" comment line
    std::ptrdiff_t end_comment_line_index = -1;
    for (size_t line_index = 0; line_index < m_org_file.size(); ++line_index)
    {
        if (m_org_file[line_index].starts_with(m_comment_line_to_find))
        {
            end_comment_line_index = static_cast<std::ptrdiff_t>(line_index);
            break;
        }
    }

    if (end_comment_line_index == -1)
    {
        return -1;  // Comment block marker not found
    }

    // Step 2: Search forward from end_comment_line to find "</auto-generated>" marker (1.3 files)
    std::ptrdiff_t auto_generated_end_index = -1;
    for (size_t line_index = static_cast<size_t>(end_comment_line_index) + 1;
         line_index < m_org_file.size(); ++line_index)
    {
        if (m_org_file[line_index].find("</auto-generated>") != std::string_view::npos)
        {
            auto_generated_end_index = static_cast<std::ptrdiff_t>(line_index);
            break;
        }
    }

    if (auto_generated_end_index == -1)
    {
        // Old 1.2 style file - no "</auto-generated>" marker found
        // The user content starts immediately after the
        // "***********************************************" line. We found that line at
        // end_comment_line_index, so user content is on the next line. Note: We cannot use
        // m_block_length here because that's the NEW 1.3 block length, which has extra lines
        // compared to the old 1.2 block.

        // Search forward from the "End of generated code" line to find the line of asterisks
        for (size_t line_index = static_cast<size_t>(end_comment_line_index) + 1;
             line_index < m_org_file.size(); ++line_index)
        {
            auto line = m_org_file[line_index];
            if (line.find("***********************************************") !=
                std::string_view::npos)
            {
                // User content starts on the line after the asterisks
                auto user_content_start = static_cast<std::ptrdiff_t>(line_index + 1);

                // Return user_content_start - don't skip any content.
                // The original file's structure should be preserved exactly.
                return user_content_start;
            }
        }

        // If we didn't find the asterisk line (shouldn't happen), fall back to old logic
        return end_comment_line_index + static_cast<std::ptrdiff_t>(m_block_length);
    }

    // Step 3: New 1.3 style file - user content starts after "</auto-generated>"
    std::ptrdiff_t user_content_start = auto_generated_end_index + 1;

    // Step 4: Return the line after </auto-generated> as the start of user content.
    // We do NOT skip fake content (Ruby 'end', C++ '};', Perl '1;') here because:
    // - If there's real user content, it may appear BEFORE the closing statement
    // - The original file's structure should be preserved exactly
    // - AppendEndOfFileBlock() will only add fake content for NEW files
    return user_content_start;
}

void FileCodeWriter::AppendCppEndBlock()
{
    if (m_flags & code::flag_add_closing_brace)
    {
        wxue::StringVector lines;
        lines.ReadString(std::string_view(end_cpp_block));
        for (auto& iter: lines)
        {
            m_buffer += iter;
            m_buffer += "\n";
        }
        // Record position where fake content would start
        m_fake_content_pos = m_buffer.size();

        // Always add the closing brace here - it will be removed later if the original
        // file has content after the comment block that should be preserved instead.
        if (m_node)
        {
            Code code(m_node, GEN_LANG_CPLUSPLUS);
            code.Eol().Eol(eol_always).Str("};").Eol();
            m_buffer += code;
        }
    }
    else
    {
        m_buffer += end_cpp_block;
        m_fake_content_pos = 0;  // No fake content for this case
    }
}

void FileCodeWriter::AppendPerlEndBlock()
{
    m_buffer += end_perl_block;
    // Record position where fake content would start
    m_fake_content_pos = m_buffer.size();

    // Always add the module return value here - it will be removed later if the original
    // file has content after the comment block that should be preserved instead.
    if (m_node)
    {
        m_buffer << "\n1;  # " << m_node->get_NodeName();
    }
}

void FileCodeWriter::AppendPythonEndBlock()
{
    m_buffer += end_python_block;
    m_fake_content_pos = 0;  // Python has no fake content
}

void FileCodeWriter::AppendRubyEndBlock()
{
    m_buffer += end_ruby_block;
    // Record position where fake content would start
    m_fake_content_pos = m_buffer.size();

    if (m_node && !m_node->is_Gen(GenEnum::gen_Images) && !m_node->is_Gen(GenEnum::gen_Data))
    {
        // Always add the 'end' statement here - it will be removed later if the original
        // file has content after the comment block that should be preserved instead.
        Code code(m_node, GEN_LANG_RUBY);
        code.Eol().Str("end  # end of ").Str(m_node->get_NodeName()).Str(" class").Eol();
        m_buffer += code;
    }
    else
    {
        m_fake_content_pos = 0;  // No fake content for Images/Data
    }
}

void FileCodeWriter::AppendEndOfFileBlock()
{
    switch (m_language)
    {
        case GEN_LANG_CPLUSPLUS:
            AppendCppEndBlock();
            break;
        case GEN_LANG_PERL:
            AppendPerlEndBlock();
            break;
        case GEN_LANG_PYTHON:
            AppendPythonEndBlock();
            break;
        case GEN_LANG_RUBY:
            AppendRubyEndBlock();
            break;
        default:
            break;
    }
}

void FileCodeWriter::AppendMissingCommentBlockWarning()
{
    const auto* const comment_char = (m_language == GEN_LANG_CPLUSPLUS) ? "//" : "#";

    m_buffer += "\n";
    m_buffer += comment_char;
    m_buffer += "\n";
    m_buffer += comment_char;
    m_buffer += " The original file was missing the comment block ending the generated code!\n";
    m_buffer += comment_char;
    m_buffer += "\n";
    m_buffer += comment_char;
    m_buffer += " The entire original file has been copied below this comment block.\n\n";

    for (const auto& line: m_org_file)
    {
        m_buffer += line;
        m_buffer += "\n";
    }
}

auto FileCodeWriter::AppendOriginalUserContent(size_t begin_new_user_content) -> bool
{
    if (m_org_file.empty())
    {
        m_org_file.ReadString(std::string_view(m_org_buffer));
        m_additional_content = FindAdditionalContentIndex();
    }

    // Check if there's any content to preserve from the original file
    if (!wxue::is_found(m_additional_content) ||
        static_cast<size_t>(m_additional_content) >= m_org_file.size())
    {
        // No user content in original file - keep the fake content we already added
        return false;
    }

    // Skip any duplicate closing comment lines (asterisk lines) that may have been
    // introduced by earlier buggy versions. These lines start with comment chars followed
    // by asterisks, e.g., "// ***" or "# ***"
    // Also skip blank/whitespace-only lines.
    auto start_idx = static_cast<size_t>(m_additional_content);
    while (start_idx < m_org_file.size())
    {
        auto line = m_org_file[start_idx];

        // Skip blank/whitespace-only lines
        if (line.find_first_not_of(" \t\r\n") == std::string_view::npos)
        {
            ++start_idx;
            continue;
        }

        // Check for asterisk-only comment lines: "// ***..." or "# ***..."
        if ((line.starts_with("// ***") || line.starts_with("# ***")) &&
            line.find_first_not_of("/*# ") == std::string_view::npos)
        {
            ++start_idx;  // Skip this duplicate line
            continue;
        }

        break;  // Found meaningful content, stop skipping
    }

    // If there's no meaningful content left to append, keep the fake content
    if (start_idx >= m_org_file.size())
    {
        return false;
    }

    // There is meaningful content in the original file after the comment block.
    // Remove any fake content we added and replace with original content.
    if (m_fake_content_pos > 0)
    {
        m_buffer.erase(m_fake_content_pos);
    }
    else if (begin_new_user_content > 0)
    {
        // Fallback for cases where m_fake_content_pos wasn't set
        m_buffer.erase(begin_new_user_content);
    }

    // Calculate total length to reserve in m_buffer before appending user content
    size_t total_length = 0;
    for (auto idx = start_idx; idx < m_org_file.size(); ++idx)
    {
        total_length += m_org_file[idx].length() + 1;  // +1 for '\n'
    }
    m_buffer.reserve(m_buffer.size() + total_length);

    for (auto idx = start_idx; idx < m_org_file.size(); ++idx)
    {
        m_buffer += m_org_file[idx];
        m_buffer += "\n";
    }

    return true;
}

[[nodiscard]] auto FileCodeWriter::ReadOriginalFile(bool is_comparing) -> int
{
    wxFileName org_filename(m_filename);
    if (is_comparing)
    {
        wxString name = org_filename.GetName();
        name.Replace("~wxue_", "");
        org_filename.SetName(name);
    }

    wxFile file(org_filename.GetFullPath());
    if (file.IsOpened())
    {
        wxFileOffset length = file.Length();
        if (length > 0)
        {
            m_org_buffer.resize(static_cast<size_t>(length));
            if (file.Read(m_org_buffer.data(), length) == wxInvalidOffset)
            {
                m_org_buffer.clear();
                return write_cant_read;
            }
        }
        file.Close();
    }
    else
    {
        m_org_buffer.clear();
        return write_cant_read;
    }
    return 0;
}

[[nodiscard]] auto FileCodeWriter::EnsureDirectoryExists(int flags) -> int
{
    // In test-only mode, we don't need to check or create directories
    if (flags & flag_test_only)
    {
        return 0;
    }

    wxFileName dir(m_filename);
    dir.ClearExt();
    dir.RemoveLastDir();

    if ((!dir.GetFullPath().empty() && dir.DirExists()) ||
        wxGetApp().AskedAboutMissingDir(dir.GetFullPath().ToStdString()))
    {
        return 0;
    }

    if (wxGetApp().is_Generating() || (flags & code::flag_no_ui))
    {
        return write_no_folder;
    }

    wxString msg("The directory:\n    \"" + dir.GetFullPath() +
                 "\"\ndoesn't exist. Would you like it to be created?");
    wxMessageDialog dlg(nullptr, msg, "Generate Files", wxICON_WARNING | wxYES_NO);
    if (dlg.ShowModal() == wxID_YES)
    {
        if (!wxFileName::Mkdir(dir.GetFullPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
        {
            msg = "The directory:\n    \"" + dir.GetFullPath() + "\"\ncould not be created.";
            wxMessageDialog dlg_error(nullptr, msg, "Generate Files", wxICON_ERROR | wxOK);
            dlg_error.ShowModal();
            return write_cant_create;
        }
    }
    else
    {
        wxGetApp().AddMissingDir(dir.GetFullPath().ToStdString());
    }
    return 0;
}

[[nodiscard]] auto FileCodeWriter::WriteToFile() -> int
{
    wxFile fileOut;
    if (!fileOut.Create(m_filename.GetFullPath(), true))
    {
        return write_cant_create;
    }

    if (fileOut.Write(m_buffer.c_str(), m_buffer.length()) != m_buffer.length())
    {
        return write_error;
    }
    return write_success;
}
