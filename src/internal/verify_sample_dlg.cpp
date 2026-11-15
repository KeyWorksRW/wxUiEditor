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
#include <tuple>

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/filename.h>
#include <wx/process.h>
#include <wx/textctrl.h>
#include <wx/tokenzr.h>
#include <wx/utils.h>

#include "verify_sample_dlg.h"

#include "gen_enums.h"        // PropName enumeration
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "wxue_namespace/wxue_string.h"

struct SampleRecipe
{
    const char* label;         // shown in the wxChoice
    const char* extension;     // e.g. ".ts", used to match the form's language file property
    const char* entry_file;    // runnable entry script, relative to the repository root
    const char* checker_exe;   // empty string when there is no checker
    const char* checker_args;  // file arguments are relative to the repository root
    const char* runner_exe;    // PATH tool name (deno, cargo) or path relative to the repo root
    const char* runner_args;   // file arguments are relative to the repository root
    const char* working_dir;   // process cwd, relative to the repository root
    GenLang lang;              // selects the form property that anchors the repository root
};

// These are the commands each language repo's tests/drive_quick.ps1 "$Spec" table uses -- that
// file is the source of truth, so do not "simplify" them here:
//   - luajit is NOT on PATH on this machine; each repo ships bin/Release/kwxlua.exe.
//   - Go's go build/go run need the CGO_CFLAGS/CGO_LDFLAGS that build_quick.cmd sets, so the
//     already-built bin/Debug/quick.exe is launched from its package directory instead.
//   - Rust has no Cargo.toml at the repo root, hence --manifest-path.
// The hand-written quick_main.* entry files are what call main; the generated quick_mainframe.* /
// quick_dlg.* form modules only define functions, so launching one of those opens no window.
static const std::array<SampleRecipe, 8> s_recipes = { {
    { .label = "TypeScript",
      .extension = ".ts",
      .entry_file = "tests/quick/quick_main.ts",
      .checker_exe = "deno",
      .checker_args = "check tests/quick/quick_main.ts",
      .runner_exe = "bin/Release/kwxts.exe",
      .runner_args = "tests/quick/quick_main.ts",
      .working_dir = ".",
      .lang = GenLang::typescript },
    { .label = "Python",
      .extension = ".py",
      .entry_file = "",
      .checker_exe = "",
      .checker_args = "",
      .runner_exe = "",
      .runner_args = "",
      .working_dir = "",
      .lang = GenLang::python },
    { .label = "Ruby",
      .extension = ".rb",
      .entry_file = "",
      .checker_exe = "",
      .checker_args = "",
      .runner_exe = "",
      .runner_args = "",
      .working_dir = "",
      .lang = GenLang::ruby },
    { .label = "LuaJIT",
      .extension = ".lua",
      .entry_file = "tests/quick/quick_main.lua",
      .checker_exe = "",
      .checker_args = "",
      .runner_exe = "bin/Release/kwxlua.exe",
      .runner_args = "tests/quick/quick_main.lua",
      .working_dir = ".",
      .lang = GenLang::luajit },
    { .label = "Fortran",
      .extension = ".f90",
      .entry_file = "tests/quick/quick_main.f90",
      .checker_exe = "",
      .checker_args = "",
      .runner_exe = "bin/Debug/quick_main.exe",
      .runner_args = "",
      .working_dir = ".",
      .lang = GenLang::fortran },
    { .label = "Go",
      .extension = ".go",
      .entry_file = "tests/quick/quick_main.go",
      .checker_exe = "",
      .checker_args = "",
      .runner_exe = "bin/Debug/quick.exe",
      .runner_args = "",
      .working_dir = "tests/quick",
      .lang = GenLang::go },
    { .label = "Rust",
      .extension = ".rs",
      .entry_file = "tests/quick/quick_main.rs",
      .checker_exe = "cargo",
      .checker_args = "check --quiet --manifest-path tests/quick/Cargo.toml",
      .runner_exe = "cargo",
      .runner_args = "run --quiet --manifest-path tests/quick/Cargo.toml",
      .working_dir = ".",
      .lang = GenLang::rust },
    { .label = "Julia",
      .extension = ".jl",
      .entry_file = "tests/quick/quick_main.jl",
      .checker_exe = "",
      .checker_args = "",
      .runner_exe = "julia",
      .runner_args = "--startup-file=no tests/quick/quick_main.jl",
      .working_dir = ".",
      .lang = GenLang::julia },
} };

// How often the redirected stdout/stderr pipes of the running app are drained into the log.
constexpr int OUTPUT_POLL_MS = 100;

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
static const Node* FindFormNodeWithFileProp(Node* node, PropName prop, const wxString& extension)
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
        if (const Node* found = FindFormNodeWithFileProp(child.get(), prop, extension))
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

// Resolves a path that a recipe writes relative to a base directory.
static wxString ResolveRelativePath(const wxString& base_dir, const wxString& path)
{
    wxFileName full_path(path);
    full_path.MakeAbsolute(base_dir);
    full_path.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);
    return full_path.GetFullPath();
}

// The repository root is the directory holding build_quick.cmd. A form's language file property
// points into that repo (e.g. ../kwxTypeScript/tests/quick/quick_mainframe.ts), which is the only
// anchor that works for every language -- the output-folder properties are project-level and
// Fortran has none at all.
static wxString FindRepoRoot(const wxString& start_dir)
{
    constexpr int MAX_ANCESTOR_LEVELS = 3;

    wxFileName dir_path = wxFileName::DirName(start_dir);
    for (int level = 0; level < MAX_ANCESTOR_LEVELS; ++level)
    {
        const wxString candidate =
            dir_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + "build_quick.cmd";
        if (wxFileName::FileExists(candidate))
        {
            return dir_path.GetPath(wxPATH_GET_VOLUME);
        }

        // A drive root has no parent directory left to strip.
        if (dir_path.GetDirCount() == 0)
        {
            break;
        }
        dir_path.RemoveLastDir();
    }

    return wxEmptyString;
}

struct SampleTarget
{
    wxString repo_root;  // empty when the target could not be resolved
    wxString reason;     // one-line explanation, set only when repo_root is empty
};

static SampleTarget ResolveSampleTarget(const SampleRecipe& recipe)
{
    // The Python and Ruby repos contain generated form modules only -- there is no quick_main.py /
    // quick_main.rb, so nothing exists to check or launch.
    if (recipe.runner_exe[0] == 0)
    {
        return { .repo_root = wxEmptyString,
                 .reason = wxString::Format(
                     "no runnable entry script for %s: this project generates form modules only",
                     wxString(recipe.label)) };
    }

    const PropName prop = GetEntryFileProp(recipe.lang);
    const Node* form = FindFormNodeWithFileProp(Project.get_ProjectNode(), prop, recipe.extension);
    const wxString property_value = (form != nullptr) ? form->as_string(prop).wx() : wxString();
    if (property_value.empty())
    {
        return { .repo_root = wxEmptyString,
                 .reason = wxString::Format(
                     "error: no form names a \"%s\" file, so the sample repository is unknown",
                     recipe.extension) };
    }

    wxFileName file_path(property_value);
    file_path.MakeAbsolute(Project.get_ProjectPath().wx());
    file_path.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);
    const wxString file_dir = file_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);

    const wxString repo_root = FindRepoRoot(file_dir);
    if (repo_root.empty())
    {
        return { .repo_root = wxEmptyString,
                 .reason = wxString::Format(
                     "error: no sample repository (build_quick.cmd) above %s resolved from the "
                     "form's %s file property",
                     file_dir, wxString(recipe.label)) };
    }

    if (recipe.entry_file[0] != 0)
    {
        const wxString entry_path = ResolveRelativePath(repo_root, recipe.entry_file);
        if (!wxFileName::FileExists(entry_path))
        {
            return { .repo_root = wxEmptyString,
                     .reason =
                         wxString::Format("error: sample entry file not found: %s", entry_path) };
        }
    }

    return { .repo_root = repo_root, .reason = wxEmptyString };
}

// A bare executable name (deno, julia, cargo) is resolved through PATH; anything containing a path
// separator is relative to the repository root.
static wxString ResolveExecutable(const wxString& repo_root, const wxString& executable)
{
    if (!executable.Contains("/") && !executable.Contains("\\"))
    {
        return executable;
    }

    return ResolveRelativePath(repo_root, executable);
}

static wxString BuildArgumentList(const wxString& repo_root, const wxString& args)
{
    wxString argument_list;
    wxStringTokenizer tokenizer(args, " ", wxTOKEN_STRTOK);
    while (tokenizer.HasMoreTokens())
    {
        const wxString token = tokenizer.GetNextToken();
        wxString argument = token;
        if (argument.Contains("/") || argument.Contains("\\"))
        {
            argument = ResolveRelativePath(repo_root, argument);
        }

        if (!argument_list.empty())
        {
            argument_list += " ";
        }
        argument_list += QuoteIfNeeded(argument);
    }

    return argument_list;
}

static wxString BuildCommand(const wxString& repo_root, const wxString& executable,
                             const wxString& args)
{
    wxString command = QuoteIfNeeded(ResolveExecutable(repo_root, executable));
    const wxString argument_list = BuildArgumentList(repo_root, args);
    if (!argument_list.empty())
    {
        command += " ";
        command += argument_list;
    }

    return command;
}

static wxString GetWorkingDir(const wxString& repo_root, const wxString& working_dir)
{
    if (working_dir.empty() || working_dir == ".")
    {
        return repo_root;
    }

    wxFileName dir_path = wxFileName::DirName(working_dir);
    dir_path.MakeAbsolute(repo_root);
    dir_path.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);
    return dir_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
}

VerifySampleDlg::VerifySampleDlg(wxWindow* parent) : VerifySampleBase(parent)
{
    for (const SampleRecipe& recipe: s_recipes)
    {
        m_choice_lang->Append(wxString(recipe.label));
    }
    m_choice_lang->SetSelection(0);

    // wxEVT_CLOSE_WINDOW is bound by the generated base class; wxEVT_END_PROCESS is not a dialog
    // event, so it must be bound here.
    Bind(wxEVT_END_PROCESS, &VerifySampleDlg::OnEndProcess, this);
    m_choice_lang->Bind(wxEVT_CHOICE, &VerifySampleDlg::OnLanguageChanged, this);

    m_output_timer.SetOwner(this);
    Bind(wxEVT_TIMER, &VerifySampleDlg::OnOutputTimer, this);

    // The Kill button is a local in the generated base class, so it has no member to reach.
    m_btn_kill = wxDynamicCast(wxWindow::FindWindowByLabel("Kill", this), wxButton);

    UpdateAppName();
    UpdateKillButton();
}

VerifySampleDlg::~VerifySampleDlg()
{
    m_output_timer.Stop();

    if (m_app_pid != 0)
    {
        // The dialog can be destroyed without going through OnClose (e.g. when the main frame
        // closes), so the app and its shell must not outlive the dialog.
        std::ignore = wxProcess::Kill(static_cast<int>(m_app_pid), wxSIGKILL, wxKILL_CHILDREN);
        m_app_pid = 0;
    }

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

void VerifySampleDlg::UpdateKillButton()
{
    if (m_btn_kill)
    {
        m_btn_kill->Enable(m_app_pid != 0);
    }
}

void VerifySampleDlg::TerminateApp()
{
    if (m_app_pid == 0)
    {
        return;
    }

    const long pid = m_app_pid;

    m_output_timer.Stop();

    // wxKILL_CHILDREN is required: a console app such as python.exe owns its console window
    // (hosted by conhost.exe) as a child process, so killing only the app leaves the shell
    // window open.
    const wxKillError kill_result =
        wxProcess::Kill(static_cast<int>(pid), wxSIGKILL, wxKILL_CHILDREN);
    if (kill_result == wxKILL_OK)
    {
        AppendLog(wxString::Format("terminated app and shell (pid %ld)", pid));
    }
    else
    {
        AppendLog(wxString::Format("wxKill failed for app pid %ld (error %d)", pid,
                                   static_cast<int>(kill_result)));
    }

    // The app may have written output right before it was killed.
    DrainRemainingOutput();

    if (m_process)
    {
        // Kill() never delivers wxEVT_END_PROCESS, so this object's notification was not processed
        // by us. Per wxProcess ownership rules we must Detach() and let it delete itself --
        // deleting it here would be a double free.
        m_process->Detach();
        m_process = nullptr;
    }

    m_app_pid = 0;
    UpdateKillButton();
}

void VerifySampleDlg::OnOutputTimer(wxTimerEvent& /* event */)
{
    std::ignore = ReadProcessOutput();
}

bool VerifySampleDlg::ReadProcessOutput()
{
    if (!m_process)
    {
        return false;
    }

    std::array<char, 4096> buffer {};
    bool read_any = false;

    // Bound the work per poll so an app that streams output cannot starve the UI -- the next
    // timer tick picks up whatever is left.
    constexpr size_t MAX_READS_PER_POLL = 16;
    for (size_t read_count = 0; read_count < MAX_READS_PER_POLL; ++read_count)
    {
        bool read_something = false;

        // IsInputAvailable()/IsErrorAvailable() use PeekNamedPipe, so the Read() calls below
        // cannot block waiting for data that has not been written yet.
        if (m_process->IsInputAvailable())
        {
            const size_t bytes_read =
                m_process->GetInputStream()->Read(buffer.data(), buffer.size()).LastRead();
            if (bytes_read != 0)
            {
                m_stdout_buffer.append(buffer.data(), bytes_read);
                DrainOutputBuffer(m_stdout_buffer, false);
                read_something = true;
            }
        }

        if (m_process->IsErrorAvailable())
        {
            const size_t bytes_read =
                m_process->GetErrorStream()->Read(buffer.data(), buffer.size()).LastRead();
            if (bytes_read != 0)
            {
                m_stderr_buffer.append(buffer.data(), bytes_read);
                DrainOutputBuffer(m_stderr_buffer, true);
                read_something = true;
            }
        }

        if (!read_something)
        {
            break;
        }
        read_any = true;
    }

    return read_any;
}

void VerifySampleDlg::DrainRemainingOutput()
{
    // A poll is bounded so a live app cannot starve the UI, but once the app has exited the pipe
    // may still hold more than one poll's worth -- a long Julia or Deno stack trace, for example.
    // Keep draining until the pipe is empty, with a hard cap so a grandchild that inherited the
    // write handle cannot spin this loop forever.
    constexpr int MAX_DRAIN_PASSES = 64;
    for (int pass = 0; pass < MAX_DRAIN_PASSES; ++pass)
    {
        if (!ReadProcessOutput())
        {
            break;
        }
    }

    FlushOutputBuffers();
}

void VerifySampleDlg::DrainOutputBuffer(std::string& buffer, bool is_stderr)
{
    // Only whole lines are logged: a read can end in the middle of a line, and the remainder has to
    // stay buffered until its newline arrives.
    size_t newline_pos = buffer.find('\n');
    while (newline_pos != std::string::npos)
    {
        std::string line = buffer.substr(0, newline_pos);
        buffer.erase(0, newline_pos + 1);
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        LogOutputLine(is_stderr, line);
        newline_pos = buffer.find('\n');
    }
}

void VerifySampleDlg::LogOutputLine(bool is_stderr, const std::string& line)
{
    const wxString text = wxString::FromUTF8(line);
    const wxString prefix = is_stderr ? wxString("err: ") : wxString("out: ");
    m_output_seen = true;
    AppendLog(prefix + text);
}

void VerifySampleDlg::FlushOutputBuffers()
{
    // A final write may not have been newline-terminated, so emit whatever is left.
    if (!m_stdout_buffer.empty())
    {
        LogOutputLine(false, m_stdout_buffer);
        m_stdout_buffer.clear();
    }

    if (!m_stderr_buffer.empty())
    {
        LogOutputLine(true, m_stderr_buffer);
        m_stderr_buffer.clear();
    }
}

void VerifySampleDlg::UpdateAppName()
{
    const int selection = m_choice_lang->GetSelection();
    const SampleRecipe& recipe = s_recipes[static_cast<size_t>(std::max(0, selection))];

    const SampleTarget target = ResolveSampleTarget(recipe);
    if (target.repo_root.empty())
    {
        m_app_name->SetLabel(target.reason);
        Layout();
        return;
    }

    // Show exactly what Run will execute, including the directory it runs in.
    wxString app_name = BuildCommand(target.repo_root, recipe.runner_exe, recipe.runner_args);
    app_name += "  [cwd ";
    app_name += GetWorkingDir(target.repo_root, recipe.working_dir);
    app_name += "]";

    m_app_name->SetLabel(app_name);
    Layout();
}

void VerifySampleDlg::OnLanguageChanged(wxCommandEvent& /* event */)
{
    UpdateAppName();
}

void VerifySampleDlg::OnCheck(wxCommandEvent& /* event */)
{
    const int selection = m_choice_lang->GetSelection();
    const SampleRecipe& recipe = s_recipes[static_cast<size_t>(std::max(0, selection))];

    const SampleTarget target = ResolveSampleTarget(recipe);
    if (target.repo_root.empty())
    {
        AppendLog(target.reason);
        return;
    }

    if (recipe.checker_exe[0] == 0)
    {
        AppendLog(wxString::Format("no checker configured for %s", wxString(recipe.label)));
        return;
    }

    const wxString command =
        BuildCommand(target.repo_root, recipe.checker_exe, recipe.checker_args);
    const wxString working_dir = GetWorkingDir(target.repo_root, recipe.working_dir);

    m_log->Freeze();
    {
        const wxBusyCursor busy_cursor;

        wxExecuteEnv env;
        env.cwd = working_dir;

        // This is the only deadlock-free way to capture both streams synchronously. Note that
        // this blocks the UI while a slow checker runs (the first cargo check or go build can
        // take a while). A timeout would require the async wxProcess + wxTimer route instead.
        wxArrayString out;
        wxArrayString err;
        const long result_code =
            wxExecute(command, out, err, wxEXEC_SYNC | wxEXEC_HIDE_CONSOLE, &env);

        AppendLog(wxString::Format("check: %s", command));
        AppendLog(wxString::Format("cwd:   %s", working_dir));
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

    const SampleTarget target = ResolveSampleTarget(recipe);
    if (target.repo_root.empty())
    {
        AppendLog(target.reason);
        return;
    }

    const wxString command = BuildCommand(target.repo_root, recipe.runner_exe, recipe.runner_args);

    m_stdout_buffer.clear();
    m_stderr_buffer.clear();
    m_output_seen = false;

    wxExecuteEnv env;
    // TypeScript, LuaJIT, Julia, Fortran and Rust run from the repo root; Go supplies
    // "tests/quick" because the app must start in its package directory.
    env.cwd = GetWorkingDir(target.repo_root, recipe.working_dir);

    m_process = new wxProcess(this);
    // Redirecting stdio makes the launcher's console output readable through GetInputStream() /
    // GetErrorStream(). Every launcher in s_recipes is a console process -- the python, ruby and
    // julia interpreters as well as the kwxts.exe and kwxlua.exe wrappers -- so without this the
    // diagnostic they print (an uncaught exception traceback, for example) dies with the console.
    m_process->Redirect();

    // wxEXEC_SHOW_CONSOLE is required: redirecting stdio makes wxWidgets add
    // STARTF_USESHOWWINDOW/SW_HIDE on MSW by default, which also hides the sample app's first
    // top-level window -- not just its console.
    m_app_pid = wxExecute(command, wxEXEC_ASYNC | wxEXEC_SHOW_CONSOLE, m_process, &env);
    if (m_app_pid == 0)
    {
        delete m_process;
        m_process = nullptr;
        AppendLog(wxString::Format("error: failed to launch: %s", command));
        return;
    }

    AppendLog(wxString::Format("launched pid %ld: %s", m_app_pid, command));
    AppendLog(wxString::Format("cwd: %s", env.cwd));
    UpdateKillButton();

    // Poll the redirected pipes so console output is shown while the app is still running.
    m_output_timer.Start(OUTPUT_POLL_MS);
}

void VerifySampleDlg::OnKill(wxCommandEvent& /* event */)
{
    if (m_app_pid == 0)
    {
        AppendLog("no app running");
        return;
    }

    TerminateApp();
}

void VerifySampleDlg::OnClose(wxCommandEvent& /* event */)
{
    // Closing the dialog must not leave the app or its console/shell window behind.
    TerminateApp();

    EndModal(wxID_CLOSE);
}

void VerifySampleDlg::OnCloseWindow(wxCloseEvent& event)
{
    // Closing the dialog must not leave the app or its console/shell window behind.
    TerminateApp();

    event.Skip();
}

void VerifySampleDlg::OnEndProcess(wxProcessEvent& event)
{
    const int pid = event.GetPid();
    const int exit_code = event.GetExitCode();

    m_output_timer.Stop();

    // Drain what the app wrote before terminating -- the streams close with the process, so
    // anything still buffered would otherwise be lost.
    DrainRemainingOutput();

    AppendLog(wxString::Format("app pid %d exited with %d", pid, exit_code));

    if (!m_output_seen)
    {
        // Distinguishes "the app printed nothing" from "capture is broken" -- a console app that
        // dies silently is a different problem than a missing redirect.
        AppendLog("(the app wrote nothing to stdout or stderr)");
    }

    // Processing wxEVT_END_PROCESS makes the handler responsible for deleting the wxProcess.
    if (m_process)
    {
        delete m_process;
        m_process = nullptr;
    }
    m_app_pid = 0;
    UpdateKillButton();
}
