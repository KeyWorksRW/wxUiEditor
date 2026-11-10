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
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/process.h>
#include <wx/textctrl.h>
#include <wx/utils.h>

#include "verify_sample_dlg.h"

#include "gen_enums.h"        // PropName enumeration
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "wxue_namespace/wxue_string.h"

struct SampleRecipe
{
    const char* label;       // shown in the wxChoice
    const char* extension;   // e.g. L".ts", used to locate the generated file
    const char* check_exe;   // empty wchar_t string when there is no checker
    const char* check_args;  // may contain {file}
    const char* run_exe;
    const char* run_args;  // may contain {file}
    GenLang lang;          // passed to ProjectHandler to resolve the output directory
};

// Every entry after the last field is hardcoded for a specific maintainer machine.
static const std::array<SampleRecipe, 8> s_recipes = { {
    { .label = "TypeScript",
      .extension = ".ts",
      .check_exe = "deno",
      .check_args = "check {file}",
      .run_exe = "kwxts",
      .run_args = "{file}",
      .lang = GenLang::typescript },
    { .label = "Python",
      .extension = ".py",
      .check_exe = "python",
      .check_args = "-m py_compile {file}",
      .run_exe = "python",
      .run_args = "{file}",
      .lang = GenLang::python },
    { .label = "Ruby",
      .extension = ".rb",
      .check_exe = "ruby",
      .check_args = "-c {file}",
      .run_exe = "ruby",
      .run_args = "{file}",
      .lang = GenLang::ruby },
    { .label = "LuaJIT",
      .extension = ".lua",
      .check_exe = "luajit",
      .check_args = "-bl {file}",
      .run_exe = "luajit",
      .run_args = "{file}",
      .lang = GenLang::luajit },
    { .label = "Fortran",
      .extension = ".f90",
      .check_exe = "",
      .check_args = "",
      .run_exe = "quick.exe",
      .run_args = "",
      .lang = GenLang::fortran },
    { .label = "Go",
      .extension = ".go",
      .check_exe = "go",
      .check_args = "build ./...",
      .run_exe = "go",
      .run_args = "run .",
      .lang = GenLang::go },
    { .label = "Rust",
      .extension = ".rs",
      .check_exe = "cargo",
      .check_args = "check",
      .run_exe = "cargo",
      .run_args = "run",
      .lang = GenLang::rust },
    { .label = "Julia",
      .extension = ".jl",
      .check_exe = "",
      .check_args = "",
      .run_exe = "julia",
      .run_args = "--project=. {file}",
      .lang = GenLang::julia },
} };

// How often the redirected stdout/stderr pipes of the running app are drained into the log.
constexpr int OUTPUT_POLL_MS = 100;

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

static wxString BuildCommand(const wxString& executable, const wxString& args,
                             const wxString& entry_file)
{
    wxString command = QuoteIfNeeded(executable);
    if (!args.empty())
    {
        wxString filled_args = args;
        filled_args.Replace("{file}", QuoteIfNeeded(entry_file));
        command += " ";
        command += filled_args;
    }

    return command;
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

wxString VerifySampleDlg::GetOutputDir(GenLang lang)
{
    wxString output_dir;

    const Node* project_node = Project.get_ProjectNode();
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
                                       const wxString& extension, bool log_error)
{
    // Prefer the form node that explicitly names the entry file for this language. Otherwise fall
    // through to the newest-generated-file scan below.
    const PropName prop = GetEntryFileProp(lang);
    if (const Node* form = FindFormNodeWithFileProp(Project.get_ProjectNode(), prop, extension);
        form)
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

    if (newest_file.empty() && log_error)
    {
        AppendLog(wxString::Format("error: no \"*%s\" file found in %s", extension, output_path));
    }

    return newest_file;
}

void VerifySampleDlg::UpdateAppName()
{
    const int selection = m_choice_lang->GetSelection();
    const SampleRecipe& recipe = s_recipes[static_cast<size_t>(std::max(0, selection))];

    wxString app_path;
    const wxString output_dir = GetOutputDir(recipe.lang);
    if (!output_dir.empty())
    {
        const wxString run_args(recipe.run_args);
        if (run_args.Contains("{file}"))
        {
            app_path = GetEntryFile(recipe.lang, output_dir, recipe.extension, false);
        }
        else if (wxFileName(recipe.run_exe).HasExt())
        {
            // Languages such as Fortran run a built executable that lives in the output dir.
            app_path = wxFileName(output_dir, recipe.run_exe).GetFullPath();
        }
        else
        {
            // Go and Rust are launched by their toolchain from the output dir, so that directory
            // is the only meaningful location to show.
            app_path = output_dir;
        }
    }

    if (app_path.empty())
    {
        app_path = "(no app found for this target)";
    }

    m_app_name->SetLabel(app_path);
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
    const wxString working_dir = output_dir;

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

    m_stdout_buffer.clear();
    m_stderr_buffer.clear();
    m_output_seen = false;

    wxExecuteEnv env;
    env.cwd = output_dir;  // samples are always run from the project's output dir for this language

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
