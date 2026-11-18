/////////////////////////////////////////////////////////////////////////////
// Purpose:   Load a kwxgen FFI UI descriptor and resolve role callables
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/*
    The descriptor is generated output from `kwxgen emit-ui` (see
    kwxFFI/kwxgen/docs/emit-ui.md). It is read from src/generate/ffi_ui/<lang>.json and cached
    for the lifetime of the process. Descriptor discovery order:

        1. WXUE_FFI_UI_DIR (local testing override)
        2. <exe dir>/../../src/generate/ffi_ui   (bin/<config>/ under the repository root)
        3. <exe dir>/../../../src/generate/ffi_ui (build/bin/<config>/)
        4. <exe dir>/src/generate/ffi_ui
        5. <cwd>/src/generate/ffi_ui

    Loading is best-effort: a missing, invalid or stale descriptor yields nullptr plus one
    diagnostic. It never throws, never writes a file, and never blocks generation.
*/

#include <array>
#include <memory>
#include <set>
#include <tuple>
#include <utility>

#include <wx/file.h>      // wxFile
#include <wx/filefn.h>    // wxFileExists
#include <wx/filename.h>  // wxFileName
#include <wx/log.h>       // wxLogStderr
#include <wx/stdpaths.h>  // wxStandardPaths
#include <wx/utils.h>     // wxGetCwd, wxGetEnv

#include <glaze/json/generic.hpp>

#include "ffi_ui.h"

#include "mainapp.h"  // App -- is_Generating(), get_CmdLineLog()

static std::map<GenLang, std::unique_ptr<FfiUiDescriptor>> s_descriptors;
static std::set<GenLang> s_attempted_languages;
static std::size_t s_diagnostic_count = 0;

// Emits one diagnostic to stderr and, when a command-line log is being written, records the
// same message there. Nothing is written to disk by the descriptor loader itself.
static void ReportDiagnostic(const wxString& message)
{
    wxLogStderr(nullptr).LogText(message);

    if (wxGetApp().is_Generating())
    {
        wxGetApp().get_CmdLineLog().emplace_back(message.utf8_string());
    }
}

static std::string_view GetLanguageFileStem(GenLang language)
{
    switch (language)
    {
        case GenLang::fortran:
            return "fortran";
        case GenLang::go:
            return "go";
        case GenLang::julia:
            return "julia";
        case GenLang::luajit:
            return "lua";
        case GenLang::rust:
            return "rust";
        case GenLang::typescript:
            return "typescript";
        default:
            return {};
    }
}

// Returns the full directory (with trailing separator) for the descriptor set.
static wxString FindDescriptorDir()
{
    wxString override_dir;
    if (wxGetEnv("WXUE_FFI_UI_DIR", &override_dir) && !override_dir.empty())
    {
        wxFileName dir;
        dir.AssignDir(override_dir);
        dir.MakeAbsolute();
        dir.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);
        return dir.GetPathWithSep();
    }

    const wxString exe_dir = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();

    constexpr auto normalize = [](const wxString& path)
    {
        wxFileName dir;
        dir.AssignDir(path);
        dir.MakeAbsolute();
        dir.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);
        return dir.GetPathWithSep();
    };

    const std::vector<wxString> candidates = { exe_dir + "/../../src/generate/ffi_ui",
                                               exe_dir + "/../../../src/generate/ffi_ui",
                                               exe_dir + "/src/generate/ffi_ui",
                                               wxGetCwd() + "/src/generate/ffi_ui" };

    for (const auto& candidate: candidates)
    {
        const wxString dir_path = normalize(candidate);
        if (wxFileExists(dir_path + "manifest.json"))
        {
            return dir_path;
        }
    }

    // No manifest anywhere: report against the layout the build normally produces.
    return normalize(candidates.front());
}

// Reads a UTF-8 text file. Returns false when the file cannot be opened or read.
static bool ReadTextFile(const wxString& file_path, std::string& text)
{
    if (!wxFileExists(file_path))
    {
        return false;
    }

    // wxFile::Open() emits a system-error log record when it fails; the caller reports the
    // failure once with context, so suppress the duplicate here.
    const wxLogNull suppress_log;

    wxFile file;
    if (!file.Open(file_path, wxFile::read))
    {
        return false;
    }

    wxString content;
    if (!file.ReadAll(&content))
    {
        return false;
    }

    text = content.utf8_string();
    return true;
}

static std::string GetString(const glz::generic& object, std::string_view key_name)
{
    if (!object.is_object() || !object.contains(key_name))
    {
        return {};
    }

    const glz::generic& value = object[key_name];
    if (!value.is_string())
    {
        return {};
    }
    return value.get<std::string>();
}

static void ParseParams(const glz::generic& value, std::vector<FfiUiParam>& params)
{
    if (!value.is_array())
    {
        return;
    }

    for (const auto& entry: value.get_array())
    {
        FfiUiParam param;
        if (entry.is_string())
        {
            param.name = entry.get<std::string>();
        }
        else if (entry.is_object())
        {
            param.name = GetString(entry, "name");
            param.type = GetString(entry, "type");
        }
        params.emplace_back(std::move(param));
    }
}

// Parses one callable. Returns false when the value is not a JSON object.
static bool ParseCallable(const glz::generic& value, FfiUiCall& callable)
{
    if (!value.is_object())
    {
        return false;
    }

    callable.symbol = GetString(value, "symbol");
    callable.idiom = GetString(value, "idiom");
    callable.owner = GetString(value, "owner");
    callable.returns = GetString(value, "returns");
    callable.reason = GetString(value, "reason");

    if (value.contains("params"))
    {
        ParseParams(value["params"], callable.params);
    }

    if (value.contains("sentinels") && value["sentinels"].is_object())
    {
        for (const auto& [sentinel_name, sentinel_value]: value["sentinels"].get_object())
        {
            if (!sentinel_value.is_string())
            {
                continue;
            }
            callable.sentinels.emplace(sentinel_name, sentinel_value.get<std::string>());
        }
    }

    return true;
}

static void ParseCallableMap(const glz::generic& value, FfiUiCallMap& calls,
                             std::string_view prefix = {})
{
    if (!value.is_object())
    {
        return;
    }

    for (const auto& [key_name, callable_value]: value.get_object())
    {
        FfiUiCall callable;
        if (ParseCallable(callable_value, callable))
        {
            std::string full_name(prefix);
            full_name += key_name;
            calls.emplace(std::move(full_name), std::move(callable));
        }
    }
}

static void ParseStringMap(const glz::generic& value, FfiUiStringMap& strings)
{
    if (!value.is_object())
    {
        return;
    }

    for (const auto& [key_name, entry]: value.get_object())
    {
        if (entry.is_string())
        {
            strings.emplace(key_name, entry.get<std::string>());
        }
    }
}

static bool ParseDescriptor(const glz::generic& document, FfiUiDescriptor& descriptor)
{
    if (!document.is_object())
    {
        return false;
    }

    if (document.contains("schema") && document["schema"].is_number())
    {
        descriptor.schema = document["schema"].as<int>();
    }
    descriptor.lang = GetString(document, "lang");

    if (document.contains("generated_by") && document["generated_by"].is_object())
    {
        const glz::generic& generated_by = document["generated_by"];
        descriptor.tool = GetString(generated_by, "tool");
        descriptor.version = GetString(generated_by, "version");
        descriptor.headers_hash = GetString(generated_by, "headers_hash");
        descriptor.profile = GetString(generated_by, "profile");
    }

    if (document.contains("naming"))
    {
        ParseStringMap(document["naming"], descriptor.naming);
    }

    if (document.contains("classes") && document["classes"].is_object())
    {
        for (const auto& [class_name, class_value]: document["classes"].get_object())
        {
            if (!class_value.is_object())
            {
                continue;
            }

            FfiUiClass wx_class_info;
            wx_class_info.type = GetString(class_value, "type");
            wx_class_info.idiom = GetString(class_value, "idiom");

            if (class_value.contains("ctor"))
            {
                std::ignore = ParseCallable(class_value["ctor"], wx_class_info.ctor);
            }
            if (class_value.contains("roles"))
            {
                ParseCallableMap(class_value["roles"], wx_class_info.roles);
            }
            if (class_value.contains("constants"))
            {
                ParseStringMap(class_value["constants"], wx_class_info.constants);
            }

            descriptor.classes.emplace(class_name, std::move(wx_class_info));
        }
    }

    if (document.contains("sizers") && document["sizers"].is_object())
    {
        const glz::generic& sizers = document["sizers"];
        if (sizers.contains("wxBoxSizer"))
        {
            ParseCallableMap(sizers["wxBoxSizer"], descriptor.sizers, "wxBoxSizer.");
        }
        if (sizers.contains("flags"))
        {
            ParseCallableMap(sizers["flags"], descriptor.sizers, "flags.");
        }
    }

    if (document.contains("events"))
    {
        ParseCallableMap(document["events"], descriptor.events);
    }
    if (document.contains("bitmap"))
    {
        ParseCallableMap(document["bitmap"], descriptor.bitmap);
    }
    if (document.contains("font"))
    {
        ParseCallableMap(document["font"], descriptor.font);
    }
    if (document.contains("std_buttons"))
    {
        ParseCallableMap(document["std_buttons"], descriptor.std_buttons);
    }

    if (document.contains("app") && document["app"].is_object())
    {
        const glz::generic& app_block = document["app"];
        descriptor.app_entry = GetString(app_block, "entry");
        descriptor.app_entry_signature = GetString(app_block, "entry_signature");
        if (app_block.contains("emit_entry") && app_block["emit_entry"].is_boolean())
        {
            descriptor.app_emit_entry = app_block["emit_entry"].get<bool>();
        }
        if (app_block.contains("set_top_window"))
        {
            std::ignore = ParseCallable(app_block["set_top_window"], descriptor.set_top_window);
        }
    }

    if (document.contains("unsupported") && document["unsupported"].is_array())
    {
        for (const auto& entry: document["unsupported"].get_array())
        {
            if (!entry.is_object())
            {
                continue;
            }

            FfiUiUnsupported unsupported;
            unsupported.wx_class = GetString(entry, "class");
            unsupported.role = GetString(entry, "role");
            unsupported.reason = GetString(entry, "reason");
            descriptor.unsupported.emplace_back(std::move(unsupported));
        }
    }

    return !descriptor.classes.empty();
}

// Fails when the descriptor was built from a different header surface than manifest.json
// records -- i.e. kwxFFI changed and .tts/gen_ffi_ui.sh was not re-run.
static bool VerifyAgainstManifest(const wxString& dir_path, const FfiUiDescriptor& descriptor)
{
    const wxString manifest_path = dir_path + "manifest.json";

    std::string manifest_text;
    if (!ReadTextFile(manifest_path, manifest_text))
    {
        ReportDiagnostic(wxString("FFI UI descriptor manifest is missing: ") + manifest_path);
        return false;
    }

    glz::generic document;
    if (glz::read_json(document, manifest_text) || !document.is_object() ||
        !document.contains("descriptors") || !document["descriptors"].is_object())
    {
        ReportDiagnostic(wxString("FFI UI descriptor manifest is not valid JSON: ") +
                         manifest_path);
        return false;
    }

    const glz::generic& entries = document["descriptors"];
    if (!entries.contains(descriptor.lang))
    {
        ReportDiagnostic(wxString::Format("FFI UI descriptor manifest has no entry for '%s': %s",
                                          wxString(descriptor.lang.c_str()), manifest_path));
        return false;
    }

    const std::string manifest_hash = GetString(entries[descriptor.lang], "headers_hash");
    if (manifest_hash.empty() || manifest_hash != descriptor.headers_hash)
    {
        ReportDiagnostic(wxString::Format(
            "FFI UI descriptor '%s.json' was built from headers %s but manifest.json records "
            "%s -- re-run .tts/gen_ffi_ui.sh",
            wxString(descriptor.lang.c_str()), wxString(descriptor.headers_hash.c_str()),
            wxString(manifest_hash.c_str())));
        return false;
    }

    return true;
}

static std::unique_ptr<FfiUiDescriptor> LoadDescriptor(GenLang language)
{
    const std::string_view stem = GetLanguageFileStem(language);
    if (stem.empty())
    {
        return nullptr;
    }

    const wxString dir_path = FindDescriptorDir();
    const wxString file_path = dir_path + wxString(stem) + ".json";

    std::string descriptor_text;
    if (!ReadTextFile(file_path, descriptor_text))
    {
        ReportDiagnostic(wxString("FFI UI descriptor is missing or unreadable: ") + file_path);
        return nullptr;
    }

    glz::generic document;
    if (glz::read_json(document, descriptor_text))
    {
        ReportDiagnostic(wxString("FFI UI descriptor is not valid JSON: ") + file_path);
        return nullptr;
    }

    std::unique_ptr<FfiUiDescriptor> descriptor = std::make_unique<FfiUiDescriptor>();
    if (!ParseDescriptor(document, *descriptor))
    {
        ReportDiagnostic(wxString("FFI UI descriptor has no classes: ") + file_path);
        return nullptr;
    }

    if (descriptor->schema != 1)
    {
        ReportDiagnostic(wxString::Format("FFI UI descriptor %s has schema %d, expected 1",
                                          file_path, descriptor->schema));
        return nullptr;
    }

    if (std::string_view(descriptor->lang) != stem)
    {
        ReportDiagnostic(
            wxString::Format("FFI UI descriptor %s declares language '%s', expected '%s'",
                             file_path, wxString(descriptor->lang.c_str()), wxString(stem.data())));
        return nullptr;
    }

    if (!VerifyAgainstManifest(dir_path, *descriptor))
    {
        return nullptr;
    }

    return descriptor;
}

bool IsFfiUiLanguage(GenLang language)
{
    return !GetLanguageFileStem(language).empty();
}

std::string GetFfiUiDirPath()
{
    return FindDescriptorDir().utf8_string();
}

std::string_view GetFfiClassName(std::string_view class_name)
{
    // A separator is a wxMenuItem whose kind wxMenuItemBase() takes from the id, so
    // `wxMenu::Append(wxID_SEPARATOR)` sets wxITEM_SEPARATOR (wxWidgets src/common/menucmn.cpp) --
    // exactly what the `wxMenu.append_by_id` role emits. A submenu node is a wxMenu handed to the
    // parent through AppendSubMenu. Neither meta name is a wx class, so a descriptor role lookup
    // on it can only fail.
    static constexpr std::array<std::pair<std::string_view, std::string_view>, 2> s_meta_classes { {
        { "separator", "wxMenuItem" },
        { "submenu", "wxMenu" },
    } };

    for (const auto& [meta_name, ffi_class_name]: s_meta_classes)
    {
        if (meta_name == class_name)
        {
            return ffi_class_name;
        }
    }
    return class_name;
}

// The C1 contract specifies a nullable probe, so the pointer return is intentional.
// NOLINTNEXTLINE(modernize-use-expected)
const FfiUiDescriptor* GetFfiUiDescriptor(GenLang language)
{
    if (!IsFfiUiLanguage(language))
    {
        return nullptr;
    }

    if (const auto cached = s_descriptors.find(language); cached != s_descriptors.end())
    {
        return cached->second.get();
    }

    // A failed load is reported once; later calls stay silent so a per-node code path cannot
    // flood the log.
    if (s_attempted_languages.contains(language))
    {
        return nullptr;
    }
    s_attempted_languages.insert(language);

    std::unique_ptr<FfiUiDescriptor> descriptor = LoadDescriptor(language);
    if (!descriptor)
    {
        return nullptr;
    }

    const FfiUiDescriptor* result = descriptor.get();
    s_descriptors.emplace(language, std::move(descriptor));
    return result;
}

static bool LookupCall(const FfiUiCallMap& calls, std::string_view key_name, FfiUiCall& out)
{
    const FfiUiCallMap::const_iterator iter = calls.find(key_name);
    if (iter == calls.end())
    {
        return false;
    }

    out = iter->second;
    return out.IsUsable();
}

bool ResolveRole(const FfiUiDescriptor& descriptor, std::string_view wx_class,
                 std::string_view role, FfiUiCall& out)
{
    // 1. The class itself owns the answer; a listed class does not fall through.
    if (const auto class_iter = descriptor.classes.find(wx_class);
        class_iter != descriptor.classes.end())
    {
        const FfiUiClass& wx_class_info = class_iter->second;
        if (role == "ctor")
        {
            out = wx_class_info.ctor;
            return out.IsUsable();
        }

        if (const auto role_iter = wx_class_info.roles.find(role);
            role_iter != wx_class_info.roles.end())
        {
            out = role_iter->second;
            return out.IsUsable();
        }
    }

    // 2. Box-sizer operations for sizer classes.
    const bool is_sizer_class = wx_class == "wxBoxSizer" || wx_class.ends_with("Sizer");
    if (is_sizer_class &&
        (role == "ctor" || role == "add_window" || role == "add_sizer" || role == "add_spacer"))
    {
        const std::string key_name = std::string("wxBoxSizer.") + std::string(role);
        return LookupCall(descriptor.sizers, key_name, out);
    }

    // 3. Sizer flags.
    if (role == "proportion" || role == "expand" || role == "border" || role == "left" ||
        role == "right" || role == "top" || role == "bottom" || role == "align_h" ||
        role == "align_v")
    {
        const std::string key_name = std::string("flags.") + std::string(role);
        return LookupCall(descriptor.sizers, key_name, out);
    }

    // 4. Events.
    if (role == "bind" || role == "bind_id")
    {
        return LookupCall(descriptor.events, role, out);
    }

    // 5. Bitmap family.
    if (role == "from_base64" || role == "get_bitmap")
    {
        return LookupCall(descriptor.bitmap, role, out);
    }

    // 6. Standard-button sizer operations.
    if (role == "add_button" || role == "realize")
    {
        return LookupCall(descriptor.std_buttons, role, out);
    }

    // 7. `create` belongs to the font or the standard-button sizer.
    if (role == "create")
    {
        if (wx_class == "wxStdDialogButtonSizer")
        {
            return LookupCall(descriptor.std_buttons, "create", out);
        }
        if (wx_class == "wxFont")
        {
            return LookupCall(descriptor.font, "create", out);
        }
        if (LookupCall(descriptor.font, "create", out))
        {
            return true;
        }
        return LookupCall(descriptor.std_buttons, "create", out);
    }

    // 8. `delete` belongs to the font or the bitmap.
    if (role == "delete")
    {
        if (wx_class == "wxFont")
        {
            return LookupCall(descriptor.font, "delete", out);
        }
        if (wx_class == "wxBitmap")
        {
            return LookupCall(descriptor.bitmap, "delete", out);
        }
        return false;
    }

    // 9. Generic sizer fallback: any widget can be handed to a box sizer.
    if (role == "add_window" || role == "add_sizer" || role == "add_spacer")
    {
        const std::string key_name = std::string("wxBoxSizer.") + std::string(role);
        return LookupCall(descriptor.sizers, key_name, out);
    }

    return false;
}

void ReportUnsupportedRole(const FfiUiDescriptor& descriptor, std::string_view wx_class,
                           std::string_view role)
{
    wxString message = wxString::Format(
        "FFI %s: %s.%s is not supported", wxString(descriptor.lang.c_str()),
        wxString(wx_class.data(), wx_class.size()), wxString(role.data(), role.size()));

    for (const auto& entry: descriptor.unsupported)
    {
        if (entry.wx_class == wx_class && entry.role == role)
        {
            message += wxString::Format(" (%s)", wxString(entry.reason.c_str()));
            break;
        }
    }

    ++s_diagnostic_count;
    ReportDiagnostic(message);
}

std::size_t GetFfiUiDiagnosticCount()
{
    return s_diagnostic_count;
}

void ResetFfiUiDiagnosticCount()
{
    s_diagnostic_count = 0;
}
