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
#include "ttwx_string_vector.h"  // StringVector -- ttwx::StringVector class
#include "ttwx_view_vector.h"    // ViewVector -- ttwx::ViewVector class

using namespace code;

// For all languages except Ruby and in some cases C++ header files, all we need to check is whether
// m_buffer matches exactly to the first part of the original file. If it doesn't match, then we
// grab everything after the final comment block as user content to append to the end of the newly
// generated code and write it out.

const std::string_view cpp_end_cmt_line = "// ************* End of generated code";

auto FileCodeWriter::WriteFile(GenLang language, int flags, Node* node) -> int
{
    ASSERT_MSG(!m_filename.GetFullPath().IsEmpty(),
               "Filename must be set before calling WriteFile()");

    if (int dir_result = EnsureDirectoryExists(flags); dir_result != 0)
    {
        return dir_result;
    }

    // Initialize member variables
    m_node = node;
    m_language = language;
    m_flags = flags;

    // Note that AppendEndOfFileBlock() may change m_block_length, so be sure to set it first.
    m_block_length = GetBlockLength(language);

    AppendEndOfFileBlock();

    m_file_exists = m_filename.FileExists();
    if (!m_file_exists)
    {
        AppendFakeUserContent();
        return (flags & flag_test_only) ? write_needed : WriteToFile();
    }

    m_additional_content = -1;
    m_comment_line_to_find = GetCommentLineToFind(language);
    bool is_comparing = (!m_filename.GetName().IsEmpty() && m_filename.GetName()[0] == '~');
    if (int result = ReadOriginalFile(is_comparing); result != 0)
    {
        // REVIEW: [Randalphwa - 11-06-2025] Need to be sure caller handles this properly
        return result;
    }

    // At this point, m_buffer contains the newly generated code, but not any possible fake user
    // content. m_org_buffer contains the contents of the original file.

    if (m_buffer.size() == m_org_buffer.size())
    {
        return HandleEqualSizeBuffers();
    }

    if (m_org_buffer.size() > m_buffer.size())
    {
        return HandleLargerOriginalFile();
    }

    return ProcessDifferentSizeFiles();
}

[[nodiscard]] auto FileCodeWriter::HandleEqualSizeBuffers() -> int
{
    if (std::equal(m_buffer.begin(), m_buffer.end(), m_org_buffer.begin()))
    {
        // They are equal through the end of the final comment block, but it may be missing the
        // fake user content that we might need to add.
        if (AppendFakeUserContent() == 0)
        {
            return write_current;
        }
        // If additional content was added after the final comment block, then we need to write
        // out the file again.
        return (m_flags & flag_test_only) ? write_needed : WriteToFile();
    }

    // Buffers aren't the same, however that might be because of missing fake user content in
    // our new buffer.
    auto begin_user_content = m_buffer.size();
    bool files_are_different = AppendOriginalUserContent(begin_user_content);

    if (!files_are_different)
    {
        return write_current;
    }

    return (m_flags & flag_test_only) ? write_needed : WriteToFile();
}

[[nodiscard]] auto FileCodeWriter::HandleLargerOriginalFile() -> int
{
    // The only thing we change is m_buffer, so as long as the m_buffer portion of m_org_buffer is
    // the same, then we don't need to write anything.
    if (std::equal(m_buffer.begin(), m_buffer.end(), m_org_buffer.begin()))
    {
        return write_current;
    }

    // Files are different, need to process them
    return ProcessDifferentSizeFiles();
}

[[nodiscard]] auto FileCodeWriter::ProcessDifferentSizeFiles() -> int
{
    // We now know that the files are different in size or content. We need to find where the end of
    // the final comment block is in the original file, and preserve anything after that as user
    // content.

    // We could try searching for the final comment block in the original string buffer, however we
    // don't know if the line endings are still the same, so the *safe* was to do it is to create a
    // vector of std::string_views that we can use to search for the comment line.

    m_org_file.ReadString(std::string_view(m_org_buffer));
    auto line = FindAdditionalContentIndex();
    if (line == -1)
    {
        // The original file is missing the final comment block. We have no choice but to copy
        // everything from the original file as user content.
        AppendFakeUserContent();
        AppendMissingCommentBlockWarning();
        return WriteToFile();
    }

    if (m_org_file.size() > static_cast<size_t>(line))
    {
        // There's real user content in the original file so add it before writing.
        m_additional_content = line;
        AppendOriginalUserContent(0);
        return WriteToFile();
    }

    // If we get here, then the original file had no additional content, but the files still differ,
    // so add any fake user content we might need, then write the file.

    AppendFakeUserContent();
    return WriteToFile();
}

auto FileCodeWriter::AppendFakeUserContent() -> size_t
{
    auto cur_buffer_size = m_buffer.size();

    // Ruby has to have an 'end' statement for the class. If there's nothing after the
    // final comment block, then we need to add it.
    if (m_node && !m_node->is_Gen(GenEnum::gen_Images) && !m_node->is_Gen(GenEnum::gen_Data))
    {
        // If the file is Ruby code, and there is no actual additional content
        // then add the "end" statement for the class.
        if (m_language == GEN_LANG_RUBY)
        {
            Code code(m_node, GEN_LANG_RUBY);
            code.Eol().Str("end  # end of ").Str(m_node->get_NodeName()).Str(" class").Eol();
            m_buffer += code;
        }
        if (m_language == GEN_LANG_CPLUSPLUS && (m_flags & code::flag_add_closing_brace))
        {
            // If the file is C++ code, and there is no actual additional content
            // then add the closing brace for the class.
            Code code(m_node, GEN_LANG_CPLUSPLUS);
            code.Eol().Tab().Str("// Clang-format on").Eol().Str("};").Eol();
            m_buffer += code;
        }
    }
    return m_buffer.size() - cur_buffer_size;
}

[[nodiscard]] auto FileCodeWriter::GetCommentLineToFind(GenLang language) -> std::string_view
{
    if (language == GEN_LANG_CPLUSPLUS)
    {
        return cpp_end_cmt_line;
    }
    if (language == GEN_LANG_PYTHON || language == GEN_LANG_RUBY || language == GEN_LANG_PERL)
    {
        return python_perl_ruby_end_cmt_line;
    }
    return {};
}

[[nodiscard]] auto FileCodeWriter::GetBlockLength(GenLang language) -> size_t
{
    return (language == GEN_LANG_CPLUSPLUS) ? GetCppEndBlockLength() :
                                              GetPythonPerlRubyEndBlockLength();
}

[[nodiscard]] auto FileCodeWriter::GetCommentCharacter(GenLang language) -> std::string_view
{
    return (language == GEN_LANG_CPLUSPLUS) ? "//" : "#";
}

[[nodiscard]] auto FileCodeWriter::IsOldStyleFile() -> bool
{
    constexpr auto npos = std::string::npos;
    return m_org_file.size() > 3 && m_org_file[1].find("Code generated by wxUiEditor") != npos &&
           m_org_file[3].find(
               "DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!") != npos;
}

[[nodiscard]] auto FileCodeWriter::FindAdditionalContentIndex() -> std::ptrdiff_t
{
    // Search for the comment line in m_org_file
    for (size_t line_index = 0; line_index < m_org_file.size(); ++line_index)
    {
        auto line = m_org_file[line_index];

        // Only the length of m_comment_line_to_find matters -- the amount of comment characters
        // after that is irrelevant.
        if (line.starts_with(m_comment_line_to_find))
        {
            return static_cast<std::ptrdiff_t>(line_index + m_block_length);
        }
    }

    return -1;
}

void FileCodeWriter::AppendCppEndBlock()
{
    if (m_flags & code::flag_add_closing_brace)
    {
        ttwx::StringVector lines;
        lines.ReadString(std::string_view(end_cpp_block));
        for (auto& iter: lines)
        {
            if (iter.starts_with("// clang-format on"))
            {
                --m_block_length;
            }
            else
            {
                m_buffer += iter;
                m_buffer += "\n";
            }
        }
    }
    else
    {
        m_buffer += end_cpp_block;
    }
}

void FileCodeWriter::AppendPerlEndBlock()
{
    m_buffer += end_python_perl_ruby_block;
    if (!m_file_exists && m_node)
    {
        m_buffer += "\n1;  # " + m_node->get_NodeName();
    }
}

void FileCodeWriter::AppendPythonEndBlock()
{
    m_buffer += end_python_perl_ruby_block;
}

void FileCodeWriter::AppendRubyEndBlock()
{
    m_buffer += end_python_perl_ruby_block;
    if (m_node && !m_node->is_Gen(GenEnum::gen_Images) && !m_node->is_Gen(GenEnum::gen_Data))
    {
        // If the file doesn't exist, or it is missing any user content, append psuedo user
        // content that adds the "end" statement for the class.
        if (!m_file_exists || !ttwx::is_found(m_additional_content))
        {
            Code code(m_node, GEN_LANG_RUBY);
            code.Eol().Str("end  # end of ").Str(m_node->get_NodeName()).Str(" class").Eol();
            m_buffer += code;
        }
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
    const auto comment_char = GetCommentCharacter(m_language);

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

    if (begin_new_user_content > 0)
    {
        ttwx::ViewVector new_content_view;
        new_content_view.ReadString(std::string_view(m_buffer).substr(begin_new_user_content));
        if (new_content_view.size() ==
            (m_org_file.size() - static_cast<size_t>(m_additional_content) + 1))
        {
            return false;  // Original user content matches newly added fake user content
        }

        // Remove any fake user content that was added
        m_buffer.erase(begin_new_user_content);
        begin_new_user_content = 0;  // In case this gets checked later (it doesn't currently)
    }

    // Calculate total length to reserve in m_buffer before appending user content
    size_t total_length = 0;
    for (auto idx = static_cast<size_t>(m_additional_content); idx < m_org_file.size(); ++idx)
    {
        total_length += m_org_file[idx].length() + 1;  // +1 for '\n'
    }
    m_buffer.reserve(m_buffer.size() + total_length);

    for (auto idx = static_cast<size_t>(m_additional_content); idx < m_org_file.size(); ++idx)
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
