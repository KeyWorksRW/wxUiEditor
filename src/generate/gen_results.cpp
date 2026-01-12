/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code generation file writing functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <cstring>  // for std::memcmp
#include <format>
#include <future>    // for std::async, std::future
#include <memory>    // for std::shared_ptr, std::make_shared
#include <optional>  // for std::optional

#include <wx/file.h>     // wxFile - raw file I/O
#include <wx/progdlg.h>  // wxProgressDialog

#include "gen_results.h"

#include "../tools/compare/diff.h"  // Diff -- Compare for file diffs
#include "file_codewriter.h"        // FileCodeWriter -- Write code to disk with test mode support
#include "gen_common.h"             // GeneratorLibrary
#include "mainapp.h"                // wxGetApp()
#include "mainframe.h"              // MainFrame -- Main window frame
#include "node.h"                   // Node class
#include "project_handler.h"        // Project

#include "wxue_string.h"       // wxue::string, wxue::SaveCwd
#include "wxue_view_vector.h"  // wxue::ViewVector (for Diff::Compare)

#include "gen_base.h"  // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_cpp.h"
#include "gen_perl.h"    // PerlCodeGenerator class
#include "gen_python.h"  // PythonCodeGenerator -- Generate wxPython code
#include "gen_ruby.h"    // RubyCodeGenerator -- Generate wxRuby code
#include "gen_xrc.h"     // XrcCodeGenerator -- Generate XRC code
#include "write_code.h"  // WriteCode -- Write code to a string

void GenResults::SetNodes(Node* startNode)
{
    m_start_node = startNode;
    m_target_nodes.clear();

    if (!startNode)
    {
        FAIL_MSG("SetNodes called with nullptr");
        m_scope = Scope::unknown;
        return;
    }

    // Infer scope from node type
    if (startNode->is_Gen(gen_Project))
    {
        m_scope = Scope::project;
    }
    else if (startNode->is_Gen(gen_folder) || startNode->is_Gen(gen_sub_folder))
    {
        m_scope = Scope::folder;
    }
    else if (startNode->is_Form())
    {
        m_scope = Scope::form;
        m_target_nodes.push_back(startNode);
    }
    else
    {
        // For non-form nodes, navigate to containing form
        auto* form = startNode->get_Form();
        if (form)
        {
            m_scope = Scope::form;
            m_target_nodes.push_back(form);
        }
        else
        {
            FAIL_MSG("SetNodes called with a node that is not a form or folder, and does not have "
                     "a parent form");
            m_scope = Scope::unknown;
        }
    }
}

void GenResults::SetNodes(const std::vector<Node*>& nodes)
{
    ASSERT_MSG(!nodes.empty(), "SetNodes called with empty node list");
    m_start_node = nullptr;
    m_target_nodes = nodes;
    m_scope = Scope::form;  // Explicit list is treated as form-level scope
}

auto GenResults::SetDisplayTarget(Node* startNode, GenLang language, WriteCode* src, WriteCode* hdr,
                                  PANEL_PAGE panel_page) -> bool
{
#ifdef _DEBUG
    if (!Project.is_UiAllowed())
    {
        FAIL_MSG("SetDisplayTarget called when UI is not allowed");
        return false;
    }
#endif  // _DEBUG

    ASSERT_MSG(startNode != nullptr, "SetDisplayTarget called with nullptr startNode");
    // TODO: [Randalphwa - 12-21-2025] Technically, we can get the currently selected node from the
    // navigation panel and use that. For now, we will require the caller to do that if needed, and
    // we should after the refactoring is done we can reconsider this.

    if (startNode == nullptr)
    {
        return false;
    }
    // Validate: at least one WriteCode must be non-null
    if (!src && !hdr)
    {
        FAIL_MSG("SetDisplayTarget called with both src and hdr as nullptr");
        return false;
    }

    // Validate: must be exactly one language (not multiple bits set)
    // Count bits set in language
    auto lang_bits = static_cast<std::uint16_t>(language);
    if (lang_bits == 0 || (lang_bits & (lang_bits - 1)) != 0)
    {
        // Either no language or multiple languages specified

        FAIL_MSG("SetDisplayTarget called with no language or multiple languages specified");
        return false;
    }

    // Validate: cannot be project or folder node
    if (startNode && startNode->is_FormParent())
    {
        FAIL_MSG("SetDisplayTarget called with project or folder node");
        // TODO: [Randalphwa - 12-21-2025] Technically it can -- the default behaviour for all
        // panels when a form parent is selected in the navigation panel, then the first child form
        // is used. For now, we will keep this restriction in place until the display generation
        // code is more fully implemented.
        return false;
    }

    // Set up for display mode
    m_scope = Scope::display;
    m_languages = language;
    m_display_src = src;
    m_display_hdr = hdr;
    m_panel_page = panel_page;

    // Navigate to form if needed
    if (startNode)
    {
        if (!startNode->is_Form())
        {
            auto* form = startNode->get_Form();
            ASSERT_MSG(form, "SetDisplayTarget called with non-form node that has no parent form");
            if (!form)
            {
                return false;
            }
            startNode = form;
        }
        m_target_nodes.clear();
        m_target_nodes.push_back(startNode);
    }

    return true;
}

auto GenResults::Generate() -> bool
{
    ASSERT_MSG(m_scope != Scope::unknown, "Generate called without setting nodes or scope");
    if (m_scope == Scope::unknown)
    {
        return false;
    }

    // Collect forms if we haven't already
    if (m_scope == Scope::project || m_scope == Scope::folder)
    {
        CollectFormsFromNodes();
    }

    if (m_scope == Scope::display)
    {
        return GenerateForDisplay();
    }

    bool comparison_only = (m_mode == Mode::compare_only);

    if (wxGetApp().isTestingMenuEnabled())
    {
        StartClock();
    }

    wxue::SaveCwd cwd(wxue::restore_cwd);
    Project.ChangeDir();

    bool generate_result = false;

    // For form scope, generate directly for each form in m_target_nodes
    if (m_scope == Scope::form && !m_target_nodes.empty())
    {
        for (auto lang: gen_lang_set)
        {
            if (m_languages & lang)
            {
                // Save current language for GenerateLanguageForm/GenerateCppForm
                auto saved_lang = m_languages;
                m_languages = lang;

                for (auto* form: m_target_nodes)
                {
                    if (GenerateLanguageForm(form->as_view(prop_class_name), form, comparison_only))
                    {
                        generate_result = true;
                    }
                }

                m_languages = saved_lang;
            }
        }
    }
    else
    {
        // For project/folder scope, use GenerateLanguageFiles which handles CMake etc.
        for (auto lang: gen_lang_set)
        {
            if (m_languages & lang)
            {
                // Save current language for GenerateLanguageForm/GenerateCppForm
                auto saved_lang = m_languages;
                m_languages = lang;

                if (GenerateLanguageFiles(lang, comparison_only))
                {
                    generate_result = true;
                }

                m_languages = saved_lang;
            }
        }
    }

    if (wxGetApp().isTestingMenuEnabled())
    {
        WaitForPendingDiffs();  // Wait for all async diffs to complete
        EndClock();
    }

    return generate_result;
}

void GenResults::CollectFormsFromNodes()
{
    m_target_nodes.clear();

    if (m_scope == Scope::project)
    {
        Project.CollectForms(m_target_nodes);
    }
    else if (m_scope == Scope::folder && m_start_node)
    {
        // Recursively collect forms from the folder
        auto CollectFromFolder = [&](auto&& self, Node* folder) -> void
        {
            for (const auto& child: folder->get_ChildNodePtrs())
            {
                if (child->is_Form())
                {
                    m_target_nodes.push_back(child.get());
                }
                else if (child->is_Gen(gen_folder) || child->is_Gen(gen_sub_folder))
                {
                    self(self, child.get());
                }
            }
        };
        CollectFromFolder(CollectFromFolder, m_start_node);
    }
}

auto GenResults::GenerateForDisplay() -> bool
{
    ASSERT_MSG(!m_target_nodes.empty(), "GenerateForDisplay called with no target forms");
    if (m_target_nodes.empty())
    {
        return false;
    }

    ASSERT_MSG(m_display_src || m_display_hdr,
               "GenerateForDisplay called without WriteCode targets");
    if (!m_display_src && !m_display_hdr)
    {
        return false;
    }

    auto* form = m_target_nodes[0];
    ASSERT_MSG(form && form->is_Form(), "GenerateForDisplay target is not a form");
    if (!form || !form->is_Form())
    {
        return false;
    }

    // Create the appropriate language-specific code generator
    std::unique_ptr<BaseCodeGenerator> code_generator;
    switch (m_languages)
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
            FAIL_MSG(wxString() << "Unknown language for GenerateForDisplay: " << m_languages);
            return false;
    }

    // Clear and set up the WriteCode targets
    if (m_display_src)
    {
        m_display_src->Clear();
        code_generator->SetSrcWriteCode(m_display_src);
    }

    if (m_display_hdr)
    {
        m_display_hdr->Clear();
        code_generator->SetHdrWriteCode(m_display_hdr);
    }

    // Generate the code using the panel page for line tracking
    code_generator->GenerateClass(m_languages, m_panel_page);

    return true;
}

void GenResults::StartClock()
{
    if (m_clock_started)
    {
        return;
    }
    m_start_time = std::chrono::steady_clock::now();
    m_clock_started = true;
}

void GenResults::EndClock()
{
    auto end_time = std::chrono::steady_clock::now();
    m_elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time - m_start_time).count();
    m_msgs.emplace_back(std::format("Elapsed time: {} milliseconds", m_elapsed));
    m_clock_started = false;
}

void GenResults::Clear()
{
    WaitForPendingDiffs();  // Ensure all pending work completes

    m_elapsed = 0;
    m_file_count = 0;
    m_clock_started = false;
    m_msgs.clear();
    m_updated_files.clear();
    m_created_files.clear();
    m_file_diffs.clear();

    // Reset new Phase 1 members
    m_mode = Mode::generate_and_write;
    m_scope = Scope::unknown;
    m_languages = GEN_LANG_NONE;
    m_target_nodes.clear();
    m_start_node = nullptr;
    m_display_src = nullptr;
    m_display_hdr = nullptr;
    m_panel_page = PANEL_PAGE::NOT_PANEL;
    m_combined_output_path.clear();
}

void GenResults::EnableProgressDialog(const wxString& title)
{
    m_show_progress = true;
    m_progress_title = title;
}

constexpr int progress_forms_step = 50;
constexpr size_t max_diff_file_size = (100 * 1024);  // 102,400 bytes

auto GenResults::GenerateLanguageFiles(GenLang language, bool comparison_only) -> bool
{
    if (Project.get_ChildCount() == 0)
    {
        return false;
    }

    // Ensure m_languages is set to the single language we're generating
    // This is critical because GenerateLanguageForm and GenerateCppForm read from m_languages
    auto saved_lang = m_languages;
    m_languages = language;

    if (wxGetApp().isTestingMenuEnabled() && !m_clock_started)
    {
        StartClock();
    }

    wxue::SaveCwd cwd(wxue::restore_cwd);
    Project.ChangeDir();

    bool generate_result = false;

    // C++ uses GenerateCppFiles which handles CMake + loops through forms via GenerateCppForm
    // XRC combined mode uses GenerateCombinedXrcFile

    if (language == GEN_LANG_CPLUSPLUS)
    {
        generate_result = GenerateCppFiles(comparison_only);
    }
    else if (language == GEN_LANG_XRC)
    {
        // Handle XRC combined forms mode separately - it requires special handling
        if (Project.as_bool(prop_combine_all_forms))
        {
            generate_result = GenerateCombinedXrcFile(comparison_only);
        }
        else
        {
            // Non-combined mode: use GenerateLanguageForm for each form
            std::vector<Node*> forms;
            Project.CollectForms(forms);

            // Note that we do *not* use a progress dialog for XRC generation here even if
            // requested to. XRC generation is so fast, that we should be able to handle
            // extremely large projects without it.

            for (auto* form: forms)
            {
                // CR: 448: Inconsistent result tracking - checking GetUpdatedFiles().size() instead
                // of the return value from GenerateLanguageForm. This differs from the pattern in
                // the else branch below (lines 495-501) where the return value is properly checked.
                // Should use: if (GenerateLanguageForm(form->as_view(prop_class_name), form,
                // comparison_only)) {

                // TODO: [Randalphwa - 12-31-2025] Above is probably correct, but need to validate
                // the the XRC code generator returns an expected value.
                std::ignore =
                    GenerateLanguageForm(form->as_view(prop_class_name), form, comparison_only);

                if (GetUpdatedFiles().size())
                {
                    generate_result = true;
                }
            }
        }
    }
    else
    {
        std::vector<Node*> forms;
        ASSERT_MSG(!m_target_nodes.empty(),
                   "GenerateLanguageFiles called with no target nodes for non-C++/XRC language");
        forms = m_target_nodes;

        RemoveFormsWithoutOutputPath(forms);
        int max_progress = 0;
        if (auto data_list = Project.get_DataForm(); data_list)
        {
            max_progress += static_cast<int>(data_list->get_ChildCount());
        }
        if (auto img_list = Project.get_ImagesForm(); img_list)
        {
            max_progress +=
                static_cast<int>(img_list->get_ChildCount() / result::progress_image_step);
        }
        if (max_progress == 0 && forms.size() < (progress_forms_step * 2))
        {
            m_show_progress = false;  // No need for progress dialog
        }
        else
        {
            max_progress += static_cast<int>(forms.size() / progress_forms_step);
        }

        std::optional<wxProgressDialog> progress;
        if (m_show_progress && !forms.empty())
        {
            progress.emplace(m_progress_title, "Processing forms...", max_progress,
                             wxGetMainFrame(), wxPD_APP_MODAL | wxPD_AUTO_HIDE);
        }

        int progress_count = 0;
        for (auto* form: forms)
        {
            // CR: 495: Inconsistent result tracking - same issue as line 448. Should directly check
            // the return value from GenerateLanguageForm instead of checking
            // GetUpdatedFiles().size(). The current approach relies on side effects and doesn't
            // match the C++ generation pattern in GenerateCppFiles (line 896).
            std::ignore =
                GenerateLanguageForm(form->as_view(prop_class_name), form, comparison_only);

            if (GetUpdatedFiles().size())
            {
                generate_result = true;
            }

            if (progress && ++progress_count >= progress_forms_step && !form->is_Gen(gen_Images) &&
                !form->is_Gen(gen_Data))
            {
                wxString msg("Processing: ");
                msg << form->as_string(prop_class_name);
                progress->Update(progress->GetValue() + 1, msg);
                progress_count = 0;
            }
        }
    }

    if (wxGetApp().isTestingMenuEnabled())
    {
        EndClock();
    }

    // Restore original m_languages value
    m_languages = saved_lang;

    return generate_result;
}

auto GenResults::GenerateLanguageForm(std::string_view /* class_name */, Node* form,
                                      bool comparison_only) -> bool
{
    // This handles Python, Ruby, Perl, and XRC - they only have a single source file per form
    // C++ is handled via GenerateCppForm which manages both .h and .cpp files

    if (!form || !form->is_Form())
    {
        return false;
    }

    // C++ requires special handling for both header and source files
    if (m_languages == GEN_LANG_CPLUSPLUS)
    {
        return GenerateCppForm(form, comparison_only);
    }

    // XRC doesn't support certain form types
    if (m_languages == GEN_LANG_XRC)
    {
        if (form->is_Gen(gen_Images) || form->is_Gen(gen_Data) ||
            form->is_Gen(gen_wxPopupTransientWindow))
        {
            return false;
        }
    }

    auto [path, has_base_file] = Project.GetOutputPath(form, m_languages);
    if (!has_base_file)
    {
        return false;  // No output path configured for this form/language
    }

    // Create the appropriate language-specific code generator
    std::unique_ptr<BaseCodeGenerator> code_generator;
    switch (m_languages)
    {
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
            FAIL_MSG(wxString() << "GenerateLanguageForm called with unsupported language: "
                                << m_languages);
            return false;
    }

    // Determine file extension based on target language
    std::string_view file_ext;
    switch (m_languages)
    {
        case GEN_LANG_PYTHON:
            file_ext = ".py";
            break;
        case GEN_LANG_RUBY:
            file_ext = ".rb";
            break;
        case GEN_LANG_PERL:
            file_ext = ".pm";
            break;
        case GEN_LANG_XRC:
            file_ext = ".xrc";
            break;
        default:
            FAIL_MSG(wxString() << "Unexpected m_languages value in extension switch: "
                                << m_languages);
            return false;
    }

    wxue::string src_path(path);
    // Check extension on the filename only, not the full path
    // (path may contain dots in directory names like "C:/Users/user.name/...")
    wxue::string_view filename_only(src_path.filename());
    if (filename_only.extension().empty())
    {
        src_path += file_ext;
    }

    auto src_cw = std::make_unique<FileCodeWriter>(src_path);
    code_generator->SetSrcWriteCode(src_cw.get());

    // Generate code into the FileCodeWriter buffer
    // m_languages should be a single language at this point (set in Generate() loop)
    ASSERT_MSG(m_languages == GEN_LANG_PYTHON || m_languages == GEN_LANG_RUBY ||
                   m_languages == GEN_LANG_PERL || m_languages == GEN_LANG_XRC,
               "GenerateLanguageForm expects m_languages to be a single language");
    code_generator->GenerateClass(m_languages);

    int write_flags = comparison_only ? (code::flag_test_only | code::flag_no_ui) : code::flag_none;

    // Check if file exists before writing (to distinguish created vs updated)
    bool file_existed = src_path.file_exists();

    int result = src_cw->WriteFile(m_languages, write_flags, form);

    if (comparison_only)
    {
        // In compare mode, check if file differs or needs creation
        if (result == code::write_needed || result == code::write_cant_read)
        {
            if (file_existed)
            {
                m_updated_files.emplace_back(src_path);
            }
            else
            {
                m_created_files.emplace_back(src_path);
            }

            // Capture detailed diff information if file exists on disk
            if (result == code::write_needed && src_path.file_exists())
            {
                auto content = std::make_shared<std::string>(src_cw->GetString());
                ProcessFileDiff(src_path, content, form);
            }
            return true;
        }
        return false;  // File is current
    }

    // In write mode
    if (result > 0)
    {
        IncrementFileCount();
        if (file_existed)
        {
            m_updated_files.emplace_back(src_path);
        }
        else
        {
            m_created_files.emplace_back(src_path);
        }
        return true;
    }
    if (result < 0)
    {
        m_msgs.emplace_back(
            std::format("Error writing file: {}", static_cast<std::string>(src_path)));
    }
    return false;
}

auto GenResults::GenerateCppForm(Node* form, bool comparison_only, wxProgressDialog* progress)
    -> bool
{
    if (!form || !form->is_Form())
    {
        return false;
    }

    auto [path, has_base_file] = Project.GetOutputPath(form, GEN_LANG_CPLUSPLUS);
    if (!has_base_file)
    {
        return false;  // No output path configured for this form
    }

    // Get file extensions from project settings
    std::string source_ext(".cpp");
    std::string header_ext(".h");

    if (const auto& ext_prop = Project.as_view(prop_source_ext); ext_prop.size())
    {
        source_ext = ext_prop;
    }
    if (const auto& ext_prop = Project.as_view(prop_header_ext); ext_prop.size())
    {
        header_ext = ext_prop;
    }

    // Create the C++ code generator
    CppCodeGenerator codegen(form);

    // Set up header file path and writer
    wxue::string hdr_path(path);
    hdr_path.replace_extension(header_ext);
    auto hdr_cw = std::make_unique<FileCodeWriter>(hdr_path);
    codegen.SetHdrWriteCode(hdr_cw.get());

    // Set up source file path and writer
    wxue::string src_path(path);
    src_path.replace_extension(source_ext);
    auto src_cw = std::make_unique<FileCodeWriter>(src_path);
    codegen.SetSrcWriteCode(src_cw.get());

    // Generate code into both buffers
    // m_languages should be GEN_LANG_CPLUSPLUS at this point (set in Generate() loop)
    ASSERT_MSG(m_languages == GEN_LANG_CPLUSPLUS,
               "GenerateCppForm expects m_languages to be GEN_LANG_CPLUSPLUS");
    codegen.GenerateClass(m_languages, PANEL_PAGE::NOT_PANEL, progress);

    bool any_updated = false;
    int write_flags = comparison_only ? (code::flag_test_only | code::flag_no_ui) : code::flag_none;

    // Check if files exist before writing (to distinguish created vs updated)
    bool hdr_existed = hdr_path.file_exists();
    bool src_existed = src_path.file_exists();

    // Add closing brace flag if needed
    if (form->as_bool(prop_no_closing_brace))
    {
        write_flags |= code::flag_add_closing_brace;
    }

    // Process header file first (order matters for C++)
    int hdr_result = hdr_cw->WriteFile(m_languages, write_flags, form);

    if (comparison_only)
    {
        if (hdr_result == code::write_needed || hdr_result == code::write_cant_read)
        {
            if (hdr_existed)
            {
                m_updated_files.emplace_back(hdr_path);
            }
            else
            {
                m_created_files.emplace_back(hdr_path);
            }
            any_updated = true;

            // Capture detailed diff information if file exists on disk
            if (hdr_result == code::write_needed && hdr_path.file_exists())
            {
                auto content = std::make_shared<std::string>(hdr_cw->GetString());
                ProcessFileDiff(hdr_path, content, form);
            }
        }
    }
    else
    {
        // In write mode
        if (hdr_result > 0)
        {
            IncrementFileCount();
            any_updated = true;
            if (hdr_existed)
            {
                m_updated_files.emplace_back(hdr_path);
            }
            else
            {
                m_created_files.emplace_back(hdr_path);
            }
        }
        else if (hdr_result < 0)
        {
            m_msgs.emplace_back(
                std::format("Error writing file: {}", static_cast<std::string>(hdr_path)));
        }
    }

    // Remove closing brace flag for source file
    if (form->as_bool(prop_no_closing_brace))
    {
        write_flags &= ~code::flag_add_closing_brace;
    }

    // Process source file
    int src_result = src_cw->WriteFile(m_languages, write_flags, form);

    if (comparison_only)
    {
        if (src_result == code::write_needed || src_result == code::write_cant_read)
        {
            if (src_existed)
            {
                m_updated_files.emplace_back(src_path);
            }
            else
            {
                m_created_files.emplace_back(src_path);
            }
            any_updated = true;

            // Capture detailed diff information if file exists on disk
            if (src_result == code::write_needed && src_path.file_exists())
            {
                auto content = std::make_shared<std::string>(src_cw->GetString());
                ProcessFileDiff(src_path, content, form);
            }
        }
    }
    else
    {
        // In write mode
        if (src_result > 0)
        {
            IncrementFileCount();
            any_updated = true;
            if (src_existed)
            {
                m_updated_files.emplace_back(src_path);
            }
            else
            {
                m_created_files.emplace_back(src_path);
            }
        }
        else if (src_result < 0)
        {
            m_msgs.emplace_back(
                std::format("Error writing file: {}", static_cast<std::string>(src_path)));
        }
    }

    return any_updated;
}

auto GenResults::GenerateCppFiles(bool comparison_only) -> bool
{
    if (Project.get_ChildCount() == 0)
    {
        return false;
    }

    std::vector<Node*> forms;
    forms = m_target_nodes;
    Project.FindWxueFunctions(forms);

    RemoveFormsWithoutOutputPath(forms);
    int max_progress = 0;
    if (auto data_list = Project.get_DataForm(); data_list)
    {
        max_progress += static_cast<int>(data_list->get_ChildCount());
    }
    if (auto img_list = Project.get_ImagesForm(); img_list)
    {
        max_progress += static_cast<int>(img_list->get_ChildCount() / result::progress_image_step);
    }
    if (max_progress == 0 && forms.size() < (progress_forms_step * 2))
    {
        m_show_progress = false;  // No need for progress dialog
    }
    else
    {
        max_progress += static_cast<int>(forms.size() / progress_forms_step);
    }

    std::optional<wxProgressDialog> progress;
    if (m_show_progress && !forms.empty())
    {
        progress.emplace(m_progress_title, "Processing forms...", max_progress, wxGetMainFrame(),
                         wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    }

    if (Project.as_bool(prop_generate_cmake))
    {
        auto is_testing = comparison_only;
        for (auto& iter: Project.get_ChildNodePtrs())
        {
            if (iter->is_Gen(gen_folder) && iter->HasValue(prop_folder_cmake_file))
            {
                auto result = WriteCMakeFile(iter.get(), *this, is_testing);
                if (result == result::created || result == result::needs_writing)
                {
                    IncrementFileCount();
                    if (is_testing)
                    {
                        m_updated_files.emplace_back(iter.get()->as_view(prop_cmake_file));
                    }
                }
            }
        }
        if (Project.HasValue(prop_cmake_file))
        {
            auto result = WriteCMakeFile(Project.get_ProjectNode(), *this, is_testing);
            if (result == result::created || result == result::needs_writing)
            {
                IncrementFileCount();
                if (is_testing)
                {
                    m_updated_files.emplace_back(
                        Project.get_ProjectNode()->as_view(prop_cmake_file));
                }
            }
        }
    }

    bool generate_result = false;
    int progress_count = 0;
    for (const auto& form: forms)
    {
        if (GenerateCppForm(form, comparison_only, progress ? progress.operator->() : nullptr))
        {
            generate_result = true;
        }

        if (progress && ++progress_count >= progress_forms_step && !form->is_Gen(gen_Images) &&
            !form->is_Gen(gen_Data))
        {
            wxString msg("Processing: ");
            msg << form->as_string(prop_class_name);
            progress->Update(progress->GetValue() + 1, msg);
            progress_count = 0;
        }
    }
    if (auto* frame = wxGetMainFrame(); frame)
    {
        frame->setStatusText("Code generation completed");
    }
    return generate_result;
}

void GenResults::RemoveFormsWithoutOutputPath(std::vector<Node*>& forms)
{
    // Remove forms that don't have an output file configured for the current language
    // Use erase-remove idiom for efficient removal
    forms.erase(std::remove_if(forms.begin(), forms.end(),
                               [this](Node* form)
                               {
                                   auto [path, has_base_file] =
                                       Project.GetOutputPath(form, m_languages);
                                   return !has_base_file;
                               }),
                forms.end());
}

auto GenResults::GenerateCombinedFile(GenLang language) -> bool
{
    // Validate: must be exactly one language (not multiple bits set)
    auto lang_bits = static_cast<std::uint16_t>(language);
    if (lang_bits == 0 || (lang_bits & (lang_bits - 1)) != 0)
    {
        FAIL_MSG("GenerateCombinedFile called with no language or multiple languages specified");
        return false;
    }

    // Currently only XRC is supported for combined file generation
    if (language != GEN_LANG_XRC)
    {
        // Future: Add support for Python, Ruby, Perl
        FAIL_MSG("GenerateCombinedFile currently only supports GEN_LANG_XRC");
        return false;
    }

    bool comparison_only = (m_mode == Mode::compare_only);

    if (wxGetApp().isTestingMenuEnabled())
    {
        StartClock();
    }

    wxue::SaveCwd cwd(wxue::restore_cwd);
    Project.ChangeDir();

    bool generate_result = GenerateCombinedXrcFile(comparison_only);

    if (wxGetApp().isTestingMenuEnabled())
    {
        EndClock();
    }

    return generate_result;
}

auto GenResults::GenerateCombinedXrcFile(bool comparison_only) -> bool
{
    // Determine output path: use override if set, otherwise use project settings
    wxue::string output_path;

    if (!m_combined_output_path.empty())
    {
        output_path = m_combined_output_path;
    }
    else
    {
        // Use project settings for combined XRC file
        auto xrc_dir = Project.as_string(prop_xrc_directory);
        auto combined_file = Project.as_string(prop_combined_xrc_file);

        if (combined_file.empty())
        {
            m_msgs.emplace_back("No combined XRC filename specified for the project.");
            return false;
        }

        if (xrc_dir.empty())
        {
            output_path = combined_file;
        }
        else
        {
            output_path = xrc_dir;
            combined_file.backslashestoforward();
            if (combined_file.contains("/"))
            {
                output_path.backslashestoforward();
                if (output_path.back() == '/')
                {
                    output_path.pop_back();
                }

                // If the first part of the combined_file is a folder and it matches the last
                // folder in output_path, then assume the folder name is duplicated in
                // combined_file. Remove the folder from output_path before adding the path.
                if (auto end_folder = combined_file.find('/'); end_folder != std::string::npos)
                {
                    if (output_path.ends_with(combined_file.substr(0, end_folder)))
                    {
                        output_path.erase(output_path.size() - end_folder, end_folder);
                    }
                }
            }
            output_path.append_filename(combined_file);
            output_path.make_absolute();
            output_path.backslashestoforward();
        }
    }

    if (output_path.extension().empty())
    {
        output_path.replace_extension(".xrc");
    }

    // Create the XRC generator and configure it
    XrcGenerator xrc_gen;
    xrc_gen.AddProjectFlags();
    xrc_gen.AddGeneratedComments();

    // Collect all forms and add them to the XRC document
    std::vector<Node*> forms;
    forms = m_target_nodes;

    for (auto* form: forms)
    {
        // Skip unsupported form types
        if (form->is_Gen(gen_Images) || form->is_Gen(gen_Data) ||
            form->is_Gen(gen_wxPopupTransientWindow))
        {
            continue;
        }

        xrc_gen.AddNode(form);
    }

    // Get the generated XML string
    auto xml_content = xrc_gen.getXmlString();

    bool file_existed = output_path.file_exists();

    if (comparison_only)
    {
        // In compare mode, check if file differs or needs creation
        if (!file_existed)
        {
            m_created_files.emplace_back(output_path);
            return true;
        }

        // Compare with existing file
        wxFile file_original(output_path.wx(), wxFile::read);
        if (!file_original.IsOpened())
        {
            m_msgs.emplace_back(std::format("Cannot read existing file: {}",
                                            static_cast<std::string>(output_path)));
            m_updated_files.emplace_back(output_path);
            return true;
        }

        auto in_size = file_original.Length();
        if (xml_content.size() != static_cast<size_t>(in_size))
        {
            // Size differs, file needs updating
            m_updated_files.emplace_back(output_path);
            auto content = std::make_shared<std::string>(xml_content);
            ProcessFileDiff(output_path, content, nullptr);
            return true;
        }

        // Same size, compare content byte-by-byte
        auto buffer = std::make_unique<unsigned char[]>(in_size);
        if (file_original.Read(buffer.get(), in_size) == in_size)
        {
            if (std::memcmp(buffer.get(), xml_content.data(), in_size) == 0)
            {
                // Files are identical
                return false;
            }
        }

        // Content differs
        m_updated_files.emplace_back(output_path);
        auto content = std::make_shared<std::string>(xml_content);
        ProcessFileDiff(output_path, content, nullptr);
        return true;
    }

    // Write mode: save the file
    if (!xrc_gen.getDocument().save_file(output_path))
    {
        m_msgs.emplace_back(std::format("Cannot create or write to the file: {}",
                                        static_cast<std::string>(output_path)));
        return false;
    }

    IncrementFileCount();
    if (file_existed)
    {
        m_updated_files.emplace_back(output_path);
    }
    else
    {
        m_created_files.emplace_back(output_path);
    }

    return true;
}

void GenResults::ProcessFileDiff(wxue::string path, std::shared_ptr<std::string> content,
                                 Node* form)
{
    // This function assumes it is being called from the main thread *only* and as such it does not
    // protect m_pending_diffs from race conditions.
    while (m_pending_diffs.size() >= std::thread::hardware_concurrency())
    {
        // Process and remove completed futures
        for (auto it = m_pending_diffs.begin(); it != m_pending_diffs.end();)
        {
            if (it->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                auto diff = it->get();
                if (diff.has_value())
                {
                    m_file_diffs.push_back(std::move(*diff));
                }
                it = m_pending_diffs.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    // Launch async task to perform diff computation
    m_pending_diffs.emplace_back(
        std::async(std::launch::async,
                   [path = std::move(path), content, form]() -> std::optional<FileDiff>
                   {
                       // Check if generated file is too large to process diff efficiently
                       // Files larger than max_diff_file_size are flagged as too large to avoid
                       // performance issues
                       if (content->size() > max_diff_file_size)
                       {
                           FileDiff file_diff;
                           file_diff.filename = path.filename();
                           file_diff.form = form;
                           file_diff.is_too_large_to_display = true;
                           return file_diff;
                       }

                       wxue::ViewVector disk_content;
                       if (!disk_content.ReadFile(std::string_view(path)))
                       {
                           // File doesn't exist or can't be read - skip diff
                           return std::nullopt;
                       }

                       wxue::ViewVector gen_content;
                       gen_content.ReadString(std::string_view(*content));

                       auto diff_result = Diff::Compare(disk_content, gen_content);
                       if (diff_result.has_differences)
                       {
                           FileDiff file_diff;
                           file_diff.filename = path.filename();
                           file_diff.original_content = disk_content.GetBuffer();
                           file_diff.new_content = *content;
                           file_diff.diff_result = std::move(diff_result);
                           file_diff.form = form;
                           return file_diff;
                       }

                       return std::nullopt;
                   }));
}

void GenResults::WaitForPendingDiffs()
{
    for (auto& future: m_pending_diffs)
    {
        if (future.valid())
        {
            try
            {
                auto diff = future.get();
                if (diff.has_value())
                {
                    m_file_diffs.push_back(std::move(diff.value()));
                }
            }
            catch (const std::exception& e)
            {
                // Log error but continue processing other diffs
                m_msgs.emplace_back(std::format("Diff computation error: {}", e.what()));
            }
        }
    }
    m_pending_diffs.clear();
}
