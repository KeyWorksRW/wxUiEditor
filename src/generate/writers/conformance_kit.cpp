/////////////////////////////////////////////////////////////////////////////
// Purpose:   Write the kwx conformance kit (one minimal .wxui per generator class)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/*
    The kit is generated output: .tts/conformance.sh regenerates it before every sweep and it
    must never be hand-edited.

    Every file is derived from the compiled generator library only:

        1. the class node is a generator declaration that is not an interface;
        2. its parent chain runs from a project child the Project node allows down to the class,
           where each link is proven by NodeDeclaration::get_AllowableChildren();
        3. its properties are written only when the declaration actually has them, so a value can
           never name a property the node does not support;
        4. its single child is the first preferred child type the class allows.

    Output is deterministic: the class list is sorted by declaration name, property order is fixed,
    and nothing time-dependent is written.
*/

#include <algorithm>
#include <array>
#include <cstddef>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <wx/dir.h>       // wxDir
#include <wx/filefn.h>    // wxDirExists, wxRemoveFile
#include <wx/filename.h>  // wxFileName::Mkdir
#include <wx/log.h>       // wxLogStderr
#include <wx/wfstream.h>  // wxFFile

#include "conformance_kit.h"

#include "gen_enums.h"     // GenType
#include "node_creator.h"  // NodeCreation
#include "node_decl.h"     // NodeDeclaration
#include "prop_decl.h"     // PropDeclaration

// Candidate forms, in preference order. `Project` allows each of these as a project child.
static constexpr std::array<std::string_view, 3> FORM_CANDIDATES = { "wxFrame", "wxDialog",
                                                                     "PanelForm" };

// The one child a generated class gets, in preference order. The first entry the class allows is
// used; a class that allows none is written without a child and recorded as `child: null`.
static constexpr std::array<std::string_view, 5> PREFERRED_CHILDREN = { "wxTextCtrl", "wxButton",
                                                                        "wxStaticText", "wxPanel",
                                                                        "wxBoxSizer" };

static constexpr size_t MAX_CHAIN_DEPTH = 6;

struct KitEntry
{
    std::string class_name;
    std::string form_class;
    std::vector<std::string> chain;  // child links below the form, ending with class_name
    std::string child_class;         // empty when the class allows no preferred child
    bool form_is_form { false };     // write class_name on the outermost node
};

struct OmittedEntry
{
    std::string class_name;
    std::string reason;
};

static std::optional<const NodeDeclaration*> FindDeclaration(std::string_view declaration_name)
{
    for (const NodeDeclaration* declaration: NodeCreation.get_NodeDeclarationArray())
    {
        if (declaration != nullptr && declaration->get_DeclName() == declaration_name)
        {
            return declaration;
        }
    }

    return {};
}

// Two child types are additionally gated by the generator, not just by the parent type: see the
// comments in node_init.cpp. The generator library's type map is coarser than the loader's gate, so
// the kit has to apply the same restriction or the generated project is rejected at load time.
static bool IsGatedPair(const NodeDeclaration* parent, const NodeDeclaration* child)
{
    const std::string_view parent_name = parent->get_DeclName();
    const GenType child_type = child->get_GenType();

    if (child_type == GenType::type_treepage)
    {
        return parent_name != "wxTreebook";
    }
    if (child_type == GenType::type_infobarbtn)
    {
        return parent_name != "wxInfoBar";
    }

    return false;
}

// `get_AllowableChildren()` returns the maximum child count, where `child_count::infinite` is -1,
// so any non-zero value means the child type is allowed.
static bool AllowsChild(const NodeDeclaration* parent, const NodeDeclaration* child)
{
    if (parent == nullptr || child == nullptr)
    {
        return false;
    }

    if (IsGatedPair(parent, child))
    {
        return false;
    }

    return parent->get_AllowableChildren(child->get_GenType()) != 0;
}

// Intermediate nodes must be containers. Forms, interfaces and leaf node types are excluded so a
// chain never routes a widget through a form or through a pseudo node such as `sizeritem`.
static bool IsContainerType(GenType type)
{
    switch (type)
    {
        case GenEnum::type_widget:
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
        case GenEnum::type_toolbar:
        case GenEnum::type_aui_toolbar:
        case GenEnum::type_ribbonbar:
        case GenEnum::type_ribbonpage:
        case GenEnum::type_ribbonpanel:
        case GenEnum::type_ribbonbuttonbar:
        case GenEnum::type_ribbontoolbar:
        case GenEnum::type_ribbongallery:
        case GenEnum::type_dataviewctrl:
        case GenEnum::type_dataviewlistctrl:
        case GenEnum::type_dataviewtreectrl:
        case GenEnum::type_propgrid:
        case GenEnum::type_propgridman:
        case GenEnum::type_propgridpage:
        case GenEnum::type_treelistctrl:
        case GenEnum::type_wizard:
        case GenEnum::type_wizardpagesimple:
        case GenEnum::type_infobarbtn:
        case GenEnum::type_ctx_menu:
        case GenEnum::type_menu:
        case GenEnum::type_menubar:
        case GenEnum::type_menuitem:
        case GenEnum::type_submenu:
        case GenEnum::type_statusbar:
        case GenEnum::type_sizer:
        case GenEnum::type_DocViewApp:
        case GenEnum::type_wx_document:
            return true;

        default:
            return false;
    }
}

// A form declaration owns a class and is instantiated by the generated form entry. Data/Images and
// folders are project children too, but they are not forms.
static bool IsFormType(GenType type)
{
    switch (type)
    {
        case GenEnum::type_form:
        case GenEnum::type_frame_form:
        case GenEnum::type_panel_form:
        case GenEnum::type_menubar_form:
        case GenEnum::type_ribbonbar_form:
        case GenEnum::type_toolbar_form:
        case GenEnum::type_aui_toolbar_form:
        case GenEnum::type_propsheetform:
        case GenEnum::type_wizard:
            return true;

        default:
            return false;
    }
}

// Breadth-first search for the shortest child chain from `form` down to `target`. Returns the chain
// below the form (ending with `target`), or an empty vector when no chain exists within
// MAX_CHAIN_DEPTH. Ties are broken by declaration name so the result is deterministic.
static std::vector<const NodeDeclaration*> FindChain(const NodeDeclaration* form,
                                                      const NodeDeclaration* target)
{
    struct QueueItem
    {
        const NodeDeclaration* node { nullptr };
        std::vector<const NodeDeclaration*> chain;
    };

    std::vector<QueueItem> queue;
    queue.emplace_back(QueueItem { form, {} });

    std::set<std::string, std::less<>> visited;
    visited.emplace(form->get_DeclName());

    // Index-based loop: the queue grows while it is walked.
    for (size_t index = 0; index < queue.size(); ++index)
    {
        const QueueItem current = queue[index];

        std::vector<const NodeDeclaration*> candidates;
        for (const NodeDeclaration* declaration: NodeCreation.get_NodeDeclarationArray())
        {
            if (AllowsChild(current.node, declaration))
            {
                candidates.emplace_back(declaration);
            }
        }

        std::ranges::sort(candidates,
                          [](const NodeDeclaration* left, const NodeDeclaration* right)
                          { return left->get_DeclName() < right->get_DeclName(); });

        for (const NodeDeclaration* candidate: candidates)
        {
            std::vector<const NodeDeclaration*> chain = current.chain;
            chain.emplace_back(candidate);

            if (candidate == target)
            {
                return chain;
            }

            if (chain.size() >= MAX_CHAIN_DEPTH || !IsContainerType(candidate->get_GenType()))
            {
                continue;
            }

            if (!visited.emplace(candidate->get_DeclName()).second)
            {
                continue;
            }

            queue.emplace_back(QueueItem { candidate, chain });
        }
    }

    return {};
}

static std::optional<const NodeDeclaration*> FindPreferredChild(const NodeDeclaration* declaration)
{
    for (const std::string_view child_name: PREFERRED_CHILDREN)
    {
        const std::optional<const NodeDeclaration*> child = FindDeclaration(child_name);
        if (child.has_value() && AllowsChild(declaration, child.value()))
        {
            return child;
        }
    }

    return {};
}

static bool HasProperty(const NodeDeclaration* declaration, std::string_view property_name)
{
    for (size_t index = 0; index < declaration->get_PropertyCount(); ++index)
    {
        const PropDeclaration* property = declaration->get_PropDeclaration(index);
        if (property != nullptr && property->get_DeclName() == property_name)
        {
            return true;
        }
    }

    return false;
}

static std::string MakeVarName(std::string_view declaration_name)
{
    std::string var_name = "conformance_";
    var_name += declaration_name;
    return var_name;
}

// Property attributes, in a fixed order. A property is written only when the declaration has it,
// so an unknown or unsupported property can never appear in the kit.
static std::vector<std::pair<std::string, std::string>> GetPropertyAttributes(
    const NodeDeclaration* declaration, bool is_form)
{
    std::vector<std::pair<std::string, std::string>> attributes;

    if (is_form)
    {
        if (HasProperty(declaration, "title"))
        {
            attributes.emplace_back(
                "title", std::string("Conformance ") + std::string(declaration->get_DeclName()));
        }
        if (HasProperty(declaration, "minimum_size"))
        {
            attributes.emplace_back("minimum_size", "400,300");
        }

        return attributes;
    }

    if (HasProperty(declaration, "var_name"))
    {
        attributes.emplace_back("var_name", MakeVarName(declaration->get_DeclName()));
    }
    if (HasProperty(declaration, "label"))
    {
        attributes.emplace_back("label", "Conformance");
    }
    else if (HasProperty(declaration, "value"))
    {
        attributes.emplace_back("value", "Conformance");
    }
    if (HasProperty(declaration, "orientation"))
    {
        attributes.emplace_back("orientation", "wxVERTICAL");
    }
    if (HasProperty(declaration, "minimum_size"))
    {
        attributes.emplace_back("minimum_size", "200,100");
    }

    return attributes;
}

// Per-language output files, so a generation run writes inside the kit and never into one of the
// six repositories. `.tts/conformance.sh` creates `gen/<language>/` before generating.
static std::vector<std::pair<std::string, std::string>> GetLanguageFileAttributes(
    const KitEntry& entry)
{
    static constexpr std::array<std::pair<std::string_view, std::string_view>, 6> LANGUAGES = {
        std::pair { "fortran_file", ".f90" }, std::pair { "go_file", ".go" },
        std::pair { "julia_file", ".jl" },    std::pair { "lua_file", ".lua" },
        std::pair { "rust_file", ".rs" },     std::pair { "typescript_file", ".ts" }
    };

    std::vector<std::pair<std::string, std::string>> attributes;
    for (const std::pair<std::string_view, std::string_view>& language_pair: LANGUAGES)
    {
        const std::string language(language_pair.first.substr(0, language_pair.first.find('_')));
        attributes.emplace_back(std::string(language_pair.first),
                                "gen/" + language + "/" + entry.class_name + "_" +
                                    entry.form_class + std::string(language_pair.second));
    }

    return attributes;
}

static void AppendNodeOpen(
    std::string& xml_text, const NodeDeclaration* declaration, bool is_form, size_t indent,
    const std::vector<std::pair<std::string, std::string>>& extra_attributes = {})
{
    const std::string padding(indent, ' ');
    xml_text += padding + "<node\n";
    xml_text += padding + "  class=\"" + std::string(declaration->get_DeclName()) + "\"";
    if (is_form)
    {
        xml_text += "\n" + padding + "  class_name=\"Conformance" +
                    std::string(declaration->get_DeclName()) + "\"";
    }

    for (const std::pair<std::string, std::string>& attribute:
         GetPropertyAttributes(declaration, is_form))
    {
        xml_text += "\n" + padding + "  " + attribute.first + "=\"" + attribute.second + "\"";
    }

    for (const std::pair<std::string, std::string>& attribute: extra_attributes)
    {
        xml_text += "\n" + padding + "  " + attribute.first + "=\"" + attribute.second + "\"";
    }
}

// Builds one kit file. Returns an empty string when a declaration the entry names cannot be
// resolved (the caller turns that into a failure rather than writing a partial form).
static std::string BuildKitFile(const KitEntry& entry)
{
    std::vector<std::string> path { entry.form_class };
    path.append_range(entry.chain);

    for (const std::string& declaration_name: path)
    {
        if (!FindDeclaration(declaration_name).has_value())
        {
            return {};
        }
    }
    if (!entry.child_class.empty() && !FindDeclaration(entry.child_class).has_value())
    {
        return {};
    }

    std::string xml_text = "<?xml version=\"1.0\"?>\n";
    xml_text += "<wxUiEditorData\n  data_version=\"21\">\n";
    xml_text += "  <node\n    class=\"Project\"\n";
    xml_text += "    generate_languages=\"Fortran|GO|Julia|LuaJIT|Rust|TypeScript\">\n";

    for (size_t index = 0; index < path.size(); ++index)
    {
        const size_t indent = 4 + (index * 2);
        const NodeDeclaration* declaration = FindDeclaration(path[index]).value_or(nullptr);
        if (index == 0 && entry.form_is_form)
        {
            AppendNodeOpen(xml_text, declaration, true, indent, GetLanguageFileAttributes(entry));
        }
        else
        {
            AppendNodeOpen(xml_text, declaration, false, indent);
        }

        const bool is_last = (index + 1 == path.size());
        if (!is_last)
        {
            xml_text += ">\n";
            continue;
        }

        if (entry.child_class.empty())
        {
            xml_text += " />\n";
        }
        else
        {
            xml_text += ">\n";
            const NodeDeclaration* child = FindDeclaration(entry.child_class).value_or(nullptr);
            AppendNodeOpen(xml_text, child, false, indent + 2);
            xml_text += " />\n";
            xml_text += std::string(indent, ' ') + "</node>\n";
        }
    }

    // Close every ancestor that was opened with an opening tag.
    for (size_t index = path.size(); index > 1; --index)
    {
        const size_t indent = 4 + ((index - 2) * 2);
        xml_text += std::string(indent, ' ') + "</node>\n";
    }

    xml_text += "  </node>\n</wxUiEditorData>\n";
    return xml_text;
}

static bool WriteUtf8File(const wxString& path, const std::string& text)
{
    wxFFile file(path, "wb");
    if (!file.IsOpened())
    {
        return false;
    }

    file.Write(text.data(), text.size());
    return file.Close();
}

static bool WriteManifest(const wxString& path, const std::vector<KitEntry>& entries,
                          const std::vector<OmittedEntry>& omitted)
{
    std::string json = "{\n  \"schema\": 1,\n";
    json += "  \"generated_by\": \"wxUiEditor --gen_conformance\",\n";
    json += "  \"data_version\": 21,\n";
    json += "  \"forms\": [";

    for (size_t index = 0; index < entries.size(); ++index)
    {
        const KitEntry& entry = entries[index];
        if (index > 0)
        {
            json += ',';
        }

        json += "\n    { \"class\": \"" + entry.class_name + "\", \"file\": \"" + entry.class_name +
                ".wxui\", \"form\": \"" + entry.form_class + "\", \"chain\": [";
        for (size_t link = 0; link < entry.chain.size(); ++link)
        {
            if (link > 0)
            {
                json += ", ";
            }
            json += "\"" + entry.chain[link] + "\"";
        }
        json += "], \"child\": ";
        if (entry.child_class.empty())
        {
            json += "null";
        }
        else
        {
            json += "\"" + entry.child_class + "\"";
        }
        json += " }";
    }

    json += "\n  ],\n  \"omitted\": [";
    for (size_t index = 0; index < omitted.size(); ++index)
    {
        if (index > 0)
        {
            json += ',';
        }
        json += "\n    { \"class\": \"" + omitted[index].class_name + "\", \"reason\": \"" +
                omitted[index].reason + "\" }";
    }
    json += "\n  ]\n}\n";

    return WriteUtf8File(path, json);
}

// Places one class in the kit: the class itself when the Project node allows it as a project child,
// otherwise the shortest chain from one of the project children.
static std::optional<KitEntry> BuildEntry(const NodeDeclaration* project,
                                          const NodeDeclaration* declaration,
                                          const std::vector<const NodeDeclaration*>& project_children)
{
    KitEntry entry;
    entry.class_name = std::string(declaration->get_DeclName());

    if (project->get_AllowableChildren(declaration->get_GenType()) != 0)
    {
        entry.form_class = entry.class_name;
        entry.form_is_form = IsFormType(declaration->get_GenType());
    }
    else
    {
        for (const std::string_view candidate_name: FORM_CANDIDATES)
        {
            const std::optional<const NodeDeclaration*> form = FindDeclaration(candidate_name);
            if (!form.has_value() || project->get_AllowableChildren(form.value()->get_GenType()) == 0)
            {
                continue;
            }

            const std::vector<const NodeDeclaration*> found = FindChain(form.value(), declaration);
            if (found.empty())
            {
                continue;
            }

            entry.form_class = std::string(form.value()->get_DeclName());
            entry.form_is_form = true;
            for (const NodeDeclaration* link: found)
            {
                entry.chain.emplace_back(link->get_DeclName());
            }
            break;
        }

        // Fall back to any other project child that can reach the class: a ribbon page is only
        // reachable from a panel form, a data string only from a Data node.
        if (entry.form_class.empty())
        {
            for (const NodeDeclaration* form: project_children)
            {
                if (form == declaration)
                {
                    continue;
                }

                const std::vector<const NodeDeclaration*> found = FindChain(form, declaration);
                if (found.empty())
                {
                    continue;
                }

                entry.form_class = std::string(form->get_DeclName());
                entry.form_is_form = IsFormType(form->get_GenType());
                for (const NodeDeclaration* link: found)
                {
                    entry.chain.emplace_back(link->get_DeclName());
                }
                break;
            }
        }
    }

    if (entry.form_class.empty())
    {
        return {};
    }

    const std::optional<const NodeDeclaration*> child = FindPreferredChild(declaration);
    if (child.has_value())
    {
        entry.child_class = std::string(child.value()->get_DeclName());
    }

    return entry;
}

bool WriteConformanceKit(const std::string& directory)
{
    const wxString kit_dir = wxString::FromUTF8(directory);
    if (!wxFileName::Mkdir(kit_dir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL) && !wxDirExists(kit_dir))
    {
        wxLogStderr(nullptr).LogText(wxString("--gen_conformance: cannot create ") + kit_dir);
        return false;
    }

    const std::optional<const NodeDeclaration*> project = FindDeclaration("Project");
    if (!project.has_value())
    {
        wxLogStderr(nullptr).LogText("--gen_conformance: the Project declaration is unavailable");
        return false;
    }

    std::vector<const NodeDeclaration*> classes;
    for (const NodeDeclaration* declaration: NodeCreation.get_NodeDeclarationArray())
    {
        if (declaration == nullptr || declaration->is_Type(GenType::type_interface))
        {
            continue;
        }
        classes.emplace_back(declaration);
    }
    std::ranges::sort(classes, [](const NodeDeclaration* left, const NodeDeclaration* right)
                      { return left->get_DeclName() < right->get_DeclName(); });

    // Every project child other than the class being placed, in sorted order.
    std::vector<const NodeDeclaration*> project_children;
    for (const NodeDeclaration* declaration: classes)
    {
        if (project.value()->get_AllowableChildren(declaration->get_GenType()) != 0)
        {
            project_children.emplace_back(declaration);
        }
    }

    std::vector<KitEntry> entries;
    std::vector<OmittedEntry> omitted;

    for (const NodeDeclaration* declaration: classes)
    {
        const std::optional<KitEntry> entry = BuildEntry(project.value(), declaration,
                                                         project_children);
        if (!entry.has_value())
        {
            omitted.push_back(
                OmittedEntry { std::string(declaration->get_DeclName()), "no-parent-path" });
            continue;
        }

        entries.emplace_back(entry.value());
    }

    std::set<std::string, std::less<>> generated;
    for (const KitEntry& entry: entries)
    {
        const wxString path = kit_dir + "/" + wxString::FromUTF8(entry.class_name) + ".wxui";
        const std::string text = BuildKitFile(entry);
        if (text.empty() || !WriteUtf8File(path, text))
        {
            wxLogStderr(nullptr).LogText(wxString("--gen_conformance: cannot write ") + path);
            return false;
        }
        generated.emplace(entry.class_name);
    }

    // Remove stale .wxui files so the directory is exactly the kit the library describes.
    const wxDir directory_reader(kit_dir);
    if (directory_reader.IsOpened())
    {
        wxString filename;
        std::vector<wxString> stale;
        for (bool has_file = directory_reader.GetFirst(&filename, "*.wxui", wxDIR_FILES); has_file;
             has_file = directory_reader.GetNext(&filename))
        {
            if (!generated.contains(filename.BeforeLast('.').utf8_string()))
            {
                stale.emplace_back(filename);
            }
        }
        for (const wxString& stale_file: stale)
        {
            wxRemoveFile(kit_dir + "/" + stale_file);
        }
    }

    if (!WriteManifest(kit_dir + "/kit.json", entries, omitted))
    {
        wxLogStderr(nullptr).LogText(wxString("--gen_conformance: cannot write ") + kit_dir +
                                     "/kit.json");
        return false;
    }

    wxLogStderr(nullptr).LogText(
        wxString::Format("--gen_conformance: %d form(s), %d omitted",
                         static_cast<int>(entries.size()), static_cast<int>(omitted.size())));
    return true;
}
