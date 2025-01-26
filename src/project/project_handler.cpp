/////////////////////////////////////////////////////////////////////////////
// Purpose:   ProjectHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <fstream>
#include <memory>
#include <thread>

#include <wx/animate.h>   // wxAnimation and wxAnimationCtrl
#include <wx/artprov.h>   // wxArtProvider class
#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/filesys.h>   // class for opening files - virtual file system
#include <wx/mstream.h>   // Memory stream classes
#include <wx/wfstream.h>  // File stream classes

#include "project_handler.h"

#include "bitmaps.h"        // Map of bitmaps accessed by name
#include "data_handler.h"   // DataHandler class
#include "image_handler.h"  // ProjectImage class
#include "mainapp.h"        // App -- App class
#include "mainframe.h"      // MainFrame -- Main window frame
#include "node.h"           // Node class
#include "utils.h"          // Utility functions that work with properties

ProjectHandler& Project = ProjectHandler::getInstance();

ProjectHandler::ProjectHandler()
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
    m_project_node = project;
    m_allow_ui = allow_ui;

    // Note that the ProjectImage class keeps a shared pointer to the project node, so the
    // reference count won't go to zero until we call this function.

    ProjectImages.Initialize(m_project_node, allow_ui);
    ProjectData.Clear();
}

void ProjectHandler::setProjectPath(const wxFileName* path)
{
    m_project_path->Assign(*path);

    // If the Project File is being set, then assume the art directory will need to be changed
    m_art_path->Clear();
}

void ProjectHandler::setProjectFile(const tt_string& file)
{
    ASSERT(m_project_path);
    m_project_path->Assign(file);
    m_project_path->MakeAbsolute();

    // If the Project File is being set, then assume the art directory will need to be changed
    m_art_path->Clear();
}

tt_string ProjectHandler::getProjectFile() const
{
    return m_project_path->GetFullPath().utf8_string();
}

tt_string ProjectHandler::getProjectPath() const
{
    return m_project_path->GetPath().utf8_string();
}

bool ProjectHandler::ChangeDir() const
{
    return m_project_path->SetCwd();
}

void ProjectHandler::CollectForms(std::vector<Node*>& forms, Node* node_start)
{
    if (!node_start)
    {
        node_start = m_project_node.get();
    }

    for (const auto& child: node_start->getChildNodePtrs())
    {
        if (child->isForm())
        {
            forms.push_back(child.get());
        }
        else
        {
            if (child->isGen(gen_folder) || child->isGen(gen_sub_folder))
            {
                CollectForms(forms, child.get());
            }
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
    std::set<std::string_view> rust_filenames;
    std::set<std::string_view> xrc_filenames;

    // Collect all of the class and filenames in use by each form so we can make sure the new
    // form doesn't use any of them.
    std::vector<Node*> forms;
    CollectForms(forms);
    for (auto& iter: forms)
    {
        if (iter->hasValue(prop_class_name))
            base_classnames.insert(iter->as_string(prop_class_name));
        if (iter->hasValue(prop_derived_class_name))
            derived_classnames.insert(iter->as_string(prop_derived_class_name));
        if (iter->hasValue(prop_base_file))
            base_filenames.insert(iter->as_string(prop_base_file));
        if (iter->hasValue(prop_derived_file))
            derived_filenames.insert(iter->as_string(prop_derived_file));
        if (iter->hasValue(prop_perl_file))
            python_filenames.insert(iter->as_string(prop_perl_file));
        if (iter->hasValue(prop_python_file))
            python_filenames.insert(iter->as_string(prop_python_file));
        if (iter->hasValue(prop_ruby_file))
            ruby_filenames.insert(iter->as_string(prop_ruby_file));
        if (iter->hasValue(prop_rust_file))
            ruby_filenames.insert(iter->as_string(prop_rust_file));
        if (iter->hasValue(prop_xrc_file))
            xrc_filenames.insert(iter->as_string(prop_xrc_file));
    }

    auto lambda = [&](std::set<std::string_view>& set_names, PropName prop)
    {
        if (new_node->hasValue(prop))
        {
            if (set_names.contains(new_node->as_string(prop)))
            {
                tt_string new_name = new_node->as_string(prop);
                if (!new_name.contains("_copy"))
                    new_name += "_copy";
                if (set_names.contains(new_name))
                {
                    tt_string copy_name = new_name;
                    while (tt::is_digit(copy_name.back()))
                    {
                        // remove any trailing digits
                        copy_name.erase(copy_name.size() - 1, 1);
                    }
                    if (copy_name.back() == '_')
                    {
                        copy_name.erase(copy_name.size() - 1, 1);
                    }

                    for (int i = 2;; ++i)
                    {
                        new_name.clear();
                        // new_name << copy_name << '_' << i;
                        new_name << copy_name << i;
                        if (!set_names.contains(new_name))
                            break;
                    }
                }
                new_node->set_value(prop, new_name);
            }
        }
    };

    lambda(base_classnames, prop_class_name);
    lambda(derived_classnames, prop_derived_class_name);
    lambda(base_filenames, prop_base_file);
    lambda(derived_filenames, prop_derived_file);
    lambda(perl_filenames, prop_perl_file);
    lambda(python_filenames, prop_python_file);
    lambda(ruby_filenames, prop_ruby_file);
    lambda(rust_filenames, prop_rust_file);
    lambda(xrc_filenames, prop_xrc_file);
}

const wxFileName* ProjectHandler::get_wxFileName() const
{
    if (m_project_path->IsOk())
    {
        return m_project_path.get();
    }
    else
    {
        if (m_project_node->hasValue(prop_art_directory))
        {
            m_project_path->Assign(m_project_node->as_string(prop_art_directory), wxEmptyString, wxEmptyString,
                                   wxPATH_NATIVE);
            m_project_path->MakeRelativeTo(m_project_path->GetPath());
            m_project_path->MakeAbsolute();
            return m_project_path.get();
        }
        else
        {
            m_project_path->Assign(m_project_path->GetFullPath());
            return m_project_path.get();
        }
    }
}

const wxFileName* ProjectHandler::getArtPath()
{
    if (m_art_path->IsOk())
    {
        return m_art_path.get();
    }
    else
    {
        if (m_project_node->hasValue(prop_art_directory))
        {
            m_art_path->Assign(m_project_node->as_string(prop_art_directory), wxEmptyString, wxEmptyString, wxPATH_NATIVE);
            m_art_path->MakeRelativeTo(m_project_path->GetPath());
            m_art_path->MakeAbsolute();
            return m_art_path.get();
        }
        else
        {
            m_art_path->Assign(m_project_path->GetFullPath());
            return m_art_path.get();
        }
    }
}

tt_string ProjectHandler::ArtDirectory()
{
    return getArtPath()->GetFullPath();
}

tt_string ProjectHandler::getBaseDirectory(Node* node, GenLang language) const
{
    if (!node || node == m_project_node.get())
    {
        return getProjectPath();
    }

    if (!node->isForm() && !node->isFolder())
    {
        node = node->getForm();
        if (!node)
        {
            return getProjectPath();
        }
    }

    auto [path, has_base_file] = GetOutputPath(node, language);
    if (has_base_file)
    {
        path.remove_filename();
    }

    return path;
}

std::pair<tt_string, bool> ProjectHandler::GetOutputPath(Node* form, GenLang language) const
{
    ASSERT(form->isForm() || form->isFolder());

    tt_string result;
    Node* folder = form->isFolder() ? form : form->getFolder();
    if (folder)
    {
        if (language == GEN_LANG_CPLUSPLUS)
        {
            if (folder->hasValue(prop_folder_base_directory))
                result = folder->as_string(prop_folder_base_directory);
            else if (folder->hasValue(prop_output_file))
                result = folder->as_string(prop_output_file);
            else if (folder->isGen(gen_data_folder))
            {
                // Move the form to the gen_Data node since the folder doesn't specify an output file.
                form = folder->getParent();
            }
        }
        else if (language == GEN_LANG_PERL && folder->hasValue(prop_folder_perl_output_folder))
            result = folder->as_string(prop_folder_perl_output_folder);
        else if (language == GEN_LANG_PYTHON && folder->hasValue(prop_folder_python_output_folder))
            result = folder->as_string(prop_folder_python_output_folder);
        else if (language == GEN_LANG_RUBY && folder->hasValue(prop_folder_ruby_output_folder))
            result = folder->as_string(prop_folder_ruby_output_folder);
        else if (language == GEN_LANG_RUST && folder->hasValue(prop_folder_rust_output_folder))
            result = folder->as_string(prop_folder_rust_output_folder);
        else if (language == GEN_LANG_XRC && folder->hasValue(prop_folder_xrc_directory))
            result = folder->as_string(prop_folder_xrc_directory);

#if GENERATE_NEW_LANG_CODE
        else if (language == GEN_LANG_FORTRAN && folder->hasValue(prop_folder_fortran_output_folder))
            result = folder->as_string(prop_folder_fortran_output_folder);
        else if (language == GEN_LANG_HASKELL && folder->hasValue(prop_folder_haskell_output_folder))
            result = folder->as_string(prop_folder_haskell_output_folder);
        else if (language == GEN_LANG_LUA && folder->hasValue(prop_folder_lua_output_folder))
            result = folder->as_string(prop_folder_lua_output_folder);
#endif  // GENERATE_NEW_LANG_CODE
    }

    // Even if the node has a folder parent, there may not be a directory set for it, so check
    // result and if it's empty use the project directory properties.
    if (result.empty() || !folder)
    {
        if (language == GEN_LANG_CPLUSPLUS)
        {
            if (m_project_node->hasValue(prop_base_directory))
                result = m_project_node->as_string(prop_base_directory);
        }
        else if (language == GEN_LANG_PERL && m_project_node->hasValue(prop_perl_output_folder))
            result = m_project_node->as_string(prop_perl_output_folder);
        else if (language == GEN_LANG_PYTHON && m_project_node->hasValue(prop_python_output_folder))
            result = m_project_node->as_string(prop_python_output_folder);
        else if (language == GEN_LANG_RUBY && m_project_node->hasValue(prop_ruby_output_folder))
            result = m_project_node->as_string(prop_ruby_output_folder);
        else if (language == GEN_LANG_RUST && m_project_node->hasValue(prop_rust_output_folder))
            result = m_project_node->as_string(prop_rust_output_folder);
        else if (language == GEN_LANG_XRC && m_project_node->hasValue(prop_xrc_directory))
            result = m_project_node->as_string(prop_xrc_directory);

#if GENERATE_NEW_LANG_CODE
        else if (language == GEN_LANG_FORTRAN && m_project_node->hasValue(prop_fortran_output_folder))
            result = m_project_node->as_string(prop_fortran_output_folder);
        else if (language == GEN_LANG_HASKELL && m_project_node->hasValue(prop_haskell_output_folder))
            result = m_project_node->as_string(prop_haskell_output_folder);
        else if (language == GEN_LANG_LUA && m_project_node->hasValue(prop_lua_output_folder))
            result = m_project_node->as_string(prop_lua_output_folder);
#endif  // GENERATE_NEW_LANG_CODE
    }

    if (result.empty())
        result = getProjectPath();

    tt_string base_file;
    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
            if (form->isGen(gen_Data))
                base_file = form->as_string(prop_output_file);
            else
                base_file = form->as_string(prop_base_file);
            break;
        case GEN_LANG_PYTHON:
            base_file = form->as_string(prop_python_file);
            break;
        case GEN_LANG_RUBY:
            base_file = form->as_string(prop_ruby_file);
            break;
        case GEN_LANG_FORTRAN:
            base_file = form->as_string(prop_fortran_file);
            break;
        case GEN_LANG_HASKELL:
            base_file = form->as_string(prop_haskell_file);
            break;
        case GEN_LANG_LUA:
            base_file = form->as_string(prop_lua_file);
            break;
        case GEN_LANG_PERL:
            base_file = form->as_string(prop_perl_file);
            break;
        case GEN_LANG_RUST:
            base_file = form->as_string(prop_rust_file);
            break;

        case GEN_LANG_XRC:
            base_file = form->as_string(prop_xrc_file);
            break;

        default:
            FAIL_MSG(tt_string() << "Unknown language: " << language);
            break;
    }

    if (base_file.empty())
    {
        return std::make_pair(result, false);
    }

    // TODO: [Randalphwa - 01-06-2024] It's possible that the user created the filename using a
    // folder prefix that is the same as the project's base directory. If that's the case, the
    // prefix should be removed here.

    base_file.backslashestoforward();
    if (base_file.contains("/"))
    {
        result.backslashestoforward();
        if (result.back() == '/')
            result.pop_back();

        // If the first part of the base_file is a folder and it matches the last folder in
        // result, then assume the folder name is duplicated in base_file. Remove the folder
        // from result before adding the base_file path.
        if (auto end_folder = base_file.find('/'); end_folder != tt::npos)
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

    return std::make_pair(result, true);
}

// Note that this will return a directory for all languages even though we currently don't generate
// derived files for any language except C++.
tt_string ProjectHandler::getDerivedDirectory(Node* node, GenLang language) const
{
    tt_string result;

    Node* folder = node->getFolder();
    if (folder)
    {
        if (language == GEN_LANG_CPLUSPLUS && folder->hasValue(prop_folder_derived_directory))
            result = folder->as_string(prop_folder_base_directory);
        else if (language == GEN_LANG_PERL && folder->hasValue(prop_folder_perl_output_folder))
            result = folder->as_string(prop_folder_perl_output_folder);
        else if (language == GEN_LANG_PYTHON && folder->hasValue(prop_folder_python_output_folder))
            result = folder->as_string(prop_folder_python_output_folder);
        else if (language == GEN_LANG_RUBY && folder->hasValue(prop_folder_ruby_output_folder))
            result = folder->as_string(prop_folder_ruby_output_folder);
        else if (language == GEN_LANG_RUST && folder->hasValue(prop_folder_rust_output_folder))
            result = folder->as_string(prop_folder_rust_output_folder);
        else if (language == GEN_LANG_XRC && folder->hasValue(prop_folder_xrc_directory))
            result = folder->as_string(prop_folder_xrc_directory);
    }

    // Even if the node has a folder parent, there may not be a directory set for it, so check
    // result and if it's empty use the project directory properties.
    if (result.empty() || !folder)
    {
        if (language == GEN_LANG_CPLUSPLUS && m_project_node->hasValue(prop_derived_directory))
            result = m_project_node->as_string(prop_base_directory);
        else if (language == GEN_LANG_PERL && m_project_node->hasValue(prop_perl_output_folder))
            result = m_project_node->as_string(prop_perl_output_folder);
        else if (language == GEN_LANG_PYTHON && m_project_node->hasValue(prop_python_output_folder))
            result = m_project_node->as_string(prop_python_output_folder);
        else if (language == GEN_LANG_RUBY && m_project_node->hasValue(prop_ruby_output_folder))
            result = m_project_node->as_string(prop_ruby_output_folder);
        else if (language == GEN_LANG_RUST && m_project_node->hasValue(prop_rust_output_folder))
            result = m_project_node->as_string(prop_rust_output_folder);
        else if (language == GEN_LANG_XRC && m_project_node->hasValue(prop_xrc_directory))
            result = m_project_node->as_string(prop_xrc_directory);
    }

    if (result.empty())
        result = getProjectPath();

    result.make_absolute();

    return result;
}

Node* ProjectHandler::getFirstFormChild(Node* node) const
{
    if (!node)
        node = m_project_node.get();
    for (const auto& child: node->getChildNodePtrs())
    {
        if (child->isForm())
        {
            return child.get();
        }
        else if (child->isGen(gen_folder) || child->isGen(gen_sub_folder))
        {
            return getFirstFormChild(child.get());
        }
    }

    return nullptr;
}

GenLang ProjectHandler::getCodePreference(Node* node) const
{
    tt_string value = Project.as_string(prop_code_preference);
    if (node)
    {
        if (node->isGen(gen_folder))
        {
            value = node->as_string(prop_code_preference);
        }
        else if (auto folder = node->getFolder(); folder)
        {
            value = folder->as_string(prop_code_preference);
        }
    }

    // Note: Be sure this list matches the languages in ../xml/project.xml

    if (value == "C++")
        return GEN_LANG_CPLUSPLUS;
    else if (value == "Perl")
        return GEN_LANG_PERL;
    else if (value == "Python")
        return GEN_LANG_PYTHON;
    else if (value == "Ruby")
        return GEN_LANG_RUBY;
    else if (value == "Rust")
        return GEN_LANG_RUST;
    else if (value == "XRC")
        return GEN_LANG_XRC;

#if GENERATE_NEW_LANG_CODE
    else if (value == "Fortran")
        return GEN_LANG_FORTRAN;
    else if (value == "Haskell")
        return GEN_LANG_HASKELL;
    else if (value == "Lua")
        return GEN_LANG_LUA;
#endif  // GENERATE_NEW_LANG_CODE

    else
        return GEN_LANG_CPLUSPLUS;
}

size_t ProjectHandler::getGenerateLanguages() const
{
    // Always set the project's code preference to the list
    size_t languages = static_cast<size_t>(getCodePreference(m_project_node.get()));

    tt_string value = Project.as_string(prop_generate_languages);

    // Note: Be sure this list matches the languages in ../xml/project.xml

    if (value.contains("C++", tt::CASE::either))
        languages |= GEN_LANG_CPLUSPLUS;
    if (value.contains("Perl", tt::CASE::either))
        languages |= GEN_LANG_PERL;
    if (value.contains("Python", tt::CASE::either))
        languages |= GEN_LANG_PYTHON;
    if (value.contains("Ruby", tt::CASE::either))
        languages |= GEN_LANG_RUBY;
    if (value.contains("Rust", tt::CASE::either))
        languages |= GEN_LANG_RUST;
    if (value.contains("XRC", tt::CASE::either))
        languages |= GEN_LANG_XRC;

#if GENERATE_NEW_LANG_CODE
    if (value.contains("Fortran", tt::CASE::either))
        languages |= GEN_LANG_FORTRAN;
    if (value.contains("Haskell", tt::CASE::either))
        languages |= GEN_LANG_HASKELL;
    if (value.contains("Lua", tt::CASE::either))
        languages |= GEN_LANG_LUA;
#endif  // GENERATE_NEW_LANG_CODE

    return languages;
}

size_t ProjectHandler::getOutputType(int flags) const
{
    size_t result = OUTPUT_NONE;

    auto rlambda = [&](Node* form, auto&& rlambda) -> void
    {
        for (const auto& child: form->getChildNodePtrs())
        {
            if (child->isFormParent())
            {
                rlambda(child.get(), rlambda);
            }
            else if (child->isForm())
            {
                if (child->hasValue(prop_base_file))
                {
                    if (child->as_string(prop_base_file) == child->getPropDefaultValue(prop_base_file) &&
                        getCodePreference(form) != GEN_LANG_CPLUSPLUS)
                    {
                        continue;
                    }
                    result |= OUTPUT_CPLUS;
                }
                if (not(flags & OUT_FLAG_IGNORE_DERIVED) && child->hasValue(prop_derived_file) &&
                    child->as_bool(prop_use_derived_class))
                {
                    if (auto path = getDerivedFilename(child.get()); path.size())
                    {
                        // Derived file is only output if it doesn't already exist
                        if (not path.file_exists())
                        {
                            result |= OUTPUT_DERIVED;
                        }
                    }
                }
                if (child->hasValue(prop_perl_file))
                {
                    if (child->isGen(gen_Images) || child->isGen(gen_Data))
                    {
                        if (child->as_string(prop_perl_file) == child->getPropDefaultValue(prop_perl_file) &&
                            getCodePreference(form) != GEN_LANG_PERL)
                        {
                            continue;
                        }
                    }
                    result |= OUTPUT_PERL;
                }
                if (child->hasValue(prop_python_file))
                {
                    if (child->isGen(gen_Images) || child->isGen(gen_Data))
                    {
                        if (child->as_string(prop_python_file) == child->getPropDefaultValue(prop_python_file) &&
                            getCodePreference(form) != GEN_LANG_PYTHON)
                        {
                            continue;
                        }
                    }
                    result |= OUTPUT_PYTHON;
                }
                if (child->hasValue(prop_ruby_file))
                {
                    if (child->isGen(gen_Images) || child->isGen(gen_Data))
                    {
                        if (child->as_string(prop_ruby_file) == child->getPropDefaultValue(prop_ruby_file) &&
                            getCodePreference(form) != GEN_LANG_RUBY)
                        {
                            continue;
                        }
                    }
                    result |= OUTPUT_RUBY;
                }
                if (child->hasValue(prop_rust_file))
                {
                    if (child->isGen(gen_Images) || child->isGen(gen_Data))
                    {
                        if (child->as_string(prop_rust_file) == child->getPropDefaultValue(prop_rust_file) &&
                            getCodePreference(form) != GEN_LANG_RUST)
                        {
                            continue;
                        }
                    }
                    result |= OUTPUT_RUST;
                }
                if (child->hasValue(prop_xrc_file))
                {
                    if (child->isGen(gen_Images) || child->isGen(gen_Data))
                    {
                        // Note that we do *not* ignore this if getCodePreference(form) !=
                        // GEN_LANG_XRC. If the language is using XRC for the UI, then the XRC must
                        // be generated as well.
                        if (child->as_string(prop_xrc_file) == child->getPropDefaultValue(prop_xrc_file))
                        {
                            continue;
                        }
                    }
                    result |= OUTPUT_XRC;
                }
#if GENERATE_NEW_LANG_CODE
                if (child->hasValue(prop_fortran_file))
                {
                    if (child->isGen(gen_Images) || child->isGen(gen_Data))
                    {
                        if (child->as_string(prop_fortran_file) == child->getPropDefaultValue(prop_fortran_file) &&
                            getCodePreference(form) != GEN_LANG_FORTRAN)
                        {
                            continue;
                        }
                    }
                    result |= OUTPUT_FORTRAN;
                }
                if (child->hasValue(prop_haskell_file))
                {
                    if (child->isGen(gen_Images) || child->isGen(gen_Data))
                    {
                        if (child->as_string(prop_haskell_file) == child->getPropDefaultValue(prop_haskell_file) &&
                            getCodePreference(form) != GEN_LANG_HASKELL)
                        {
                            continue;
                        }
                    }
                    result |= OUTPUT_HASKELL;
                }
                if (child->hasValue(prop_lua_file))
                {
                    if (child->isGen(gen_Images) || child->isGen(gen_Data))
                    {
                        if (child->as_string(prop_lua_file) == child->getPropDefaultValue(prop_lua_file) &&
                            getCodePreference(form) != GEN_LANG_LUA)
                        {
                            continue;
                        }
                    }
                    result |= OUTPUT_LUA;
                }
#endif  // GENERATE_NEW_LANG_CODE
            }
        }
    };

    rlambda(m_project_node.get(), rlambda);

    return result;
}

tt_string ProjectHandler::getDerivedFilename(Node* form) const
{
    tt_string path;

    ASSERT(form->isForm());

    if (not form->isForm() || !form->hasValue(prop_derived_file))
        return path;

    path = getDerivedDirectory(form, GEN_LANG_CPLUSPLUS);
    path.append_filename(form->as_string(prop_derived_file).filename());
    path.make_absolute();

    tt_string source_ext(".cpp");
    if (auto& extProp = as_string(prop_source_ext); extProp.size())
    {
        source_ext = extProp;
    }
    path.replace_extension(source_ext);
    return path;
}

void ProjectHandler::FindWxueFunctions(std::vector<Node*>& forms)
{
    auto ParseImageProps = [&](Node* node, auto&& ParseImageProps) -> void
    {
        for (const auto& child: node->getChildNodePtrs())
        {
            for (const auto& iter: child->getPropsVector())
            {
                if ((iter.type() == type_image || iter.type() == type_animation) && iter.hasValue())
                {
                    tt_string_vector parts(iter.as_string(), BMP_PROP_SEPARATOR, tt::TRIM::both);
                    if (parts.size() < IndexImage + 1)
                        continue;
                    if (parts[IndexType] == "Embed")
                    {
                        if (iter.type() == type_animation)
                        {
                            if (!m_form_Animation)
                            {
                                m_form_Animation = m_ImagesForm ? m_ImagesForm : child->getForm();
                            }
                        }
                        else
                        {
                            if (!m_form_Image)
                            {
                                m_form_Image = m_ImagesForm ? m_ImagesForm : child->getForm();
                            }
                            if (!m_form_BundleBitmaps && ProjectImages.GetPropertyImageBundle(parts))
                            {
                                m_form_BundleBitmaps = m_ImagesForm ? m_ImagesForm : child->getForm();
                            }
                        }
                    }
                    else if ((parts[IndexType] == "SVG"))
                    {
                        if (!m_form_BundleSVG)
                        {
                            m_form_BundleSVG = m_ImagesForm ? m_ImagesForm : child->getForm();
                        }
                    }

                    if (m_form_Animation && m_form_BundleSVG && m_form_BundleBitmaps && m_form_Image)
                    {
                        return;
                    }
                }
            }
            if (child->getChildCount())
            {
                ParseImageProps(child.get(), ParseImageProps);

                if (m_form_Animation && m_form_BundleSVG && m_form_BundleBitmaps && m_form_Image)
                {
                    return;
                }
            }
        }
    };

    m_form_BundleSVG = nullptr;
    m_form_BundleBitmaps = nullptr;
    m_form_Image = nullptr;
    m_form_Animation = nullptr;
    m_ImagesForm = nullptr;

    for (auto form: forms)
    {
        if (form->isGen(gen_Images))
        {
            m_ImagesForm = form;
            continue;
        }

        if (form->hasValue(prop_icon))
        {
            tt_string_vector parts(form->as_string(prop_icon), BMP_PROP_SEPARATOR, tt::TRIM::both);
            if (parts.size() >= IndexImage + 1)
            {
                if (parts[IndexType] == "Embed")
                {
                    if (!m_form_Image)
                        m_form_Image = form;
                    if (!m_form_BundleBitmaps && ProjectImages.GetPropertyImageBundle(parts))
                        m_form_BundleBitmaps = form;
                }
                else if ((parts[IndexType] == "SVG"))
                {
                    if (!m_form_BundleSVG)
                        m_form_BundleSVG = form;
                }
            }
        }

        ParseImageProps(form, ParseImageProps);
        if (m_form_Animation && m_form_BundleSVG && m_form_BundleBitmaps && m_form_Image)
        {
            return;
        }
    }
}

Node* ProjectHandler::getImagesForm()
{
    if (!m_ImagesForm && m_project_node->getChildCount() > 0 && m_project_node->getChild(0)->isGen(gen_Images))
    {
        m_ImagesForm = m_project_node->getChild(0);
    }
    return m_ImagesForm;
}

Node* ProjectHandler::getDataForm()
{
    if (!m_DataForm)
    {
        if (m_project_node->getChildCount() > 0 && m_project_node->getChild(0)->isGen(gen_Data))
        {
            m_DataForm = m_project_node->getChild(0);
        }
        else if (m_project_node->getChildCount() > 1 && m_project_node->getChild(1)->isGen(gen_Data))
        {
            m_DataForm = m_project_node->getChild(1);
        }
    }
    return m_DataForm;
}

int ProjectHandler::getLangVersion(GenLang language) const
{
    tt_string_view version = tt::emptystring;

    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
            version = m_project_node->as_string(prop_wxWidgets_version);
            break;

        case GEN_LANG_PERL:
            version = m_project_node->as_string(prop_wxPerl_version);
            break;

        case GEN_LANG_PYTHON:
            version = m_project_node->as_string(prop_wxPython_version);
            break;

        case GEN_LANG_RUBY:
            version = m_project_node->as_string(prop_wxRuby_version);
            break;

        case GEN_LANG_RUST:
            version = m_project_node->as_string(prop_wxRust_version);
            break;

        case GEN_LANG_XRC:
            version = m_project_node->as_string(prop_wxWidgets_version);
            break;

#if GENERATE_NEW_LANG_CODE
        case GEN_LANG_FORTRAN:
            version = m_project_node->as_string(prop_wxFortran_version);
            break;

        case GEN_LANG_HASKELL:
            version = m_project_node->as_string(prop_wxHaskell_version);
            break;

        case GEN_LANG_LUA:
            version = m_project_node->as_string(prop_wxLua_version);
            break;
#endif  // GENERATE_NEW_LANG_CODE

        default:
            FAIL_MSG(tt_string() << "Unknown language: " << language);
            break;
    }

    int major = 1;
    int minor = 0;
    int patch = 0;

    if (version.size())
    {
        if (version.contains("."))
        {
            tt_string_vector parts(version, '.');
            if (parts.size() > 0)
                major = tt::atoi(parts[0]);
            if (parts.size() > 1)
                minor = tt::atoi(parts[1]);
            if (parts.size() > 2)
                patch = tt::atoi(parts[2]);
        }
        else if (version.contains("-"))
        {
            tt_string_vector parts(version, '-');
            if (parts.size() > 0)
                major = tt::atoi(parts[0]);
            if (parts.size() > 1)
                minor = tt::atoi(parts[1]);
            if (parts.size() > 2)
                patch = tt::atoi(parts[2]);
        }
        else
        {
            major = tt::atoi(version);
            for (size_t pos = 0; pos < version.size(); ++pos)
            {
                if (!tt::is_digit(version[pos]))
                {
                    while (pos < version.size() && !tt::is_digit(version[pos]))
                    {
                        ++pos;
                    }
                    version.remove_prefix(pos);
                }
            }
            if (version.size())
            {
                minor = tt::atoi(version);
            }
            for (size_t pos = 0; pos < version.size(); ++pos)
            {
                if (!tt::is_digit(version[pos]))
                {
                    while (pos < version.size() && !tt::is_digit(version[pos]))
                    {
                        ++pos;
                    }
                    version.remove_prefix(pos);
                }
            }
            if (version.size())
            {
                patch = tt::atoi(version);
            }
        }
    }

    return major * 10000 + minor * 100 + patch;
}
