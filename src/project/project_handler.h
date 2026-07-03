/////////////////////////////////////////////////////////////////////////////
// Purpose:   ProjectHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

#include <cstdint>        // for std::uint8_t
#include <unordered_map>  // for std::unordered_map
#include <utility>        // for pair<>

#include "gen_enums.h"                   // Enumerations for generators
#include "node.h"                        // Node class
#include "utils/utils.h"                 // NodesFormChild structure
#include "wxue_namespace/wxue_string.h"  // wxue::string

namespace pugi
{
    class xml_document;
}

class ImportXML;

enum class GenOutput
{
    none = 0,
    cplusplus = 1 << 0,
    python = 1 << 1,
    ruby = 1 << 2,

    // These 5 are the kwx languages (kwxFortran, kwxGO, etc.)
    fortran = 1 << 3,
    go = 1 << 4,
    julia = 1 << 5,
    luajit = 1 << 6,
    typescript = 1 << 7,

    // Not a language — set when a C++ derived class file needs to be generated
    derived = 1 << 8,

    xrc = 1 << 9,

    reserved1 = 1 << 10,  // Reserved for future use

};

constexpr GenOutput operator|(GenOutput out_a, GenOutput out_b)
{
    return static_cast<GenOutput>(std::to_underlying(out_a) | std::to_underlying(out_b));
}
constexpr unsigned int operator&(GenOutput out_a, GenOutput out_b)
{
    return std::to_underlying(out_a) & std::to_underlying(out_b);
}
constexpr GenOutput& operator|=(GenOutput& out_a, GenOutput out_b)
{
    return out_a = out_a | out_b;
}
constexpr GenOutput& operator&=(GenOutput& out_a, GenOutput out_b)
{
    out_a = static_cast<GenOutput>(std::to_underlying(out_a) & std::to_underlying(out_b));
    return out_a;
}

enum : std::uint8_t
{
    OUT_FLAG_NONE = 0,
    OUT_FLAG_IGNORE_DERIVED = 1 << 0,  // Ignore derived output files
};

class wxFileName;  // forward declaration

class ProjectHandler
{
private:
    // ProjectHandler() {}
    ProjectHandler();
    ~ProjectHandler();

public:
    ProjectHandler(ProjectHandler&&) = delete;
    ProjectHandler& operator=(ProjectHandler&&) = delete;
    ProjectHandler(ProjectHandler const&) = delete;

    void operator=(ProjectHandler const&) = delete;

    static ProjectHandler& getInstance()
    {
        static ProjectHandler instance;
        return instance;
    }

    // Calling this will also initialize the ProjectImage class
    void Initialize(NodeSharedPtr project, bool allow_ui = true);

    // This will convert the project path into a full path
    void set_ProjectFile(std::string_view file);
    void set_ProjectPath(const wxFileName& path);

    // Returns the full path to the directory the project file is in
    [[nodiscard]] wxue::string get_ProjectPath() const;

    // Returns the full path to the project filename
    [[nodiscard]] wxue::string get_ProjectFile() const;

    [[nodiscard]] const wxFileName* get_wxFileName() const;

    // Get a bit flag indicating which output types are enabled.
    //
    // GenOutput::derived is only set if the file is specified and does *not* exist.
    // Uses an in-memory cache (m_derived_file_exists) to avoid repeated disk I/O.
    [[nodiscard]] GenOutput get_OutputType(int flags = OUT_FLAG_NONE) const;

    // Populate the derived file existence cache. Call after LoadProject or when the entire
    // cache needs to be rebuilt (e.g., derived_directory changed).
    void InitializeDerivedFileCache();

    // Update the cache entry for a single form. Call when derived_file, use_derived_class,
    // or the derived class name changes for a specific form, or after a derived file is written.
    void UpdateDerivedFileCache(Node* form);

    // Clear and rebuild the entire derived file cache.
    void InvalidateDerivedFileCache();

    // Returns true if the form's derived source file does NOT exist on disk (from cache).
    [[nodiscard]] bool IsDerivedFileMissing(Node* form) const;

    // Returns true if any form has a missing derived file (from cache).
    [[nodiscard]] bool HasMissingDerivedFiles() const;

    // Change to the project's directory
    void ChangeDir() const;

    wxue::string ArtDirectory();
    const wxFileName* get_ArtPath();

    // If the node is within a folder, and the folder specifies a directory, then that
    // directory is returned. Otherwise the project base directory is returned.
    wxue::string get_BaseDirectory(Node* node, GenLang language = GenLang::cplusplus) const;

    // Returns the absolute path to the output file for this node. If no output filename is
    // specified, first will still contain a path with no filename, and second will be false.
    std::pair<wxue::string, bool> GetOutputPath(Node* form,
                                                GenLang language = GenLang::cplusplus) const;

    // If the node is within a folder, and the folder specifies a directory, then that
    // directory is returned. Otherwise the project derived directory is returned.
    std::string get_DerivedDirectory(Node* node, GenLang language = GenLang::cplusplus) const;

    // Returns the full path to the derived filename or an empty string if no derived file
    // was specified.
    wxue::string get_DerivedFilename(Node*) const;

    [[nodiscard]] Node* get_ProjectNode() const { return m_project_node.get(); }
    std::vector<NodeSharedPtr>& get_ChildNodePtrs() { return m_project_node->get_ChildNodePtrs(); }
    Node* get_Child(size_t index) { return m_project_node->get_Child(index); }

    // This includes forms in folders and sub-folders
    void CollectForms(std::vector<Node*>& forms, Node* node_start = nullptr);

    // Returns the first project child that is a form, or nullptr if no form children found.
    Node* get_FirstFormChild(Node* node = nullptr) const;

    // Make class and filenames unique to the project
    void FixupDuplicatedNode(Node* new_node);

    // Check all variable names in the project tree for uniqueness.
    // Local variables (var_name, checkbox_var_name, radiobtn_var_name) are checked
    // per-form and auto-fixed. validator_variable duplicates are checked project-wide
    // and the user is given the option to fix them.
    void FixDuplicateVarNames(bool allow_ui);

    [[nodiscard]] int get_ProjectVersion() const { return m_ProjectVersion; }
    [[nodiscard]] int get_OriginalProjectVersion() const { return m_OriginalProjectVersion; }
    void ForceProjectVersion(int version) { m_ProjectVersion = version; }

    // Call this after the user has been warned about saving a project file that is incompatible
    // with older versions of wxUiEditor
    void UpdateOriginalProjectVersion() { m_OriginalProjectVersion = m_ProjectVersion; }

    [[nodiscard]] bool AddOptionalComments() const
    {
        return m_project_node->as_bool(prop_optional_comments);
    }

    // Call set_ProjectUpdated() if the project file's minimum version needs to be updated
    void set_ProjectUpdated() { m_isProject_updated = true; }
    // Call is_ProjectUpdated() to determine if the project file's minimum version needs to be
    // updated
    [[nodiscard]] bool is_ProjectUpdated() const { return m_isProject_updated; }

    [[nodiscard]] bool is_UiAllowed() const { return m_allow_ui; }

    [[nodiscard]] bool is_NewProject() const { return m_isNewProject; }
    void clear_NewProject() { m_isNewProject = false; }

    [[nodiscard]] size_t get_ChildCount() const { return m_project_node->get_ChildCount(); }

    // Returns a GenLang::... enum value. Specify a node if you want to check for a folder
    // override of the language.
    GenLang get_CodePreference(Node* node = nullptr) const;

    // Returns all of the languages that are enabled for this project. The project's Code
    // Preference is always included.
    [[nodiscard]] GenLang get_GenerateLanguages() const;

    // Assume major, minor, and patch have 99 possible values.
    // Returns major * 10000 + minor * 100 + patch
    [[nodiscard]] int get_LangVersion(GenLang language) const;

    // const wxue::string& value(GenEnum::PropName name) const { return
    // m_project_node->as_string(name); }
    [[nodiscard]] wxue::string_view view(PropName name) const
    {
        return m_project_node->as_view(name);
    }

    [[nodiscard]] const wxue::string& as_string(PropName name) const
    {
        return m_project_node->as_string(name);
    }

    [[nodiscard]] std::string_view as_view(PropName name) const
    {
        return m_project_node->as_view(name);
    }

    [[nodiscard]] bool as_bool(PropName name) const { return m_project_node->as_bool(name); }
    [[nodiscard]] size_t as_size_t(PropName name) const
    {
        return static_cast<size_t>(m_project_node->as_int(name));
    }

    // Returns true if the property exists, has a value (!= wxDefaultSize, !=
    // wxDefaultPosition, or non-specified bitmap)
    [[nodiscard]] bool HasValue(PropName name) const { return m_project_node->HasValue(name); }

    bool LoadProject(const wxue::string& file, bool allow_ui = true);
    NodeSharedPtr LoadProject(pugi::xml_document& doc, bool allow_ui = true);

    bool Import(ImportXML& import, std::string& file, bool append = false, bool allow_ui = true);
    bool ImportProject(std::string_view file, bool allow_ui = true);

    void AppendCrafter(wxArrayString& files);
    void AppendDialogBlocks(wxArrayString& files);
    void AppendFormBuilder(wxArrayString& files);
    void AppendGlade(wxArrayString& files);
    void AppendSmith(wxArrayString& files);
    void AppendXRC(wxArrayString& files);
    void AppendWinRes(const wxue::string& rc_file, std::vector<wxue::string>& dialogs);

    bool NewProject(bool create_empty = false, bool allow_ui = true);

    // Call this after a project is imported and converted into a Node tree. This will do a
    // final check and fixup for things like inconsistent styles, invalid gridbag sizer rows
    // and columns, etc. Because it runs on the Node tree, it doesn't matter what importer
    // was used.
    void FinalImportCheck(Node* parent, bool set_line_length = true);

    // Called by FinalImportCheck() to recursively check the node and all of it's children,
    // grandchildren, etc.
    void RecursiveNodeCheck(Node* node);

    // Assumes CollectForms() has already been called. Finds out which form should be used
    // to generate wxue() functions for embedded images.
    void FindWxueFunctions(std::vector<Node*>& forms);

    // After calling FindWxueFunctions(), this will return the form that should be used to
    // generate the one copy of wxueBundleSVG() that is used by all forms.
    [[nodiscard]] Node* get_Form_BundleSVG() const { return m_form_BundleSVG; }

    // After calling FindWxueFunctions(), this will return the form that should be used to
    // generate the one copy of wxueImage() that is used by all forms.
    [[nodiscard]] Node* get_Form_Image() const { return m_form_Image; }

    // After calling FindWxueFunctions(), this will return the form that should be used to
    // generate the one copy of wxueAnimation() that is used by all forms.
    [[nodiscard]] Node* get_Form_Animation() const { return m_form_Animation; }

    // This will assume any ImagesList class will be the first child of the project, and will
    // either return that Node* or nullptr if no ImagesList class is found.
    [[nodiscard]] Node* get_ImagesForm();

    // This will assume any Data class will be the first or second child of the project, and
    // will either return that Node* or nullptr if no Data class is found.
    [[nodiscard]] Node* get_DataForm();

    // Sets project property value only if the property exists, returns false if it doesn't
    // exist.
    template <typename T>
    bool set_value(PropName name, T value)
    {
        if (auto* prop = m_project_node->get_PropPtr(name); prop)
        {
            prop->set_value(value);
            return true;
        }

        return false;
    }

private:
    // Helper function to determine if a language should generate output for a child node
    [[nodiscard]] bool ShouldOutputLanguage(const NodesFormChild& nodes,
                                            const PropName& base_file_property,
                                            GenLang language) const;

    // Helper functions for GetOutputPath complexity reduction
    [[nodiscard]] wxue::string GetFolderOutputPath(Node* folder, GenLang language,
                                                   Node*& form) const;
    [[nodiscard]] wxue::string GetProjectOutputPath(GenLang language) const;
    [[nodiscard]] wxue::string GetBaseFilename(Node* form, GenLang language) const;
    void MergeBaseFilePath(wxue::string& result, const wxue::string& base_file) const;

    // Helper functions for FindWxueFunctions complexity reduction
    [[nodiscard]] bool AllFormTypesFound() const;
    void ProcessImageProperty(const NodeProperty& prop, Node* child);
    void ParseImagePropsRecursive(Node* node);
    void ProcessFormIcon(Node* form);

    NodeSharedPtr m_project_node { nullptr };

    Node* m_form_BundleSVG { nullptr };
    Node* m_form_Image { nullptr };
    Node* m_form_BundleBitmaps { nullptr };
    Node* m_form_Animation { nullptr };
    Node* m_ImagesForm { nullptr };
    Node* m_DataForm { nullptr };

    // Cache of absolute derived source file paths -> whether the file exists on disk.
    // Key is the full path with .cpp extension (as returned by get_DerivedFilename).
    std::unordered_map<std::string, bool> m_derived_file_exists;

    // Creating the wxFileName class this way means callers don't need to include
    // wx/filename.h and all the files it includes.

    mutable std::unique_ptr<wxFileName> m_project_path;
    std::unique_ptr<wxFileName> m_art_path;

    int m_ProjectVersion;
    int m_OriginalProjectVersion;

    bool m_allow_ui { true };
    bool m_isNewProject { false };
    bool m_isProject_updated { false };
};

extern ProjectHandler& Project;  // NOLINT (non-const reference) // cppcheck-suppress constReference
