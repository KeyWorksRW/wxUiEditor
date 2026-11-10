/////////////////////////////////////////////////////////////////////////////
// Purpose:   Check and launch the app generated for a language target
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// Maintainer-only tool. The recipes below are hardcoded for the maintainer's machine -- edit the
// table locally when your toolchain differs. This mirrors the philosophy of cmake/internal.cmake.

#include <algorithm>
#include <array>
#include <map>

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/font.h>
#include <wx/process.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/utils.h>

#include "verify_sample_dlg.h"

#include "gen_enums.h"        // PropName enumeration
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "wxue_namespace/wxue_string.h"

struct SampleRecipe
{
    const wchar_t* label;       // shown in the wxChoice
    const wchar_t* extension;   // e.g. L".ts", used to locate the generated file
    const wchar_t* check_exe;   // empty wchar_t string when there is no checker
    const wchar_t* check_args;  // may contain {file}
    const wchar_t* run_exe;
    const wchar_t* run_args;  // may contain {file}
    GenLang lang;             // passed to ProjectHandler to resolve the output directory
};

// Every entry after the last field is hardcoded for a specific maintainer machine.
static const std::array<SampleRecipe, 8> s_recipes = { {
    { L"TypeScript", L".ts", L"deno", L"check {file}", L"kwxts", L"{file}", GenLang::typescript },
    { L"Python", L".py", L"python", L"-m py_compile {file}", L"python", L"{file}",
      GenLang::python },
    { L"Ruby", L".rb", L"ruby", L"-c {file}", L"ruby", L"{file}", GenLang::ruby },
    { L"LuaJIT", L".lua", L"luajit", L"-bl {file}", L"luajit", L"{file}", GenLang::luajit },
    { L"Fortran", L".f90", L"", L"", L"quick.exe", L"", GenLang::fortran },
    { L"Go", L".go", L"go", L"build ./...", L"go", L"run .", GenLang::go },
    { L"Rust", L".rs", L"cargo", L"check", L"cargo", L"run", GenLang::rust },
    { L"Julia", L".jl", L"", L"", L"julia", L"--project=. {file}", GenLang::julia },
} };

static PropName GetOutputFolderProp(GenLang lang)
{
    static const std::map<GenLang, PropName> lang_output_prop_map = {
        { GenLang::python, prop_python_output_folder },
        { GenLang::ruby, prop_ruby_output_folder },
        { GenLang::fortran, prop_fortran_output_folder },
        { GenLang::go, prop_go_output_folder },
        { GenLang::julia, prop_julia_output_folder },
        { GenLang::luajit, prop_lua_output_folder },
        { GenLang::rust, prop_rust_output_folder },
        { GenLang::typescript, prop_typescript_output_folder },
    };

    const std::map<GenLang, PropName>::const_iterator iter = lang_output_prop_map.find(lang);
    return (iter == lang_output_prop_map.end()) ? prop_python_output_folder : iter->second;
}

static PropName GetEntryFileProp(GenLang lang)
{
    static const std::map<GenLang, PropName> lang_file_prop_map = {
        { GenLang::python, prop_python_file },   { GenLang::ruby, prop_ruby_file },
        { GenLang::fortran, prop_fortran_file }, { GenLang::go, prop_go_file },
        { GenLang::julia, prop_julia_file },     { GenLang::luajit, prop_lua_file },
        { GenLang::rust, prop_rust_file },       { GenLang::typescript, prop_typescript_file },
    };

    const std::map<GenLang, PropName>::const_iterator iter = lang_file_prop_map.find(lang);
    return (iter == lang_file_prop_map.end()) ? prop_python_file : iter->second;
}

// wxExecute takes the command as one string, so any path containing spaces must be quoted.
// Find the form node that owns the language's file property. These properties live on form
// nodes (see ProjectHandler::GetBaseFilename), not on the project node.
static Node* FindFormNodeWithFileProp(Node* node, PropName prop, const wxString& extension)
{
    if (!node)
    {
        return nullptr;
    }

    if (node->HasValue(prop))
    {
        const wxString value = node->as_string(prop).wx();
        if (!value.empty() && value.EndsWith(extension))
        {
            return node;
        }
    }

    for (auto& child: node->get_ChildNodePtrs())
    {
        if (Node* found = FindFormNodeWithFileProp(child.get(), prop, extension))
        {
            return found;
        }
    }

    return nullptr;
}

static wxString QuoteIfNeeded(const wxString& text)
{
    if (text.Contains(" ") && !text.StartsWith("\""))
    {
        return "\"" + text + "\"";
    }

    return text;
}

static wxString BuildCommand(const wxString& exe, const wxString& args, const wxString& entry_file)
{
    wxString command = QuoteIfNeeded(exe);
    if (!args.empty())
    {
        wxString filled_args = args;
        filled_args.Replace("{file}", QuoteIfNeeded(entry_file));
        command += " ";
        command += filled_args;
    }

    return command;
}

VerifySampleDlg::VerifySampleDlg(wxWindow* parent) :
    wxDialog(parent, wxID_ANY, "Verify Sample App", wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    auto* main_sizer = new wxBoxSizer(wxVERTICAL);

    auto* target_sizer = new wxBoxSizer(wxHORIZONTAL);
    target_sizer->Add(new wxStaticText(this, wxID_ANY, "Target:"), wxSizerFlags().CenterVertical());
    m_choice_lang = new wxChoice(this, wxID_ANY);
    for (const SampleRecipe& recipe: s_recipes)
    {
        m_choice_lang->Append(wxString(recipe.label));
    }
    m_choice_lang->SetSelection(0);
    target_sizer->Add(m_choice_lang, wxSizerFlags(1).CenterVertical().Border(wxLEFT));
    main_sizer->Add(target_sizer, wxSizerFlags().Expand().Border());

    m_log = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                           wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);
    m_log->SetFont(wxFont(wxFontInfo().Family(wxFONTFAMILY_TELETYPE)));
    m_log->SetMinSize(wxSize(700, 400));
    main_sizer->Add(m_log, wxSizerFlags(1).Expand().Border());

    auto* button_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto* btn_check = new wxButton(this, wxID_APPLY, "Check");
    btn_check->SetToolTip("Run the language syntax/compile check and show its output");
    btn_check->Bind(wxEVT_BUTTON, &VerifySampleDlg::OnCheck, this);

    auto* btn_run = new wxButton(this, wxID_ANY, "Run");
    btn_run->SetToolTip("Launch the generated GUI app asynchronously");
    btn_run->Bind(wxEVT_BUTTON, &VerifySampleDlg::OnRun, this);

    auto* btn_kill = new wxButton(this, wxID_ANY, "Kill");
    btn_kill->SetToolTip("Kill the running app");
    btn_kill->Bind(wxEVT_BUTTON, &VerifySampleDlg::OnKill, this);

    auto* btn_close = new wxButton(this, wxID_CLOSE, "Close");
    btn_close->Bind(wxEVT_BUTTON, &VerifySampleDlg::OnClose, this);

    button_sizer->AddStretchSpacer();
    button_sizer->Add(btn_check, wxSizerFlags().Border(wxRIGHT));
    button_sizer->Add(btn_run, wxSizerFlags().Border(wxRIGHT));
    button_sizer->Add(btn_kill, wxSizerFlags().Border(wxRIGHT));
    button_sizer->Add(btn_close);
    main_sizer->Add(button_sizer, wxSizerFlags().Expand().Border());

    SetSizerAndFit(main_sizer);

    Bind(wxEVT_END_PROCESS, &VerifySampleDlg::OnEndProcess, this);
    Bind(wxEVT_CLOSE_WINDOW, &VerifySampleDlg::OnCloseWindow, this);
}

VerifySampleDlg::~VerifySampleDlg()
{
    if (m_process)
    {
        // Detached wxProcess deletes itself when the app terminates, so no dangling pointer can
        // be used after this dialog is gone.
        m_process->Detach();
        m_process = nullptr;
    }
}

void VerifySampleDlg::AppendLog(const wxString& text)
{
    m_log->AppendText(text);
    m_log->AppendText("\n");
}

wxString VerifySampleDlg::GetOutputDir(GenLang lang) const
{
    wxString output_dir;

    Node* project_node = Project.get_ProjectNode();
    if (project_node)
    {
        const PropName prop = GetOutputFolderProp(lang);
        if (project_node->HasValue(prop))
        {
            output_dir = project_node->as_string(prop).wx();
        }
    }

    if (output_dir.empty())
    {
        output_dir = Project.get_ProjectPath().wx();
    }

    wxFileName dir_path = wxFileName::DirName(output_dir);
    if (!dir_path.IsAbsolute())
    {
        dir_path.MakeAbsolute(Project.get_ProjectPath().wx());
    }

    const wxString result = dir_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
    if (!wxFileName::DirExists(result))
    {
        return wxEmptyString;
    }

    return result;
}

wxString VerifySampleDlg::GetEntryFile(GenLang lang, const wxString& output_path,
                                       const wxString& extension)
{
    // Prefer the form node that explicitly names the entry file for this language. Otherwise fall
    // through to the newest-generated-file scan below.
    const PropName prop = GetEntryFileProp(lang);
    if (Node* form = FindFormNodeWithFileProp(Project.get_ProjectNode(), prop, extension); form)
    {
        const wxue::string base_file = form->as_string(prop);
        wxue::string merged = output_path;
        merged.append_filename(base_file);
        if (wxFileName::FileExists(merged.wx()))
        {
            return merged.wx();
        }
    }

    wxArrayString files;
    wxDir::GetAllFiles(output_path, &files, "*" + extension);

    wxString newest_file;
    wxDateTime newest_time;
    for (const wxString& file: files)
    {
        const wxFileName file_name(file);
        const wxDateTime modified = file_name.GetModificationTime();
        if (!modified.IsValid())
        {
            continue;
        }
        if (!newest_time.IsValid() || modified.IsLaterThan(newest_time))
        {
            newest_time = modified;
            newest_file = file;
        }
    }

    if (newest_file.empty())
    {
        AppendLog(wxString::Format("error: no \"*%s\" file found in %s", extension, output_path));
    }

    return newest_file;
}

void VerifySampleDlg::OnCheck(wxCommandEvent& /* event */)
{
    const int selection = m_choice_lang->GetSelection();
    const SampleRecipe& recipe = s_recipes[static_cast<size_t>(std::max(0, selection))];

    if (recipe.check_exe[0] == 0)
    {
        AppendLog(wxString::Format("no checker configured for %s", wxString(recipe.label)));
        return;
    }

    const wxString output_dir = GetOutputDir(recipe.lang);
    if (output_dir.empty())
    {
        AppendLog("error: could not resolve an existing output directory for this language");
        return;
    }

    const wxString entry_file = GetEntryFile(recipe.lang, output_dir, recipe.extension);
    const wxString check_args(recipe.check_args);
    if (check_args.Contains("{file}") && entry_file.empty())
    {
        return;
    }

    const wxString command = BuildCommand(recipe.check_exe, check_args, entry_file);
    const wxString cwd = output_dir;

    m_log->Freeze();
    {
        wxBusyCursor busy_cursor;

        wxExecuteEnv env;
        env.cwd = cwd;

        // This is the only deadlock-free way to capture both streams synchronously. Note that
        // this blocks the UI while a slow checker runs (the first cargo check or go build can
        // take a while). A timeout would require the async wxProcess + wxTimer route instead.
        wxArrayString out;
        wxArrayString err;
        const long result_code =
            wxExecute(command, out, err, wxEXEC_SYNC | wxEXEC_HIDE_CONSOLE, &env);

        AppendLog(wxString::Format("check: %s", command));
        AppendLog(wxString::Format("cwd:   %s", cwd));
        AppendLog(wxString::Format("exit:  %ld", result_code));

        if (out.IsEmpty() && err.IsEmpty())
        {
            AppendLog("(no output)");
        }
        for (const wxString& line: out)
        {
            AppendLog(line);
        }
        for (const wxString& line: err)
        {
            AppendLog("err: " + line);
        }
    }
    m_log->Thaw();
}

void VerifySampleDlg::OnRun(wxCommandEvent& /* event */)
{
    if (m_app_pid != 0)
    {
        AppendLog(wxString::Format("app pid %ld is already running", m_app_pid));
        return;
    }

    const int selection = m_choice_lang->GetSelection();
    const SampleRecipe& recipe = s_recipes[static_cast<size_t>(std::max(0, selection))];

    const wxString output_dir = GetOutputDir(recipe.lang);
    if (output_dir.empty())
    {
        AppendLog("error: could not resolve an existing output directory for this language");
        return;
    }

    const wxString run_args(recipe.run_args);
    wxString entry_file;
    if (run_args.Contains("{file}"))
    {
        entry_file = GetEntryFile(recipe.lang, output_dir, recipe.extension);
        if (entry_file.empty())
        {
            return;
        }
    }

    const wxString command = BuildCommand(recipe.run_exe, run_args, entry_file);

    wxExecuteEnv env;
    env.cwd = output_dir;  // samples are always run from the project's output dir for this language

    m_process = new wxProcess(this);
    m_app_pid = wxExecute(command, wxEXEC_ASYNC | wxEXEC_HIDE_CONSOLE, m_process, &env);
    if (m_app_pid == 0)
    {
        delete m_process;
        m_process = nullptr;
        AppendLog(wxString::Format("error: failed to launch: %s", command));
        return;
    }

    AppendLog(wxString::Format("launched pid %ld: %s", m_app_pid, command));
    AppendLog(wxString::Format("cwd: %s", env.cwd));
}

void VerifySampleDlg::OnKill(wxCommandEvent& /* event */)
{
    if (m_app_pid == 0)
    {
        AppendLog("no app running");
        return;
    }

    const long pid = m_app_pid;
    const wxKillError kill_result = wxProcess::Kill(static_cast<int>(pid), wxSIGKILL);
    if (kill_result == wxKILL_OK)
    {
        AppendLog(wxString::Format("killed app pid %ld", pid));
    }
    else
    {
        AppendLog(wxString::Format("wxKill failed for app pid %ld (error %d)", pid,
                                   static_cast<int>(kill_result)));
    }

    if (m_process)
    {
        // Kill() never delivers wxEVT_END_PROCESS, so this object's notification was not processed
        // by us. Per wxProcess ownership rules we must Detach() and let it delete itself --
        // deleting it here would be a double free.
        m_process->Detach();
        m_process = nullptr;
    }
    m_app_pid = 0;
}

void VerifySampleDlg::OnClose(wxCommandEvent& /* event */)
{
    if (m_app_pid != 0)
    {
        // Deliberately leave the app running -- use Kill if you want it gone.
        AppendLog(
            wxString::Format("closing dialog; app pid %ld still running (use Kill)", m_app_pid));
    }

    EndModal(wxID_CLOSE);
}

void VerifySampleDlg::OnCloseWindow(wxCloseEvent& event)
{
    if (m_app_pid != 0)
    {
        AppendLog(
            wxString::Format("closing dialog; app pid %ld still running (use Kill)", m_app_pid));
    }

    event.Skip();
}

void VerifySampleDlg::OnEndProcess(wxProcessEvent& event)
{
    const int pid = event.GetPid();
    const int exit_code = event.GetExitCode();

    AppendLog(wxString::Format("app pid %d exited with %d", pid, exit_code));

    // Processing wxEVT_END_PROCESS makes the handler responsible for deleting the wxProcess.
    if (m_process)
    {
        delete m_process;
        m_process = nullptr;
    }
    m_app_pid = 0;
}
