///////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for choosing and generating specific language file(s)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/button.h>    // wxButton -- used by the debug-only Select All handler
#include <wx/config.h>    // wxConfig base header
#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/panel.h>     // wxPanel -- hosts disabled checkboxes so their tooltip can be shown

#include <format>

#include "generate_dlg.h"

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
static bool gen_fortran_code = false;
static bool gen_go_code = false;
static bool gen_julia_code = false;
static bool gen_luajit_code = false;
static bool gen_python_code = false;
static bool gen_ruby_code = false;
static bool gen_rust_code = false;
static bool gen_typescript_code = false;
static bool gen_xrc_code = false;

constexpr int generation_timer_interval_ms = 250;

// Returns true if any form in the node tree contributes an output file for the given language.
// Not guarded by _DEBUG/INTERNAL_TESTING because OnInit() uses it in every configuration.
static bool HasOutputFile(Node* node, GenLang language)
{
    for (const auto& child: node->get_ChildNodePtrs())
    {
        if (child->is_Form())
        {
            auto [path, has_base_file] = Project.GetOutputPath(child.get(), language);
            if (has_base_file)
            {
                return true;
            }
        }
        if (HasOutputFile(child.get(), language))
        {
            return true;
        }
    }
    return false;
}

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
    const GenOutput output_type = Project.get_OutputType();

    // If the user *only* wants the derived classes (is that even possible to
    // specify?) then generate and return.
    if (output_type == GenOutput::derived)
    {
        GenInheritedClass(results);
        return true;
    }

    // Check to see if the output type is for a single language. If it is, then we don't need to
    // show a dialog to the user asking which languages to generate. It is *not* a bug that we use
    // '==' here for a bit flag instead of '&' because if more than one bit is set, then we need to
    // show the dialog to the user to ask which languages they want to generate. dialog to the user
    // to ask which languages they want to generate.
    GenLang language = GenLang::none;
    if (output_type == GenOutput::cplusplus)
    {
        language = GenLang::cplusplus;
    }
    if (output_type == GenOutput::python)
    {
        language = GenLang::python;
    }
    if (output_type == GenOutput::ruby)
    {
        language = GenLang::ruby;
    }
    if (output_type == GenOutput::xrc)
    {
        language = GenLang::xrc;
    }

    if (language != GenLang::none)
    {
        results.SetNodes(Project.get_ProjectNode());
        results.SetLanguages(language);
        results.SetMode(GenResults::Mode::generate_and_write);
        std::ignore = results.Generate();
        return true;
    }

    return false;
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
    GenLang lang_flags = GenLang::none;

    // Always generate XRC files first in case the XRC files need to be added to a gen_Data
    // section of the other languages.
    gen_xrc_code = dialog.is_gen_xrc();
    if (gen_xrc_code)
    {
        lang_flags |= GenLang::xrc;
    }

    gen_base_code = dialog.is_gen_base();
    if (gen_base_code)
    {
        lang_flags |= GenLang::cplusplus;
    }

    gen_python_code = dialog.is_gen_python();
    if (gen_python_code)
    {
        lang_flags |= GenLang::python;
    }

    gen_ruby_code = dialog.is_gen_ruby();
    if (gen_ruby_code)
    {
        lang_flags |= GenLang::ruby;
    }

    gen_fortran_code = dialog.is_gen_fortran();
    if (gen_fortran_code)
    {
        lang_flags |= GenLang::fortran;
    }

    gen_go_code = dialog.is_gen_go();
    if (gen_go_code)
    {
        lang_flags |= GenLang::go;
    }

    gen_julia_code = dialog.is_gen_julia();
    if (gen_julia_code)
    {
        lang_flags |= GenLang::julia;
    }

    gen_luajit_code = dialog.is_gen_luajit();
    if (gen_luajit_code)
    {
        lang_flags |= GenLang::luajit;
    }

    gen_rust_code = dialog.is_gen_rust();
    if (gen_rust_code)
    {
        lang_flags |= GenLang::rust;
    }

    gen_typescript_code = dialog.is_gen_typescript();
    if (gen_typescript_code)
    {
        lang_flags |= GenLang::typescript;
    }

    // Generate all selected languages in one call
    if (lang_flags != GenLang::none)
    {
        results.SetNodes(Project.get_ProjectNode());
        results.SetLanguages(lang_flags);
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

#if defined(INTERNAL_TESTING)
    SaveGenerationPreferences();
#endif

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
    config->Write("gen_fortran_code", gen_fortran_code);
    config->Write("gen_go_code", gen_go_code);
    config->Write("gen_julia_code", gen_julia_code);
    config->Write("gen_luajit_code", gen_luajit_code);
    config->Write("gen_rust_code", gen_rust_code);
    config->Write("gen_typescript_code", gen_typescript_code);
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
        // The mainframe must be the parent so that the dialog is centered over the mainframe's
        // display -- without a parent it would be centered on the primary display, even when the
        // mainframe is on a secondary monitor.
        GeneratedResultsDlg results_dlg;
        if (!results_dlg.Create(wxGetMainFrame()))
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

        // Create() restores the dialog's persisted geometry, which can leave it on the display it
        // was last shown on. Re-center it over the mainframe now that all items have been added.
        results_dlg.Centre(wxBOTH);
        results_dlg.ShowModal();
    }
    else if (results.GetFileCount())
    {
        wxString msg;
        msg << '\n' << "All generated files are current";
        wxMessageBox(msg, "Code Generation", wxOK, this);
    }
}

GenerateDlg::GenerateDlg(wxWindow* parent)
{
    std::ignore = Create(parent);
}

void GenerateDlg::OnInit(wxInitDialogEvent& event)
{
    const GenLang languages = Project.get_GenerateLanguages();

    switch (Project.get_CodePreference())
    {
        case GenLang::cplusplus:
            gen_base_code = true;
            break;

        case GenLang::python:
            gen_python_code = true;
            break;
        case GenLang::ruby:
            gen_ruby_code = true;
            break;
        case GenLang::fortran:
            gen_fortran_code = true;
            break;
        case GenLang::go:
            gen_go_code = true;
            break;
        case GenLang::julia:
            gen_julia_code = true;
            break;
        case GenLang::luajit:
            gen_luajit_code = true;
            break;
        case GenLang::rust:
            gen_rust_code = true;
            break;
        case GenLang::typescript:
            gen_typescript_code = true;
            break;
        case GenLang::xrc:
            gen_xrc_code = true;
            break;

        default:
            break;
    }

    const bool show_cpp_section = (languages & GenLang::cplusplus) || gen_base_code;
    const bool show_derived = gen_derived_code || Project.HasMissingDerivedFiles();

    // Adds a checkbox for the given language. When no form in the project produces an output file
    // for that language, the checkbox is disabled and its value forced off.
    auto add_language_checkbox = [this](GenLang language, bool gen_flag, bool& value,
                                        wxCheckBox*& checkbox, const wxString& label)
    {
        value = gen_flag;
        checkbox = new wxCheckBox(this, wxID_ANY, label);
        checkbox->SetValidator(wxGenericValidator(&value));

        if (!HasOutputFile(Project.get_ProjectNode(), language))
        {
            checkbox->Enable(false);
            checkbox->SetValue(false);
        }
        m_grid_sizer->Add(checkbox, wxSizerFlags().Border(wxALL));
    };

    if (show_cpp_section || show_derived)
    {
        if (show_cpp_section)
        {
            add_language_checkbox(GenLang::cplusplus, gen_base_code, m_gen_base_code,
                                  m_checkBaseCode, "C++ &Base");
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

    if (languages & GenLang::python || gen_python_code)
    {
        add_language_checkbox(GenLang::python, gen_python_code, m_gen_python_code, m_checkPython,
                              "Python");
    }
    if (languages & GenLang::ruby || gen_ruby_code)
    {
        add_language_checkbox(GenLang::ruby, gen_ruby_code, m_gen_ruby_code, m_checkRuby, "Ruby");
    }
    if (languages & GenLang::fortran || gen_fortran_code)
    {
        add_language_checkbox(GenLang::fortran, gen_fortran_code, m_gen_fortran_code,
                              m_checkFortran, "Fortran");
    }
    if (languages & GenLang::go || gen_go_code)
    {
        add_language_checkbox(GenLang::go, gen_go_code, m_gen_go_code, m_checkGo, "GO");
    }
    if (languages & GenLang::julia || gen_julia_code)
    {
        add_language_checkbox(GenLang::julia, gen_julia_code, m_gen_julia_code, m_checkJulia,
                              "Julia");
    }
    if (languages & GenLang::luajit || gen_luajit_code)
    {
        add_language_checkbox(GenLang::luajit, gen_luajit_code, m_gen_luajit_code, m_checkLuaJIT,
                              "LuaJIT");
    }
    if (languages & GenLang::rust || gen_rust_code)
    {
        add_language_checkbox(GenLang::rust, gen_rust_code, m_gen_rust_code, m_checkRust, "Rust");
    }
    if (languages & GenLang::typescript || gen_typescript_code)
    {
        add_language_checkbox(GenLang::typescript, gen_typescript_code, m_gen_typescript_code,
                              m_checkTypeScript, "TypeScript");
    }
    if (languages & GenLang::xrc || gen_xrc_code)
    {
        add_language_checkbox(GenLang::xrc, gen_xrc_code, m_gen_xrc_code, m_checkXRC, "XRC");
    }

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    auto* select_all = new wxButton(this, wxID_ANY, "Select &All");
    select_all->SetToolTip("Select every language that has at least one output file");
    if (wxSizer* dlg_sizer = GetSizer(); dlg_sizer != nullptr)
    {
        // Insert above the standard OK/Cancel row rather than below it.
        dlg_sizer->Insert(dlg_sizer->GetItemCount() - 1, select_all, wxSizerFlags().Border(wxALL));
    }
    select_all->Bind(wxEVT_BUTTON, &GenerateDlg::OnSelectAll, this);
#endif

    // You have to reset minimum size to allow the window to shrink
    SetMinSize(wxSize(-1, -1));
    Fit();

    event.Skip();
}

#if defined(_DEBUG) || defined(INTERNAL_TESTING)

void GenerateDlg::OnSelectAll(wxCommandEvent& /* event */)
{
    struct LanguageCheckbox
    {
        GenLang language;
        wxCheckBox* checkbox;
        bool* value;
    };
    const std::array<LanguageCheckbox, 10> languages = { {
        { .language = GenLang::cplusplus, .checkbox = m_checkBaseCode, .value = &m_gen_base_code },
        { .language = GenLang::python, .checkbox = m_checkPython, .value = &m_gen_python_code },
        { .language = GenLang::ruby, .checkbox = m_checkRuby, .value = &m_gen_ruby_code },
        { .language = GenLang::fortran, .checkbox = m_checkFortran, .value = &m_gen_fortran_code },
        { .language = GenLang::go, .checkbox = m_checkGo, .value = &m_gen_go_code },
        { .language = GenLang::julia, .checkbox = m_checkJulia, .value = &m_gen_julia_code },
        { .language = GenLang::luajit, .checkbox = m_checkLuaJIT, .value = &m_gen_luajit_code },
        { .language = GenLang::rust, .checkbox = m_checkRust, .value = &m_gen_rust_code },
        { .language = GenLang::typescript,
          .checkbox = m_checkTypeScript,
          .value = &m_gen_typescript_code },
        { .language = GenLang::xrc, .checkbox = m_checkXRC, .value = &m_gen_xrc_code },
    } };
    for (const auto& entry: languages)
    {
        if (entry.checkbox == nullptr)
        {
            continue;  // this language is not offered for the current project
        }
        if (!HasOutputFile(Project.get_ProjectNode(), entry.language))
        {
            continue;
        }
        entry.checkbox->SetValue(true);
        *entry.value = true;
    }
}

#endif  // defined(_DEBUG) || defined(INTERNAL_TESTING)
