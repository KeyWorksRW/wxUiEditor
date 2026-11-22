/////////////////////////////////////////////////////////////////////////////
// Purpose:   Load and validate linked (included) .wxui project files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>  // for std::ranges::any_of
#include <array>      // for std::array
#include <format>     // for std::format
#include <string>     // for std::string
#include <tuple>      // for std::ignore
#include <vector>     // for std::vector

#include <wx/filename.h>  // wxFileName - encapsulates a file path

#include "linked_project.h"

#include "gen_enums.h"        // Enumerations for generators
#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator class
#include "node_prop.h"        // NodeProperty class
#include "project_handler.h"  // ProjectHandler class

#include "wxue_namespace/wxue_string.h"         // wxue::string
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector

#include "pugixml.hpp"  // pugixml parser

using namespace GenEnum;

// A linked file may never be nested deeper than this many linked levels.
constexpr int MAX_LINK_DEPTH = 4;

// Absolute paths of the linked files currently being loaded. Used to reject cycles and to limit
// nesting depth. This is transient state for one call chain only -- no parse or validation
// result is ever cached.
static std::vector<std::string> s_load_stack;

// Information gathered while validating a linked file.
struct LinkedProjectInfo
{
    int data_version { curSupportedVer };
    wxue::string art_directory;
};

// Everything the image-path rebase needs about the linked file. Passed as a single object so the
// rebase helpers do not take several identical string parameters.
struct LinkedImageContext
{
    wxue::string linked_file;
    wxue::string linked_dir;
    wxue::string link_art_dir;    // resolved to an absolute directory
    wxue::string linked_art_dir;  // resolved to an absolute directory
};

// Reports a linked-project problem. A dialog is only shown when the caller allows UI -- a
// headless load must fail without blocking on a message box.
static void ReportLinkedError(const wxue::string& message, bool allow_ui)
{
    MSG_ERROR(message.ToStdString());
    if (allow_ui)
    {
        wxMessageBox(message.wx(), "Linked Project", wxOK | wxICON_ERROR);
    }
}

// Returns true for the classes that a linked file may never contribute to the outer project.
static bool HasBlockedClass(const std::string& class_name)
{
    return wxue::is_sameas(class_name, "Images", wxue::CASE::either) ||
           wxue::is_sameas(class_name, "Data", wxue::CASE::either) ||
           wxue::is_sameas(class_name, "data_folder", wxue::CASE::either);
}

static bool IsFolderClass(const std::string& class_name)
{
    return wxue::is_sameas(class_name, "folder", wxue::CASE::either) ||
           wxue::is_sameas(class_name, "sub_folder", wxue::CASE::either);
}

// Recursively collects the class names of every node in the linked file that cannot be imported:
// Images/Data lists (they would write into the linked tree) and forms that need a derived class
// file (this feature never generates one).
static void CollectRejectedNodes(const pugi::xml_node& parent, std::vector<std::string>& rejected)
{
    for (pugi::xml_node child = parent.child("node"); child; child = child.next_sibling("node"))
    {
        const std::string class_name = child.attribute("class").as_str();
        if (class_name.empty())
        {
            rejected.emplace_back("<missing class>");
            continue;
        }

        if (HasBlockedClass(class_name))
        {
            rejected.emplace_back(class_name);
            continue;
        }

        if (child.attribute("use_derived_class").as_bool() &&
            !child.attribute("derived_file").as_sview().empty())
        {
            rejected.emplace_back(class_name);
            continue;
        }

        if (IsFolderClass(class_name))
        {
            CollectRejectedNodes(child, rejected);
        }
    }
}

// Re-reads path from disk and validates it. Always parses the file -- nothing about a previous
// load is reused.
static bool ValidateLinkedProject(const wxue::string& path, pugi::xml_document& xml_doc,
                                  LinkedProjectInfo& info, wxue::string& error)
{
    const pugi::xml_parse_result parse_result = xml_doc.load_file_string(path);
    if (!parse_result)
    {
        error << path << " could not be parsed: " << parse_result.description();
        return false;
    }

    const pugi::xml_node root = xml_doc.first_child();
    if (!root || !wxue::is_sameas(root.name(), "wxUiEditorData", wxue::CASE::either))
    {
        error << path << " is not a wxUiEditor XML file.";
        return false;
    }

    const pugi::xml_node project_node = root.child("node");
    if (!project_node)
    {
        error << path << " does not contain a project node.";
        return false;
    }

    info.data_version = root.attribute("data_version").as_int(curSupportedVer);
    if (info.data_version > curSupportedVer)
    {
        error << path << " was created by a newer version of wxUiEditor (data version "
              << info.data_version << ").";
        return false;
    }

    std::vector<std::string> rejected;
    CollectRejectedNodes(project_node, rejected);
    if (!rejected.empty())
    {
        error << path << " contains content that cannot be linked: ";
        for (size_t index = 0; index < rejected.size(); ++index)
        {
            if (index > 0)
            {
                error << ", ";
            }
            error << rejected[index];
        }
        return false;
    }

    info.art_directory = project_node.attribute("art_directory").as_sview();
    return true;
}

// Returns true when a node of this class may be imported into a folder.
static bool IsImportableClass(const std::string& class_name)
{
    if (class_name.empty())
    {
        return false;
    }

    // Use the reverse name map directly -- get_NodeDeclaration() asserts on an unknown class,
    // and an external file may legitimately contain a class this build does not know about.
    const std::map<std::string_view, GenName, std::less<>>::const_iterator iter =
        rmap_GenNames.find(class_name);
    if (iter == rmap_GenNames.end())
    {
        return false;
    }

    NodeDeclaration* declaration = NodeCreation.get_declaration(iter->second);
    if (!declaration)
    {
        return false;
    }

    const NodeSharedPtr node = NodeCreation.NewNode(declaration);
    return node && (node->is_Form() || node->is_Folder());
}

// Resolves a path that may be relative to base_dir into an absolute, forward-slash path.
static wxue::string ToAbsolutePath(const wxue::string& path, const wxue::string& base_dir)
{
    wxFileName file(path.wx());
    if (!file.IsAbsolute())
    {
        file.MakeAbsolute(base_dir.wx());
    }
    file.Normalize(wxPATH_NORM_DOTS);

    wxue::string result = file.GetFullPath().ToStdString();
    result.backslashestoforward();
    return result;
}

// Resolves an art directory, which may be relative to base_dir. "." and "./" mean base_dir.
static wxue::string ToAbsoluteDirectory(const wxue::string& dir, const wxue::string& base_dir)
{
    wxue::string path = dir;
    path.backslashestoforward();

    if (path.empty() || path == "." || path == "./")
    {
        return base_dir;
    }

    if (!wxFileName(path.wx()).IsAbsolute())
    {
        wxue::string absolute = base_dir;
        absolute.append_filename(path);
        path = absolute;
    }

    return path;
}

// Resolves a stored image path and rewrites it relative to the outer project. Returns an empty
// string when the path cannot be resolved or cannot be expressed relative to the project -- an
// absolute path is never stored in a property.
static wxue::string ResolveAndRebaseImagePath(const wxue::string& stored_path,
                                              const LinkedImageContext& context)
{
    wxue::string resolved;

    // 1. as given
    const wxFileName as_given(stored_path.wx());
    if (as_given.FileExists())
    {
        resolved = as_given.GetFullPath().ToStdString();
        resolved.backslashestoforward();
    }

    // 2/3. against the link folder's art directory, then the linked project's art directory
    if (resolved.empty())
    {
        for (const wxue::string* art_dir: { &context.link_art_dir, &context.linked_art_dir })
        {
            if (art_dir->empty())
            {
                continue;
            }

            wxue::string candidate = *art_dir;
            candidate.append_filename(stored_path);
            if (candidate.file_exists())
            {
                resolved = candidate;
                break;
            }
        }
    }

    // 4. against the linked project's own directory
    if (resolved.empty())
    {
        wxue::string candidate = context.linked_dir;
        candidate.append_filename(stored_path);
        if (candidate.file_exists())
        {
            resolved = candidate;
        }
    }

    if (resolved.empty())
    {
        MSG_WARNING(std::format("Linked project {}: could not resolve image path \"{}\"",
                                context.linked_file.ToStdString(), stored_path.ToStdString()));
        return {};
    }

    wxFileName rebased_file(resolved.wx());
    rebased_file.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE);
    rebased_file.MakeRelativeTo(Project.get_ProjectPath().wx());
    if (rebased_file.IsAbsolute())
    {
        MSG_WARNING(std::format(
            "Linked project {}: image path \"{}\" cannot be made relative to the project",
            context.linked_file.ToStdString(), stored_path.ToStdString()));
        return {};
    }

    wxue::string rebased = rebased_file.GetFullPath().ToStdString();
    rebased.backslashestoforward();
    return rebased;
}

// Rebases the image, animation and bitmap-combo-item paths of one node. Called for every node of
// the adopted subtree.
static void RebaseImagePropsForNode(Node* node, const LinkedImageContext& context)
{
    for (NodeProperty& prop: node->get_PropsVector())
    {
        if (prop.type() == type_image || prop.type() == type_animation)
        {
            if (!prop.HasValue())
            {
                continue;
            }

            const wxue::string original = prop.as_string();
            wxue::StringVector parts(original, BMP_PROP_SEPARATOR, wxue::TRIM::both);
            if (parts.size() <= IndexImage || parts[IndexType] == "Art")
            {
                continue;
            }

            const wxue::string stored_path = parts[IndexImage];
            if (stored_path.empty())
            {
                continue;
            }

            const wxue::string rebased = ResolveAndRebaseImagePath(stored_path, context);
            if (rebased.empty() || rebased == stored_path)
            {
                continue;
            }

            // Rebuild using the original part text so size information is preserved.
            wxue::string new_value = parts[IndexType];
            new_value << BMP_PROP_SEPARATOR << rebased;
            for (size_t index = IndexSize; index < parts.size(); ++index)
            {
                new_value << BMP_PROP_SEPARATOR << parts[index];
            }

            if (new_value != original)
            {
                prop.set_value(new_value);
            }
        }
        else if (prop.type() == type_bmp_combo_item)
        {
            std::vector<NODEPROP_BMP_COMBO_ITEM> items = prop.as_bmp_combo_items();
            bool items_changed = false;

            for (auto& item: items)
            {
                if (item.bitmap.empty() || item.bitmap.starts_with("Art"))
                {
                    continue;
                }

                const wxue::string rebased = ResolveAndRebaseImagePath(item.bitmap, context);
                if (!rebased.empty() && rebased != item.bitmap)
                {
                    item.bitmap = rebased;
                    items_changed = true;
                }
            }

            if (items_changed)
            {
                prop.set_value(items);
            }
        }
    }

    for (const auto& child: node->get_ChildNodePtrs())
    {
        RebaseImagePropsForNode(child.get(), context);
    }
}

// Rebases every image path below folder (the newly adopted linked content) so the rest of the
// session sees project-relative values.
static void RebaseLinkedImagePaths(Node* folder, const LinkedImageContext& context)
{
    for (const auto& child: folder->get_ChildNodePtrs())
    {
        RebaseImagePropsForNode(child.get(), context);
    }
}

// Restores the project version state after a linked file has been loaded, so the included file's
// data version drives CreateNodeFromXml() fixups without ever marking the outer project modified.
class LinkedVersionGuard
{
public:
    explicit LinkedVersionGuard(int version) :
        m_saved_version(Project.get_ProjectVersion()),
        m_saved_original(Project.get_OriginalProjectVersion()),
        m_saved_updated(Project.is_ProjectUpdated())
    {
        Project.ForceProjectVersion(version);
        Project.ForceOriginalProjectVersion(version);
        Project.set_ProjectUpdated(false);
    }

    ~LinkedVersionGuard()
    {
        Project.ForceProjectVersion(m_saved_version);
        Project.ForceOriginalProjectVersion(m_saved_original);
        Project.set_ProjectUpdated(m_saved_updated);
    }

    LinkedVersionGuard(const LinkedVersionGuard&) = delete;
    LinkedVersionGuard& operator=(const LinkedVersionGuard&) = delete;
    LinkedVersionGuard(LinkedVersionGuard&&) = delete;
    LinkedVersionGuard& operator=(LinkedVersionGuard&&) = delete;

private:
    int m_saved_version;
    int m_saved_original;
    bool m_saved_updated;
};

// Pushes/pops the currently loading linked file for cycle detection and depth limiting.
class LoadStackGuard
{
public:
    explicit LoadStackGuard(const std::string& path) { s_load_stack.emplace_back(path); }

    ~LoadStackGuard() { s_load_stack.pop_back(); }

    LoadStackGuard(const LoadStackGuard&) = delete;
    LoadStackGuard& operator=(const LoadStackGuard&) = delete;
    LoadStackGuard(LoadStackGuard&&) = delete;
    LoadStackGuard& operator=(LoadStackGuard&&) = delete;
};

// A link folder declared inside a linked file. Its adopted node is a sub-folder, which has no
// link_project property, so the resolved path is remembered here and loaded after adoption.
struct NestedLink
{
    Node* folder { nullptr };
    wxue::string linked_file;
    wxue::string link_art_dir;
};

// Adopts the forms and folders of the linked project node as children of folder. Nested link
// folders are recorded in nested_links -- their content is loaded after this function returns.
static bool AdoptLinkedNodes(const pugi::xml_node& project_node, Node* folder, bool allow_ui,
                             std::vector<NestedLink>& nested_links)
{
    try
    {
        for (pugi::xml_node child = project_node.child("node"); child;
             child = child.next_sibling("node"))
        {
            std::string class_name = child.attribute("class").as_str();

            wxue::string nested_link;
            wxue::string nested_art_dir;

            // A folder cannot contain another folder, so the linked file's top-level folders are
            // adopted as sub-folders of the link folder. Nested folders in a linked file are
            // already sub-folders.
            if (wxue::is_sameas(class_name, "folder", wxue::CASE::either))
            {
                nested_link = child.attribute("link_project").as_sview();
                nested_art_dir = child.attribute("art_directory").as_sview();

                std::ignore = child.attribute("class").set_value("sub_folder");
                class_name = "sub_folder";

                // A sub-folder has none of the link properties. Remove them so loading the node
                // does not report them as unrecognized properties.
                std::ignore = child.remove_attribute("link_project");
                std::ignore = child.remove_attribute("link_generate_code");
                std::ignore = child.remove_attribute("art_directory");
            }

            if (!IsImportableClass(class_name))
            {
                MSG_WARNING(std::format("Linked project: skipping unsupported node class \"{}\"",
                                        class_name));
                continue;
            }

            const NodeSharedPtr new_node =
                NodeCreation.CreateNodeFromXml(child, folder, false, allow_ui);
            if (!new_node)
            {
                return false;
            }

            if (!nested_link.empty())
            {
                NestedLink record;
                record.folder = new_node.get();
                record.linked_file = ToAbsolutePath(nested_link, Project.get_ProjectPath());
                record.link_art_dir = nested_art_dir;
                nested_links.emplace_back(record);
            }
        }
    }
    catch (const std::exception& err)
    {
        MSG_ERROR(std::format("Linked project: failed to create node: {}", err.what()));
        return false;
    }

    return true;
}

// Compares the just-adopted forms against the native forms. A collision makes the whole link
// invalid, since two forms may not generate the same file or class.
static bool CheckForNameConflicts(const wxue::string& linked_file, bool allow_ui)
{
    std::vector<Node*> forms;
    Project.CollectForms(forms);

    std::vector<const Node*> linked_forms;
    std::vector<const Node*> native_forms;
    for (const Node* form: forms)
    {
        if (form->get_LinkedProjectFolder())
        {
            linked_forms.emplace_back(form);
        }
        else
        {
            native_forms.emplace_back(form);
        }
    }

    if (linked_forms.empty() || native_forms.empty())
    {
        return true;
    }

    static constexpr std::array<PropName, 13> name_properties = {
        prop_class_name,      prop_base_file,   prop_derived_class_name, prop_derived_file,
        prop_fortran_file,    prop_go_file,     prop_julia_file,         prop_lua_file,
        prop_typescript_file, prop_python_file, prop_ruby_file,          prop_xrc_file,
        prop_rust_file
    };

    wxue::string conflicts;
    for (const Node* linked_form: linked_forms)
    {
        for (const PropName name: name_properties)
        {
            if (!linked_form->HasValue(name))
            {
                continue;
            }

            const wxue::string& linked_value = linked_form->as_string(name);
            for (const Node* native_form: native_forms)
            {
                if (!native_form->HasValue(name))
                {
                    continue;
                }

                if (wxue::is_sameas(native_form->as_string(name), linked_value, wxue::CASE::either))
                {
                    if (!conflicts.empty())
                    {
                        conflicts << ", ";
                    }
                    conflicts << linked_value;
                    break;
                }
            }
        }
    }

    if (conflicts.empty())
    {
        return true;
    }

    ReportLinkedError(wxue::string() << "Linked project " << linked_file
                                     << " conflicts with existing forms: " << conflicts,
                      allow_ui);
    return false;
}

// A linked file plus the art-directory override declared on the folder that references it.
struct LinkSource
{
    wxue::string file;     // absolute path to the linked .wxui
    wxue::string art_dir;  // value of the link folder's art_directory property (may be empty)
};

// Asks the user before the folder's existing children are replaced. Returns false when the load
// must be aborted. Nothing is asked when allow_ui is false or the folder is empty, so a headless
// or silent reload can never block on a dialog.
static bool ConfirmReplaceChildren(Node* folder, bool allow_ui)
{
    if (!allow_ui || folder->get_ChildCount() == 0)
    {
        return true;
    }

    const wxue::string message =
        wxue::string() << "Linking " << folder->as_string(prop_label) << " will discard the "
                       << folder->get_ChildCount() << " node(s) it currently contains.\n\n"
                       << "Do you want to replace them with the contents of the linked file?";

    return wxMessageBox(message.wx(), "Linked Project",
                        wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION) == wxYES;
}

// Loads source into folder. The file is re-read from disk and fully re-validated on every call.
static bool LoadLinkedProjectInto(Node* folder, const LinkSource& source, bool allow_ui,
                                  LinkReload reload)
{
    if (!folder || source.file.empty())
    {
        return false;
    }

    const wxue::string& linked_file = source.file;

    if (!linked_file.file_exists())
    {
        ReportLinkedError(wxue::string() << "Linked project file not found: " << linked_file,
                          allow_ui);
        return false;
    }

    if (s_load_stack.size() >= static_cast<size_t>(MAX_LINK_DEPTH))
    {
        ReportLinkedError(wxue::string() << linked_file << " is nested more than " << MAX_LINK_DEPTH
                                         << " linked levels deep, which is not supported.",
                          allow_ui);
        return false;
    }

    if (std::ranges::any_of(s_load_stack,
                            [&linked_file](const std::string& entry)
                            {
                                return wxue::is_sameas(entry, linked_file, wxue::CASE::either);
                            }))
    {
        ReportLinkedError(wxue::string() << linked_file << " produces a cyclic link.", allow_ui);
        return false;
    }

    pugi::xml_document linked_doc;
    LinkedProjectInfo info;
    wxue::string error;
    if (!ValidateLinkedProject(linked_file, linked_doc, info, error))
    {
        ReportLinkedError(error, allow_ui);
        return false;
    }

    // Ask before the existing children are discarded. The prompt comes after validation so an
    // invalid file never destroys the currently loaded subtree.
    if (reload == LinkReload::interactive && !ConfirmReplaceChildren(folder, allow_ui))
    {
        return false;
    }

    // A reload must never duplicate children.
    UnloadLinkedProjectNode(folder);

    const LoadStackGuard stack_guard(linked_file.ToStdString());
    const LinkedVersionGuard version_guard(info.data_version);

    const pugi::xml_node project_node = linked_doc.first_child().child("node");

    std::vector<NestedLink> nested_links;
    if (!AdoptLinkedNodes(project_node, folder, allow_ui, nested_links))
    {
        UnloadLinkedProjectNode(folder);
        return false;
    }

    LinkedImageContext image_context;
    image_context.linked_file = linked_file;
    image_context.linked_dir = wxFileName(linked_file.wx()).GetPath().ToStdString();
    image_context.linked_dir.backslashestoforward();
    if (!info.art_directory.empty())
    {
        image_context.linked_art_dir =
            ToAbsoluteDirectory(info.art_directory, image_context.linked_dir);
    }
    if (!source.art_dir.empty())
    {
        image_context.link_art_dir = ToAbsoluteDirectory(source.art_dir, Project.get_ProjectPath());
    }

    RebaseLinkedImagePaths(folder, image_context);

    // A nested link is independent: its failure leaves that sub-folder empty without discarding
    // the content of this linked file.
    for (const NestedLink& nested: nested_links)
    {
        LinkSource nested_source;
        nested_source.file = nested.linked_file;
        nested_source.art_dir = nested.link_art_dir;
        // A nested folder was just created by this load, so it has no children to confirm.
        std::ignore =
            LoadLinkedProjectInto(nested.folder, nested_source, allow_ui, LinkReload::silent);
    }

    if (!CheckForNameConflicts(linked_file, allow_ui))
    {
        UnloadLinkedProjectNode(folder);
        return false;
    }

    return true;
}

bool LoadLinkedProjectNode(Node* folder, bool allow_ui, LinkReload reload)
{
    if (!folder || !folder->is_LinkedProject())
    {
        return false;
    }

    LinkSource source;
    source.file = ToAbsolutePath(folder->as_string(prop_link_project), Project.get_ProjectPath());
    if (folder->HasValue(prop_art_directory))
    {
        source.art_dir = folder->as_string(prop_art_directory);
    }

    return LoadLinkedProjectInto(folder, source, allow_ui, reload);
}

void UnloadLinkedProjectNode(Node* folder)
{
    if (!folder)
    {
        return;
    }

    // Copy the shared pointers so the nodes stay alive while they are removed.
    const std::vector<NodeSharedPtr> children = folder->get_ChildNodePtrs();
    for (const auto& child: children)
    {
        folder->RemoveChild(child.get());
    }
}

static bool LoadLinksBelow(Node* node, bool allow_ui)
{
    bool all_loaded = true;

    for (const auto& child: node->get_ChildNodePtrs())
    {
        if (child->is_LinkedProject())
        {
            // Loading at project-open time never prompts -- the children are not serialized, so
            // there is nothing the user could have chosen to keep.
            if (!LoadLinkedProjectNode(child.get(), allow_ui, LinkReload::silent))
            {
                all_loaded = false;
            }
        }
        else if (child->is_Folder())
        {
            if (!LoadLinksBelow(child.get(), allow_ui))
            {
                all_loaded = false;
            }
        }
    }

    return all_loaded;
}

bool LoadAllLinkedProjects(bool allow_ui)
{
    Node* project_node = Project.get_ProjectNode();
    if (!project_node)
    {
        return false;
    }

    return LoadLinksBelow(project_node, allow_ui);
}

// Collects every folder that references a linked file. Nested links are adopted as sub-folders
// that carry no link_project property, so only top-level link folders are collected -- reloading
// one reloads its nested links as well.
static void CollectLinkFolders(Node* node, std::vector<Node*>& link_folders)
{
    for (const auto& child: node->get_ChildNodePtrs())
    {
        if (child->is_LinkedProject())
        {
            link_folders.emplace_back(child.get());
        }
        else if (child->is_Folder())
        {
            CollectLinkFolders(child.get(), link_folders);
        }
    }
}

bool ValidateAllLinkedProjects(std::vector<Node*>& invalid_folders,
                               std::vector<wxue::string>& errors)
{
    Node* project_node = Project.get_ProjectNode();
    if (!project_node)
    {
        return true;
    }

    std::vector<Node*> link_folders;
    CollectLinkFolders(project_node, link_folders);

    bool all_valid = true;
    for (size_t index = 0; index < link_folders.size(); ++index)
    {
        const Node* folder = link_folders[index];
        const wxue::string linked_file =
            ToAbsolutePath(folder->as_string(prop_link_project), Project.get_ProjectPath());

        // Re-read the file from disk and run the same validation used by the loader. No parsed
        // document or validation result is reused here.
        pugi::xml_document linked_doc;
        LinkedProjectInfo info;
        wxue::string error;
        if (!ValidateLinkedProject(linked_file, linked_doc, info, error))
        {
            invalid_folders.emplace_back(link_folders[index]);
            errors.emplace_back(error);
            all_valid = false;
        }
    }

    return all_valid;
}
