//////////////////////////////////////////////////////////////////////////
// Purpose:   Classs to write code to disk
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/file.h>      // wxFile - encapsulates low-level "file descriptor"
#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/msgdlg.h>    // common header and base class for wxMessageDialog

#include "file_codewriter.h"

#include "mainapp.h"  // App -- Main application class

using namespace code;

// clang-format off

inline constexpr const auto end_cpp_block =
R"===(
// ************* End of generated code ***********
// DO NOT EDIT THIS COMMENT BLOCK!
//
// Code below this comment block will be preserved
// if the code for this class is re-generated.
//
// clang-format on
// ***********************************************
)===";

inline constexpr const auto end_fortran_block =
R"===(! ************* End of generated code ***********
! DO NOT EDIT THIS COMMENT BLOCK!
!
! Code below this comment block will be preserved
! if the code for this class is re-generated.
! ***********************************************
)===";

inline constexpr const auto end_python_perl_ruby_block =
R"===(# ************* End of generated code ***********
# DO NOT EDIT THIS COMMENT BLOCK!
#
# Code below this comment block will be preserved
# if the code for this class is re-generated.
# ***********************************************
)===";

inline constexpr const auto end_lua_haskell_block =
R"===(-- ************* End of generated code ***********
-- DO NOT EDIT THIS COMMENT BLOCK!
--
-- Code below this comment block will be preserved
-- if the code for this class is re-generated.
-- ***********************************************
)===";

const char* cpp_rust_end_cmt_line = "// ************* End of generated code";
const char* fortran_end_cmt_line = "! ************* End of generated code";
const char* python_perl_ruby_end_cmt_line = "# ************* End of generated code";
const char* lua_haskell_cmt_line = "-- ************* End of generated code";

// clang-format on

int FileCodeWriter::WriteFile(GenLang language, int flags, Node* node)
{
    ASSERT_MSG(m_filename.size(), "Filename must be set before calling WriteFile()");
    m_node = node;
    bool file_exists = m_filename.file_exists();
    if (!file_exists && (flags & flag_test_only))
        return write_needed;

    // It's critical that this be accurate as it allows us to step over the generated comment
    // block.
    size_t block_length = (language == GEN_LANG_CPLUSPLUS) ? 8 : 6;

    if (language == GEN_LANG_CPLUSPLUS)
    {
        if (flags & flag_add_closing_brace)
        {
            tt_string_vector lines;
            lines.ReadString(end_cpp_block);
            for (auto& iter: lines)
            {
                if (iter.is_sameprefix("// clang-format on"))
                {
                    // Skip this line, and reduce the block length acccordingly
                    --block_length;
                }
                else
                {
                    m_buffer << iter << "\n";
                }
            }
            if (!file_exists)
            {
                m_buffer += "  // clang-format on\n};\n";
            }
        }
        else
        {
            m_buffer += end_cpp_block;
        }
    }
    else if (language == GEN_LANG_PERL)
    {
        m_buffer += end_python_perl_ruby_block;

        // If the file has never been written before, then add "1;" line that is required to close
        // the package. This is written outside of the comment block, so presumably any
        // user edits will be made above this line or they will remove it and replace it with their
        // own "1;" line.
        if (!file_exists)
        {
            // TODO: [Randalphwa - 01-09-2025] Ideally, this should be followed with a comment
            // indicating that it is the end of the form's package. However, that requires knowing
            // the node we are writing.
            m_buffer += "\n1;";
            if (m_node)
            {
                m_buffer += "  # " + m_node->getNodeName();
            }
        }
    }
    else if (language == GEN_LANG_PYTHON)
    {
        m_buffer += end_python_perl_ruby_block;
    }
    else if (language == GEN_LANG_RUBY)
    {
        m_buffer += end_python_perl_ruby_block;

        // If the file has never been written before, then add "end" line that is required to close
        // the class definition. This is written outside of the comment block, so presumably any
        // user edits will be made above this line or they will remove it and replace it with their
        // own "end" line.
        if (!file_exists)
        {
            m_buffer += "\nend";
            if (m_node)
            {
                m_buffer += "  # " + m_node->getNodeName();
            }
        }
    }
    else if (language == GEN_LANG_RUST)
    {
        m_buffer += end_cpp_block;
    }

#if GENERATE_NEW_LANG_CODE
    else if (language == GEN_LANG_FORTRAN)
    {
        m_buffer += end_fortran_block;
    }
    else if (language == GEN_LANG_LUA || language == GEN_LANG_HASKELL)
    {
        m_buffer += end_lua_haskell_block;
    }
#endif  // GENERATE_NEW_LANG_CODE

    size_t additional_content = (to_size_t) -1;
    bool old_style_file = false;  // true if this doesn't have a trailing comment block

    bool is_comparing = (m_filename.filename()[0] == '~');

    if (file_exists || is_comparing)
    {
        // If the file exists, then we need to read it and compare the generated portion to
        // our generated portion, ignoring initial whitespace and end-of-line character(s).
        // If all of the generated code matches, then we don't need to write the file.

        tt_view_vector org_file;
        if (is_comparing)
        {
            tt_string org_filename(m_filename);
            org_filename.Replace("~wxue_", "");
            if (!org_file.ReadFile(org_filename))
            {
                return write_cant_read;
            }
        }
        else if (!org_file.ReadFile(m_filename))
        {
            return write_cant_read;
        }

        tt_view_vector new_file;
        new_file.ReadString(m_buffer);

        std::string_view look_for = {};
        if (language == GEN_LANG_CPLUSPLUS || language == GEN_LANG_RUST)
            look_for = cpp_rust_end_cmt_line;
        else if (language == GEN_LANG_PYTHON || language == GEN_LANG_RUBY || language == GEN_LANG_PERL)
            look_for = python_perl_ruby_end_cmt_line;

#if GENERATE_NEW_LANG_CODE
        else if (language == GEN_LANG_FORTRAN)
            look_for = fortran_end_cmt_line;
        else if (language == GEN_LANG_LUA || language == GEN_LANG_HASKELL)
            look_for = lua_haskell_cmt_line;
#endif  // GENERATE_NEW_LANG_CODE

        if (org_file.size() > 3 && org_file[1].contains("Code generated by wxUiEditor") &&
            org_file[3].contains("DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!"))
        {
            if (flags & flag_test_only)
                return write_needed;
            old_style_file = true;
        }

        if (!old_style_file)
        {
            size_t line_index;
            bool files_are_different = false;
            for (line_index = 0; line_index < new_file.size(); ++line_index)
            {
                if (line_index >= org_file.size())
                {
                    files_are_different = true;
                    break;
                }
#if defined(_DEBUG)
                // In DEBUG builds, we also want to check leading indentation
                auto org_start = org_file[line_index];
                auto new_start = new_file[line_index];
#else
                auto org_start = org_file[line_index].view_nonspace();
                auto new_start = new_file[line_index].view_nonspace();
#endif  // _DEBUG
                if (org_start != new_start)
                {
                    files_are_different = true;
                    break;
                }

                if (org_file[line_index].is_sameprefix(look_for))
                {
                    additional_content = line_index + block_length;
                    break;
                }
            }

            if (!files_are_different)
                return write_current;

            if (!tt::is_found(additional_content))
            {
                for (; line_index < org_file.size(); ++line_index)
                {
                    if (org_file[line_index].is_sameprefix(look_for))
                    {
                        additional_content = line_index + block_length;
                        break;
                    }
                }
            }

            if (!tt::is_found(additional_content))
            {
                // This is bad -- it means the original file no longer has the comment block
                // ending the generated code. We don't want to overwrite the changes, so instead
                // we add a comment to the end of the new file indicating that the original
                // comment block is missing, and copying the new file after that comment block.

                tt_string comment_begin;
                if (language == GEN_LANG_CPLUSPLUS)
                    comment_begin = "\n// ";
                else if (language == GEN_LANG_PYTHON || language == GEN_LANG_PERL)
                    comment_begin = "\n# ";

                m_buffer += ((language == GEN_LANG_CPLUSPLUS) ? "\n//" : "\n#");
                m_buffer << comment_begin << "The original file was missing the comment block ending the generated code!\n";
                m_buffer += ((language == GEN_LANG_CPLUSPLUS) ? "//" : "#");
                m_buffer << comment_begin << "The entire original file has been copied below this comment block.\n\n";
                for (size_t idx = 0; idx < org_file.size(); ++idx)
                {
                    m_buffer += org_file[idx];
                    m_buffer += "\n";
                }
            }
            else
            {
                // Allocate additional space for the user content, plus a newline for each line
                size_t expansion_needed = 0;
                for (auto& iter: org_file)
                {
                    expansion_needed += (iter.size() + 1);
                }
                m_buffer.reserve(m_buffer.size() + expansion_needed);

                // Add the user content after our generated code comment block
                for (size_t idx = additional_content; idx < org_file.size(); ++idx)
                {
                    m_buffer += org_file[idx];
                    m_buffer += "\n";
                }
            }
        }
    }

    if (flags & flag_test_only)
        return write_needed;

    // At this point, m_buffer contains the new generated content plus any additional content
    // the user already added. Note that indentation of the user added code is preserved, but
    // line endings are forced to '\n'.

    // Make certain the folder we are supposed to write to exists
    tt_string copy(m_filename);
    copy.remove_filename();
    if (copy.size() && !copy.dir_exists() && !wxGetApp().AskedAboutMissingDir(copy))
    {
        if (wxGetApp().isGenerating())
        {
            return write_no_folder;
        }

        // Use wxMessageDialog() rather than wxMessageBox() because it will correctly handle a
        // long filename, whereas wxMessageBox() would truncate a long filename.

        std::string msg("The directory:\n    \"" + copy + "\"\ndoesn't exist. Would you like it to be created?");
        wxMessageDialog dlg(nullptr, wxString::FromUTF8(msg), "Generate Files", wxICON_WARNING | wxYES_NO);
        if (dlg.ShowModal() == wxID_YES)
        {
            if (!tt_string::MkDir(copy))
            {
                msg = "The directory:\n    \"" + copy + "\"\ncould not be created.";
                wxMessageDialog dlg_error(nullptr, wxString::FromUTF8(msg), "Generate Files", wxICON_ERROR | wxOK);
                dlg_error.ShowModal();
                return write_cant_create;
            }
        }
        else
        {
            wxGetApp().AddMissingDir(copy);
        }
    }

    wxFile fileOut;
    if (!fileOut.Create(m_filename.make_wxString(), true))
    {
        return write_cant_create;
    }

    if (fileOut.Write(m_buffer.c_str(), m_buffer.length()) != m_buffer.length())
    {
        return write_error;
    }

    if (tt::is_found(additional_content))
        return write_edited;
    else
        return write_success;
}
