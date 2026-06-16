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
#include <wx/msgdlg.h>

#include "diff_viewer.h"      // DiffViewer -- Dialog for displaying file differences
#include "gen_results.h"      // Code generation file writing functions
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class

void MainFrame::OnCodeCompare([[maybe_unused]] wxCommandEvent& event)
{
    CodeCompare dialog(this);
    dialog.ShowModal();
}

void CodeCompare::OnInit([[maybe_unused]] wxInitDialogEvent& event)
{
    Node* node = wxGetFrame().getSelectedNode();
    ASSERT_OR_WARN(node != nullptr, "No form node selected for code comparison dialog");
    if (!node)
    {
        return;
    }
    if (node->is_Form())
    {
        m_changed_classes_text->SetLabel(node->as_string(prop_class_name));
    }
    else if (node->is_Folder())
    {
        const wxString text = node->as_string(prop_label).wx() << " (Folder)";
        m_changed_classes_text->SetLabel(text);
    }

    GenLang language = Project.get_CodePreference(node);
    const wxCommandEvent dummy;
    switch (language)
    {
        case GenLang::python:
            m_radio_python->SetValue(true);
            break;

        case GenLang::ruby:
            m_radio_ruby->SetValue(true);
            break;

        case GenLang::cplusplus:
            m_radio_cplusplus->SetValue(true);
            break;

        case GenLang::fortran:
            m_radio_fortran->SetValue(true);
            break;

        case GenLang::go:
            m_radio_go->SetValue(true);
            break;

        case GenLang::julia:
            m_radio_julia->SetValue(true);
            break;

        case GenLang::luajit:
            m_radio_luajit->SetValue(true);
            break;

        case GenLang::typescript:
            m_radio_typescript->SetValue(true);
            break;

            // TODO: [Randalphwa - 12-17-2025] We need to support XRC, but we don't currently
            // have a verified way of comparing XRC files

        default:
            {
                const std::string message = std::format("Unsupported code generation language: {}",
                                                        GenLangToString(language));
                FAIL_MSG(message);

                // The dialog has not been shown yet, so displaying a user
                // message box won't make sense. Instead, default to C++
                // generation.
                m_radio_cplusplus->SetValue(true);
                language = GenLang::cplusplus;
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
        const std::string message = std::format("Unknown language: {}", GenLangToString(language));
        FAIL_MSG(message);
        return;
    }

    Node* current_node = wxGetFrame().getSelectedNode();
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
            const std::string text =
                std::format(std::locale(""), "{} file difference{} found.", m_file_diffs.size(),
                            m_file_diffs.size() == 1 ? "" : "s");
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

void CodeCompare::OnCPlusPlus([[maybe_unused]] wxCommandEvent& event)
{
    OnRadioButton(GenLang::cplusplus);
}

void CodeCompare::OnPython([[maybe_unused]] wxCommandEvent& event)
{
    OnRadioButton(GenLang::python);
}

void CodeCompare::OnRuby([[maybe_unused]] wxCommandEvent& event)
{
    OnRadioButton(GenLang::ruby);
}

void CodeCompare::OnFortran([[maybe_unused]] wxCommandEvent& event)
{
    OnRadioButton(GenLang::fortran);
}

void CodeCompare::OnGO([[maybe_unused]] wxCommandEvent& event)
{
    OnRadioButton(GenLang::go);
}

void CodeCompare::OnJulia([[maybe_unused]] wxCommandEvent& event)
{
    OnRadioButton(GenLang::julia);
}

void CodeCompare::OnLuaJIT([[maybe_unused]] wxCommandEvent& event)
{
    OnRadioButton(GenLang::luajit);
}

void CodeCompare::OnTypeScript([[maybe_unused]] wxCommandEvent& event)
{
    OnRadioButton(GenLang::typescript);
}

void CodeCompare::OnXRC([[maybe_unused]] wxCommandEvent& event)
{
    OnRadioButton(GenLang::xrc);
}

void CodeCompare::OnDiff([[maybe_unused]] wxCommandEvent& event)
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
std::vector<FileDiff> CodeCompare::CollectFileDiffsForLanguage(GenLang language, Node* form_node)
{
    // Use GenResults in compare_only mode to generate code and capture diffs
    GenResults results;
    results.SetLanguages(language);
    results.SetMode(GenResults::Mode::compare_only);

    // Use filtered form if specified, otherwise entire project
    if (form_node)
    {
        results.SetNodes(form_node);
    }
    else
    {
        results.SetNodes(Project.get_ProjectNode());
    }

    std::ignore = results.Generate();

    return std::move(results.GetFileDiffs());
}
