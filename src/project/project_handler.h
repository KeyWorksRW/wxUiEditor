/////////////////////////////////////////////////////////////////////////////
// Purpose:   ProjectHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements ProjectHandler, the singleton (Meyer's pattern) managing the
// active wxUiEditor project. The class owns m_project_node (root Node containing all
// forms/folders), m_project_path (wxFileName for .wxui file location), and version tracking
// (m_ProjectVersion, m_OriginalProjectVersion for compatibility warnings). Key responsibilities:
// project loading (LoadProject parsing XML, ImportProject for external formats), file path
// resolution (get_OutputPath handling folder overrides, language-specific extensions, base/derived
// file separation), language preference management (get_CodePreference with folder overrides,
// get_GenerateLanguages for multi-language projects), and form collection (CollectForms,
// get_FirstFormChild). Helper nodes (m_form_BundleSVG, m_ImagesForm, m_DataForm) cache special
// forms found by FindWxueFunctions. Methods like get_BaseDirectory/get_DerivedDirectory resolve
// output paths considering folder-specific overrides. FixupDuplicatedNode ensures unique
// class/filenames. Property access delegates to m_project_node (as_string, as_bool, HasValue) for
// project-level configuration.

#pragma once  // NOLINT(#pragma once in main file)

#include <cstdint>  // for std::uint8_t
#include <utility>  // for pair<>

#include "gen_enums.h"                   // Enumerations for generators
#include "node.h"                        // Node class
#include "utils/utils.h"                 // NodesFormChild structure
#include "wxue_namespace/wxue_string.h"  // wxue::string

namespace pugi
{
    class xml_document;
}

class ImportXML;

enum : std::uint8_t
{
    OUTPUT_NONE = 0,
    OUTPUT_CPLUS = 1 << 0,
    OUTPUT_DERIVED = 1 << 1,
    OUTPUT_PYTHON = 1 << 2,
    OUTPUT_RUBY = 1 << 3,
    OUTPUT_XRC = 1 << 4,
    OUTPUT_PERL = 1 << 5,
};

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
    auto operator=(ProjectHandler&&) -> ProjectHandler& = delete;
    ProjectHandler(ProjectHandler const&) = delete;

    void operator=(ProjectHandler const&) = delete;

    static auto getInstance() -> ProjectHandler&
    {
        static ProjectHandler instance;
        return instance;
    }

    // Calling this will also initialize the ProjectImage class
    void Initialize(NodeSharedPtr project, bool allow_ui = true);

    // This will convert the project path into a full path
    void set_ProjectFile(std::string_view file);
    void set_ProjectPath(const wxFileName* path);

    // Returns the full path to the directory the project file is in
    [[nodiscard]] auto get_ProjectPath() const -> wxue::string;

    // Returns the full path to the directory the project file is in
    [[nodiscard]] auto get_wxProjectPath() const -> wxue::string;

    // Returns the full path to the project filename
    [[nodiscard]] auto get_ProjectFile() const -> wxue::string;

    [[nodiscard]] auto get_wxFileName() const -> const wxFileName*;

    // Get a bit flag indicating which output types are enabled.
    //
    // OUTPUT_DERIVED is only set if the file is specified and does *not* exist.
    [[nodiscard]] auto get_OutputType(int flags = OUT_FLAG_NONE) const -> size_t;

    // Change to the project's directory
    void ChangeDir() const;

    auto ArtDirectory() -> wxue::string;
    auto get_ArtPath() -> const wxFileName*;

    // If the node is within a folder, and the folder specifies a directory, then that
    // directory is returned. Otherwise the project base directory is returned.
    auto get_BaseDirectory(Node* node, GenLang language = GEN_LANG_CPLUSPLUS) const -> wxue::string;

    // Returns the absolute path to the output file for this node. If no output filename is
    // specified, first will still contain a path with no filename, and second will be false.
    auto GetOutputPath(Node* form, GenLang language = GEN_LANG_CPLUSPLUS) const
        -> std::pair<wxue::string, bool>;

    // If the node is within a folder, and the folder specifies a directory, then that
    // directory is returned. Otherwise the project derived directory is returned.
    auto get_DerivedDirectory(Node* node, GenLang language = GEN_LANG_CPLUSPLUS) const
        -> std::string;

    // Returns the full path to the derived filename or an empty string if no derived file
    // was specified.
    auto get_DerivedFilename(Node*) const -> wxue::string;

    [[nodiscard]] auto get_ProjectNode() const -> Node* { return m_project_node.get(); }
    auto get_ChildNodePtrs() -> auto& { return m_project_node->get_ChildNodePtrs(); }
    auto get_Child(size_t index) -> Node* { return m_project_node->get_Child(index); }

    // This includes forms in folders and sub-folders
    void CollectForms(std::vector<Node*>& forms, Node* node_start = nullptr);

    // Returns the first project child that is a form, or nullptr if no form children found.
    auto get_FirstFormChild(Node* node = nullptr) const -> Node*;

    // Make class and filenames unique to the project
    void FixupDuplicatedNode(Node* new_node);

    [[nodiscard]] auto get_ProjectVersion() const { return m_ProjectVersion; }
    [[nodiscard]] auto get_OriginalProjectVersion() const { return m_OriginalProjectVersion; }
    void ForceProjectVersion(int version) { m_ProjectVersion = version; }

    // Call this after the user has been warned about saving a project file that is incompatible
    // with older versions of wxUiEditor
    void UpdateOriginalProjectVersion() { m_OriginalProjectVersion = m_ProjectVersion; }

    [[nodiscard]] auto AddOptionalComments() const -> bool
    {
        return m_project_node->as_bool(prop_optional_comments);
    }

    // Call set_ProjectUpdated() if the project file's minimum version needs to be updated
    void set_ProjectUpdated() { m_isProject_updated = true; }
    // Call is_ProjectUpdated() to determine if the project file's minimum version needs to be
    // updated
    [[nodiscard]] auto is_ProjectUpdated() const -> bool { return m_isProject_updated; }

    [[nodiscard]] auto is_UiAllowed() const -> bool { return m_allow_ui; }

    [[nodiscard]] auto get_ChildCount() const -> size_t { return m_project_node->get_ChildCount(); }

    // Returns a GEN_LANG_... enum value. Specify a node if you want to check for a folder
    // override of the language.
    auto get_CodePreference(Node* node = nullptr) const -> GenLang;

    // Returns all of the languages that are enabled for this project. The project's Code
    // Preference is always included.
    [[nodiscard]] auto get_GenerateLanguages() const -> size_t;

    // Assume major, minor, and patch have 99 possible values.
    // Returns major * 10000 + minor * 100 + patch
    // E.g., wxWidgets 3.1.6 returns 30106, 3.2.0 returns 30200
    [[nodiscard]] auto get_LangVersion(GenLang language) const -> int;

    // const wxue::string& value(GenEnum::PropName name) const { return
    // m_project_node->as_string(name); }
    [[nodiscard]] auto view(PropName name) const -> wxue::string_view
    {
        return m_project_node->as_view(name);
    }

    [[nodiscard]] auto as_string(PropName name) const -> const wxue::string&
    {
        return m_project_node->as_string(name);
    }

    [[nodiscard]] auto as_view(PropName name) const -> std::string_view
    {
        return m_project_node->as_view(name);
    }

    [[nodiscard]] auto as_bool(PropName name) const -> bool
    {
        return m_project_node->as_bool(name);
    }
    [[nodiscard]] auto as_size_t(PropName name) const -> size_t
    {
        return (to_size_t) m_project_node->as_int(name);
    }

    // Returns true if the property exists, has a value (!= wxDefaultSize, !=
    // wxDefaultPosition, or non-specified bitmap)
    [[nodiscard]] auto HasValue(PropName name) const -> bool
    {
        return m_project_node->HasValue(name);
    }

    auto LoadProject(const wxue::string& file, bool allow_ui = true) -> bool;
    auto LoadProject(pugi::xml_document& doc, bool allow_ui = true) -> NodeSharedPtr;

    auto Import(ImportXML& import, std::string& file, bool append = false, bool allow_ui = true)
        -> bool;
    auto ImportProject(std::string_view file, bool allow_ui = true) -> bool;

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
    void FinalImportCheck(Node* project, bool set_line_length = true);

    // Called by FinalImportCheck() to recursively check the node and all of it's children,
    // grandchildren, etc.
    void RecursiveNodeCheck(Node* node);

    // Assumes CollectForms() has already been called. Finds out which form should be used
    // to generate wxue() functions for embedded images.
    void FindWxueFunctions(std::vector<Node*>& forms);

    // After calling FindWxueFunctions(), this will return the form that should be used to
    // generate the one copy of wxueBundleSVG() that is used by all forms.
    [[nodiscard]] auto get_Form_BundleSVG() const -> Node* { return m_form_BundleSVG; }

    // After calling FindWxueFunctions(), this will return the form that should be used to
    // generate the one copy of wxueImage() that is used by all forms.
    [[nodiscard]] auto get_Form_Image() const -> Node* { return m_form_Image; }

    // After calling FindWxueFunctions(), this will return the form that should be used to
    // generate the one copy of wxueAnimation() that is used by all forms.
    [[nodiscard]] auto get_Form_Animation() const -> Node* { return m_form_BundleBitmaps; }

    // This will assume any ImagesList class will be the first child of the project, and will
    // either return that Node* or nullptr if no ImagesList class is found.
    [[nodiscard]] auto get_ImagesForm() -> Node*;

    // This will assume any Data class will be the first or second child of the project, and
    // will either return that Node* or nullptr if no Data class is found.
    [[nodiscard]] auto get_DataForm() -> Node*;

    // Sets project property value only if the property exists, returns false if it doesn't
    // exist.
    template <typename T>
    auto set_value(PropName name, T value) -> bool
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
    [[nodiscard]] auto ShouldOutputLanguage(const NodesFormChild& nodes,
                                            const PropName& base_file_property,
                                            GenLang language) const -> bool;

    // Helper functions for GetOutputPath complexity reduction
    [[nodiscard]] auto GetFolderOutputPath(Node* folder, GenLang language, Node*& form) const
        -> wxue::string;
    [[nodiscard]] auto GetProjectOutputPath(GenLang language) const -> wxue::string;
    [[nodiscard]] auto GetBaseFilename(Node* form, GenLang language) const -> wxue::string;
    void MergeBaseFilePath(wxue::string& result, const wxue::string& base_file) const;

    // Helper functions for FindWxueFunctions complexity reduction
    [[nodiscard]] auto AllFormTypesFound() const -> bool;
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

    // Creating the wxFileName class this way means callers don't need to include
    // wx/filename.h and all the files it includes.

    std::unique_ptr<wxFileName> m_project_path;
    std::unique_ptr<wxFileName> m_art_path;

    int m_ProjectVersion;
    int m_OriginalProjectVersion;

    bool m_allow_ui { true };
    bool m_isProject_updated { false };
};

extern ProjectHandler& Project;  // NOLINT (non-const reference) // cppcheck-suppress constReference
