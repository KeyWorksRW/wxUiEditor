/////////////////////////////////////////////////////////////////////////////
// Purpose:   ProjectHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

#include <map>
#include <mutex>
#include <thread>

#include "gen_enums.h"  // Enumerations for generators
#include "node.h"       // Node class

namespace pugi
{
    class xml_document;
}

class ImportXML;

class ProjectHandler
{
private:
    ProjectHandler() {}

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
    void SetProjectFile(const tt_wxString& file);

    // Returns the full path to the directory the project file is in
    tt_wxString ProjectPath() const { return m_projectPath; }

    // Returns the full path to the directory the project file is in
    tt_wxString ProjectFile() const { return m_projectFile; }

    // Change to the project's directory
    bool ChangeDir() const { return m_projectPath.ChangeDir(); }

    tt_wxString ArtDirectory() const;
    tt_wxString BaseDirectory(int language = GEN_LANG_CPLUSPLUS) const;
    tt_wxString DerivedDirectory() const;

    Node* ProjectNode() const { return m_project_node.get(); }
    auto& ChildNodePtrs() { return m_project_node->GetChildNodePtrs(); }
    Node* GetChild(size_t index) { return m_project_node->GetChild(index); }

    void CollectForms(std::vector<Node*>& forms, Node* node_start = nullptr);

    // Returns the first project child that is a form, or nullptr if no form children found.
    Node* GetFirstFormChild(Node* node = nullptr) const;

    // Make class and filenames unique to the project
    void FixupDuplicatedNode(Node* new_node);

    auto GetProjectVersion() { return m_ProjectVersion; }
    auto GetOriginalProjectVersion() { return m_OriginalProjectVersion; }
    void ForceProjectVersion(int version) { m_ProjectVersion = version; }
    void SetProjectUpdated() { m_isProject_updated = true; }

    bool is_UiAllowed() const { return m_allow_ui; }

    size_t ChildCount() const { return m_project_node->GetChildCount(); }

    // Returns a GEN_LANG_... enum value
    int get_PreferredLanguage();

    const tt_string& value(GenEnum::PropName name) const { return m_project_node->prop_as_string(name); }
    const tt_string_view view(PropName name) const { return m_project_node->prop_as_string(name); }
    const tt_string& as_string(PropName name) const { return m_project_node->prop_as_string(name); }
    tt_wxString as_ttString(PropName name) const { return m_project_node->prop_as_wxString(name); }

    bool as_bool(PropName name) const { return m_project_node->prop_as_bool(name); }
    size_t as_size_t(PropName name) const { return (to_size_t) m_project_node->prop_as_int(name); }

    // Returns true if the property exists, has a value (!= wxDefaultSize, !=
    // wxDefaultPosition, or non-sepcified bitmap)
    bool HasValue(PropName name) const { return m_project_node->HasValue(name); }

    bool LoadProject(const tt_wxString& file, bool allow_ui = true);
    NodeSharedPtr LoadProject(pugi::xml_document& doc, bool allow_ui = true);

    bool Import(ImportXML& import, tt_wxString& file, bool append = false, bool allow_ui = true);
    bool ImportProject(tt_wxString& file, bool allow_ui = true);

    void AppendCrafter(wxArrayString& files);
    void AppendDialogBlocks(wxArrayString& files);
    void AppendFormBuilder(wxArrayString& files);
    void AppendGlade(wxArrayString& files);
    void AppendSmith(wxArrayString& files);
    void AppendXRC(wxArrayString& files);
    void AppendWinRes(const tt_string& rc_file, std::vector<tt_string>& dialogs);

    bool NewProject(bool create_empty = false, bool allow_ui = true);

private:
    NodeSharedPtr m_project_node { nullptr };

    tt_wxString m_projectFile;
    tt_wxString m_projectPath;

    int m_ProjectVersion;
    int m_OriginalProjectVersion;

    bool m_allow_ui { true };
    bool m_isProject_updated { false };
};

extern ProjectHandler& Project;
