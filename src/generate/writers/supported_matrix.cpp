/////////////////////////////////////////////////////////////////////////////
// Purpose:   Write the full kwx capability matrix (.private/kwxSupported.md)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/*
    The matrix is generated output: it joins checked-in sources and must never be hand-edited
    (Cross-Phase Constraints).

        1. the compiled generator library -- every generator declaration, so classes that are
           not used by quick.wxui still appear;
        2. the six schema-1 descriptors in src/generate/ffi_ui/ -- the class/role surface each
           FFI language claims;
        3. src/generate/ffi_ui/coverage.json -- the per-cell matrix states derived by
           .tts/kwx_coverage.ps1, including the native (Python/Ruby) and XRC blocks.

    coverage.json is authoritative wherever it has a cell; the descriptors resolve the remaining
    classes/roles exactly the way `kwxgen check-ui` and ResolveRole() do. Nothing here invents a
    state.

    Phase 7 column set: c++, python, ruby, fortran, go, julia, lua, rust, typescript, xrc.
    States: verified-emitter, verified-port, binding-missing, xrc-missing, handler-missing,
    absent, not-generated, unknown. The FFI derivation vocabulary in coverage.json still carries
    `descriptor-missing` and `exercised-unverified`; those are derivation-only values and are
    mapped here (see ToMatrixState), never emitted as a matrix cell.

    `verified-emitter` is only ever read from coverage.json, which derives it from
    src/generate/ffi_ui/verified.json; `verified-port` is only ever read from coverage.json,
    which derives it from a passing drive log for this cycle. The C++ column therefore never
    claims `verified-emitter`: there is no verified.json entry for it, and its evidence is the
    driven baseline (a `verified-port` cell).

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
#include <tuple>
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

// Phase 7 column order: the C++ baseline, the two native emitters, the six kwx FFI languages and
// the XRC writer.
static constexpr std::array<std::string_view, 10> MATRIX_COLUMNS = {
    "c++",   "python",   "ruby",     "fortran",    "go",
    "julia", "lua",      "rust",     "typescript", "xrc"
};

// The six FFI languages carry a schema-1 descriptor. C++ is the driven baseline, Python and Ruby
// are native emitters and XRC is a writer: none of the four has a descriptor.
static constexpr std::array<GenLang, 6> LANGUAGE_IDS = { GenLang::fortran, GenLang::go,
                                                         GenLang::julia,   GenLang::luajit,
                                                         GenLang::rust,    GenLang::typescript };

// Matrix column indices used by the derivation helpers.
static constexpr size_t COL_CPP = 0;
static constexpr size_t COL_PYTHON = 1;
static constexpr size_t COL_RUBY = 2;
static constexpr size_t COL_FFI_FIRST = 3;
static constexpr size_t COL_XRC = 9;

// Phase 7 matrix states. `descriptor-missing` and `exercised-unverified` are deliberately not
// members: they remain the derivation vocabulary in coverage.json and are mapped by
// ToMatrixState(). A symbol grep is not a drive PASS, and a wrapped class with no callable cannot
// be emitted, so those two resolve to `unknown` and `absent` respectively.
static constexpr std::array<std::string_view, 8> STATES = {
    "verified-emitter", "verified-port", "binding-missing", "xrc-missing",
    "handler-missing",  "absent",       "not-generated",   "unknown"
};

// One (class, role) row read from coverage.json: matrix column -> matrix state.
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

// Class-level matrix states. The native (Python/Ruby) and XRC blocks are per class, not per pair,
// so the value is replicated across every row of that class.
struct ClassMatrix
{
    std::map<std::string, std::string, std::less<>> python;
    std::map<std::string, std::string, std::less<>> ruby;
    std::map<std::string, std::string, std::less<>> xrc;
};

// Metadata carried beside the pair table, read from coverage.json.
struct CoverageMeta
{
    std::string generated;
    std::string kwxgen_version;
    std::string headers_hash;
    std::string xrc_source;
    std::string xrc_verdict;
};

// One output line of the pair-level table. Every cell is a matrix state; there is no blank.
struct MatrixRow
{
    std::string wx_class;
    std::string role;
    std::array<std::string, MATRIX_COLUMNS.size()> states;
    int emitter_count {};
    int port_count {};
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

// Maps the FFI derivation vocabulary onto the Phase 7 matrix vocabulary.
static std::string ToMatrixState(std::string_view derivation_state)
{
    if (derivation_state == "verified")
    {
        return "verified-emitter";
    }

    // A wrapped class with no callable cannot be emitted; that is what `absent` means here.
    if (derivation_state == "absent" || derivation_state == "descriptor-missing")
    {
        return "absent";
    }

    // `exercised-unverified` (a descriptor symbol found in a port's source) and `unknown` carry no
    // drive evidence, so neither may claim a verified state.
    return "unknown";
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

// Reads one cell object: `matrix_state` wins, else the derivation `state` is mapped.
static std::string ReadCellState(const glz::generic& cell)
{
    if (!cell.is_object())
    {
        return {};
    }

    std::string state = GetJsonString(cell, "matrix_state");
    if (!state.empty())
    {
        return state;
    }

    const std::string derivation = GetJsonString(cell, "state");
    if (derivation.empty())
    {
        return {};
    }

    return ToMatrixState(derivation);
}

// Loads the native (Python/Ruby) class cells. Every target class has a matrix_state of
// verified-emitter, binding-missing, not-generated or unknown.
static void LoadNativeClasses(const glz::generic& document, ClassMatrix& class_matrix)
{
    if (!document.contains("native") || !document["native"].is_object())
    {
        return;
    }

    const glz::generic& native = document["native"];
    if (!native.contains("classes") || !native["classes"].is_array())
    {
        return;
    }

    for (const auto& entry: native["classes"].get_array())
    {
        const std::string class_name = GetJsonString(entry, "class");
        if (class_name.empty() || !entry.contains("cells") || !entry["cells"].is_object())
        {
            continue;
        }

        const glz::generic& cells = entry["cells"];
        if (cells.contains("python"))
        {
            const std::string state = ReadCellState(cells["python"]);
            if (!state.empty())
            {
                class_matrix.python.emplace(class_name, state);
            }
        }

        if (cells.contains("ruby"))
        {
            const std::string state = ReadCellState(cells["ruby"]);
            if (!state.empty())
            {
                class_matrix.ruby.emplace(class_name, state);
            }
        }
    }
}

// Loads the XRC class cells. XRC is a writer: a class whose generated node was loaded by the XRC
// harness is `verified-emitter`; a declared node the loaded form lacks is `xrc-missing`.
static void LoadXrcClasses(const glz::generic& document, ClassMatrix& class_matrix)
{
    if (!document.contains("xrc") || !document["xrc"].is_object())
    {
        return;
    }

    const glz::generic& xrc_block = document["xrc"];
    if (!xrc_block.contains("classes") || !xrc_block["classes"].is_array())
    {
        return;
    }

    for (const auto& entry: xrc_block["classes"].get_array())
    {
        const std::string class_name = GetJsonString(entry, "class");
        if (class_name.empty())
        {
            continue;
        }

        std::string state = GetJsonString(entry, "matrix_state");
        if (state.empty())
        {
            state = ToMatrixState(GetJsonString(entry, "state"));
        }

        if (!state.empty())
        {
            class_matrix.xrc.emplace(class_name, state);
        }
    }
}

// Loads coverage.json. Returns false only when the report is missing or unparseable, because a
// matrix built without it would silently claim `unknown` for pairs the coverage run resolved.
static bool LoadCoverage(const std::string& file_path, CoverageTable& table, CoverageMeta& meta,
                         ClassMatrix& class_matrix)
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

    if (document.contains("xrc") && document["xrc"].is_object())
    {
        const glz::generic& xrc_block = document["xrc"];
        meta.xrc_source = GetJsonString(xrc_block, "source");
        meta.xrc_verdict = GetJsonString(xrc_block, "verdict");
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
            for (const std::string_view column: MATRIX_COLUMNS)
            {
                if (!cells.contains(column))
                {
                    continue;
                }

                const std::string state = ReadCellState(cells[column]);
                if (!state.empty())
                {
                    coverage_row.states.emplace(std::string(column), state);
                }
            }
        }

        table[class_name].emplace(role_name, std::move(coverage_row));
    }

    LoadNativeClasses(document, class_matrix);
    LoadXrcClasses(document, class_matrix);

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

// Resolves a state for an FFI pair coverage.json has no row for, using the same precedence and the
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

// Resolves one matrix cell for a pair coverage.json has no cell for. A class that no generator
// declares is `absent` in every language column; nothing else is inferred.
static std::string DeriveMatrixState(size_t column_index, std::string_view wx_class,
                                     std::string_view role, const ClassSet& generator_classes,
                                     const ClassMatrix& class_matrix)
{
    const bool has_generator = generator_classes.contains(wx_class);

    // C++ is the driven baseline. There is no verified.json entry for it, so the only C++ evidence
    // is a `verified-port` cell from coverage.json; without one the class is `unknown`.
    if (column_index == COL_CPP)
    {
        return has_generator ? "unknown" : "absent";
    }

    if (column_index == COL_PYTHON || column_index == COL_RUBY)
    {
        const std::map<std::string, std::string, std::less<>>& states =
            (column_index == COL_PYTHON) ? class_matrix.python : class_matrix.ruby;
        const std::map<std::string, std::string, std::less<>>::const_iterator iter =
            states.find(wx_class);
        if (iter != states.end())
        {
            return iter->second;
        }

        return has_generator ? "unknown" : "absent";
    }

    if (column_index == COL_XRC)
    {
        const std::map<std::string, std::string, std::less<>>::const_iterator iter =
            class_matrix.xrc.find(wx_class);
        if (iter != class_matrix.xrc.end())
        {
            return iter->second;
        }

        return has_generator ? "unknown" : "absent";
    }

    const size_t ffi_index = column_index - COL_FFI_FIRST;
    return ToMatrixState(DeriveState(GetFfiUiDescriptor(LANGUAGE_IDS[ffi_index]), wx_class, role));
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
    ClassMatrix class_matrix;
    if (!LoadCoverage(coverage_path, coverage, meta, class_matrix))
    {
        return false;
    }

    ClassSet generator_classes;
    ClassSet classes;
    ClassRoles roles;

    AddGeneratorClasses(generator_classes);
    classes = generator_classes;

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

    // The native and XRC blocks contribute classes too: a class with no descriptor role still
    // needs its row so its Python, Ruby or XRC state is accounted for.
    for (const auto& class_entry: class_matrix.python)
    {
        classes.emplace(class_entry.first);
        std::ignore = roles[class_entry.first];
    }
    for (const auto& class_entry: class_matrix.ruby)
    {
        classes.emplace(class_entry.first);
        std::ignore = roles[class_entry.first];
    }
    for (const auto& class_entry: class_matrix.xrc)
    {
        classes.emplace(class_entry.first);
        std::ignore = roles[class_entry.first];
    }

    std::vector<MatrixRow> rows;
    size_t no_role_classes = 0;

    for (const std::string& class_name: classes)
    {
        const ClassRoles::const_iterator roles_iter = roles.find(class_name);
        if (roles_iter == roles.end() || roles_iter->second.empty())
        {
            MatrixRow no_role_row;
            no_role_row.wx_class = class_name;
            no_role_row.role = "---";

            // A class with no role row still needs a state in every column: it is a class the
            // matrix must account for, not a blank.
            for (size_t index = 0; index < MATRIX_COLUMNS.size(); ++index)
            {
                std::string state;
                const CoverageTable::const_iterator class_iter = coverage.find(class_name);
                if (class_iter != coverage.end())
                {
                    const std::map<std::string, CoverageRow, std::less<>>::const_iterator
                        role_iter = class_iter->second.find("---");
                    if (role_iter != class_iter->second.end())
                    {
                        const std::map<std::string, std::string, std::less<>>::const_iterator
                            state_iter =
                                role_iter->second.states.find(std::string(MATRIX_COLUMNS[index]));
                        if (state_iter != role_iter->second.states.end())
                        {
                            state = state_iter->second;
                        }
                    }
                }

                if (state.empty())
                {
                    state = DeriveMatrixState(index, class_name, "---", generator_classes,
                                              class_matrix);
                }

                no_role_row.states[index] = std::move(state);
            }

            rows.emplace_back(std::move(no_role_row));
            ++no_role_classes;
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

            for (size_t index = 0; index < MATRIX_COLUMNS.size(); ++index)
            {
                std::string state;
                if (has_coverage_row)
                {
                    const CoverageRow& coverage_row = class_iter->second.at(role_name);
                    const std::map<std::string, std::string, std::less<>>::const_iterator
                        state_iter = coverage_row.states.find(std::string(MATRIX_COLUMNS[index]));
                    if (state_iter != coverage_row.states.end())
                    {
                        state = state_iter->second;
                    }
                }

                if (state.empty())
                {
                    state = DeriveMatrixState(index, class_name, role_name, generator_classes,
                                              class_matrix);
                }

                if (state == "verified-emitter")
                {
                    ++matrix_row.emitter_count;
                }
                else if (state == "verified-port")
                {
                    ++matrix_row.port_count;
                }
                matrix_row.states[index] = std::move(state);
            }

            rows.emplace_back(std::move(matrix_row));
        }
    }

    std::array<std::array<size_t, STATES.size()>, MATRIX_COLUMNS.size()> counts {};
    for (const MatrixRow& matrix_row: rows)
    {
        for (size_t index = 0; index < matrix_row.states.size(); ++index)
        {
            const std::string& state = matrix_row.states[index];
            if (state.empty())
            {
                continue;
            }

            const size_t state_index = GetStateIndex(state);
            if (state_index < STATES.size())
            {
                ++counts[index][state_index];
            }
        }
    }

    std::string markdown;
    markdown.reserve(128U * 1024U);

    markdown += "# kwx supported matrix (full surface, nine languages + XRC)\n\n";
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
    markdown += std::format("| classes (generator + FFI + native + XRC) | {} |\n", classes.size());
    markdown += std::format("| `(class, role)` pairs | {} |\n", rows.size() - no_role_classes);
    markdown += std::format("| classes with no role row (`---`) | {} |\n", no_role_classes);
    markdown += std::format("| generator classes | {} |\n", generator_classes.size());
    if (!meta.xrc_source.empty())
    {
        markdown += std::format("| xrc source | `{}` (verdict {}) |\n", meta.xrc_source,
                                meta.xrc_verdict.empty() ? "unknown" : meta.xrc_verdict);
    }

    markdown += "| matrix states |";
    for (const std::string_view state: STATES)
    {
        markdown += std::format(" `{}`", state);
    }
    markdown += " |\n\n";

    markdown += "Rows are `(class, role)` pairs. The row universe is the union of the compiled "
                "generator declarations, the six\n";
    markdown += "descriptors, `coverage.json` rows and the native/XRC class blocks; a class no "
                "language declares is `absent`,\n";
    markdown += "never blank. `verified-emitter` requires a "
                "`src/generate/ffi_ui/verified.json` entry; `verified-port` requires a\n";
    markdown += "drive PASS in `.private/logs` for this cycle. The C++ column is the driven "
                "baseline, so it never claims\n";
    markdown += "`verified-emitter` (there is no C++ `verified.json` entry).\n\n";

    markdown += "Derivation states retained inside `coverage.json` (`rows[].cells[].state`) and "
                "mapped here:\n";
    markdown += "`verified` -> `verified-emitter`, `descriptor-missing` -> `absent`, "
                "`exercised-unverified` -> `unknown`,\n";
    markdown += "`absent` -> `absent`, `unknown` -> `unknown`. A port grep is not a drive PASS, "
                "so it never promotes a cell.\n\n";

    markdown += "## Counts per state per language\n\n| language |";
    for (const std::string_view state: STATES)
    {
        markdown += std::format(" {} |", state);
    }
    markdown += " total |\n|---|---|";
    for (size_t index = 0; index < STATES.size(); ++index)
    {
        markdown += "---|";
    }
    markdown += "\n";

    for (size_t index = 0; index < MATRIX_COLUMNS.size(); ++index)
    {
        markdown += std::format("| {} |", MATRIX_COLUMNS[index]);
        size_t total = 0;
        for (const size_t count: counts[index])
        {
            markdown += std::format(" {} |", count);
            total += count;
        }
        markdown += std::format(" {} |\n", total);
    }

    markdown += "\n## Verified-emitter in all six kwx languages (ratchet)\n\n";
    std::vector<std::string> ratchet_pairs;
    std::vector<std::string> port_only_pairs;
    for (const MatrixRow& matrix_row: rows)
    {
        if (matrix_row.emitter_count == 6)
        {
            ratchet_pairs.emplace_back(matrix_row.wx_class + "." + matrix_row.role);
        }
        else if (matrix_row.port_count == 6)
        {
            port_only_pairs.emplace_back(matrix_row.wx_class + "." + matrix_row.role);
        }
    }

    if (ratchet_pairs.empty())
    {
        markdown += "None. No `(class, role)` pair is `verified-emitter` in all six kwx "
                    "languages yet.\n";
    }
    else
    {
        for (const std::string& pair_name: ratchet_pairs)
        {
            markdown += std::format("- `{}`\n", pair_name);
        }
    }

    markdown += "\n## Verified-port in all six, not verified-emitter (promotion backlog)\n\n";
    if (port_only_pairs.empty())
    {
        markdown += "None.\n";
    }
    else
    {
        for (const std::string& pair_name: port_only_pairs)
        {
            markdown += std::format("- `{}`\n", pair_name);
        }
    }

    markdown += "\n## Matrix\n\n| class | role |";
    for (const std::string_view column: MATRIX_COLUMNS)
    {
        markdown += std::format(" {} |", column);
    }
    markdown += "\n|---|---|";
    for (size_t index = 0; index < MATRIX_COLUMNS.size(); ++index)
    {
        markdown += "---|";
    }
    markdown += "\n";

    for (const MatrixRow& matrix_row: rows)
    {
        markdown += std::format("| {} | {} |", matrix_row.wx_class, matrix_row.role);
        for (const std::string& state: matrix_row.states)
        {
            markdown += std::format(" `{}` |", state);
        }
        markdown += "\n";
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
