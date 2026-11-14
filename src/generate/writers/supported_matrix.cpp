/////////////////////////////////////////////////////////////////////////////
// Purpose:   Write the full kwx capability matrix (.private/kwxSupported.md)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/*
    The matrix is generated output: it joins three checked-in sources and must never be
    hand-edited (Cross-Phase Constraints).

        1. the compiled generator library -- every generator declaration, so classes that are
           not used by quick.wxui still appear;
        2. the six schema-1 descriptors in src/generate/ffi_ui/ -- the class/role surface each
           language claims;
        3. src/generate/ffi_ui/coverage.json -- the C4 states derived by scripts/kwx_coverage.ps1.

    coverage.json is authoritative wherever it has a row; the descriptors resolve the remaining
    classes/roles exactly the way `kwxgen check-ui` and ResolveRole() do. Nothing here invents a
    state: a pair with no observation stays `unknown` unless a descriptor marks it unsupported
    (`absent`) or has no callable for a listed class (`descriptor-missing`).

    Determinism: no wall-clock value is written (the coverage date comes from coverage.json),
    and every collection is ordered by std::map / std::set.
*/

#include <algorithm>
#include <array>
#include <cstddef>
#include <format>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <wx/filefn.h>    // wxFileExists
#include <wx/log.h>       // wxLogNull, wxLogStderr
#include <wx/wfstream.h>  // wxFFile

#include <glaze/json/generic.hpp>

#include "supported_matrix.h"

#include "ffi_ui.h"        // FfiUiDescriptor, GetFfiUiDescriptor, ResolveRole
#include "gen_enums.h"     // GenLang, map_GenNames
#include "node_creator.h"  // NodeCreation
#include "node_decl.h"     // NodeDeclaration

// Column order matches C4, coverage.json and the descriptor file stems.
static constexpr std::array<std::string_view, 6> LANGUAGES = { "fortran", "go",   "julia",
                                                               "lua",     "rust", "typescript" };

static constexpr std::array<GenLang, 6> LANGUAGE_IDS = { GenLang::fortran, GenLang::go,
                                                         GenLang::julia,   GenLang::luajit,
                                                         GenLang::rust,    GenLang::typescript };

// C4 states, highest precedence first.
static constexpr std::array<std::string_view, 5> STATES = { "verified", "absent",
                                                            "descriptor-missing",
                                                            "exercised-unverified", "unknown" };

// One (class, role) row read from coverage.json: language name -> C4 state.
struct CoverageRow
{
    std::map<std::string, std::string, std::less<>> states;
};

// class -> role -> row
using CoverageTable =
    std::map<std::string, std::map<std::string, CoverageRow, std::less<>>, std::less<>>;
using ClassSet = std::set<std::string, std::less<>>;
using RoleSet = std::set<std::string, std::less<>>;
using ClassRoles = std::map<std::string, RoleSet, std::less<>>;

// One output line of the pair-level table. An empty `states` entry means the class has no FFI
// path and the cell is written as `---`.
struct MatrixRow
{
    std::string wx_class;
    std::string role;
    std::array<std::string, 6> states;
    int verified_count {};
};

// Returns the string value of `key_name`, or an empty string when it is absent or not a string.
static std::string GetJsonString(const glz::generic& object, std::string_view key_name)
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

// Reads a UTF-8 text file. Returns false when the file cannot be opened or read.
static bool ReadTextFile(const std::string& file_path, std::string& text)
{
    if (!wxFileExists(wxString::FromUTF8(file_path)))
    {
        return false;
    }

    const wxLogNull suppress_log;

    wxFFile file(wxString::FromUTF8(file_path), "rb");
    if (!file.IsOpened())
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

// Metadata carried beside the pair table, read from coverage.json.
struct CoverageMeta
{
    std::string generated;
    std::string kwxgen_version;
    std::string headers_hash;
};

// Loads coverage.json. Returns false only when the report is missing or unparseable, because a
// matrix built without it would silently claim `unknown` for pairs the Phase 3 join resolved.
static bool LoadCoverage(const std::string& file_path, CoverageTable& table, CoverageMeta& meta)
{
    std::string text;
    if (!ReadTextFile(file_path, text))
    {
        wxLogStderr(nullptr).LogText(wxString("--gen_supported: cannot read coverage report: ") +
                                     wxString::FromUTF8(file_path));
        return false;
    }

    glz::generic document;
    if (glz::read_json(document, text) || !document.is_object() || !document.contains("rows") ||
        !document["rows"].is_array())
    {
        wxLogStderr(nullptr).LogText(
            wxString("--gen_supported: coverage report is not valid JSON: ") +
            wxString::FromUTF8(file_path));
        return false;
    }

    meta.generated = GetJsonString(document, "generated");
    if (document.contains("kwxgen") && document["kwxgen"].is_object())
    {
        const glz::generic& kwxgen = document["kwxgen"];
        meta.kwxgen_version = GetJsonString(kwxgen, "version");
        meta.headers_hash = GetJsonString(kwxgen, "headers_hash");
    }

    for (const auto& row_value: document["rows"].get_array())
    {
        if (!row_value.is_object())
        {
            continue;
        }

        const std::string class_name = GetJsonString(row_value, "class");
        const std::string role_name = GetJsonString(row_value, "role");
        if (class_name.empty() || role_name.empty())
        {
            continue;
        }

        CoverageRow coverage_row;
        if (row_value.contains("cells") && row_value["cells"].is_object())
        {
            const glz::generic& cells = row_value["cells"];
            for (const std::string_view language: LANGUAGES)
            {
                if (!cells.contains(language) || !cells[language].is_object())
                {
                    continue;
                }

                const std::string state = GetJsonString(cells[language], "state");
                if (!state.empty())
                {
                    coverage_row.states.emplace(std::string(language), state);
                }
            }
        }

        table[class_name].emplace(role_name, std::move(coverage_row));
    }

    return true;
}

// Adds every generator declaration that is not an interface. Interfaces (validators, settings,
// foldering helpers) have NodeDeclarations but implement no generator.
static void AddGeneratorClasses(ClassSet& classes)
{
    const NodeDeclarationArray& declarations = NodeCreation.get_NodeDeclarationArray();

    for (const auto& name_entry: GenEnum::map_GenNames)
    {
        const size_t index = static_cast<size_t>(name_entry.first);
        if (index >= declarations.size())
        {
            continue;
        }

        const NodeDeclaration* declaration = declarations[index];
        if (declaration == nullptr || declaration->is_Type(GenEnum::type_interface))
        {
            continue;
        }

        classes.emplace(declaration->get_DeclName());
    }
}

// Adds the class/role surface one descriptor claims, including unsupported[] entries so a pair
// that no language can express still gets a row (its cell resolves to `absent`).
static void AddDescriptorClasses(const FfiUiDescriptor& descriptor, ClassSet& classes,
                                 ClassRoles& roles)
{
    for (const auto& class_entry: descriptor.classes)
    {
        classes.emplace(class_entry.first);

        RoleSet& class_roles = roles[class_entry.first];
        const FfiUiClass& wx_class = class_entry.second;
        if (!wx_class.ctor.symbol.empty() || !wx_class.ctor.idiom.empty())
        {
            class_roles.insert("ctor");
        }

        for (const auto& role_entry: wx_class.roles)
        {
            class_roles.insert(role_entry.first);
        }
    }

    for (const auto& entry: descriptor.unsupported)
    {
        if (entry.wx_class.empty() || entry.role.empty())
        {
            continue;
        }

        classes.emplace(entry.wx_class);
        roles[entry.wx_class].insert(entry.role);
    }
}

// Resolves a state for a pair coverage.json has no row for, using the same precedence and the
// same resolver the emitters use. Returns `unknown` when the descriptor itself is unavailable.
static std::string DeriveState(const FfiUiDescriptor* descriptor, std::string_view wx_class,
                               std::string_view role)
{
    if (descriptor == nullptr)
    {
        return "unknown";
    }

    for (const auto& entry: descriptor->unsupported)
    {
        if (entry.wx_class == wx_class && entry.role == role)
        {
            return "absent";
        }
    }

    FfiUiCall callable;
    if (ResolveRole(*descriptor, wx_class, role, callable))
    {
        return "unknown";
    }

    // ResolveRole() fills `callable` only when it found an entry; an unusable entry is `absent`,
    // a missing one is `descriptor-missing`.
    if (!callable.symbol.empty() || !callable.idiom.empty())
    {
        return "absent";
    }

    return "descriptor-missing";
}

static size_t GetStateIndex(std::string_view state)
{
    const std::array<std::string_view, STATES.size()>::const_iterator iter =
        std::ranges::find(STATES, state);
    return static_cast<size_t>(std::distance(STATES.begin(), iter));
}

bool WriteSupportedMatrix(const std::string& path)
{
    std::string coverage_path = GetFfiUiDirPath();
    coverage_path += "coverage.json";

    CoverageTable coverage;
    CoverageMeta meta;
    if (!LoadCoverage(coverage_path, coverage, meta))
    {
        return false;
    }

    ClassSet classes;
    ClassRoles roles;

    AddGeneratorClasses(classes);

    for (const GenLang language: LANGUAGE_IDS)
    {
        const FfiUiDescriptor* descriptor = GetFfiUiDescriptor(language);
        if (descriptor != nullptr)
        {
            AddDescriptorClasses(*descriptor, classes, roles);
        }
    }

    for (const auto& class_entry: coverage)
    {
        classes.emplace(class_entry.first);

        RoleSet& class_roles = roles[class_entry.first];
        for (const auto& role_entry: class_entry.second)
        {
            class_roles.insert(role_entry.first);
        }
    }

    std::vector<MatrixRow> rows;
    size_t no_ffi_classes = 0;

    for (const std::string& class_name: classes)
    {
        const ClassRoles::const_iterator roles_iter = roles.find(class_name);
        if (roles_iter == roles.end() || roles_iter->second.empty())
        {
            MatrixRow no_ffi_row;
            no_ffi_row.wx_class = class_name;
            no_ffi_row.role = "---";
            rows.emplace_back(std::move(no_ffi_row));
            ++no_ffi_classes;
            continue;
        }

        const CoverageTable::const_iterator class_iter = coverage.find(class_name);
        const bool has_coverage_class = class_iter != coverage.end();

        for (const std::string& role_name: roles_iter->second)
        {
            MatrixRow matrix_row;
            matrix_row.wx_class = class_name;
            matrix_row.role = role_name;

            const bool has_coverage_row =
                has_coverage_class && class_iter->second.contains(role_name);

            for (size_t index = 0; index < LANGUAGES.size(); ++index)
            {
                std::string state;
                if (has_coverage_row)
                {
                    const CoverageRow& coverage_row = class_iter->second.at(role_name);
                    const std::map<std::string, std::string, std::less<>>::const_iterator
                        state_iter = coverage_row.states.find(LANGUAGES[index]);
                    if (state_iter != coverage_row.states.end())
                    {
                        state = state_iter->second;
                    }
                }

                if (state.empty())
                {
                    state =
                        DeriveState(GetFfiUiDescriptor(LANGUAGE_IDS[index]), class_name, role_name);
                }

                if (state == "verified")
                {
                    ++matrix_row.verified_count;
                }
                matrix_row.states[index] = std::move(state);
            }

            rows.emplace_back(std::move(matrix_row));
        }
    }

    std::array<std::array<size_t, STATES.size()>, LANGUAGES.size()> counts {};
    for (const MatrixRow& matrix_row: rows)
    {
        for (size_t index = 0; index < matrix_row.states.size(); ++index)
        {
            if (matrix_row.states[index].empty())
            {
                continue;
            }

            const size_t state_index = GetStateIndex(matrix_row.states[index]);
            if (state_index < STATES.size())
            {
                ++counts[index][state_index];
            }
        }
    }

    std::string markdown;
    markdown.reserve(64U * 1024U);

    markdown += "# kwx supported matrix (full surface)\n\n";
    markdown += "Generated by `wxUiEditor --gen_supported`; do not edit by hand.\n\n";

    markdown += "| field | value |\n|---|---|\n";
    markdown += "| sources | generator library + `src/generate/ffi_ui/<lang>.json` + "
                "`src/generate/ffi_ui/coverage.json` |\n";
    if (!meta.generated.empty())
    {
        markdown += std::format("| coverage date | {} |\n", meta.generated);
    }
    if (!meta.kwxgen_version.empty())
    {
        markdown += std::format("| kwxgen | {} |\n", meta.kwxgen_version);
    }
    if (!meta.headers_hash.empty())
    {
        markdown += std::format("| headers_hash | {} |\n", meta.headers_hash);
    }
    markdown += std::format("| classes (generator + FFI) | {} |\n", classes.size());
    markdown += std::format("| `(class, role)` pairs | {} |\n", rows.size() - no_ffi_classes);
    markdown += std::format("| classes with no FFI path (`---`) | {} |\n", no_ffi_classes);

    markdown += "| states |";
    for (const std::string_view state: STATES)
    {
        markdown += std::format(" `{}`", state);
    }
    markdown += " |\n\n";

    markdown += "Precedence (C4): `verified` -> `absent` -> `descriptor-missing` -> "
                "`exercised-unverified` -> `unknown`.\n";
    markdown += "Rows are `(class, role)` pairs. A class with no FFI path in any language is "
                "listed once with role `---` and\n";
    markdown += "`---` in every language cell; that is the only place `---` appears.\n\n";

    markdown += "## Counts per language\n\n| language |";
    for (const std::string_view state: STATES)
    {
        markdown += std::format(" {} |", state);
    }
    markdown += " total |\n|---|---|---|---|---|---|---|\n";

    for (size_t index = 0; index < LANGUAGES.size(); ++index)
    {
        markdown += std::format("| {} |", LANGUAGES[index]);
        size_t total = 0;
        for (const size_t count: counts[index])
        {
            markdown += std::format(" {} |", count);
            total += count;
        }
        markdown += std::format(" {} |\n", total);
    }

    markdown += "\n## Ratchet set (verified 6/6)\n\n";
    std::vector<std::string> ratchet_pairs;
    for (const MatrixRow& matrix_row: rows)
    {
        if (matrix_row.verified_count == static_cast<int>(LANGUAGES.size()))
        {
            ratchet_pairs.emplace_back(matrix_row.wx_class + "." + matrix_row.role);
        }
    }

    if (ratchet_pairs.empty())
    {
        markdown += "None. No `(class, role)` pair is `verified` in all six languages yet.\n";
    }
    else
    {
        for (const std::string& pair_name: ratchet_pairs)
        {
            markdown += std::format("- `{}`\n", pair_name);
        }
    }

    markdown += "\n## Matrix\n\n| class | role |";
    for (const std::string_view language: LANGUAGES)
    {
        markdown += std::format(" {} |", language);
    }
    markdown += " ratchet |\n|---|---|---|---|---|---|---|---|---|\n";

    for (const MatrixRow& matrix_row: rows)
    {
        markdown += std::format("| {} | {} |", matrix_row.wx_class, matrix_row.role);
        for (const std::string& state: matrix_row.states)
        {
            if (state.empty())
            {
                markdown += " --- |";
            }
            else
            {
                markdown += std::format(" `{}` |", state);
            }
        }

        if (matrix_row.states[0].empty())
        {
            markdown += " --- |\n";
        }
        else if (matrix_row.verified_count == static_cast<int>(LANGUAGES.size()))
        {
            markdown += " yes |\n";
        }
        else
        {
            markdown += std::format(" no ({}/6) |\n", matrix_row.verified_count);
        }
    }

    wxFFile file(wxString::FromUTF8(path), "wb");
    if (!file.IsOpened())
    {
        wxLogStderr(nullptr).LogText(wxString("--gen_supported: cannot write ") +
                                     wxString::FromUTF8(path));
        return false;
    }

    if (file.Write(markdown.data(), markdown.size()) != markdown.size())
    {
        wxLogStderr(nullptr).LogText(wxString("--gen_supported: short write to ") +
                                     wxString::FromUTF8(path));
        return false;
    }

    return file.Close();
}
