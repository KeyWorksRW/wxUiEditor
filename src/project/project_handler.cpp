/////////////////////////////////////////////////////////////////////////////
// Purpose:   ProjectHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>
#include <memory>
#include <utility>

#include <wx/animate.h>   // wxAnimation and wxAnimationCtrl
#include <wx/artprov.h>   // wxArtProvider class
#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/filesys.h>   // class for opening files - virtual file system
#include <wx/mstream.h>   // Memory stream classes
#include <wx/wfstream.h>  // File stream classes

#include "project_handler.h"

#include "data_handler.h"   // DataHandler class
#include "image_handler.h"  // ProjectImage class
#include "node.h"           // Node class
#include "utils.h"

#include "ttwx.h"              // ttwx namespace functions and declarations
#include "ttwx_view_vector.h"  // ViewVector -- ttwx::ViewVector class

ProjectHandler& Project = ProjectHandler::getInstance();  // NOLINT (non-const reference) //
                                                          // cppcheck-suppress constReference

ProjectHandler::ProjectHandler() : m_ProjectVersion(0), m_OriginalProjectVersion(0)
{
    m_project_path = std::make_unique<wxFileName>();
    m_art_path = std::make_unique<wxFileName>();
}

ProjectHandler::~ProjectHandler()
{
    // m_project_path will be automatically deleted. We need the destructor to be defined in the
    // source module so that wx/filename.h doesn't need to be included in the header file (and
    // therefore in every file that needs to include the ProjectHandler header file).
}

void ProjectHandler::Initialize(NodeSharedPtr project, bool allow_ui)
{
    m_project_node = std::move(project);
    m_allow_ui = allow_ui;

    // Note that the ProjectImage class keeps a shared pointer to the project node, so the
    // reference count won't go to zero until we call this function.

    ProjectImages.Initialize(m_project_node, allow_ui);
    ProjectData.Clear();
}

void ProjectHandler::set_ProjectPath(const wxFileName* path)
{
    m_project_path->Assign(*path);

    // If the Project File is being set, then assume the art directory will need to be changed
    m_art_path->Clear();
}

void ProjectHandler::set_ProjectFile(std::string_view file)
{
    ASSERT(m_project_path);
    m_project_path->Assign(wxString(file));
    m_project_path->MakeAbsolute();

    // If the Project File is being set, then assume the art directory will need to be changed
    m_art_path->Clear();
}

auto ProjectHandler::get_ProjectFile() const -> tt_string
{
    return m_project_path->GetFullPath().utf8_string();
}

auto ProjectHandler::get_ProjectPath() const -> tt_string
{
    return m_project_path->GetPath().utf8_string();
}

void ProjectHandler::ChangeDir() const
{
    m_project_path->SetCwd();
}

void ProjectHandler::CollectForms(std::vector<Node*>& forms, Node* node_start)
{
    if (!node_start)
    {
        node_start = m_project_node.get();
    }

    for (const auto& child: node_start->get_ChildNodePtrs())
    {
        if (child->is_Form())
        {
            forms.push_back(child.get());
            if (child->is_Type(type_DocViewApp) || child->is_Type(GenEnum::type_wx_document))
            {
                // Doc/View applications and Documents have additional child forms that need to
                // be included in addition to folders and sub-folders
                CollectForms(forms, child.get());
            }
        }
        if (child->is_Gen(gen_folder) || child->is_Gen(gen_sub_folder))
        {
            CollectForms(forms, child.get());
        }
    }
}

void ProjectHandler::FixupDuplicatedNode(Node* new_node)
{
    std::set<std::string_view> base_classnames;
    std::set<std::string_view> derived_classnames;
    std::set<std::string_view> base_filenames;
    std::set<std::string_view> derived_filenames;
    std::set<std::string_view> perl_filenames;
    std::set<std::string_view> python_filenames;
    std::set<std::string_view> ruby_filenames;
    std::set<std::string_view> xrc_filenames;

    // Collect all of the class and filenames in use by each form so we can make sure the new
    // form doesn't use any of them.
    std::vector<Node*> forms;
    CollectForms(forms);

    for (auto& iter: forms)
    {
        auto insert_if_has_value = [](Node* node, PropName prop, std::set<std::string_view>& set)
        {
            if (node->HasValue(prop))
            {
                set.insert(node->as_string(prop));
            }
        };

        insert_if_has_value(iter, prop_class_name, base_classnames);
        insert_if_has_value(iter, prop_derived_class_name, derived_classnames);
        insert_if_has_value(iter, prop_base_file, base_filenames);
        insert_if_has_value(iter, prop_derived_file, derived_filenames);
        insert_if_has_value(iter, prop_perl_file, perl_filenames);
        insert_if_has_value(iter, prop_python_file, python_filenames);
        insert_if_has_value(iter, prop_ruby_file, ruby_filenames);
        insert_if_has_value(iter, prop_xrc_file, xrc_filenames);
    }

    auto make_unique_name = [](std::string_view base,
                               const std::set<std::string_view>& set_names) -> std::string
    {
        std::string new_name(base);
        // By default, just add "_copy" to the end of the name.
        if (new_name.find("_copy") == std::string::npos)
        {
            new_name += "_copy";
        }

        // If the name plus a "_copy" suffix is uniqe, then we're done. Otherewise find a
        // unique name by adding an increasing number to the end of the name.
        if (set_names.contains(new_name))
        {
            std::string copy_name(new_name);
            // Remove trailing digits and underscores
            if (auto pos = copy_name.find_last_not_of("0123456789_"); pos != std::string::npos)
            {
                copy_name = copy_name.substr(0, pos + 1);
            }

            // Start with 2 since the first copy is just "_copy"
            for (int i = 2;; ++i)
            {
                // Append an increasing number to the end until we find a name that isn't already in
                // use.
                new_name = copy_name + std::to_string(i);

                if (!set_names.contains(new_name))
                {
                    break;
                }
            }
        }
        return new_name;
    };

    auto SetNewNodeName = [&](std::set<std::string_view>& set_names, PropName prop)
    {
        if (new_node->HasValue(prop))
        {
            if (set_names.contains(new_node->as_string(prop)))
            {
                std::string new_name = make_unique_name(new_node->as_view(prop), set_names);
                new_node->set_value(prop, new_name);
            }
        }
    };

    SetNewNodeName(base_classnames, prop_class_name);
    SetNewNodeName(derived_classnames, prop_derived_class_name);
    SetNewNodeName(base_filenames, prop_base_file);
    SetNewNodeName(derived_filenames, prop_derived_file);
    SetNewNodeName(perl_filenames, prop_perl_file);
    SetNewNodeName(python_filenames, prop_python_file);
    SetNewNodeName(ruby_filenames, prop_ruby_file);
    SetNewNodeName(xrc_filenames, prop_xrc_file);
}

auto ProjectHandler::get_wxFileName() const -> const wxFileName*
{
    if (m_project_path->IsOk())
    {
        return m_project_path.get();
    }

    if (m_project_node->HasValue(prop_art_directory))
    {
        m_project_path->Assign(m_project_node->as_string(prop_art_directory), wxEmptyString,
                               wxEmptyString, wxPATH_NATIVE);
        m_project_path->MakeRelativeTo(m_project_path->GetPath());
        m_project_path->MakeAbsolute();
        return m_project_path.get();
    }

    m_project_path->Assign(m_project_path->GetFullPath());
    return m_project_path.get();
}

auto ProjectHandler::get_ArtPath() -> const wxFileName*
{
    if (m_art_path->IsOk())
    {
        return m_art_path.get();
    }

    if (m_project_node->HasValue(prop_art_directory))
    {
        m_art_path->Assign(m_project_node->as_string(prop_art_directory), wxEmptyString,
                           wxEmptyString, wxPATH_NATIVE);
        m_art_path->MakeRelativeTo(m_project_path->GetPath());
        m_art_path->MakeAbsolute();
        return m_art_path.get();
    }

    m_art_path->Assign(m_project_path->GetFullPath());
    return m_art_path.get();
}

auto ProjectHandler::ArtDirectory() -> tt_string
{
    return get_ArtPath()->GetFullPath();
}

auto ProjectHandler::get_BaseDirectory(Node* node, GenLang language) const -> tt_string
{
    if (!node || node == m_project_node.get())
    {
        return get_ProjectPath();
    }

    if (!node->is_Form() && !node->is_Folder())
    {
        node = node->get_Form();
        if (!node)
        {
            return get_ProjectPath();
        }
    }

    auto [path, has_base_file] = GetOutputPath(node, language);
    if (has_base_file)
    {
        path.remove_filename();
    }

    return path;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto ProjectHandler::GetFolderOutputPath(Node* folder, GenLang language, Node*& form) const
    -> tt_string
{
    tt_string result;

    if (language == GEN_LANG_CPLUSPLUS)
    {
        if (folder->HasValue(prop_folder_base_directory))
        {
            result = folder->as_string(prop_folder_base_directory);
        }
        else if (folder->HasValue(prop_output_file))
        {
            result = folder->as_string(prop_output_file);
        }
        else if (folder->is_Gen(gen_data_folder))
        {
            form = folder->get_Parent();
        }
    }
    else
    {
        static const std::map<GenLang, PropName> langFolderPropMap = {
            { GEN_LANG_PERL, prop_folder_perl_output_folder },
            { GEN_LANG_PYTHON, prop_folder_python_output_folder },
            { GEN_LANG_RUBY, prop_folder_ruby_output_folder },
            { GEN_LANG_XRC, prop_folder_xrc_directory }
        };

        if (auto iter = langFolderPropMap.find(language);
            iter != langFolderPropMap.end() && folder->HasValue(iter->second))
        {
            result = folder->as_string(iter->second);
        }
    }

    return result;
}

auto ProjectHandler::GetProjectOutputPath(GenLang language) const -> tt_string
{
    static const std::map<GenLang, PropName> langProjectPropMap = {
        { GEN_LANG_CPLUSPLUS, prop_base_directory },
        { GEN_LANG_PERL, prop_perl_output_folder },
        { GEN_LANG_PYTHON, prop_python_output_folder },
        { GEN_LANG_RUBY, prop_ruby_output_folder },
        { GEN_LANG_XRC, prop_xrc_directory }
    };

    if (auto iter = langProjectPropMap.find(language);
        iter != langProjectPropMap.end() && m_project_node->HasValue(iter->second))
    {
        return m_project_node->as_string(iter->second);
    }

    return get_ProjectPath();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto ProjectHandler::GetBaseFilename(Node* form, GenLang language) const -> tt_string
{
    if (language == GEN_LANG_CPLUSPLUS && form->is_Gen(gen_Data))
    {
        return form->as_string(prop_output_file);
    }

    static const std::map<GenLang, PropName> langbase_file_propertyMap = {
        { GEN_LANG_CPLUSPLUS, prop_base_file },
        { GEN_LANG_PYTHON, prop_python_file },
        { GEN_LANG_RUBY, prop_ruby_file },
        { GEN_LANG_PERL, prop_perl_file },
        { GEN_LANG_XRC, prop_xrc_file }
    };

    auto iter = langbase_file_propertyMap.find(language);
    if (iter != langbase_file_propertyMap.end())
    {
        return form->as_string(iter->second);
    }

    FAIL_MSG(tt_string() << "Unknown language: " << language);
    return "";
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto ProjectHandler::MergeBaseFilePath(tt_string& result, const tt_string& base_file) const -> void
{
    // TODO: [Randalphwa - 01-06-2024] It's possible that the user created the filename using a
    // folder prefix that is the same as the project's base directory. If that's the case, the
    // prefix should be removed here.

    result.backslashestoforward();
    if (base_file.contains("/"))
    {
        result.backslashestoforward();
        if (result.back() == '/')
        {
            result.pop_back();
        }

        // If the first part of the base_file is a folder and it matches the last folder in
        // result, then assume the folder name is duplicated in base_file. Remove the folder
        // from result before adding the base_file path.
        if (auto end_folder = base_file.find('/'); end_folder != std::string::npos)
        {
            if (result.ends_with(base_file.substr(0, end_folder)))
            {
                result.erase(result.size() - end_folder, end_folder);
            }
        }
    }
    result.append_filename(base_file);
    result.make_absolute();
    result.backslashestoforward();
}

auto ProjectHandler::GetOutputPath(Node* form, GenLang language) const -> std::pair<tt_string, bool>
{
    ASSERT(form->is_Form() || form->is_Folder());

    tt_string result;
    Node* folder = form->is_Folder() ? form : form->get_Folder();

    if (folder)
    {
        result = GetFolderOutputPath(folder, language, form);
    }

    if (result.empty())
    {
        result = GetProjectOutputPath(language);
    }

    tt_string base_file = GetBaseFilename(form, language);

    if (base_file.empty())
    {
        return std::make_pair(result, false);
    }

    MergeBaseFilePath(result, base_file);

    return std::make_pair(result, true);
}

// Note that this will return a directory for all languages even though we currently don't generate
// derived files for any language except C++.
auto ProjectHandler::get_DerivedDirectory(Node* node, GenLang language) const -> std::string
{
    std::string result;

    Node* folder = node->get_Folder();
    if (folder)
    {
        static const std::map<GenLang, PropName> folderLangPropMap = {
            { GEN_LANG_CPLUSPLUS, prop_folder_derived_directory },
            { GEN_LANG_PERL, prop_folder_perl_output_folder },
            { GEN_LANG_PYTHON, prop_folder_python_output_folder },
            { GEN_LANG_RUBY, prop_folder_ruby_output_folder },
            { GEN_LANG_XRC, prop_folder_xrc_directory }
        };
        if (auto iter = folderLangPropMap.find(language); iter != folderLangPropMap.end())
        {
            if (folder->HasValue(iter->second))
            {
                // Special case for C++: use base directory for derived files
                if (language == GEN_LANG_CPLUSPLUS)
                {
                    result = folder->as_string(prop_folder_base_directory);
                }
                else
                {
                    result = folder->as_string(iter->second);
                }
            }
        }
    }

    // Even if the node has a folder parent, there may not be a directory set for it, so check
    // result and if it's empty use the project directory properties.
    if (result.empty() || !folder)
    {
        static const std::map<GenLang, PropName> projectLangPropMap = {
            { GEN_LANG_CPLUSPLUS, prop_derived_directory },
            { GEN_LANG_PERL, prop_perl_output_folder },
            { GEN_LANG_PYTHON, prop_python_output_folder },
            { GEN_LANG_RUBY, prop_ruby_output_folder },
            { GEN_LANG_XRC, prop_xrc_directory }
        };

        if (auto iter = projectLangPropMap.find(language); iter != projectLangPropMap.end())
        {
            if (m_project_node->HasValue(iter->second))
            {
                // Special case for C++: use base directory for derived files
                if (language == GEN_LANG_CPLUSPLUS)
                {
                    result = m_project_node->as_string(prop_base_directory);
                }
                else
                {
                    result = m_project_node->as_string(iter->second);
                }
            }
        }
    }

    if (result.empty())
    {
        result = get_ProjectPath();
    }

    wxFileName abs_path(result);
    if (abs_path.MakeAbsolute())
    {
        result = abs_path.GetFullPath().utf8_string();
    }

    return result;
}

auto ProjectHandler::get_FirstFormChild(Node* node) const -> Node*
{
    if (!node)
    {
        node = m_project_node.get();
    }
    for (const auto& child: node->get_ChildNodePtrs())
    {
        if (child->is_Form())
        {
            return child.get();
        }
        if (child->is_Gen(gen_folder) || child->is_Gen(gen_sub_folder))
        {
            return get_FirstFormChild(child.get());
        }
    }

    return nullptr;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto ProjectHandler::get_CodePreference(Node* node) const -> GenLang
{
    tt_string value = Project.as_string(prop_code_preference);
    if (node)
    {
        if (node->is_Gen(gen_folder))
        {
            value = node->as_string(prop_code_preference);
        }
        else if (auto* folder = node->get_Folder(); folder)
        {
            value = folder->as_string(prop_code_preference);
        }
    }

    // Note: Be sure this list matches the languages in ../xml/project.xml
    // clang-format off
    static const std::map<std::string_view, GenLang> langStringMap = {
        { "C++", GEN_LANG_CPLUSPLUS },
        { "Perl", GEN_LANG_PERL },
        { "Python", GEN_LANG_PYTHON },
        { "Ruby", GEN_LANG_RUBY },
        { "XRC", GEN_LANG_XRC }
    };
    // clang-format on
    if (auto iter = langStringMap.find(value); iter != langStringMap.end())
    {
        return iter->second;
    }

    return GEN_LANG_CPLUSPLUS;
}

auto ProjectHandler::get_GenerateLanguages() const -> size_t
{
    // Always set the project's code preference to the list
    auto languages = static_cast<size_t>(get_CodePreference(m_project_node.get()));

    auto value = Project.as_view(prop_generate_languages);

    // Note: Be sure this list matches the languages in ../xml/project.xml
    // clang-format off
    static const std::map<std::string_view, size_t> langBitMap = {
        { "C++", GEN_LANG_CPLUSPLUS },
        { "Perl", GEN_LANG_PERL },
        { "Python", GEN_LANG_PYTHON },
        { "Ruby", GEN_LANG_RUBY },
        { "XRC", GEN_LANG_XRC }
    };
    // clang-format on
    for (const auto& [langStr, langBit]: langBitMap)
    {
        if (value.find(langStr) != std::string_view::npos)
        {
            languages |= langBit;
        }
    }

    return languages;
}

auto ProjectHandler::ShouldOutputLanguage(const NodesFormChild& nodes,
                                          const PropName& base_file_property,
                                          GenLang language) const -> bool
{
    if (!nodes.child->HasValue(base_file_property))
    {
        return false;
    }

    // Check if the property has its default value
    if (nodes.child->as_string(base_file_property) ==
        nodes.child->get_PropDefaultValue(base_file_property))
    {
        // C++ always outputs with default value
        if (language == GEN_LANG_CPLUSPLUS)
        {
            return get_CodePreference(nodes.form) == GEN_LANG_CPLUSPLUS;
        }

        // Data/Image nodes with default values follow code preference (except XRC)
        if (language != GEN_LANG_XRC)
        {
            if ((nodes.child->is_Gen(gen_Images) || nodes.child->is_Gen(gen_Data)))
            {
                return get_CodePreference(nodes.form) == language;
            }
        }
        else
        {
            // XRC doesn't output Data/Images with default values
            if (nodes.child->is_Gen(gen_Images) || nodes.child->is_Gen(gen_Data))
            {
                return false;
            }
        }
    }

    return true;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto ProjectHandler::get_OutputType(int flags) const -> size_t
{
    size_t result = OUTPUT_NONE;

    auto traverse_forms_recursively = [&](Node* form, auto&& traverse_forms_recursively) -> void
    {
        for (const auto& child: form->get_ChildNodePtrs())
        {
            if (child->is_FormParent())
            {
                traverse_forms_recursively(child.get(), traverse_forms_recursively);
            }
            else if (child->is_Form())
            {
                // Table-driven output flag logic
                struct OutputLangInfo
                {
                    PropName base_file_property;
                    GenLang language;
                    size_t output_flag;
                };
                static constexpr auto outputLangs =
                    std::to_array<OutputLangInfo>({ { .base_file_property = prop_base_file,
                                                      .language = GEN_LANG_CPLUSPLUS,
                                                      .output_flag = OUTPUT_CPLUS },
                                                    { .base_file_property = prop_perl_file,
                                                      .language = GEN_LANG_PERL,
                                                      .output_flag = OUTPUT_PERL },
                                                    { .base_file_property = prop_python_file,
                                                      .language = GEN_LANG_PYTHON,
                                                      .output_flag = OUTPUT_PYTHON },
                                                    { .base_file_property = prop_ruby_file,
                                                      .language = GEN_LANG_RUBY,
                                                      .output_flag = OUTPUT_RUBY },
                                                    { .base_file_property = prop_xrc_file,
                                                      .language = GEN_LANG_XRC,
                                                      .output_flag = OUTPUT_XRC } });

                for (const auto& info: outputLangs)
                {
                    if (ShouldOutputLanguage({ .form = form, .child = child.get() },
                                             info.base_file_property, info.language))
                    {
                        result |= info.output_flag;
                    }
                }

                // Derived output logic
                if (!(flags & OUT_FLAG_IGNORE_DERIVED) && child->HasValue(prop_derived_file) &&
                    child->as_bool(prop_use_derived_class))
                {
                    if (auto path = get_DerivedFilename(child.get()); path.size())
                    {
                        if (!path.file_exists())
                        {
                            result |= OUTPUT_DERIVED;
                        }
                    }
                }
            }
        }
    };

    traverse_forms_recursively(m_project_node.get(), traverse_forms_recursively);

    return result;
}

auto ProjectHandler::get_DerivedFilename(Node* form) const -> tt_string
{
    tt_string path;

    ASSERT(form->is_Form());

    if (not form->is_Form() || !form->HasValue(prop_derived_file))
    {
        return path;
    }

    path = get_DerivedDirectory(form, GEN_LANG_CPLUSPLUS);
    path.append_filename(form->as_string(prop_derived_file).filename());
    path.make_absolute();

    tt_string source_ext(".cpp");
    if (const auto& extProp = as_string(prop_source_ext); extProp.size())
    {
        source_ext = extProp;
    }
    path.replace_extension(source_ext);
    return path;
}

[[nodiscard]] auto ProjectHandler::AllFormTypesFound() const -> bool
{
    return m_form_Animation && m_form_BundleSVG && m_form_BundleBitmaps && m_form_Image;
}

void ProjectHandler::ProcessImageProperty(const NodeProperty& prop, Node* child)
{
    if ((prop.type() != type_image && prop.type() != type_animation) || !prop.HasValue())
    {
        return;
    }

    tt_string_vector parts(prop.as_string(), BMP_PROP_SEPARATOR, tt::TRIM::both);
    if (parts.size() < IndexImage + 1)
    {
        return;
    }

    Node* form_to_use = m_ImagesForm ? m_ImagesForm : child->get_Form();
    if (!form_to_use)
    {
        return;
    }

    if (parts[IndexType] == "Embed")
    {
        if (prop.type() == type_animation)
        {
            if (!m_form_Animation)
            {
                m_form_Animation = form_to_use;
            }
        }
        else
        {
            if (!m_form_Image)
            {
                m_form_Image = form_to_use;
            }
            if (!m_form_BundleBitmaps && ProjectImages.GetPropertyImageBundle(parts))
            {
                m_form_BundleBitmaps = form_to_use;
            }
        }
    }
    else if (parts[IndexType] == "SVG")
    {
        if (!m_form_BundleSVG)
        {
            m_form_BundleSVG = form_to_use;
        }
    }
}

void ProjectHandler::ParseImagePropsRecursive(Node* node)
{
    for (const auto& child_ptr: node->get_ChildNodePtrs())
    {
        Node* child = child_ptr.get();
        for (const auto& prop: child->get_PropsVector())
        {
            ProcessImageProperty(prop, child);
            if (AllFormTypesFound())
            {
                return;
            }
        }

        if (child->get_ChildCount())
        {
            ParseImagePropsRecursive(child);
            if (AllFormTypesFound())
            {
                return;
            }
        }
    }
}

void ProjectHandler::ProcessFormIcon(Node* form)
{
    if (!form->HasValue(prop_icon))
    {
        return;
    }

    tt_string_vector parts(form->as_string(prop_icon), BMP_PROP_SEPARATOR, tt::TRIM::both);
    if (parts.size() < IndexImage + 1)
    {
        return;
    }

    if (parts[IndexType] == "Embed")
    {
        if (!m_form_Image)
        {
            m_form_Image = form;
        }
        if (!m_form_BundleBitmaps && ProjectImages.GetPropertyImageBundle(parts))
        {
            m_form_BundleBitmaps = form;
        }
    }
    else if (parts[IndexType] == "SVG")
    {
        if (!m_form_BundleSVG)
        {
            m_form_BundleSVG = form;
        }
    }
}

void ProjectHandler::FindWxueFunctions(std::vector<Node*>& forms)
{
    m_form_BundleSVG = nullptr;
    m_form_BundleBitmaps = nullptr;
    m_form_Image = nullptr;
    m_form_Animation = nullptr;
    m_ImagesForm = nullptr;

    for (auto* form: forms)
    {
        if (form->is_Gen(gen_Images))
        {
            m_ImagesForm = form;
            continue;
        }

        ProcessFormIcon(form);
        ParseImagePropsRecursive(form);

        if (AllFormTypesFound())
        {
            return;
        }
    }
}

auto ProjectHandler::get_ImagesForm() -> Node*
{
    if (!m_ImagesForm && m_project_node->get_ChildCount() > 0 &&
        m_project_node->get_Child(0)->is_Gen(gen_Images))
    {
        m_ImagesForm = m_project_node->get_Child(0);
    }
    return m_ImagesForm;
}

auto ProjectHandler::get_DataForm() -> Node*
{
    if (!m_DataForm)
    {
        if (m_project_node->get_ChildCount() > 0 && m_project_node->get_Child(0)->is_Gen(gen_Data))
        {
            m_DataForm = m_project_node->get_Child(0);
        }
        else if (m_project_node->get_ChildCount() > 1 &&
                 m_project_node->get_Child(1)->is_Gen(gen_Data))
        {
            m_DataForm = m_project_node->get_Child(1);
        }
    }
    return m_DataForm;
}

namespace
{
    // clang-format off
    const std::map<GenLang, PropName> langPropMap = {
        { GEN_LANG_CPLUSPLUS, prop_wxWidgets_version },
        { GEN_LANG_PERL, prop_wxPerl_version },
        { GEN_LANG_PYTHON, prop_wxPython_version },
        { GEN_LANG_RUBY, prop_wxRuby_version },
        { GEN_LANG_XRC, prop_wxWidgets_version }
    };
    // clang-format on

    // Helper function to parse version string and extract major, minor, patch components
    auto parseVersionString(std::string_view version) -> std::tuple<int, int, int>
    {
        int major = 1;
        int minor = 0;
        int patch = 0;

        if (version.empty())
        {
            return { major, minor, patch };
        }

        // Try parsing with '.' or '-' separator
        auto parseParts = [&](char sep) -> bool
        {
            if (version.find(sep) != std::string_view::npos)
            {
                ttwx::ViewVector parts(version, sep);
                if (!parts.empty())
                {
                    major = ttwx::atoi(parts[0]);
                }
                if (parts.size() > 1)
                {
                    minor = ttwx::atoi(parts[1]);
                }
                if (parts.size() > 2)
                {
                    patch = ttwx::atoi(parts[2]);
                }
                return true;
            }
            return false;
        };
        if (parseParts('.'))
        {
            return { major, minor, patch };
        }
        if (parseParts('-'))
        {
            return { major, minor, patch };
        }

        // Fallback: parse digits separated by non-digit characters
        auto skip_non_digits = [](std::string_view& str)
        {
            while (!str.empty() && !ttwx::is_digit(str[0]))
            {
                str.remove_prefix(1);
            }
        };

        auto ExtractNumber = [&skip_non_digits](std::string_view& str) -> int
        {
            if (str.empty())
            {
                return 0;
            }

            size_t digitEnd = 0;
            while (digitEnd < str.size() && ttwx::is_digit(str[digitEnd]))
            {
                ++digitEnd;
            }

            int result = digitEnd > 0 ? ttwx::atoi(str.substr(0, digitEnd)) : 0;
            str.remove_prefix(digitEnd);
            skip_non_digits(str);
            return result;
        };

        // extractNumbers will modify the version string view to remove processed parts
        major = ExtractNumber(version);
        minor = ExtractNumber(version);
        patch = ExtractNumber(version);

        return { major, minor, patch };
    }

}  // namespace

// Version encoding constants for get_LangVersion()
// Assumes major, minor, and patch each have 99 possible values
constexpr int VERSION_MAJOR_MULTIPLIER = 10000;
constexpr int VERSION_MINOR_MULTIPLIER = 100;

auto ProjectHandler::get_LangVersion(GenLang language) const -> int
{
    std::string_view version;

    if (auto iter = langPropMap.find(language); iter != langPropMap.end())
    {
        version = m_project_node->as_string(iter->second);
    }
    else
    {
        FAIL_MSG(tt_string() << "Unknown language: " << language);
    }

    auto [major, minor, patch] = parseVersionString(version);

    return (major * VERSION_MAJOR_MULTIPLIER) + (minor * VERSION_MINOR_MULTIPLIER) + patch;
}
