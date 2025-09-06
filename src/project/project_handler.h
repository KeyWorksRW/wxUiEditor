/////////////////////////////////////////////////////////////////////////////
// Purpose:   ProjectHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

#include <utility>  // for pair<>

#include "gen_enums.h"  // Enumerations for generators
#include "node.h"       // Node class

namespace pugi
{
    class xml_document;
}

class ImportXML;

enum
{
    OUTPUT_NONE = 0,
    OUTPUT_CPLUS = 1 << 0,
    OUTPUT_DERIVED = 1 << 1,
    OUTPUT_PYTHON = 1 << 2,
    OUTPUT_RUBY = 1 << 3,
    OUTPUT_XRC = 1 << 4,
    OUTPUT_PERL = 1 << 5,
    OUTPUT_RUST = 1 << 6,
};

enum
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
    void set_ProjectFile(const tt_string& file);
    void set_ProjectPath(const wxFileName* path);

    // Returns the full path to the directory the project file is in
    tt_string get_ProjectPath() const;

    // Returns the full path to the directory the project file is in
    tt_string get_wxProjectPath() const;

    // Returns the full path to the project filename
    tt_string get_ProjectFile() const;

    const wxFileName* get_wxFileName() const;

    // Get a bit flag indicating which output types are enabled.
    //
    // OUTPUT_DERIVED is only set if the file is specified and does *not* exist.
    size_t get_OutputType(int flags = OUT_FLAG_NONE) const;

    // Change to the project's directory
    bool ChangeDir() const;

    tt_string ArtDirectory();
    const wxFileName* get_ArtPath();

    // If the node is within a folder, and the folder specifies a directory, then that
    // directory is returned. Otherwise the project base directory is returned.
    tt_string get_BaseDirectory(Node* node, GenLang language = GEN_LANG_CPLUSPLUS) const;

    // Returns the absolute path to the output file for this node. If no output filename is
    // specified, first will still contain a path with no filename, and second will be false.
    std::pair<tt_string, bool> GetOutputPath(Node* form,
                                             GenLang language = GEN_LANG_CPLUSPLUS) const;

    // If the node is within a folder, and the folder specifies a directory, then that
    // directory is returned. Otherwise the project derived directory is returned.
    tt_string get_DerivedDirectory(Node* node, GenLang language = GEN_LANG_CPLUSPLUS) const;

    // Returns the full path to the derived filename or an empty string if no derived file
    // was specified.
    tt_string get_DerivedFilename(Node*) const;

    Node* get_ProjectNode() const { return m_project_node.get(); }
    auto& get_ChildNodePtrs() { return m_project_node->get_ChildNodePtrs(); }
    Node* get_Child(size_t index) { return m_project_node->get_Child(index); }

    // This includes forms in folders and sub-folders
    void CollectForms(std::vector<Node*>& forms, Node* node_start = nullptr);

    // Returns the first project child that is a form, or nullptr if no form children found.
    Node* get_FirstFormChild(Node* node = nullptr) const;

    // Make class and filenames unique to the project
    void FixupDuplicatedNode(Node* new_node);

    auto get_ProjectVersion() const { return m_ProjectVersion; }
    auto get_OriginalProjectVersion() const { return m_OriginalProjectVersion; }
    void ForceProjectVersion(int version) { m_ProjectVersion = version; }

    // Call this after the user has been warned about saving a project file that is incompatible
    // with older versions of wxUiEditor
    void UpdateOriginalProjectVersion() { m_OriginalProjectVersion = m_ProjectVersion; }

    bool AddOptionalComments() const { return m_project_node->as_bool(prop_optional_comments); }

    // Call set_ProjectUpdated() if the project file's minimum version needs to be updated
    void set_ProjectUpdated() { m_isProject_updated = true; }
    // Call is_ProjectUpdated() to determine if the project file's minimum version needs to be
    // updated
    bool is_ProjectUpdated() const { return m_isProject_updated; }

    bool is_UiAllowed() const { return m_allow_ui; }

    size_t get_ChildCount() const { return m_project_node->get_ChildCount(); }

    // Returns a GEN_LANG_... enum value. Specify a node if you want to check for a folder
    // override of the language.
    GenLang get_CodePreference(Node* node = nullptr) const;

    // Returns all of the languages that are enabled for this project. The project's Code
    // Preference is always included.
    size_t get_GenerateLanguages() const;

    // Assume major, minor, and patch have 99 possible values.
    // Returns major * 10000 + minor * 100 + patch
    // E.g., wxWidgets 3.1.6 returns 30106, 3.2.0 returns 30200
    int get_LangVersion(GenLang language) const;

    // const tt_string& value(GenEnum::PropName name) const { return
    // m_project_node->as_string(name); }
    const tt_string_view view(PropName name) const { return m_project_node->as_string(name); }
    const tt_string& as_string(PropName name) const { return m_project_node->as_string(name); }

    bool as_bool(PropName name) const { return m_project_node->as_bool(name); }
    size_t as_size_t(PropName name) const { return (to_size_t) m_project_node->as_int(name); }

    // Returns true if the property exists, has a value (!= wxDefaultSize, !=
    // wxDefaultPosition, or non-sepcified bitmap)
    bool HasValue(PropName name) const { return m_project_node->HasValue(name); }

    bool LoadProject(const tt_string& file, bool allow_ui = true);
    NodeSharedPtr LoadProject(pugi::xml_document& doc, bool allow_ui = true);

    bool Import(ImportXML& import, tt_string& file, bool append = false, bool allow_ui = true);
    bool ImportProject(tt_string& file, bool allow_ui = true);

    void AppendCrafter(wxArrayString& files);
    void AppendDialogBlocks(wxArrayString& files);
    void AppendFormBuilder(wxArrayString& files);
    void AppendGlade(wxArrayString& files);
    void AppendSmith(wxArrayString& files);
    void AppendXRC(wxArrayString& files);
    void AppendWinRes(const tt_string& rc_file, std::vector<tt_string>& dialogs);

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
    Node* get_Form_BundleSVG() const { return m_form_BundleSVG; }

    // After calling FindWxueFunctions(), this will return the form that should be used to
    // generate the one copy of wxueImage() that is used by all forms.
    Node* get_Form_Image() const { return m_form_Image; }

    // After calling FindWxueFunctions(), this will return the form that should be used to
    // generate the one copy of wxueAnimation() that is used by all forms.
    Node* get_Form_Animation() const { return m_form_BundleBitmaps; }

    // This will assume any ImagesList class will be the first child of the project, and will
    // either return that Node* or nullptr if no ImagesList class is found.
    Node* get_ImagesForm();

    // This will assume any Data class will be the first or second child of the project, and
    // will either return that Node* or nullptr if no Data class is found.
    Node* get_DataForm();

    // Sets project property value only if the property exists, returns false if it doesn't
    // exist.
    template <typename T>
    bool set_value(PropName name, T value)
    {
        if (auto prop = m_project_node->get_PropPtr(name); prop)
        {
            prop->set_value(value);
            return true;
        }
        else
        {
            return false;
        }
    }

private:
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

extern ProjectHandler& Project;
