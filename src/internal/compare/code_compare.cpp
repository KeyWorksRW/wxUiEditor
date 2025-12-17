/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code Generation Comparison
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "code_compare.h"

#include <wx/dir.h>  // wxDir is a class for enumerating the files in a directory

#include "diff.h"              // Diff -- Simple diff algorithm
#include "diff_viewer.h"       // DiffViewer -- Dialog for displaying file differences
#include "file_codewriter.h"   // FileCodeWriter -- Write code to disk with test mode support
#include "gen_common.h"        // Common code generation functions
#include "gen_cpp.h"           // CppCodeGenerator -- Generate C++ source code files
#include "gen_perl.h"          // PerlCodeGenerator -- Generate Perl code
#include "gen_python.h"        // PythonCodeGenerator -- Generate Python code
#include "gen_results.h"       // Code generation file writing functions
#include "gen_ruby.h"          // RubyCodeGenerator -- Generate Ruby code
#include "gen_xrc.h"           // XrcCodeGenerator -- Generate XRC code files
#include "mainframe.h"         // MainFrame -- Main window frame
#include "node.h"              // Node class
#include "project_handler.h"   // ProjectHandler class
#include "ttwx_view_vector.h"  // ttwx::ViewVector

// clang-format on

// AI Context: Get the range of lines containing actual generated code (excluding comment blocks).
// Returns pair of [start_index, end_index) for the generated code section.
auto CodeCompare::GetGeneratedCodeRange(const ttwx::ViewVector& content)
    -> std::pair<size_t, size_t>
{
    if (content.empty())
    {
        return { 0, 0 };
    }

    const std::string_view comment_char = (m_current_language == GEN_LANG_CPLUSPLUS) ? "//" : "#";
    const std::string_view end_marker = (m_current_language == GEN_LANG_CPLUSPLUS) ?
                                            "// ************* End of generated code" :
                                            "# ************* End of generated code";

    // Find and skip the beginning comment block (typically first 7-9 lines)
    size_t start_idx = 0;
    for (size_t idx = 0; idx < content.size() && idx < 15; ++idx)
    {
        auto line = content[idx];
        // Look for the end of the header comment block (usually "// clang-format off" or "#")
        if (line.starts_with(comment_char) && line.size() <= comment_char.size() + 1)
        {
            start_idx = idx + 1;
            break;
        }
        // Also check for "// clang-format off" which comes after the header
        if (line.find("clang-format off") != std::string_view::npos)
        {
            start_idx = idx + 1;
            break;
        }
    }

    // Find the ending comment block
    size_t end_idx = content.size();
    for (size_t idx = start_idx; idx < content.size(); ++idx)
    {
        if (content[idx].starts_with(end_marker))
        {
            end_idx = idx;
            // Back up to remove blank lines before the end marker
            while (end_idx > start_idx && content[end_idx - 1].empty())
            {
                --end_idx;
            }
            break;
        }
    }

    return { start_idx, end_idx };
}

namespace
{

    // Reconstruct string from ViewVector range
    auto ReconstructString(const ttwx::ViewVector& content, size_t start_idx, size_t end_idx)
        -> std::string
    {
        std::string result;
        for (size_t i = start_idx; i < end_idx; ++i)
        {
            result += content[i];
            if (i < end_idx - 1)
            {
                result += '\n';
            }
        }
        return result;
    }

}  // namespace

CodeCompare::~CodeCompare()
{
    wxArrayString files;

    // Some project files will be placed in a subdirectory which will be our current cwd.
    // However, the actual generated files can be pretty much anywhere. In the following, we
    // check to see if the parent directory is named "src" and if so, we change to the parent
    // directory. This allows us to find the generated files no matter where they are located,
    // or at least as long as they were generated under the src/ directory.
    tt_cwd cwd(tt_cwd::restore);
    cwd.remove_filename();
    if (cwd.size() && (cwd.back() == '\\' || cwd.back() == '/'))
    {
        cwd.pop_back();
    }
    if (cwd.filename() == "src")
    {
        cwd.ChangeDir("..");
    }

    wxDir::GetAllFiles(".", &files, "~wxue_**.*");

    for (auto& iter: files)
    {
        // ~wxue_.WinMerge will often be added to this list, but deleted before we start
        // processing, so check first
        if (wxFileExists(iter))
        {
            wxRemoveFile(iter);
        }
    }

    if (Project.HasValue(prop_base_directory))
    {
        wxDir::GetAllFiles(Project.as_string(prop_base_directory).make_wxString(), &files,
                           "~wxue_**.*");

        for (auto& iter: files)
        {
            // ~wxue_.WinMerge will often be added to this list, but deleted before we start
            // processing, so check first
            if (wxFileExists(iter))
            {
                wxRemoveFile(iter);
            }
        }
    }
}

void CodeCompare::OnInit(wxInitDialogEvent& /* event */)
{
    GenLang language = Project.get_CodePreference(wxGetFrame().getSelectedNode());
    wxCommandEvent dummy;
    switch (language)
    {
        case GEN_LANG_PYTHON:
            m_radio_python->SetValue(true);
            OnPython(dummy);
            break;
        case GEN_LANG_RUBY:
            m_radio_ruby->SetValue(true);
            OnRuby(dummy);
            break;
        case GEN_LANG_CPLUSPLUS:
        default:
            m_radio_cplusplus->SetValue(true);
            OnCPlusPlus(dummy);
            break;
    }
}

void CodeCompare::OnRadioButton(GenLang language)
{
    m_current_language = language;
    GenResults results;

    m_class_list.clear();
    m_list_changes->Clear();
    m_btn->Enable(false);

    bool result = false;
    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
            result = GenerateLanguageFiles(results, &m_class_list, GEN_LANG_CPLUSPLUS);
            break;
        case GEN_LANG_PERL:
            result = GenerateLanguageFiles(results, &m_class_list, GEN_LANG_PERL);
            break;
        case GEN_LANG_PYTHON:
            result = GenerateLanguageFiles(results, &m_class_list, GEN_LANG_PYTHON);
            break;
        case GEN_LANG_RUBY:
            result = GenerateLanguageFiles(results, &m_class_list, GEN_LANG_RUBY);
            break;
        case GEN_LANG_XRC:
            result = GenerateLanguageFiles(results, &m_class_list, GEN_LANG_XRC);
            break;

        default:
            m_current_language = GEN_LANG_CPLUSPLUS;
            FAIL_MSG(tt_string() << "Unknown language: " << language);
            break;
    }

    if (result)
    {
        for (auto& iter: m_class_list)
        {
            m_list_changes->AppendString(wxString::FromUTF8(iter));
        }
        m_btn->Enable();
    }
}

void CodeCompare::OnCPlusPlus(wxCommandEvent& /* event */)
{
    OnRadioButton(GEN_LANG_CPLUSPLUS);
}

void CodeCompare::OnPython(wxCommandEvent& /* event */)
{
    OnRadioButton(GEN_LANG_PYTHON);
}

void CodeCompare::OnRuby(wxCommandEvent& /* event unused */)
{
    OnRadioButton(GEN_LANG_RUBY);
}

void CodeCompare::OnPerl(wxCommandEvent& /* event */)
{
    OnRadioButton(GEN_LANG_PERL);
}

void CodeCompare::OnXRC(wxCommandEvent& /* event */)
{
    OnRadioButton(GEN_LANG_XRC);
}

auto CodeCompare::CollectFileDiffs() -> std::vector<FileDiff>
{
    std::vector<FileDiff> diffs;

    tt_cwd cwd(tt_cwd::restore);
    Project.ChangeDir();

    std::vector<Node*> forms;
    Project.CollectForms(forms);

    // Build flags for test-only comparison with no UI
    int write_flags = code::flag_test_only | code::flag_no_ui;

    for (const auto& class_name: m_class_list)
    {
        for (const auto& form: forms)
        {
            tt_string form_class_name(form->as_string(prop_class_name));
            if (form->is_Gen(gen_Images))
            {
                if (m_current_language != GEN_LANG_CPLUSPLUS)
                {
                    continue;
                }
                form_class_name = "Images";
            }
            else if (form->is_Gen(gen_Data))
            {
                if (m_current_language != GEN_LANG_CPLUSPLUS)
                {
                    continue;
                }
                form_class_name = "Data";
            }

            if (!form_class_name.is_sameas(class_name))
            {
                continue;
            }

            auto [path, has_base_file] = Project.GetOutputPath(form, m_current_language);
            if (!has_base_file)
            {
                continue;
            }

            std::unique_ptr<class BaseCodeGenerator> code_generator;
            switch (m_current_language)
            {
                case GEN_LANG_CPLUSPLUS:
                    code_generator = std::make_unique<CppCodeGenerator>(form);
                    break;

                case GEN_LANG_PYTHON:
                    code_generator = std::make_unique<PythonCodeGenerator>(form);
                    break;

                case GEN_LANG_RUBY:
                    code_generator = std::make_unique<RubyCodeGenerator>(form);
                    break;

                case GEN_LANG_PERL:
                    code_generator = std::make_unique<PerlCodeGenerator>(form);
                    break;

                case GEN_LANG_XRC:
                    code_generator = std::make_unique<XrcCodeGenerator>(form);
                    break;

                default:
                    FAIL_MSG(tt_string() << "Unknown m_current_language: " << m_current_language);
                    break;
            }

            if (!code_generator)
            {
                continue;
            }

            if (m_current_language == GEN_LANG_CPLUSPLUS)
            {
                // For C++: compare header file (.h)
                tt_string hdr_path(path);
                hdr_path += Project.get_ProjectNode()->as_string(prop_header_ext);

                auto h_cw = std::make_unique<FileCodeWriter>(hdr_path);
                code_generator->SetHdrWriteCode(h_cw.get());

                // For C++: also compare source file (.cpp)
                tt_string src_path(path);
                src_path += Project.get_ProjectNode()->as_string(prop_source_ext);

                auto cpp_cw = std::make_unique<FileCodeWriter>(src_path);
                code_generator->SetSrcWriteCode(cpp_cw.get());

                // Generate code into the FileCodeWriter buffers
                code_generator->GenerateClass(m_current_language);

                // Check if header file needs updating using FileCodeWriter's comparison logic
                int hdr_flags = write_flags;
                if (form->as_bool(prop_no_closing_brace))
                {
                    hdr_flags |= code::flag_add_closing_brace;
                }
                int hdr_result = h_cw->WriteFile(m_current_language, hdr_flags, form);

                if (hdr_result == code::write_needed)
                {
                    // File needs updating - create a diff for display
                    ttwx::ViewVector hdr_disk_content;
                    if (hdr_path.file_exists())
                    {
                        hdr_disk_content.ReadFile(std::string_view(hdr_path));
                    }

                    ttwx::ViewVector hdr_generated;
                    hdr_generated.ReadString(std::string_view(h_cw->GetString()));

                    // Get ranges for comparison (strips comment blocks for cleaner diff)
                    auto [disk_start, disk_end] = GetGeneratedCodeRange(hdr_disk_content);
                    auto [gen_start, gen_end] = GetGeneratedCodeRange(hdr_generated);

                    // Create stripped vectors for comparison
                    ttwx::ViewVector hdr_disk_stripped;
                    for (size_t i = disk_start; i < disk_end; ++i)
                    {
                        hdr_disk_stripped.push_back(hdr_disk_content[i]);
                    }

                    ttwx::ViewVector hdr_gen_stripped;
                    for (size_t i = gen_start; i < gen_end; ++i)
                    {
                        hdr_gen_stripped.push_back(hdr_generated[i]);
                    }

                    auto hdr_diff = Diff::Compare(hdr_disk_stripped, hdr_gen_stripped);
                    if (hdr_diff.has_differences)
                    {
                        FileDiff file_diff;
                        file_diff.filename = hdr_path.filename();
                        file_diff.original_content =
                            ReconstructString(hdr_disk_content, disk_start, disk_end);
                        file_diff.new_content =
                            ReconstructString(hdr_generated, gen_start, gen_end);
                        file_diff.diff_result = std::move(hdr_diff);
                        diffs.push_back(std::move(file_diff));
                    }
                }

                // Check if source file needs updating using FileCodeWriter's comparison logic
                int src_result = cpp_cw->WriteFile(m_current_language, write_flags, form);

                if (src_result == code::write_needed)
                {
                    // File needs updating - create a diff for display
                    ttwx::ViewVector src_disk_content;
                    if (src_path.file_exists())
                    {
                        src_disk_content.ReadFile(std::string_view(src_path));
                    }

                    ttwx::ViewVector src_generated;
                    src_generated.ReadString(std::string_view(cpp_cw->GetString()));

                    // Get ranges for comparison (strips comment blocks for cleaner diff)
                    auto [src_disk_start, src_disk_end] = GetGeneratedCodeRange(src_disk_content);
                    auto [src_gen_start, src_gen_end] = GetGeneratedCodeRange(src_generated);

                    // Create stripped vectors for comparison
                    ttwx::ViewVector src_disk_stripped;
                    for (size_t i = src_disk_start; i < src_disk_end; ++i)
                    {
                        src_disk_stripped.push_back(src_disk_content[i]);
                    }

                    ttwx::ViewVector src_gen_stripped;
                    for (size_t i = src_gen_start; i < src_gen_end; ++i)
                    {
                        src_gen_stripped.push_back(src_generated[i]);
                    }

                    auto src_diff = Diff::Compare(src_disk_stripped, src_gen_stripped);
                    if (src_diff.has_differences)
                    {
                        FileDiff src_file_diff;
                        src_file_diff.filename = src_path.filename();
                        src_file_diff.original_content =
                            ReconstructString(src_disk_content, src_disk_start, src_disk_end);
                        src_file_diff.new_content =
                            ReconstructString(src_generated, src_gen_start, src_gen_end);
                        src_file_diff.diff_result = std::move(src_diff);
                        diffs.push_back(std::move(src_file_diff));
                    }
                }
            }
            else
            {
                // For script languages: only compare source file
                tt_string script_path(path);
                switch (m_current_language)
                {
                    case GEN_LANG_PERL:
                        script_path += ".pl";
                        break;
                    case GEN_LANG_PYTHON:
                        script_path += ".py";
                        break;
                    case GEN_LANG_RUBY:
                        script_path += ".rb";
                        break;
                    case GEN_LANG_XRC:
                        script_path += ".xrc";
                        break;
                    default:
                        break;
                }

                auto script_cw = std::make_unique<FileCodeWriter>(script_path);
                code_generator->SetSrcWriteCode(script_cw.get());

                // Generate code into the FileCodeWriter buffer
                code_generator->GenerateClass(m_current_language);

                // Check if script file needs updating using FileCodeWriter's comparison logic
                int script_result = script_cw->WriteFile(m_current_language, write_flags, form);

                if (script_result == code::write_needed)
                {
                    // File needs updating - create a diff for display
                    ttwx::ViewVector script_disk_content;
                    if (script_path.file_exists())
                    {
                        script_disk_content.ReadFile(std::string_view(script_path));
                    }

                    ttwx::ViewVector script_generated;
                    script_generated.ReadString(std::string_view(script_cw->GetString()));

                    // Get ranges for comparison (strips comment blocks for cleaner diff)
                    auto [script_disk_start, script_disk_end] =
                        GetGeneratedCodeRange(script_disk_content);
                    auto [script_gen_start, script_gen_end] =
                        GetGeneratedCodeRange(script_generated);

                    // Create stripped vectors for comparison
                    ttwx::ViewVector script_disk_stripped;
                    for (size_t i = script_disk_start; i < script_disk_end; ++i)
                    {
                        script_disk_stripped.push_back(script_disk_content[i]);
                    }

                    ttwx::ViewVector script_gen_stripped;
                    for (size_t i = script_gen_start; i < script_gen_end; ++i)
                    {
                        script_gen_stripped.push_back(script_generated[i]);
                    }

                    auto script_diff = Diff::Compare(script_disk_stripped, script_gen_stripped);
                    if (script_diff.has_differences)
                    {
                        FileDiff file_diff;
                        file_diff.filename = script_path.filename();
                        file_diff.original_content = ReconstructString(
                            script_disk_content, script_disk_start, script_disk_end);
                        file_diff.new_content =
                            ReconstructString(script_generated, script_gen_start, script_gen_end);
                        file_diff.diff_result = std::move(script_diff);
                        diffs.push_back(std::move(file_diff));
                    }
                }
            }
        }
    }

    return diffs;
}

void CodeCompare::OnDiff(wxCommandEvent& /* event unused */)
{
    if (auto diffs = CollectFileDiffs(); !diffs.empty())
    {
        DiffViewer viewer(this, diffs);
        viewer.ShowModal();
        return;
    }

    wxMessageBox("No differences found between generated code and files on disk.",
                 "Code Comparison", wxOK | wxICON_INFORMATION);
}

// Static method for non-UI code comparison (used by verify_codegen)
auto CodeCompare::CollectFileDiffsForLanguage(GenLang language) -> std::vector<FileDiff>
{
    // Create a temporary instance to use the existing comparison logic
    CodeCompare comparer;
    comparer.m_current_language = language;

    // Generate language files to populate class_list with forms that would change
    GenResults results;
    GenerateLanguageFiles(results, &comparer.m_class_list, language);

    // Now collect the actual diffs
    return comparer.CollectFileDiffs();
}
