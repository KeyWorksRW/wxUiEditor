/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code generation file writing functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace pugi
{
    class xml_node;
}

class Node;
class WriteCode;
class wxProgressDialog;

#include <chrono>
#include <future>
#include <memory>
#include <optional>
#include <vector>

#include "../panels/base_panel.h"   // PANEL_PAGE enum
#include "../tools/compare/diff.h"  // DiffResult, DiffLine, FileDiff

class GenResults
{
public:
    // Generation modes
    enum class Mode : std::uint8_t
    {
        generate_and_write,  // Generate code and write to disk
        compare_only         // Generate and compare with disk (no write)
    };

    void SetMode(Mode mode) { m_mode = mode; }
    [[nodiscard]] auto GetMode() const { return m_mode; }

    // Set languages to generate (GenLang values are bit flags, can be combined)
    // e.g., SetLanguages(GEN_LANG_CPLUSPLUS | GEN_LANG_PYTHON)
    void SetLanguages(GenLang languages) { m_languages = languages; }
    [[nodiscard]] auto GetLanguages() const { return m_languages; }

    // Set target node(s) for generation - scope is inferred from node type:
    //   - Project node: generate entire project
    //   - Folder node: generate all forms in folder recursively
    //   - Form node: generate just that form
    void SetNodes(Node* startNode);

    // Set explicit list of forms to generate (for custom selection)
    void SetNodes(const std::vector<Node*>& nodes);

    // Optimized entry point for panel display (most frequent use case)
    // - Sets mode to generate_for_display internally
    // - startNode: if not a form, uses the form containing the node
    // - language: must be exactly one language (error if multiple bits set)
    // - src/hdr: at least one must be non-null
    // - panel_page: which panel is currently active (for line tracking)
    // Returns false if startNode is Project/Folder or multiple languages specified
    [[nodiscard]] auto SetDisplayTarget(Node* startNode, GenLang language, WriteCode* src,
                                        WriteCode* hdr = nullptr,
                                        PANEL_PAGE panel_page = PANEL_PAGE::NOT_PANEL) -> bool;

    // Main entry point - runs generation based on configured mode/nodes/languages
    [[nodiscard]] auto Generate() -> bool;

    // Set the output path for combined file generation
    // This overrides project settings when specified
    void SetCombinedOutputPath(std::string_view path) { m_combined_output_path = path; }

    // Generate all forms into a single combined output file for the specified language.
    // language: must be exactly one language (error if multiple bits set)
    // Requires SetCombinedOutputPath() or project prop_combined_xrc_file to be set.
    // Currently supports GEN_LANG_XRC. Future: Python, Ruby, Perl.
    // Returns true if file was written/needs updating, false otherwise.
    [[nodiscard]] auto GenerateCombinedFile(GenLang language) -> bool;

    void StartClock();
    void EndClock();
    void Clear();

    // Enable progress dialog for next Generate() call.
    // Title will be shown in the dialog (e.g., "Comparing C++ files...")
    void EnableProgressDialog(const wxString& title = "Generating code...");

    // Disable progress dialog (default state)
    void DisableProgressDialog() { m_show_progress = false; }

    [[nodiscard]] auto GetFileCount() const { return m_file_count; }
    void SetFileCount(size_t count) { m_file_count = count; }
    void IncrementFileCount() { ++m_file_count; }

    [[nodiscard]] auto GetElapsed() const { return m_elapsed; }

    [[nodiscard]] auto GetMsgs() -> auto& { return m_msgs; }
    [[nodiscard]] auto GetMsgs() const -> const auto& { return m_msgs; }

    [[nodiscard]] auto GetUpdatedFiles() -> auto& { return m_updated_files; }
    [[nodiscard]] auto GetUpdatedFiles() const -> const auto& { return m_updated_files; }

    // Get files that were newly created (didn't exist before)
    [[nodiscard]] auto GetCreatedFiles() -> auto& { return m_created_files; }
    [[nodiscard]] auto GetCreatedFiles() const -> const auto& { return m_created_files; }

    // Get file diffs collected during compare_only mode
    [[nodiscard]] auto GetFileDiffs() -> auto& { return m_file_diffs; }
    [[nodiscard]] auto GetFileDiffs() const -> const auto& { return m_file_diffs; }

    // Generate code files for all forms. If comparison_only is true, only checks if files
    // need updating without writing
    [[nodiscard]] auto GenerateLanguageFiles(GenLang language, bool comparison_only = false)
        -> bool;

private:
    // Scope inference from node type
    enum class Scope : std::uint8_t
    {
        unknown,
        project,  // Generate entire project
        folder,   // Generate all forms in folder recursively
        form,     // Generate a single form
        display   // Generate for panel display (no file ops)
    };

    // Collects forms from m_target_nodes based on inferred scope
    void CollectFormsFromNodes();

    // Generate C++ code files for all forms in the project
    // If comparison_only is true, only checks if files need updating without writing
    [[nodiscard]] auto GenerateCppFiles(bool comparison_only = false) -> bool;

    // Generate C++ code for a single form (both .h and .cpp files)
    // In compare mode, captures FileDiff for both files
    // Returns true if any file was updated/needs updating
    [[nodiscard]] auto GenerateCppForm(Node* form, bool comparison_only = false,
                                       wxProgressDialog* progress = nullptr) -> bool;

    // Returns true if any file was updated
    [[nodiscard]] auto GenerateLanguageForm(std::string_view class_name, Node* form,
                                            bool comparison_only = false) -> bool;

    // Generate code for display in a panel (no file operations)
    [[nodiscard]] auto GenerateForDisplay() -> bool;

    // Generate all XRC forms into a single combined file
    // In compare mode, captures FileDiff. Returns true if file was updated/needs updating.
    [[nodiscard]] auto GenerateCombinedXrcFile(bool comparison_only = false) -> bool;

    // Remove forms from the vector that don't have an output file configured for the current
    // language
    void RemoveFormsWithoutOutputPath(std::vector<Node*>& forms);

    // Process file diff asynchronously - checks size limit and either creates minimal entry for
    // large files or performs full diff comparison. Launches async task for performance.
    void ProcessFileDiff(wxue::string path, std::shared_ptr<std::string> content, Node* form);

    // Wait for all pending diff computations to complete and collect results
    void WaitForPendingDiffs();

    // Class members
    Mode m_mode { Mode::generate_and_write };
    Scope m_scope { Scope::unknown };
    GenLang m_languages { 0 };  // Bit flags for languages to generate

    std::vector<Node*> m_target_nodes;  // Forms to process
    Node* m_start_node { nullptr };     // Original node passed to SetNodes()

    // For display mode
    WriteCode* m_display_src { nullptr };
    WriteCode* m_display_hdr { nullptr };
    PANEL_PAGE m_panel_page { PANEL_PAGE::NOT_PANEL };  // Which panel is active (for line tracking)

    // For combined file output
    std::string m_combined_output_path;  // Override for combined output path

    size_t m_file_count { 0 };
    size_t m_elapsed { 0 };

    std::vector<std::string> m_msgs;
    std::vector<std::string> m_updated_files;
    std::vector<std::string> m_created_files;  // Files newly created (didn't exist before)
    std::vector<FileDiff> m_file_diffs;        // Detailed diffs for compare_only mode

    std::chrono::steady_clock::time_point m_start_time;
    bool m_clock_started { false };

    // Threading support for ProcessFileDiff
    std::vector<std::future<std::optional<FileDiff>>> m_pending_diffs;

    // Progress dialog support
    bool m_show_progress { false };  // Enable progress dialog in Generate()
    wxString m_progress_title;       // Dialog title
};

// DEPRECATED: Use GenResults::Generate() with SetNodes(ProjectNode),
// SetLanguages(GEN_LANG_CPLUSPLUS). If pClassList is non-null, it must contain the base class name
// of every form that needs updating.
//
// ../generate/gen_codefiles.cpp
[[deprecated("Use GenResults::Generate() with SetNodes(), SetLanguages(GEN_LANG_CPLUSPLUS)")]]
auto GenerateCppFiles(GenResults& results, std::vector<std::string>* pClassList = nullptr) -> bool;

// ../generate/gen_codefiles.cpp
void GenInheritedClass(GenResults& results);
