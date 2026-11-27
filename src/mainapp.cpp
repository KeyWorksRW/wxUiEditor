/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main application class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [11-09-2026]

#include <algorithm>
#include <chrono>
#include <format>
#include <ranges>
#include <string_view>
#include <tuple>
#include <utility>  // std::to_underlying
#include <vector>

#include <wx/arrstr.h>    // wxArrayString class
#include <wx/cmdline.h>   // wxCmdLineParser and related classes for parsing the command
#include <wx/config.h>    // wxConfig base header
#include <wx/cshelp.h>    // Context-sensitive help support classes
#include <wx/dir.h>       // wxDir is a class for enumerating the files in a directory
#include <wx/filedlg.h>   // wxFileDialog base header
#include <wx/log.h>       // wxLogStderr
#include <wx/msgdlg.h>    // wxMessageDialog
#include <wx/msgout.h>    // wxMessageOutput and related classes
#include <wx/sysopt.h>    // wxSystemOptions
#include <wx/utils.h>     // Miscellaneous utilities
#include <wx/wfstream.h>  // wxFFile

#include "mainapp.h"

#include "cli_ui_guard.h"              // cli_ui::InstallModalGuard
#include "conformance_kit.h"           // WriteConformanceKit -- generated conformance kit
#include "ffi_ui.h"                    // FfiUiDescriptor load and role resolution
#include "gen_common.h"                // Common component functions
#include "gen_results.h"               // Code generation file writing functions
#include "internal/msg_logging.h"      // MsgLogging -- Message logging class
#include "internal/node_search_dlg.h"  // FindNodeByClassName
#include "mainframe.h"                 // MainFrame -- Main window frame
#include "node_creator.h"              // NodeCreator class
#include "preferences.h"               // Set/Get wxUiEditor preferences
#include "project_handler.h"           // ProjectHandler class
#include "supported_matrix.h"          // WriteSupportedMatrix -- full capability matrix
#include "utils.h"                     // Utility functions that work with properties
#include "verify_codegen.h"  // VerifyCodeGen -- Verify that code generation did not change
#include "verify_import.h"   // VerifyImport -- Verify that importing a project file did not change
#include "version.h"         // Version numbers and other constants

#include "frozen/map.h"  // frozen::map

#include "ui/startup_dlg.h"  // StartupDlg -- Dialog to display if wxUE is launched with no arguments

#if defined(INTERNAL_TESTING)
    #include "internal/ui_harness.h"  // ui_harness::Start -- debug-only UI command harness
#endif

#include "helptext/doc_view_frame.h"

#if defined(_WIN32) && defined(_MSC_VER)
    #pragma comment(lib, "kernel32.lib")
    #pragma comment(lib, "user32.lib")
    #pragma comment(lib, "gdi32.lib")
    #pragma comment(lib, "comctl32.lib")
    #pragma comment(lib, "comdlg32.lib")
    #pragma comment(lib, "shell32.lib")

    #pragma comment(lib, "rpcrt4.lib")
    #pragma comment(lib, "advapi32.lib")

    #if wxUSE_URL_NATIVE
        #pragma comment(lib, "wininet.lib")
    #endif
#endif

// add_executable(wxUiEditor WIN32 -- this must be used if wxIMPLEMENT_APP is used.

// If wxIMPLEMENT_APP is used, then std::cout and std::cerr will not work.

// If wxIMPLEMENT_APP_CONSOLE is used, a console will be created if the app isn't being run from a
// console, however std::cout and std::cerr will work.

wxIMPLEMENT_APP(App);  // NOLINT (cppcheck-suppress)
// wxIMPLEMENT_APP_CONSOLE(App);

#if defined(_WIN32) && defined(_DEBUG)

// There are several classes that are created during initialization which are not explicitly deleted
// before the memory check, so don't be surprised to see quite a few memory "leaks"

// Uncomment the following to turn on a memory leak dump on exit.
// #define USE_CRT_MEMORY_DUMP

#endif  // _WIN32 && defined(_DEBUG)

// Static storage for command-line filename (populated by ParseGenerationType or
// GetCommandLineFilename)
static wxString s_filename;  // NOLINT (cppcheck-suppress)

// Helper to process and normalize a filename (add extension, handle wildcards)
static void ProcessFilename(wxString& filename)
{
    if (filename.empty())
    {
        return;
    }

    // Handle missing extension before wildcard expansion
    // Check if filename contains wildcards - if so, only add extension if no dot present
    const bool has_wildcards = filename.Contains('*') || filename.Contains('?');

    if (has_wildcards)
    {
        // For wildcards, only add extension if there's no dot at all
        if (!filename.Contains('.'))
        {
            filename += ".wxui";
        }
    }
    else
    {
        // For non-wildcards, use wxFileName to properly check for extension
        wxFileName wxfn(filename);
        if (!wxfn.HasExt())
        {
            wxfn.SetExt("wxui");
            filename = wxfn.GetFullPath();
        }
    }

    // Handle wildcards after extension is added
    if (has_wildcards)
    {
        wxDir dir;
        dir.Open("./");
        const wxString filespec = filename;
        if (!dir.GetFirst(&filename, filespec, wxDIR_FILES))
        {
            // No match found
            filename.clear();
        }
    }
}

// Helper to retrieve and process filename from command line
static wxString& GetCommandLineFilename(wxCmdLineParser& parser)
{
    if (!s_filename.empty())
    {
        return s_filename;
    }

    // Try to get filename from parameter
    if (parser.GetParamCount() > 0)
    {
        s_filename = parser.GetParam(0);
        ProcessFilename(s_filename);
    }
    // Otherwise s_filename may have been set by ParseGenerationType

    return s_filename;
}
// ######################## --dump_ui_ir support ########################

// Appends `text` with JSON string escaping. The surrounding quotes are added by
// AppendJsonString.
static void AppendJsonEscaped(std::string& json, std::string_view text)
{
    for (const char character: text)
    {
        switch (character)
        {
            case '"':
                json += "\\\"";
                break;
            case '\\':
                json += "\\\\";
                break;
            case '\b':
                json += "\\b";
                break;
            case '\f':
                json += "\\f";
                break;
            case '\n':
                json += "\\n";
                break;
            case '\r':
                json += "\\r";
                break;
            case '\t':
                json += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(character) < 0x20)
                {
                    json += std::format("\\u{:04x}", static_cast<unsigned int>(
                                                         static_cast<unsigned char>(character)));
                }
                else
                {
                    json += character;
                }
                break;
        }
    }
}

static void AppendJsonString(std::string& json, std::string_view text)
{
    json += '"';
    AppendJsonEscaped(json, text);
    json += '"';
}

static void AddRoleUsed(std::vector<std::string>& roles, std::string_view role)
{
    if (std::ranges::find(roles, role) == roles.end())
    {
        roles.emplace_back(role);
    }
}

// Derives the role set a node's properties require, per C2 in .private/kwxPlan.md.
static std::vector<std::string> GetRolesUsed(Node* node)
{
    std::vector<std::string> roles;
    AddRoleUsed(roles, "ctor");

    if (node->is_Gen(gen_wxStdDialogButtonSizer))
    {
        AddRoleUsed(roles, "create");
        AddRoleUsed(roles, "add_button");
        AddRoleUsed(roles, "realize");
    }

    if (node->HasValue(prop_value))
    {
        AddRoleUsed(roles, "set_value");
    }

    if (node->HasValue(prop_font))
    {
        AddRoleUsed(roles, "set_font");
        AddRoleUsed(roles, "create");
        AddRoleUsed(roles, "delete");
    }

    if (node->HasValue(prop_bitmap))
    {
        AddRoleUsed(roles, "from_base64");
        AddRoleUsed(roles, "get_bitmap");
        AddRoleUsed(roles, "set_bitmap");
        AddRoleUsed(roles, "delete");
    }

    if (const Node* parent = node->get_Parent(); parent && parent->is_Sizer())
    {
        AddRoleUsed(roles, node->is_Sizer() ? "add_sizer" : "add_window");

        if (node->HasValue(prop_proportion) && node->as_int(prop_proportion) != 0)
        {
            AddRoleUsed(roles, "proportion");
        }
        if (node->HasProp(prop_flags) && node->as_string(prop_flags).contains("wxEXPAND"))
        {
            AddRoleUsed(roles, "expand");
        }
        if (node->HasValue(prop_border) && node->as_int(prop_border) > 0)
        {
            AddRoleUsed(roles, "border");
        }
    }

    for (const auto& event: node->get_MapEvents() | std::views::values)
    {
        if (!event.get_value().empty())
        {
            AddRoleUsed(roles, "bind");
            break;
        }
    }

    return roles;
}

// Returns true when the node's generator constructs a wx class of its own inside the form, so the
// FFI emitters must resolve a descriptor class and a role set for it.
//
// Excluded are pseudo nodes that only describe a parent's properties (sizeritem, spacer, toolbar
// tools, ribbon buttons, data view columns, property grid items, timers) and the project
// bookkeeping nodes (folders, images, data).
//
// Node::is_Widget() is true only for `type_widget`, so a panel, notebook, data view, property grid,
// splitter, toolbar or bar is not a widget. Testing `is_Widget() || is_Sizer()` alone silently
// dropped every such class from the IR: measured 2026-11-14, a form declaring wxPanel emitted only
// the panel's wxTextCtrl child, and `quick.wxui`'s wxMenuBar, wxMenu, wxMenuItem and wxStatusBar
// were absent.
static bool IsInstantiatedNode(const Node* node)
{
    if (node->is_Widget() || node->is_Sizer() || node->is_MenuBar() || node->is_Menu() ||
        node->is_StatusBar() || node->is_ToolBar() || node->is_RibbonBar())
    {
        return true;
    }

    switch (node->get_GenType())
    {
        case GenEnum::type_container:
        case GenEnum::type_panel:
        case GenEnum::type_notebook:
        case GenEnum::type_auinotebook:
        case GenEnum::type_choicebook:
        case GenEnum::type_listbook:
        case GenEnum::type_simplebook:
        case GenEnum::type_bookpage:
        case GenEnum::type_oldbookpage:
        case GenEnum::type_treepage:
        case GenEnum::type_splitter:
        case GenEnum::type_staticbox:
        case GenEnum::type_dataviewctrl:
        case GenEnum::type_dataviewlistctrl:
        case GenEnum::type_dataviewtreectrl:
        case GenEnum::type_propgrid:
        case GenEnum::type_propgridman:
        case GenEnum::type_propgridpage:
        case GenEnum::type_treelistctrl:
        case GenEnum::type_wizardpagesimple:
        case GenEnum::type_infobarbtn:
        case GenEnum::type_menuitem:
        case GenEnum::type_ctx_menu:
        case GenEnum::type_ribbonpage:
        case GenEnum::type_ribbonpanel:
        case GenEnum::type_ribbonbuttonbar:
        case GenEnum::type_ribbontoolbar:
        case GenEnum::type_ribbongallery:
            return true;

        default:
            return false;
    }
}

// Depth-first collection of every instantiated class below `parent`.
static void CollectInstantiatedNodes(Node* parent, std::vector<Node*>& nodes)
{
    for (size_t index = 0; index < parent->get_ChildCount(); ++index)
    {
        Node* child = parent->get_Child(index);
        if (IsInstantiatedNode(child))
        {
            nodes.emplace_back(child);
        }
        CollectInstantiatedNodes(child, nodes);
    }
}

// Depth-first collection of forms, descending through folders and other non-form nodes.
static void CollectForms(Node* parent, std::vector<Node*>& forms)
{
    for (size_t index = 0; index < parent->get_ChildCount(); ++index)
    {
        Node* child = parent->get_Child(index);
        if (child->is_Form())
        {
            forms.emplace_back(child);
        }
        else
        {
            CollectForms(child, forms);
        }
    }
}

// The inventory name of `node` for `--dump_ui_ir`. A meta name (`separator`, `submenu`) is not a
// class any kwx descriptor can resolve, so it is translated to the class that performs the
// operation.
static std::string_view GetNodeClass(Node* node)
{
    std::string_view class_name = node->get_DeclName();
    if (const auto iter = map_GenNames.find(node->get_GenName()); iter != map_GenNames.end())
    {
        class_name = iter->second;
    }
    return GetFfiClassName(class_name);
}

// Writes the schema-1 language-neutral IR described by C2 in .private/kwxPlan.md.
static bool WriteUiIr(const wxString& output_path)
{
    Node* project_node = Project.get_ProjectNode();
    if (!project_node)
    {
        return false;
    }

    std::vector<Node*> forms;
    CollectForms(project_node, forms);

    std::string json;
    json += "{\n   \"schema\": 1,\n   \"project\": ";
    AppendJsonString(json, std::string_view(Project.get_ProjectFile()));
    json += ",\n   \"forms\": [";

    bool first_form = true;
    for (Node* form: forms)
    {
        if (!first_form)
        {
            json += ',';
        }
        first_form = false;

        std::string_view class_name = form->as_view(prop_class_name);
        if (class_name.empty())
        {
            class_name = form->get_NodeName();
        }

        json += "\n      {\n         \"class_name\": ";
        AppendJsonString(json, class_name);
        json += ",\n         \"node_class\": ";
        AppendJsonString(json, GetNodeClass(form));
        json += ",\n         \"children\": [";

        std::vector<Node*> nodes;
        CollectInstantiatedNodes(form, nodes);

        bool first_node = true;
        for (Node* node: nodes)
        {
            if (!first_node)
            {
                json += ',';
            }
            first_node = false;

            json += "\n            { \"node_class\": ";
            AppendJsonString(json, GetNodeClass(node));
            json += ", \"var_name\": ";
            AppendJsonString(json, node->get_NodeName());
            json += ", \"roles_used\": [";

            const std::vector<std::string> roles = GetRolesUsed(node);
            for (size_t index = 0; index < roles.size(); ++index)
            {
                if (index > 0)
                {
                    json += ", ";
                }
                AppendJsonString(json, roles[index]);
            }

            json += "], \"in_sizer\": ";
            const Node* parent = node->get_Parent();
            json += (parent && parent->is_Sizer()) ? "true" : "false";
            json += " }";
        }

        json += "\n         ]\n      }";
    }

    json += "\n   ]\n}\n";

    wxFFile file(output_path, "wb");
    if (!file.IsOpened())
    {
        wxLogStderr(nullptr).LogText(wxString("Unable to write UI IR: ") + output_path);
        return false;
    }

    file.Write(json.data(), json.size());
    return file.Close();
}

#if defined(_WIN32)
    #include <wx/msw/darkmode.h>

class DarkSettings : public wxDarkModeSettings
{
public:
    [[nodiscard]] wxColour GetColour(wxSystemColour index) { return UserPrefs.GetColour(index); }
};

#endif

App::App() = default;

bool App::OnInit()
{
    wxInitAllImageHandlers();

    // The name is sort of a standard. More importantly, it is sometimes used as the mask in Windows
    // bitmaps for toolbar images.
    wxTheColourDatabase->AddColour("Grey94", wxColour(240, 240, 240));

#if defined(_MSC_VER)
    #if defined(wxUSE_ON_FATAL_EXCEPTION)
    ::wxHandleFatalExceptions(true);
    #endif
#endif

#if defined(_WIN32)
    // Docs say we need this since our toolbar bitmaps >16 colors
    wxSystemOptions::SetOption(
        "msw.remap", 0);  // only remap background of toolbar bitmaps, not the actual content
#endif

    // If we're just providing text-popups for help, then this is all we need.
    wxHelpProvider::Set(new wxSimpleHelpProvider);

    SetVendorName("KeyWorks");
    UserPrefs.ReadConfig();

    if (UserPrefs.is_DarkMode())
    {
#if defined(_WIN32)
        auto* DarkModeSettings = new DarkSettings;
        MSWEnableDarkMode(wxApp::DarkMode_Auto, DarkModeSettings);
#else
        // Unlike MSW, this can be set at any time and it will affect all future windows. Note,
        // however, that we have no control over the specific colors used, so we can't support our
        // High Contrast mode that we support in Windows.
        SetAppearance(wxApp::Appearance::Dark);
#endif
    }

#if defined(_DEBUG)
    m_stderr_output = std::make_unique<wxMessageOutputStderr>();
#endif

    return true;
}

int App::OnRun()
{
    // Intercept modal dialogs whenever Project.is_UiAllowed() is false so an unattended
    // command-line run can never block on a dialog no one can dismiss. In GUI mode
    // is_UiAllowed() is true and the hook passes every dialog through unchanged.
    cli_ui::InstallModalGuard();

#if defined(_DEBUG)
    #if defined(_WIN32)
    // Attach to parent console for command-line output
    // This must be done early, before any output attempts
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        FILE* fp_out { nullptr };
        FILE* fp_err { nullptr };
        if (freopen_s(&fp_out, "CONOUT$", "w", stdout) != 0)
        {
            OutputDebugStringW(L"Failed to redirect stdout to console\n");
        }
        if (freopen_s(&fp_err, "CONOUT$", "w", stderr) != 0)
        {
            OutputDebugStringW(L"Failed to redirect stderr to console\n");
        }
    }
    #else
    // On Unix/Mac, stdout/stderr are automatically connected when run from terminal
    #endif
#endif

    NodeCreation.Initialize();

    wxCmdLineParser parser(argc, argv);
    OnInitCmdLine(parser);
    parser.AddParam("Project filename", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);

    // Because this is a GUI app and may not have been run from a console, std::cout and
    // std::cerr will not work. Instead, messages are written to a log file. The log file is
    // the project filename with the extension changed to ".log".

    // Used with gen_*, test_*, or verify_* to limit generation to a single form
    parser.AddLongOption("form", "limit generation to specified form class name",
                         wxCMD_LINE_VAL_STRING, wxCMD_LINE_HIDDEN);

    // The project filename is taken from the positional "Project filename" parameter, which
    // allows a value-less --import switch to be combined with any generation switch.
    parser.AddLongSwitch("gen_cpp", "generate C++ files and exit");
    parser.AddLongSwitch("gen_python", "generate python files and exit");

    parser.AddLongSwitch("gen_fortran", "generate Fortran files and exit");
    parser.AddLongSwitch("gen_go", "generate Go files and exit");
    parser.AddLongSwitch("gen_julia", "generate Julia files and exit");
    parser.AddLongSwitch("gen_luajit", "generate LuaJIT files and exit");
    parser.AddLongSwitch("gen_ruby", "generate ruby files and exit");
    parser.AddLongSwitch("gen_rust", "generate Rust files and exit");
    parser.AddLongSwitch("gen_typescript", "generate TypeScript files and exit");

    parser.AddLongSwitch("gen_xrc", "generate XRC files and exit");

    parser.AddLongSwitch("gen_all", "generate all language files and exit");

    parser.AddLongSwitch("import",
                         "import the project filename instead of loading a .wxui project");

    // verify_* options generate code internally and compare against existing files on disk.
    // If differences are detected, the diff output is written to a .log file (same name as the
    // project file but with .log extension) and a non-zero exit code is returned.
    //
    // With --form "ClassName", verification is limited to a single form's generated files.
    // This is useful for debugging code generation issues with a specific form.
    //
    // Exit codes:
    //   0 = Success (no differences found)
    //   1 = Failure (differences detected - check log file)
    //   2 = File not found
    //   3 = Invalid (e.g., form name not found in project)
    //
    // Agent usage: To properly capture exit codes in PowerShell, use Start-Process:
    //
    //   $proc = Start-Process -FilePath "wxUiEditord.exe" `
    //       -ArgumentList "--verify_cpp","project.wxui","--form","FormClassName" `
    //       -Wait -NoNewWindow -PassThru
    //   $exitCode = $proc.ExitCode
    //
    // If exit code is non-zero, open the log file to examine differences:
    //   key_open with filePath: "<project_dir>/<project_name>.log"

    parser.AddLongSwitch("verify_cpp", "verify generating C++ files did not change",
                         wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("verify_python", "verify generating Python files did not change",
                         wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("verify_fortran", "verify generating Fortran files did not change",
                         wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("verify_go", "verify generating Go files did not change",
                         wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("verify_julia", "verify generating Julia files did not change",
                         wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("verify_luajit", "verify generating LuaJIT files did not change",
                         wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("verify_rust", "verify generating Rust files did not change",
                         wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("verify_ruby", "verify generating Ruby files did not change",
                         wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("verify_typescript", "verify generating TypeScript files did not change",
                         wxCMD_LINE_HIDDEN);

    parser.AddLongSwitch("verify_all", "verify generating all language files did not change",
                         wxCMD_LINE_HIDDEN);

    // Import/project-file verification (no code generation).
    parser.AddLongSwitch("verify_import", "verify importing a project file did not change",
                         wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("save_import", "import a project file and save it as a .wxui file",
                         wxCMD_LINE_HIDDEN);
    parser.AddLongOption("golden", "path to the golden .wxui file", wxCMD_LINE_VAL_STRING,
                         wxCMD_LINE_HIDDEN);

    // Just a quick way to python, and ruby
    parser.AddLongSwitch("gen_quick", "generate all script files and exit", wxCMD_LINE_HIDDEN);

    // Primarily used for codegen_test/ but could be used for other coverage testing as well.
    parser.AddLongSwitch("gen_coverage", "generate all language types for test coverage",
                         wxCMD_LINE_HIDDEN);

    // Writes the language-neutral UI IR consumed by `kwxgen check-ui`, then exits.
    parser.AddLongOption("dump_ui_ir", "write the project's UI IR and exit", wxCMD_LINE_VAL_STRING,
                         wxCMD_LINE_HIDDEN);

    // Writes the full pair-level capability matrix (.private/kwxSupported.md), then exits.
    parser.AddLongOption("gen_supported", "write the full kwx supported matrix and exit",
                         wxCMD_LINE_VAL_STRING, wxCMD_LINE_HIDDEN);

    // Writes the conformance kit (one minimal .wxui per generator class), then exits.
    parser.AddLongOption("gen_conformance", "write the conformance kit and exit",
                         wxCMD_LINE_VAL_STRING, wxCMD_LINE_HIDDEN);

    // TODO: [Randalphwa - 11-29-2025] Add an "-auto_load" hidden switch that will find all the
    // *.wxui files in the current directory. If there's only one, set that as the project file. If
    // there's more than one, load a dialog to let the user select which project file to load.

    // The "test" options will not write any files, it simply runs the code generation skipping
    // the part where files get written, and generates the log file.

    // TODO: [Randalphwa - 11-29-2025] These could potentially be used similar to verify, with the
    // possibility of using wxMessageOutputDebug() to write the file differences to Debug Console.

    parser.AddLongSwitch("verbose", "verbose log file", wxCMD_LINE_HIDDEN);

    // test_* options run code generation logic without writing any files to disk.
    // Unlike verify_*, test_* does NOT compare against existing files - it only exercises the
    // code generation paths and writes timing/diagnostic info to a log file.
    //
    // Use test_* when you want to:
    //   - Verify that code generation completes without errors/crashes
    //   - Measure code generation performance (timing info in log)
    //   - Debug code generation paths without modifying any files
    //
    // Use verify_* when you want to:
    //   - Detect if generated code would differ from files on disk
    //   - Validate that refactoring didn't change code generation output
    //   - CI/CD pipelines that need to fail if code generation is out of sync
    //
    // With --form "ClassName", test generation is limited to a single form.
    //
    // Exit codes: Always returns 0 unless the project file cannot be loaded.
    // The log file (project_name.log) contains generation timing and any warnings.

    parser.AddLongSwitch("test_cpp", "generate C++ code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_python", "generate Python code and exit", wxCMD_LINE_HIDDEN);

    parser.AddLongSwitch("test_fortran", "generate Fortran code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_go", "generate Go code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_julia", "generate Julia code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_luajit", "generate LuaJIT code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_ruby", "generate Ruby code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_typescript", "generate TypeScript code and exit", wxCMD_LINE_HIDDEN);

    parser.AddLongSwitch("test_xrc", "generate XRC code and exit", wxCMD_LINE_HIDDEN);
    parser.AddLongSwitch("test_all", "generate all code and exit", wxCMD_LINE_HIDDEN);

    parser.AddLongSwitch("load_last", "Load last opened project",
                         wxCMD_LINE_HIDDEN | wxCMD_LINE_SWITCH_NEGATABLE);

    parser.AddLongSwitch("data-version", "return current data_version", wxCMD_LINE_HIDDEN);

    parser.AddLongOption("docview", "Open documentation viewer", wxCMD_LINE_VAL_STRING,
                         wxCMD_LINE_HIDDEN);

    // Debug-only: drive the editor UI from a script file (see src/internal/ui_harness.cpp).
    parser.AddLongOption("dbg-ui-script", "run editor UI commands from a script file",
                         wxCMD_LINE_VAL_STRING, wxCMD_LINE_HIDDEN);
    parser.AddLongOption("dbg-ui-log", "write the UI harness log to this file",
                         wxCMD_LINE_VAL_STRING, wxCMD_LINE_HIDDEN);

    parser.Parse();

    // Return current data_version for AI tools and exit immediately
    if (parser.Found("data-version"))
    {
        wxMessageOutput::Get()->Printf("%d", curSupportedVer);
        return 0;
    }

    // --docview: launch standalone documentation viewer, skipping MainFrame and project loading
    if (wxString zip_str; parser.Found("docview", &zip_str))
    {
        const wxString zip_path = zip_str;
        m_docViewFrame = new DocViewFrame(nullptr, zip_path);
        m_docViewFrame->Show();
        SetTopWindow(m_docViewFrame);
        return wxApp::OnRun();
    }

    // --dump_ui_ir: write the language-neutral UI IR consumed by `kwxgen check-ui`, then exit.
    if (wxString ir_output; parser.Found("dump_ui_ir", &ir_output))
    {
        Project.set_UiAllowed(false);

        wxue::string project_filename = GetCommandLineFilename(parser);
        if (project_filename.empty())
        {
            wxLogStderr(nullptr).LogText("--dump_ui_ir requires a project filename");
            return 1;
        }
        project_filename.make_absolute();

        if (!Project.LoadProject(project_filename, false) || !Project.get_ProjectNode())
        {
            wxLogStderr(nullptr).LogText(wxString("Unable to load project file: ") +
                                         wxString(project_filename.c_str()));
            return 1;
        }

        return WriteUiIr(ir_output) ? 0 : 1;
    }

    // --gen_supported: write the pair-level capability matrix, then exit. It joins the generator
    // library against the six descriptors and coverage.json, so no project is loaded.
    if (wxString supported_path; parser.Found("gen_supported", &supported_path))
    {
        Project.set_UiAllowed(false);

        if (supported_path.empty())
        {
            wxLogStderr(nullptr).LogText("--gen_supported requires an output path");
            return 1;
        }

        return WriteSupportedMatrix(supported_path.utf8_string()) ? 0 : 1;
    }

    // --gen_conformance: write the generated conformance kit, then exit. It walks the generator
    // library, so no project is loaded.
    if (wxString kit_directory; parser.Found("gen_conformance", &kit_directory))
    {
        Project.set_UiAllowed(false);

        if (kit_directory.empty())
        {
            wxLogStderr(nullptr).LogText("--gen_conformance requires an output directory");
            return 1;
        }

        return WriteConformanceKit(kit_directory.utf8_string()) ? 0 : 1;
    }

#if defined(INTERNAL_TESTING)
    if (!g_pMsgLogging)
    {
        g_pMsgLogging = new MsgLogging();
        wxLog::SetActiveTarget(g_pMsgLogging);

        // Replace wxWidgets' default assertion handler with our own (ttAssertionHandler
        // in assertion_dlg.cpp). It provides a three-button dialog (DebugBreak/Continue/Exit)
        // matching our custom ASSERT macros. Once set, wxASSERT will use this handler
        // instead of calling App::OnAssertFailure().
        wxSetAssertHandler(ttAssertionHandler);
    }
#endif

    /*
        Command-line options are categorized into three types for non-interactive operation:

        gen_* options - Generate code files for the specified language(s) and write them to disk,
                        then exit. Used for build automation and CI/CD pipelines.

        verify_* options - Generate code internally and compare against existing files to verify
                           that code generation produces identical output. Returns non-zero exit
                           code if differences are detected. Does not modify any files.

        test_* options - Execute code generation logic without writing any files. Primarily used
                         for testing code generation paths and generating log files for debugging.
    */

    bool is_verify_mode = false;
    constexpr auto verify_options = std::to_array(
        { "verify_cpp", "verify_python", "verify_fortran", "verify_go", "verify_julia",
          "verify_luajit", "verify_rust", "verify_ruby", "verify_typescript", "verify_all" });

    for (const auto& opt: verify_options)
    {
        if (parser.Found(wxString(opt)))
        {
            is_verify_mode = true;
            break;
        }
    }

    bool is_project_loaded = false;

    // Import/project-file verification is handled separately from the verify_* code-generation
    // switches above because it does not require a language switch.
    if (parser.FoundSwitch("verify_import") == wxCMD_SWITCH_ON ||
        parser.FoundSwitch("save_import") == wxCMD_SWITCH_ON)
    {
        // Headless: the modal guard suppresses any dialog these paths would otherwise show.
        Project.set_UiAllowed(false);
        return VerifyImport(parser, is_project_loaded);
    }

    if (is_verify_mode)
    {
        // Headless: a missing project file, an unknown language, or a bad project would
        // otherwise pop a dialog before the project is ever loaded.
        Project.set_UiAllowed(false);
        return VerifyCodeGen(parser, is_project_loaded);
    }

    // A positive return value means code generation was for command-line only
    const int result = Generate(parser, is_project_loaded);
    if (result == cmd_gen_project_not_loaded || result == cmd_gen_write_error)
    {
        return 1;
    }
    if (result == cmd_gen_success)
    {
        return 0;
    }

    /*
        Normal GUI mode: Create the main window and handle project loading.

        If a project file was specified on the command line, load it directly. Otherwise, if
        --load_last was specified or enabled in preferences, attempt to load the most recent
        project from history. If no project is loaded by this point, display the startup dialog
        to let the user choose or create a project. The main window is shown only after a project
        is successfully loaded.
    */

    m_frame = new MainFrame();
    ASSERT(m_frame);

    if (result == cmd_project_file_only)
    {
        const wxString& filename = GetCommandLineFilename(parser);
        if (!Project.LoadProject(filename, true))
        {
            wxMessageBox(wxString("Unable to load project file: ") << filename,
                         "Project Load Error", wxOK | wxICON_ERROR);
            is_project_loaded = false;
        }
        else
        {
            is_project_loaded = true;
        }
    }
    else if (result == cmd_no_params || result == cmd_gen_project_not_found)
    {
        if (const wxCmdLineSwitchState switch_result = parser.FoundSwitch("load_last");
            switch_result != wxCMD_SWITCH_NOT_FOUND || UserPrefs.is_LoadLastProject())
        {
            wxFileHistory& file_history = m_frame->getFileHistory();
            if (file_history.GetCount() == 0)
            {
                is_project_loaded = false;
            }
            else
            {
                const wxue::string file = file_history.GetHistoryFile(0).utf8_string();
                if (!file.file_exists())
                {
                    file_history.RemoveFileFromHistory(0);
                    wxMessageBox(wxue::string("Last project file does not exist: ") << file,
                                 "Missing Project File", wxOK | wxICON_ERROR);
                }
                else
                {
                    is_project_loaded = Project.LoadProject(file);
                }
            }
        }
    }

    if (!is_project_loaded)
    {
        is_project_loaded = DisplayStartupDlg(nullptr);
    }

    // If the user launched the documentation viewer from the startup dialog,
    // switch to docview mode (same as the --docview command-line option).
    if (m_docViewFrame)
    {
        // The MainFrame was created above but is no longer needed; destroy it
        // before running the doc-viewer event loop.
        if (m_frame)
        {
            m_frame->Destroy();
            m_frame = nullptr;
        }
        return wxApp::OnRun();
    }

    if (is_project_loaded)
    {
        m_frame->Show();
        SetTopWindow(m_frame);

#if defined(INTERNAL_TESTING)
        if (wxString dbg_script; parser.Found("dbg-ui-script", &dbg_script))
        {
            // Default the log next to the script so the agent always knows where to read it.
            wxString dbg_log;
            if (!parser.Found("dbg-ui-log", &dbg_log))
            {
                dbg_log = dbg_script + ".log";
            }
            ui_harness::Start(dbg_script, dbg_log);
        }
#endif

#if defined(_DEBUG)
        // if (AutoMsgWindow())
        // ShowMsgWindow();
#endif  // _DEBUG

        return wxApp::OnRun();
    }

    if (m_frame)
    {
        m_frame->Close();
        m_frame = nullptr;
    }
    return 1;
}

int App::OnExit()
{
#if defined(INTERNAL_TESTING)
    if (g_pMsgLogging)
    {
        wxLog::SetActiveTarget(nullptr);
        delete g_pMsgLogging;
        g_pMsgLogging = nullptr;
    }
#endif
    return wxApp::OnExit();
}

#if defined(_DEBUG)
void App::DebugOutput(const wxString& str)
{
    if (m_stderr_output)
    {
        m_stderr_output->Output(str);
        fflush(stderr);
    }
}
#endif

bool App::isFireCreationMsgs()
{
    return (UserPrefs.GetDebugFlags() & Prefs::PREFS_CREATION_MSG);
}

bool App::isPjtMemberPrefix()
{
    return (UserPrefs.GetProjectFlags() & Prefs::PREFS_PJT_MEMBER_PREFIX);
}

bool App::AutoMsgWindow()
{
    return (UserPrefs.GetDebugFlags() & Prefs::PREFS_MSG_WINDOW);
}

#if defined(_WIN32) && defined(_DEBUG) && defined(wxUSE_ON_FATAL_EXCEPTION) && \
    defined(wxUSE_STACKWALKER)

    #include <wx/stackwalk.h>

class StackLogger : public wxStackWalker
{
public:
    [[nodiscard]] auto& GetCalls() { return m_calls; }

protected:
    void OnStackFrame(const wxStackFrame& frame) override
    {
        if (frame.HasSourceLocation())
        {
            wxue::string source;
            source << frame.GetFileName().utf8_string() << ':' << (to_int) frame.GetLine();

            wxString params;
            if (const size_t paramCount = frame.GetParamCount(); paramCount > 0)
            {
                params << "(";

                for (size_t i = 0; i < paramCount; ++i)
                {
                    wxString type;
                    wxString name;
                    wxString value;
                    if (frame.GetParam(i, &type, &name, &value))
                    {
                        params << type << " " << name << " = " << value << ", ";
                    }
                }

                params << ")";
            }

            if (params.size() > 100)
            {
                params = "(...)";
            }

            m_calls.emplace_back()
                << (to_int) frame.GetLevel() << ' ' << frame.GetName().utf8_string()
                << params.utf8_string() << ' ' << source;
        }
        else
        {
            m_calls.emplace_back()
                << (to_int) frame.GetLevel() << ' ' << frame.GetName().utf8_string();
        }
    }

private:
    std::vector<wxue::string> m_calls;
};

#endif  // defined(_DEBUG) && defined(wxUSE_ON_FATAL_EXCEPTION) && defined(wxUSE_STACKWALKER)

#if defined(_MSC_VER) && defined(wxUSE_ON_FATAL_EXCEPTION)

void App::OnFatalException()
{
    #if defined(_DEBUG) && defined(wxUSE_STACKWALKER)

    StackLogger logger;
    logger.WalkFromException();
    for (auto& iter: logger.GetCalls())
    {
        // We're only interested in our own source code, so ignore the rest.
        if (!iter.contains("wxUiEditor"))
        {
            continue;
        }

        wxLogDebug(iter.c_str());
    }

    // We now have the relevant call stack displayed in the debugger, so break into it.
    wxTrap();

    #endif  // _DEBUG

    // Let the user know something terrible happened.
    wxMessageBox("A fatal exception has occurred!", txtVersion);
}

#endif  // defined(_MSC_VER) && defined(wxUSE_ON_FATAL_EXCEPTION)

#if defined(INTERNAL_TESTING)
void App::ShowMsgWindow()
{
    g_pMsgLogging->ShowLogger();
}
#endif

#if defined(_DEBUG) || defined(INTERNAL_TESTING)

    #include "newdialogs/new_mdi.h"  // NewMdiForm -- Dialog for creating a new MDI application

// Don't make this static or Bind() will not work
void App::DbgCurrentTest(wxCommandEvent& /* event unused */)  // NOLINT (cppcheck-suppress)
{
    wxGetMainFrame()->SelectNode(Project.get_ProjectNode(), evt_flags::force_selection);

    NewMdiForm mdi_dlg(wxGetFrame().getWindow());
    if (mdi_dlg.ShowModal() == wxID_OK)
    {
        mdi_dlg.CreateNode();
    }
}

#endif

// Helper: Parse command-line options to determine generation type
std::pair<size_t, bool> App::ParseGenerationType(wxCmdLineParser& parser)
{
    // Map option names to their corresponding generation type values
    constexpr frozen::map<std::string_view, size_t, 13> gen_options = {
        { "gen_cpp", std::to_underlying(GenLang::cplusplus) },
        { "gen_python", std::to_underlying(GenLang::python) },
        { "gen_fortran", std::to_underlying(GenLang::fortran) },
        { "gen_go", std::to_underlying(GenLang::go) },
        { "gen_julia", std::to_underlying(GenLang::julia) },
        { "gen_luajit", std::to_underlying(GenLang::luajit) },
        { "gen_ruby", std::to_underlying(GenLang::ruby) },
        { "gen_rust", std::to_underlying(GenLang::rust) },
        { "gen_typescript", std::to_underlying(GenLang::typescript) },
        { "gen_xrc", std::to_underlying(GenLang::xrc) },
        { "gen_all", std::to_underlying(GenLang::cplusplus | GenLang::python | GenLang::ruby) },
        { "gen_quick", std::to_underlying(GenLang::python | GenLang::ruby) },
        { "gen_coverage",
          std::to_underlying(GenLang::cplusplus | GenLang::python | GenLang::ruby |
                             GenLang::fortran | GenLang::go | GenLang::julia | GenLang::luajit |
                             GenLang::rust | GenLang::typescript | GenLang::xrc) },
    };

    size_t generate_type = std::to_underlying(GenLang::none);
    bool test_only = false;

    // Check gen_* options (mutually exclusive)
    for (const auto& [option_name, option_type]: gen_options)
    {
        if (parser.FoundSwitch(wxString(option_name)) == wxCMD_SWITCH_ON)
        {
            generate_type = option_type;

            if (option_name == "gen_coverage")
            {
                wxGetApp().m_is_coverage_testing = true;
            }
            break;  // Only first match counts for gen_* options
        }
    }

    // Check test_* options (can be combined)
    // REVIEW: [Randalphwa - 02-21-2026] Do we still need these?
    constexpr frozen::map<std::string_view, GenLang, 4> test_options = {
        { "test_cpp", GenLang::cplusplus },
        { "test_python", GenLang::python },
        { "test_ruby", GenLang::ruby },
        { "test_xrc", GenLang::xrc },
    };

    for (const auto& [option_name, option_type]: test_options)
    {
        wxString option_filename;
        if (parser.Found(wxString(option_name), &option_filename))
        {
            generate_type |= std::to_underlying(option_type);
            test_only = true;

            // Store the filename from the option value (first one wins)
            if (!option_filename.empty() && s_filename.empty())
            {
                s_filename = option_filename;
                ProcessFilename(s_filename);
            }
        }
    }

    return std::make_pair(generate_type, test_only);
}

// Helper: Find project file if filename is empty
[[nodiscard]] bool App::FindProjectFile(wxString& filename)
{
    wxDir dir;
    dir.Open("./");
    if (!dir.GetFirst(&filename, "*" + std::string(PROJECT_FILE_EXTENSION), wxDIR_FILES))
    {
        wxMessageBox("No project file found in current directory. Filenane is required if "
                     "switch is used.",
                     "Command-line Switch Error", wxOK | wxICON_ERROR);
        return false;
    }
    return true;
}

// Helper: Load or import the project file
bool App::LoadProjectFile(const wxue::string& filename, size_t generate_type,
                          bool& is_project_loaded, bool force_import)
{
    if (force_import)
    {
        is_project_loaded = Project.ImportProject(filename, false);
    }
    else if (!filename.extension().is_sameas(PROJECT_FILE_EXTENSION, wxue::CASE::either) &&
             !filename.extension().is_sameas(PROJECT_LEGACY_FILE_EXTENSION, wxue::CASE::either))
    {
        is_project_loaded = Project.ImportProject(filename, generate_type == 0);
    }
    else
    {
        is_project_loaded = Project.LoadProject(filename, generate_type == 0);
    }
    return is_project_loaded;
}

// Helper: Log results for each language generation
void App::LogGenerationResults(GenResults& results, std::vector<std::string>& class_list,
                               bool test_only, std::string_view language_type)
{
    if (!results.GetUpdatedFiles().empty() || !results.GetCreatedFiles().empty() ||
        !class_list.empty())
    {
        if (test_only)
        {
            for (auto& iter: class_list)
            {
                wxue::string& log_msg = wxGetApp().get_CmdLineLog().emplace_back();
                log_msg << "Needs updating: " << iter;
            }
        }
        else
        {
            for (auto& iter: results.GetUpdatedFiles())
            {
                wxue::string& log_msg = wxGetApp().get_CmdLineLog().emplace_back();
                log_msg << "Updated: " << iter;
            }
            for (auto& iter: results.GetCreatedFiles())
            {
                wxue::string& log_msg = wxGetApp().get_CmdLineLog().emplace_back();
                log_msg << "Created: " << iter;
            }
        }
    }
    else
    {
        wxue::string& log_msg = wxGetApp().get_CmdLineLog().emplace_back();
        log_msg << "All generated " << language_type << " files are current";
    }

    for (auto& iter: results.GetMsgs())
    {
        if (iter.contains("Elapsed time"))
        {
            continue;
        }
        wxue::string& log_msg = wxGetApp().get_CmdLineLog().emplace_back();
        log_msg << iter;
    }
}

// Helper: Generate code for all requested languages
size_t App::GenerateAllLanguages(size_t generate_type, bool test_only, GenResults& results,
                                 std::vector<std::string>& class_list)
{
    // If a form filter is specified, find that form
    Node* form_node = nullptr;
    const std::string& form_filter = wxGetApp().get_FormFilter();
    if (!form_filter.empty())
    {
        form_node = FindNodeByClassName(Project.get_ProjectNode(), form_filter);
        if (!form_node)
        {
            wxue::string& log_msg = wxGetApp().get_CmdLineLog().emplace_back();
            log_msg << "Error: Form '" << form_filter << "' not found in project";
            return 0;
        }
    }

    size_t total_errors = 0;

    auto GenCode = [&](GenLang language)
    {
        if (generate_type & std::to_underlying(language))
        {
            // The FFI writers consume the kwxgen descriptor; load it once here so a missing or
            // stale descriptor is reported before any FFI file is written.
            if (IsFfiUiLanguage(language))
            {
                std::ignore = GetFfiUiDescriptor(language);
            }

            results.Clear();
            class_list.clear();

            // Use filtered form if specified, otherwise entire project
            if (form_node)
            {
                results.SetNodes(form_node);
            }
            else
            {
                results.SetNodes(Project.get_ProjectNode());
            }
            results.SetLanguages(language);

            if (test_only)
            {
                // Use compare_only mode to check what needs updating
                results.SetMode(GenResults::Mode::compare_only);
                std::ignore = results.Generate();

                // Extract filenames from diffs for logging
                for (const auto& diff: results.GetFileDiffs())
                {
                    class_list.emplace_back(diff.filename);
                }
            }
            else
            {
                results.SetMode(GenResults::Mode::generate_and_write);
                std::ignore = results.Generate();
            }

            LogGenerationResults(results, class_list, test_only, GenLangToString(language));
            total_errors += results.GetErrorCount();
        }
    };

    GenCode(GenLang::cplusplus);
    GenCode(GenLang::python);

    GenCode(GenLang::fortran);
    GenCode(GenLang::go);
    GenCode(GenLang::julia);
    GenCode(GenLang::luajit);
    GenCode(GenLang::rust);
    GenCode(GenLang::typescript);
    GenCode(GenLang::ruby);

    GenCode(GenLang::xrc);

    return total_errors;
}

int App::Generate(wxCmdLineParser& parser, bool& is_project_loaded)
{
    const bool is_import = (parser.FoundSwitch("import") == wxCMD_SWITCH_ON);

    // An explicit --import must never fall through to the GUI, even without a filename.
    if (!is_import && !parser.GetParamCount() && !parser.GetArguments().size())
    {
        return cmd_no_params;
    }

    if (parser.FoundSwitch("verbose") == wxCMD_SWITCH_ON)
    {
        m_is_verbose_codegen = true;
    }

    // Check for form filter option
    wxString form_filter;
    if (parser.Found("form", &form_filter))
    {
        m_form_filter = form_filter.ToStdString();
    }

    auto [generate_type, test_only] = ParseGenerationType(parser);

    const wxString& filename_str = GetCommandLineFilename(parser);

    if (generate_type == 0)
    {
        if (is_import)
        {
            if (filename_str.empty())
            {
                return 1;
            }

            wxue::string import_filename = filename_str;
            import_filename.make_absolute();
            wxue::string import_log_file = import_filename;
            import_log_file.replace_extension(".log");
            m_cmdline_log.clear();
            m_cmdline_log.emplace_back(
                wxue::string("Error: --import requires a --gen_* switch (for example --gen_cpp)"));
            std::ignore = m_cmdline_log.WriteFile(import_log_file);
            return 1;
        }

        if (filename_str.empty())
        {
            return cmd_no_params;
        }

        return cmd_project_file_only;
    }

    // If we get here then we were asked to generate at least one language type

    if (is_import && filename_str.empty())
    {
        return 1;
    }

    wxue::string filename = filename_str;
    filename.make_absolute();
    wxue::string log_file = filename;
    log_file.replace_extension(".log");

    if (!filename.file_exists())
    {
        m_cmdline_log.clear();
        m_cmdline_log.emplace_back(wxue::string("Unable to find project file: ") += filename);
        std::ignore = m_cmdline_log.WriteFile(log_file);
        return 1;
    }

    m_is_generating = true;
    // Command-line generation is unattended -- suppress any dialog via the modal guard.
    Project.set_UiAllowed(false);
    GenResults results;
#if defined(INTERNAL_TESTING)
    results.StartClock();
#endif

    LoadProjectFile(filename, generate_type, is_project_loaded, is_import);
    if (!is_project_loaded)
    {
        m_cmdline_log.clear();
        if (is_import)
        {
            m_cmdline_log.emplace_back(wxue::string("Unable to import project file: ") += filename);
        }
        else
        {
            m_cmdline_log.emplace_back(wxue::string("Unable to load project file: ") += filename);
        }
        std::ignore = m_cmdline_log.WriteFile(log_file);
        return cmd_gen_project_not_loaded;
    }

    m_cmdline_log.clear();
    std::vector<std::string> class_list;
    const std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

    ResetFfiUiDiagnosticCount();
    const size_t generate_errors =
        GenerateAllLanguages(generate_type, test_only, results, class_list);

    if (const std::size_t ffi_diagnostics = GetFfiUiDiagnosticCount(); ffi_diagnostics > 0)
    {
        wxue::string& summary = m_cmdline_log.emplace_back();
        summary << "FFI: " << ffi_diagnostics << " unsupported role call(s) reported";
    }

    wxue::string& log_msg = m_cmdline_log.emplace_back();
    const std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
    const size_t total_elapsed_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    log_msg << "Total elapsed time: " << total_elapsed_time << " milliseconds";
    std::ignore = m_cmdline_log.WriteFile(log_file);

    // A file that could not be written is a failure for non-interactive generation, so the
    // caller/CI script can detect it from the exit code.
    return generate_errors > 0 ? cmd_gen_write_error : cmd_gen_success;
}
