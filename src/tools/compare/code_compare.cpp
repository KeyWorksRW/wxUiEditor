/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code Generation Comparison
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <format>
#include <set>

#include "code_compare.h"

#include <wx/dir.h>  // wxDir is a class for enumerating the files in a directory

#include "diff_viewer.h"      // DiffViewer -- Dialog for displaying file differences
#include "gen_results.h"      // Code generation file writing functions
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class

void MainFrame::OnCodeCompare(wxCommandEvent& /* event */)
{
    CodeCompare dlg(this);
    dlg.ShowModal();
}

void CodeCompare::OnInit(wxInitDialogEvent& /* event */)
{
    auto* node = wxGetFrame().getSelectedNode();
    ASSERT_MSG(node, "No node selected for code comparison dialog");  // this should be impossible
    if (node->is_Form())
    {
        m_changed_classes_text->SetLabel(node->as_string(prop_class_name));
    }
    else if (node->is_Folder())
    {
        wxString text = node->as_string(prop_label).wx() << " (Folder)";
        m_changed_classes_text->SetLabel(text);
    }

    GenLang language = Project.get_CodePreference(node);
    wxCommandEvent dummy;
    switch (language)
    {
        case GEN_LANG_PERL:
            m_radio_perl->SetValue(true);
            break;

        case GEN_LANG_PYTHON:
            m_radio_python->SetValue(true);
            break;

        case GEN_LANG_RUBY:
            m_radio_ruby->SetValue(true);
            break;

        case GEN_LANG_CPLUSPLUS:
            m_radio_cplusplus->SetValue(true);
            break;

            // TODO: [Randalphwa - 12-17-2025] We need to support XRC, but we don't currently
            // have a verified way of comparing XRC files

        default:
            {
                auto msg = std::format("Unsupported code generation language: {}",
                                       GenLangToString(language));
                FAIL_MSG(msg);

                // The dialog has not been shown yet, so we displaying a user message box won't
                // make sense. Instead, default to C++ generation.
                m_radio_cplusplus->SetValue(true);
                language = GEN_LANG_CPLUSPLUS;
            }
            break;
    }
    OnRadioButton(language);
}

constexpr int MIN_CHILD_COUNT_FOR_PROGRESS = 25;

void CodeCompare::OnRadioButton(GenLang language)
{
    wxGetMainFrame()->UpdateWakaTime();

    m_current_language = language;
    m_file_diffs.clear();
    m_list_changes->Clear();
    m_btn->Enable(false);

    if (!gen_lang_set.contains(language))
    {
        auto msg = std::format("Unknown language: {}", GenLangToString(language));
        FAIL_MSG(msg);
        return;
    }

    auto* current_node = wxGetFrame().getSelectedNode();
    if (!current_node)
    {
        current_node = Project.get_ProjectNode();
    }

    // Use GenResults in compare_only mode to generate and capture diffs
    GenResults results;
    results.SetLanguages(language);
    results.SetMode(GenResults::Mode::compare_only);
    results.SetNodes(current_node);
    if ((current_node->is_Gen(gen_Project) || current_node->is_Folder()) &&
        current_node->get_ChildCount() > MIN_CHILD_COUNT_FOR_PROGRESS)
    {
        results.EnableProgressDialog("Comparing Generated Code...");
    }

    if (results.Generate())
    {
        m_file_diffs = std::move(results.GetFileDiffs());

        // Populate list with unique class names from the forms
        std::set<std::string> class_names;
        for (const auto& diff: m_file_diffs)
        {
            if (diff.form)
            {
                if (diff.form->is_Gen(gen_Images))
                {
                    class_names.insert("Images");
                }
                else if (diff.form->is_Gen(gen_Data))
                {
                    class_names.insert("Data");
                }
                else
                {
                    class_names.insert(diff.form->as_string(prop_class_name));
                }
            }
        }

        for (const auto& name: class_names)
        {
            m_list_changes->AppendString(wxString::FromUTF8(name));
        }
        m_btn->Enable(!m_file_diffs.empty());
        if (m_file_diffs.empty())
        {
            m_diff_results->SetLabel("No differences found.");
        }
        else
        {
            auto text = std::format(std::locale(""), "{} file difference{} found.",
                                    m_file_diffs.size(), m_file_diffs.size() == 1 ? "" : "s");
            m_diff_results->SetLabel(text);
        }
    }
    else
    {
        m_btn->Enable(false);
        m_diff_results->SetLabel("No differences found.");
    }
    wxGetMainFrame()->UpdateWakaTime();
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

void CodeCompare::OnDiff(wxCommandEvent& /* event unused */)
{
    if (!m_file_diffs.empty())
    {
        DiffViewer viewer(this, m_file_diffs);
        viewer.ShowModal();
        return;
    }

    wxMessageBox("No differences found between generated code and files on disk.",
                 "Code Comparison", wxOK | wxICON_INFORMATION);
}

// Static method for non-UI code comparison (used by verify_codegen)
auto CodeCompare::CollectFileDiffsForLanguage(GenLang language) -> std::vector<FileDiff>
{
    // Use GenResults in compare_only mode to generate code and capture diffs
    GenResults results;
    results.SetLanguages(language);
    results.SetMode(GenResults::Mode::compare_only);
    results.SetNodes(Project.get_ProjectNode());

    std::ignore = results.Generate();

    return std::move(results.GetFileDiffs());
}
