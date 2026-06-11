///////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for choosing and generating specific language file(s)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>    // wxConfig base header
#include <wx/filename.h>  // wxFileName - encapsulates a file path

#include "generate_dlg_gen.h"

#include <format>

#include "gen_common.h"       // Common component functions
#include "gen_results.h"      // Code generation file writing functions
#include "image_handler.h"    // ImageHandler class
#include "mainapp.h"          // App -- Main application class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class

#include "../wxui/dlg_gen_results.h"

static bool gen_base_code = false;
static bool gen_derived_code = false;
static bool gen_python_code = false;
static bool gen_ruby_code = false;
static bool gen_xrc_code = false;

constexpr int generation_timer_interval_ms = 250;

// This generates the base class files. For the derived class files, see GenInheritedClass()
// in generate/writers/gen_codefiles.cpp
void MainFrame::OnGenerateCode(wxCommandEvent& /* event unused */)
{
    std::ignore = ProjectImages.UpdateEmbedNodes();
    GenResults results;
    bool code_generated = false;
    wxBeginBusyCursor();
    UpdateWakaTime();

    m_generation_timer.Start(generation_timer_interval_ms);

    code_generated = GenerateFromOutputType(results);

    if (!code_generated)
    {
        code_generated = GenerateFromDialog(results);
    }

    m_generation_timer.Stop();
    wxEndBusyCursor();

    if (code_generated)
    {
        ShowGenerationResults(results);
    }

    // After generation, re-validate the cache to pick up any derived files that
    // may now exist (e.g., created by GenInheritedClass or externally).
    if (Project.HasMissingDerivedFiles())
    {
        Project.InvalidateDerivedFileCache();
    }

    UpdateWakaTime();
}

bool MainFrame::GenerateFromOutputType(GenResults& results)
{
    const size_t output_type = Project.get_OutputType();

    bool code_generated = false;

    // Handle derived class generation separately (not part of GenResults)
    if (output_type & OUTPUT_DERIVED)
    {
        GenInheritedClass(results);
        code_generated = true;
    }

    // Build combined language flags from detected output types
    std::uint16_t lang_flags = GEN_LANG_NONE;
    if (output_type & OUTPUT_CPLUS)
    {
        lang_flags |= GEN_LANG_CPLUSPLUS;
    }
    if (output_type & OUTPUT_PYTHON)
    {
        lang_flags |= GEN_LANG_PYTHON;
    }
    if (output_type & OUTPUT_RUBY)
    {
        lang_flags |= GEN_LANG_RUBY;
    }
    if (output_type & OUTPUT_XRC)
    {
        lang_flags |= GEN_LANG_XRC;
    }

    if (lang_flags != GEN_LANG_NONE)
    {
        results.SetNodes(Project.get_ProjectNode());
        results.SetLanguages(static_cast<GenLang>(lang_flags));
        results.SetMode(GenResults::Mode::generate_and_write);
        std::ignore = results.Generate();
        code_generated = true;
    }

    return code_generated;
}

bool MainFrame::GenerateFromDialog(GenResults& results)
{
    GenerateDlg dialog(this);
    if (dialog.ShowModal() != wxID_OK)
    {
        return false;
    }

    bool code_generated = false;

    // Collect all selected languages into a combined flag
    std::uint16_t lang_flags = GEN_LANG_NONE;

    // Always generate XRC files first in case the XRC files need to be added to a gen_Data
    // section of the other languages.
    gen_xrc_code = dialog.is_gen_xrc();
    if (gen_xrc_code)
    {
        lang_flags |= GEN_LANG_XRC;
    }

    gen_base_code = dialog.is_gen_base();
    if (gen_base_code)
    {
        lang_flags |= GEN_LANG_CPLUSPLUS;
    }

    gen_python_code = dialog.is_gen_python();
    if (gen_python_code)
    {
        lang_flags |= GEN_LANG_PYTHON;
    }

    gen_ruby_code = dialog.is_gen_ruby();
    if (gen_ruby_code)
    {
        lang_flags |= GEN_LANG_RUBY;
    }

    // Generate all selected languages in one call
    if (lang_flags != GEN_LANG_NONE)
    {
        results.SetNodes(Project.get_ProjectNode());
        results.SetLanguages(static_cast<GenLang>(lang_flags));
        results.SetMode(GenResults::Mode::generate_and_write);
        std::ignore = results.Generate();
        code_generated = true;
    }

    // Handle derived class generation separately (not part of GenResults yet)
    gen_derived_code = dialog.is_gen_inherited();
    if (gen_derived_code)
    {
        GenInheritedClass(results);
        code_generated = true;
    }

    if (wxGetApp().isTestingMenuEnabled())
    {
        SaveGenerationPreferences();
    }

    return code_generated;
}

void MainFrame::SaveGenerationPreferences()
{
    wxConfigBase* config = wxConfig::Get();
    config->SetPath("/preferences");
    config->Write("gen_xrc_code", gen_xrc_code);
    config->Write("gen_base_code", gen_base_code);

    config->Write("gen_python_code", gen_python_code);
    config->Write("gen_ruby_code", gen_ruby_code);
    config->SetPath("/");
}

void MainFrame::ResetDerivedCodeState()
{
    gen_derived_code = Project.HasMissingDerivedFiles();
}

void MainFrame::UpdateGenerationStatus()
{
    Update();
    wxSafeYield();
}

void MainFrame::OnGenerationTimer(wxTimerEvent& /* event unused */)
{
    UpdateGenerationStatus();
}

void MainFrame::ShowGenerationResults(const GenResults& results)
{
    if (!results.GetUpdatedFiles().empty() || !results.GetCreatedFiles().empty() ||
        !results.GetMsgs().empty())
    {
        GeneratedResultsDlg results_dlg;
        if (!results_dlg.Create(this))
        {
            return;
        }

        // Show updated files first
        for (const auto& iter: results.GetUpdatedFiles())
        {
            wxFileName relative_path(iter);
            relative_path.MakeRelativeTo(Project.get_ProjectPath());
            results_dlg.m_lb_files->Append(wxString("Updated: ") + relative_path.GetFullPath());
        }

        // Then show created files
        for (const auto& iter: results.GetCreatedFiles())
        {
            wxFileName relative_path(iter);
            relative_path.MakeRelativeTo(Project.get_ProjectPath());
            results_dlg.m_lb_files->Append(wxString("Created: ") + relative_path.GetFullPath());
        }

        // TODO: [Randalphwa - 11-29-2025] If we derive from GeneratedResultsDlg then we could make
        // a hidden section that contains "Updated files: and a dropdown combo box that contains the
        // names of all the files that have been updated.
        std::vector<std::string> msgs = results.GetMsgs();  // Make a mutable copy

        // Report counts for updated and created files
        if (results.GetUpdatedFiles().size() == 1)
        {
            msgs.emplace_back("1 file was updated");
        }
        if (results.GetUpdatedFiles().size() > 1)
        {
            msgs.emplace_back(
                std::format("{} files were updated", results.GetUpdatedFiles().size()));
        }

        if (results.GetCreatedFiles().size() == 1)
        {
            msgs.emplace_back("1 new file was created");
        }
        if (results.GetCreatedFiles().size() > 1)
        {
            msgs.emplace_back(
                std::format("{} new files were created", results.GetCreatedFiles().size()));
        }

        for (const auto& iter: msgs)
        {
            results_dlg.m_lb_info->Append(iter);
        }

        results_dlg.ShowModal();
    }
    else if (results.GetFileCount())
    {
        wxString msg;
        msg << '\n' << "All " << results.GetFileCount() << " generated files are current";
        wxMessageBox(msg, "Code Generation", wxOK, this);
    }
}

void GenerateDlg::OnInit(wxInitDialogEvent& event)
{
    const size_t languages = Project.get_GenerateLanguages();

    switch (Project.get_CodePreference())
    {
        case GEN_LANG_CPLUSPLUS:
            gen_base_code = true;
            break;

        case GEN_LANG_PYTHON:
            gen_python_code = true;
            break;
        case GEN_LANG_RUBY:
            gen_ruby_code = true;
            break;
        case GEN_LANG_XRC:
            gen_xrc_code = true;
            break;

        default:
            break;
    }

    const bool show_cpp_section = (languages & GEN_LANG_CPLUSPLUS) || gen_base_code;
    const bool show_derived = gen_derived_code || Project.HasMissingDerivedFiles();

    if (show_cpp_section || show_derived)
    {
        if (show_cpp_section)
        {
            m_gen_base_code = gen_base_code;
            m_checkBaseCode = new wxCheckBox(this, wxID_ANY, "C++ &Base");
            m_checkBaseCode->SetValidator(wxGenericValidator(&m_gen_base_code));
            m_grid_sizer->Add(m_checkBaseCode, wxSizerFlags().Border(wxALL));
        }

        if (show_derived)
        {
            m_gen_inherited_code = gen_derived_code;
            m_checkDerived = new wxCheckBox(this, wxID_ANY, "C++ &Derived");
            m_checkDerived->SetValidator(wxGenericValidator(&m_gen_inherited_code));
            m_checkDerived->SetToolTip("Generate any derived files that don\'t currently exist");
            m_grid_sizer->Add(m_checkDerived, wxSizerFlags().Border(wxALL));
        }
    }

    if (languages & GEN_LANG_PYTHON || gen_python_code)
    {
        m_gen_python_code = gen_python_code;
        m_checkPython = new wxCheckBox(this, wxID_ANY, "Python");
        m_checkPython->SetValidator(wxGenericValidator(&m_gen_python_code));
        m_grid_sizer->Add(m_checkPython, wxSizerFlags().Border(wxALL));
    }
    if (languages & GEN_LANG_RUBY || gen_ruby_code)
    {
        m_gen_ruby_code = gen_ruby_code;
        m_checkRuby = new wxCheckBox(this, wxID_ANY, "Ruby");
        m_checkRuby->SetValidator(wxGenericValidator(&m_gen_ruby_code));
        m_grid_sizer->Add(m_checkRuby, wxSizerFlags().Border(wxALL));
    }
    if (languages & GEN_LANG_XRC || gen_xrc_code)
    {
        m_gen_xrc_code = gen_xrc_code;
        m_checkXRC = new wxCheckBox(this, wxID_ANY, "XRC");
        m_checkXRC->SetValidator(wxGenericValidator(&m_gen_xrc_code));
        m_grid_sizer->Add(m_checkXRC, wxSizerFlags().Border(wxALL));
    }

    // You have to reset minimum size to allow the window to shrink
    SetMinSize(wxSize(-1, -1));
    Fit();

    event.Skip();
}
