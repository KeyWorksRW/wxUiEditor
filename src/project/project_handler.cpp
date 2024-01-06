/////////////////////////////////////////////////////////////////////////////
// Purpose:   ProjectHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <fstream>
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

void ProjectHandler::Initialize(NodeSharedPtr project, bool allow_ui)
{
    m_project_node = project;
    m_allow_ui = allow_ui;

    // Note that the ProjectImage class keeps a shared pointer to the project node, so the
    // reference count won't go to zero until we call this function.

    ProjectImages.Initialize(m_project_node, allow_ui);
    ProjectData.Clear();
}

void ProjectHandler::setProjectFile(const tt_string& file)
{
    m_projectFile = file;
    m_projectPath = m_projectFile;
    m_projectPath.make_absolute();
    m_projectPath.remove_filename();
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
    std::set<std::string_view> xrc_filenames;
    std::set<std::string_view> python_filenames;
    std::set<std::string_view> ruby_filenames;

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
        if (iter->hasValue(prop_xrc_file))
            xrc_filenames.insert(iter->as_string(prop_xrc_file));
        if (iter->hasValue(prop_python_file))
            python_filenames.insert(iter->as_string(prop_python_file));
        if (iter->hasValue(prop_ruby_file))
            ruby_filenames.insert(iter->as_string(prop_ruby_file));
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
    lambda(xrc_filenames, prop_xrc_file);
    lambda(python_filenames, prop_python_file);
    lambda(ruby_filenames, prop_ruby_file);
}

tt_string ProjectHandler::ArtDirectory() const
{
    tt_string result;

    if (m_project_node->hasValue(prop_art_directory))
        result = m_project_node->as_string(prop_art_directory);
    if (result.empty())
        result = m_projectPath;

    result.make_absolute();

    return result;
}

tt_string ProjectHandler::getBaseDirectory(Node* node, int language) const
{
    // GetOutputPath() will handle a situation where the base filename contains a directory
    // prefix, which might or might not be a duplicate of any project or folder's output
    // directory.
    auto result = GetOutputPath(node, language);
    if (result.second)
    {
        result.first.remove_filename();
    }

    return result.first;
}

std::pair<tt_string, bool> ProjectHandler::GetOutputPath(Node* form, int language) const
{
    tt_string result;
    Node* folder = form->getFolder();
    if (folder)
    {
        if (language == GEN_LANG_CPLUSPLUS && folder->hasValue(prop_folder_base_directory))
            result = folder->as_string(prop_folder_base_directory);
        else if (language == GEN_LANG_PYTHON && folder->hasValue(prop_folder_python_output_folder))
            result = folder->as_string(prop_folder_python_output_folder);
        else if (language == GEN_LANG_RUBY && folder->hasValue(prop_folder_ruby_output_folder))
            result = folder->as_string(prop_folder_ruby_output_folder);
        else if (language == GEN_LANG_XRC && folder->hasValue(prop_folder_xrc_directory))
            result = folder->as_string(prop_folder_xrc_directory);
#if defined(INTERNAL_TESTING)
        else if (language == GEN_LANG_GOLANG && folder->hasValue(prop_folder_golang_output_folder))
            result = folder->as_string(prop_folder_golang_output_folder);
        else if (language == GEN_LANG_LUA && folder->hasValue(prop_folder_lua_output_folder))
            result = folder->as_string(prop_folder_lua_output_folder);
        else if (language == GEN_LANG_PERL && folder->hasValue(prop_folder_perl_output_folder))
            result = folder->as_string(prop_folder_perl_output_folder);
        else if (language == GEN_LANG_RUST && folder->hasValue(prop_folder_rust_output_folder))
            result = folder->as_string(prop_folder_rust_output_folder);
#endif
    }

    // Even if the node has a folder parent, there may not be a directory set for it, so check
    // result and if it's empty use the project directory properties.
    if (result.empty() || !folder)
    {
        if (language == GEN_LANG_CPLUSPLUS && m_project_node->hasValue(prop_base_directory))
            result = m_project_node->as_string(prop_base_directory);
        else if (language == GEN_LANG_PYTHON && m_project_node->hasValue(prop_python_output_folder))
            result = m_project_node->as_string(prop_python_output_folder);
        else if (language == GEN_LANG_RUBY && m_project_node->hasValue(prop_ruby_output_folder))
            result = m_project_node->as_string(prop_ruby_output_folder);
        else if (language == GEN_LANG_XRC && m_project_node->hasValue(prop_xrc_directory))
            result = m_project_node->as_string(prop_xrc_directory);
#if defined(INTERNAL_TESTING)
        else if (language == GEN_LANG_GOLANG && m_project_node->hasValue(prop_golang_output_folder))
            result = m_project_node->as_string(prop_golang_output_folder);
        else if (language == GEN_LANG_LUA && m_project_node->hasValue(prop_lua_output_folder))
            result = m_project_node->as_string(prop_lua_output_folder);
        else if (language == GEN_LANG_PERL && m_project_node->hasValue(prop_perl_output_folder))
            result = m_project_node->as_string(prop_perl_output_folder);
        else if (language == GEN_LANG_RUST && m_project_node->hasValue(prop_rust_output_folder))
            result = m_project_node->as_string(prop_rust_output_folder);
#endif
    }

    if (result.empty())
        result = m_projectPath;

    tt_string base_file;
    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
            base_file = form->as_string(prop_base_file);
            break;
        case GEN_LANG_PYTHON:
            base_file = form->as_string(prop_python_file);
            break;
        case GEN_LANG_RUBY:
            base_file = form->as_string(prop_ruby_file);
            break;
        case GEN_LANG_XRC:
            base_file = form->as_string(prop_xrc_file);
            break;
#if defined(INTERNAL_TESTING)
        case GEN_LANG_GOLANG:
            base_file = form->as_string(prop_golang_file);
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
#endif
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
        else
        {
            result += '/';
        }

        result += base_file;
    }
    else
    {
        result.append_filename(base_file);
    }

    result.make_absolute();
    result.backslashestoforward();

    return std::make_pair(result, true);
}

// Note that this will return a directory for GEN_LANG_PYTHON and GEN_LANG_XRC even though we currently
// don't generate derived files for those languages.
tt_string ProjectHandler::getDerivedDirectory(Node* node, int language) const
{
    tt_string result;

    Node* folder = node->getFolder();
    if (folder)
    {
        if (language == GEN_LANG_CPLUSPLUS && folder->hasValue(prop_folder_derived_directory))
            result = folder->as_string(prop_folder_base_directory);
        else if (language == GEN_LANG_PYTHON && folder->hasValue(prop_folder_python_output_folder))
            result = folder->as_string(prop_folder_python_output_folder);
        else if (language == GEN_LANG_RUBY && folder->hasValue(prop_folder_ruby_output_folder))
            result = folder->as_string(prop_folder_python_output_folder);
        else if (language == GEN_LANG_XRC && folder->hasValue(prop_folder_xrc_directory))
            result = folder->as_string(prop_folder_xrc_directory);
    }

    // Even if the node has a folder parent, there may not be a directory set for it, so check
    // result and if it's empty use the project directory properties.
    if (result.empty() || !folder)
    {
        if (language == GEN_LANG_CPLUSPLUS && m_project_node->hasValue(prop_derived_directory))
            result = m_project_node->as_string(prop_base_directory);
        else if (language == GEN_LANG_PYTHON && m_project_node->hasValue(prop_python_output_folder))
            result = m_project_node->as_string(prop_python_output_folder);
        else if (language == GEN_LANG_RUBY && m_project_node->hasValue(prop_ruby_output_folder))
            result = m_project_node->as_string(prop_ruby_output_folder);
        else if (language == GEN_LANG_XRC && m_project_node->hasValue(prop_xrc_directory))
            result = m_project_node->as_string(prop_xrc_directory);
    }

    if (result.empty())
        result = m_projectPath;

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

int ProjectHandler::getCodePreference(Node* node) const
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

    if (value == "C++")
        return GEN_LANG_CPLUSPLUS;
    else if (value == "Python")
        return GEN_LANG_PYTHON;
    else if (value == "Ruby")
        return GEN_LANG_RUBY;
    else if (value == "XRC")
        return GEN_LANG_XRC;
    else
        return GEN_LANG_CPLUSPLUS;
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
                if (not(flags & OUT_FLAG_IGNORE_XRC) && child->hasValue(prop_xrc_file))
                {
                    result |= OUTPUT_XRC;
                }
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

int ProjectHandler::get_WidgetsMinorVersion()
{
    tt_string_view version = m_project_node->as_string(prop_wxWidgets_version);
    if (version.empty())
        return 1;
    ASSERT(version.find('.') != tt::npos);
    version = version.substr(version.find('.') + 1);
    return tt::atoi(version);
}
