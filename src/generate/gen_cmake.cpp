//////////////////////////////////////////////////////////////////////////
// Purpose:   Auto-generate a .cmake file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_base.h"         // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_results.h"      // Code generation file writing functions
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "tt_view_vector.h"   // tt_view_vector -- Read/Write line-oriented strings/files

namespace
{
    enum CMakeWriteFlag
    {
        CMAKE_WRITE_NORMAL = 0,      // Normal write operation
        CMAKE_WRITE_CHECK_ONLY = 1,  // Only check if file needs writing
        CMAKE_WRITE_TEMP_FILE = 2    // Write to a temporary file
    };
}

int WriteCMakeFile(Node* parent_node, GenResults& results, int flag)
{
    if (parent_node->isGen(gen_folder) && !parent_node->hasValue(prop_folder_cmake_file))
    {
        if (!Project.as_bool(prop_generate_cmake) ||
            (parent_node->isGen(gen_Project) && !Project.hasValue(prop_cmake_file)))
        {
            return result::exists;
        }
    }

    tt_cwd cwd(true);

    // The generated files may be in a different directory then the project file, and if so, we
    // need to tread that directory as the root of the file.

    tt_string cmake_file;
    if (flag == CMAKE_WRITE_TEMP_FILE)
    {
        ASSERT(results.updated_files.size());
        cmake_file = results.updated_files[0];
    }
    else if (parent_node->isGen(gen_folder) && parent_node->hasValue(prop_folder_cmake_file))
    {
        cmake_file = parent_node->as_string(prop_folder_cmake_file);
    }
    else
    {
        cmake_file = Project.as_string(prop_cmake_file);
    }

    if (cmake_file.starts_with(".."))
    {
        tt_string new_dir(cmake_file);
        new_dir.remove_filename();
        tt::ChangeDir(new_dir);
    }
    else
    {
        Project.getProjectPath().ChangeDir();
    }
    if (cmake_file.find('.') == tt::npos)
    {
        cmake_file << ".cmake";
    }

    tt_cwd cur_dir;
    cur_dir.make_absolute();

    cmake_file.make_relative(cur_dir);
    tt_string cmake_file_dir(cmake_file);
    cmake_file_dir.remove_filename();
    if (cmake_file.size())
        cmake_file_dir.make_absolute();

    tt_string_vector out;
    out.emplace_back("# Generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor/");
    out.emplace_back("#");
    out.emplace_back("# DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!");
    out.emplace_back();

    out.emplace_back();
    tt_string var_name(Project.as_string(prop_cmake_varname));
    if (parent_node->isGen(gen_folder) && parent_node->hasValue(prop_folder_cmake_varname))
    {
        var_name = parent_node->as_string(prop_folder_cmake_varname);
    }
    out.at(out.size() - 1) << "set (" << var_name;
    out.emplace_back();

    // Recrusive lambda to collect forms in derived and non-derived vectors
    auto CollectForms = [](std::vector<Node*>& forms, std::vector<Node*>& derived_forms,
                           Node* node_start, auto&& CollectForms) -> void
    {
        if (!node_start)
        {
            node_start = Project.getProjectNode();
        }
        for (const auto& child: node_start->getChildNodePtrs())
        {
            if (node_start == Project.getProjectNode())
            {
                if (auto* node_folder = child->getFolder();
                    node_folder && node_folder->hasValue(prop_folder_cmake_file))
                {
                    // This file already got added to a different .cmake file
                    continue;
                }
            }

            if (child->isForm())
            {
                if (child->as_bool(prop_use_derived_class))
                {
                    derived_forms.push_back(child.get());
                }
                else
                {
                    forms.push_back(child.get());
                }
            }
            else
            {
                if (child->isGen(gen_folder) || child->isGen(gen_sub_folder))
                {
                    CollectForms(forms, derived_forms, child.get(), CollectForms);
                }
            }
        }
    };

    std::vector<Node*> forms;
    std::vector<Node*> derived_forms;
    CollectForms(forms, derived_forms, parent_node, CollectForms);

    auto OutputFilenames = [&](std::vector<Node*>& form_list)
    {
        std::set<tt_string> base_files;

        for (const auto& form: form_list)
        {
            if (!form->hasValue(prop_base_file) && !form->isGen(gen_Data))
            {
                continue;
            }

            if (parent_node == Project.getProjectNode())
            {
                if (auto* node_folder = form->getFolder();
                    node_folder && node_folder->hasValue(prop_folder_cmake_file))
                {
                    // This file already got added to a different .cmake file
                    continue;
                }
            }

            auto [path, has_base_file] = Project.GetOutputPath(form, GEN_LANG_CPLUSPLUS);
            if (!has_base_file)
            {
                // No file was specified. It's unlikely this would actually happen given the
                // form->hasValue(prop_base_file) above, but it does serve as a template check
                // -- an will prevent a problem if the above check is removed.
                continue;
            }

            if (cmake_file_dir.size())
                path.make_relative(cmake_file_dir);
            path.backslashestoforward();
            base_files.emplace(path);
        }

        for (auto base_file: base_files)
        {
            base_file.make_relative(cur_dir);
            base_file.backslashestoforward();
            base_file.remove_extension();

            tt_string source_ext(".cpp");
            if (auto& extProp = Project.as_string(prop_source_ext); extProp.size())
            {
                source_ext = extProp;
            }
            base_file.replace_extension(source_ext);

            out.emplace_back();
            out.at(out.size() - 1) << "    ${CMAKE_CURRENT_LIST_DIR}/" << base_file;
        }
    };

    if (forms.size())
    {
        if (derived_forms.size())
        {
            out.emplace_back("    # Non-base classes");
        }
        OutputFilenames(forms);
        if (derived_forms.size())
        {
            out.emplace_back();
            out.emplace_back("    # Base classes");
            OutputFilenames(derived_forms);
        }
    }
    else
    {
        OutputFilenames(derived_forms);
    }

    out.emplace_back();
    out.emplace_back(")");

    if (auto* data_form = Project.getDataForm(); data_form && data_form->getChildCount())
    {
        out.emplace_back();
        out.emplace_back();
        var_name = Project.as_string(prop_cmake_varname);
        var_name += "_data";
        out.at(out.size() - 1) << "set (" << var_name;
        out.emplace_back();

        for (auto& iter: data_form->getChildNodePtrs())
        {
            tt_string base_file = iter->as_string(prop_data_file);
            if (base_file.size())
            {
                base_file.make_relative(cur_dir);
                base_file.backslashestoforward();

                out.emplace_back();
                out.at(out.size() - 1) << "    ${CMAKE_CURRENT_LIST_DIR}/" << base_file;
            }
        }

        out.emplace_back();
        out.emplace_back(")");
    }

    if (flag == CMAKE_WRITE_TEMP_FILE)
    {
        return out.WriteFile(cmake_file) ? result::created : result::fail;
    }

    tt_view_vector current;

    // The return value is ignored because if the file doesn't exist then it will be created,
    // returning result::fail if it cannot be written.
    current.ReadFile(cmake_file);

    if (out.is_sameas(current))
    {
        return result::exists;
    }

    if (flag == CMAKE_WRITE_CHECK_ONLY)
    {
        results.updated_files.emplace_back(cmake_file);
        return result::needs_writing;
    }

    if (!out.WriteFile(cmake_file))
    {
        results.msgs.emplace_back() << "Cannot create or write to the file " << cmake_file << '\n';
        return result::fail;
    }

    results.updated_files.emplace_back(cmake_file);
    return result::created;
}
