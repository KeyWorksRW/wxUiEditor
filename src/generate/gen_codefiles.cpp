/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate C++ Base code files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "ttcwd_wx.h"  // cwd -- Class for storing and optionally restoring the current directory

#include "mainframe.h"

#include "gen_base.h"       // BaseCodeGenerator -- Generate Base class
#include "generate_dlg.h"   // GenerateDlg -- Dialog for choosing and generating specific language file(s)
#include "node.h"           // Node class
#include "project_class.h"  // Project class
#include "write_code.h"     // Write code to Scintilla or file

bool GenerateCodeFiles(GenResults& results, std::vector<ttlib::cstr>* pClassList)
{
    auto project = GetProject();
    if (project->GetChildCount() == 0)
    {
        wxMessageBox("You cannot generate any code until you have added a top level form.", "Code Generation");
        return false;
    }
    ttSaveCwd cwd;
    GetProject()->GetProjectPath().ChangeDir();

    if (project->prop_as_bool(prop_generate_cmake) && !pClassList)
    {
        for (auto& iter: project->GetChildNodePtrs())
        {
            if (iter->isGen(gen_folder) && iter->HasValue(prop_folder_cmake_file))
            {
                if (WriteCMakeFile(iter.get(), results.updated_files, results.msgs) == result::created)
                {
                    ++results.file_count;
                }
            }
        }
        if (project->HasValue(prop_cmake_file))
        {
            if (WriteCMakeFile(project, results.updated_files, results.msgs) == result::created)
            {
                ++results.file_count;
            }
        }
    }

    ttlib::cstr path;

    ttlib::cstr source_ext(".cpp");
    ttlib::cstr header_ext(".h");

    if (auto& extProp = project->prop_as_string(prop_source_ext); extProp.size())
    {
        source_ext = extProp;
    }

    if (auto& extProp = project->prop_as_string(prop_header_ext); extProp.size())
    {
        header_ext = extProp;
    }

    bool generate_result = true;
    std::vector<Node*> forms;
    project->CollectForms(forms);
    for (const auto& form: forms)
    {
        if (auto& base_file = form->prop_as_string(prop_base_file); base_file.size())
        {
            path = base_file;
            // "filename_base" is the default filename given to all form files. Unless it's changed, no code will be
            // generated.
            if (path == "filename_base")
                continue;
            if (auto* node_folder = form->get_folder(); node_folder && node_folder->HasValue(prop_folder_base_directory))
            {
                path = node_folder->as_string(prop_folder_base_directory);
                path.append_filename(base_file.filename());
            }
            else if (GetProject()->HasValue(prop_base_directory) && !path.contains("/"))
            {
                path = GetProject()->GetBaseDirectory().utf8_string();
                path.append_filename(base_file);
            }
            path.make_absolute();
            path.backslashestoforward();
        }
        else
        {
            results.msgs.emplace_back() << "No filename specified for " << form->prop_as_string(prop_class_name) << '\n';
            continue;
        }

        try
        {
            BaseCodeGenerator codegen(GEN_LANG_CPLUSPLUS);

            path.replace_extension(header_ext);
            auto h_cw = std::make_unique<FileCodeWriter>(path.wx_str());
            codegen.SetHdrWriteCode(h_cw.get());

            path.replace_extension(source_ext);
            auto cpp_cw = std::make_unique<FileCodeWriter>(path.wx_str());
            codegen.SetSrcWriteCode(cpp_cw.get());

            codegen.GenerateBaseClass(form);

            path.replace_extension(header_ext);
            auto retval = h_cw->WriteFile(pClassList != nullptr);

            if (retval > 0)
            {
                if (!pClassList)
                {
                    results.updated_files.emplace_back(path);
                }
                else
                {
                    if (form->isGen(gen_Images))
                    {
                        // While technically this is a "form" it doesn't have the usual properties set

                        pClassList->emplace_back(GenEnum::map_GenNames[gen_Images]);
                    }
                    else
                    {
                        pClassList->emplace_back(form->prop_as_string(prop_class_name));
                    }
                    continue;
                }
            }
            else if (retval < 0)
            {
                results.msgs.emplace_back() << "Cannot create or write to the file " << path << '\n';
                generate_result = false;
            }
            else  // retval == result::exists)
            {
                ++results.file_count;
            }

            path.replace_extension(source_ext);
            retval = cpp_cw->WriteFile(pClassList != nullptr);

            if (retval > 0)
            {
                if (!pClassList)
                {
                    results.updated_files.emplace_back(path);
                }
                else
                {
                    pClassList->emplace_back(form->prop_as_string(prop_class_name));
                    continue;
                }
            }

            else if (retval < 0)
            {
                results.msgs.emplace_back() << "Cannot create or write to the file " << path << '\n';
            }
            else  // retval == result::exists
            {
                ++results.file_count;
            }
        }
        catch (const std::exception& TESTING_PARAM(e))
        {
            MSG_ERROR(e.what());
            wxMessageBox(ttlib::cstr("An internal error occurred generating code files for ")
                             << form->prop_as_string(prop_base_file),
                         "Code generation");
            continue;
        }
    }
    return generate_result;
}

void GenInhertedClass(GenResults& results)
{
    auto project = GetProject();
    ttlib::cwd cwd;
    ttlib::ChangeDir(GetProject()->getProjectPath());
    ttlib::cstr path;

    ttlib::cstr source_ext(".cpp");
    ttlib::cstr header_ext(".h");

    if (auto& extProp = project->prop_as_string(prop_source_ext); extProp.size())
    {
        source_ext = extProp;
    }

    if (auto extProp = project->prop_as_string(prop_header_ext); extProp.size())
    {
        header_ext = extProp;
    }

    std::vector<Node*> forms;
    project->CollectForms(forms);

    for (const auto& form: forms)
    {
        if (auto& file = form->prop_as_string(prop_derived_file); file.size())
        {
            path = file;
            if (path.empty())
                continue;
            path.backslashestoforward();
            if (GetProject()->HasValue(prop_derived_directory) && !path.contains("/"))
            {
                path = GetProject()->as_string(prop_derived_directory);
                path.append_filename(file);
            }
            path.make_absolute();
            path.backslashestoforward();
            path.replace_extension(source_ext);
            if (path.file_exists())
            {
                // Count both source and header file
                path.replace_extension(header_ext);
                if (path.file_exists())
                {
                    results.file_count += 2;
                    continue;
                }
                else
                {
                    ++results.file_count;
                }
            }
            path.remove_extension();
        }
        else
        {
            continue;
        }

        BaseCodeGenerator codegen(GEN_LANG_CPLUSPLUS);

        path.replace_extension(header_ext);
        auto h_cw = std::make_unique<FileCodeWriter>(path.wx_str());
        codegen.SetHdrWriteCode(h_cw.get());

        path.replace_extension(source_ext);
        auto cpp_cw = std::make_unique<FileCodeWriter>(path.wx_str());
        codegen.SetSrcWriteCode(cpp_cw.get());

        auto retval = codegen.GenerateDerivedClass(project, form);
        if (retval == result::fail)
        {
            results.msgs.emplace_back() << "Cannot create or write to the file " << path << '\n';
            continue;
        }
        else if (retval == result::exists)
        {
            path.replace_extension(header_ext);
            if (path.file_exists())
            {
                ++results.file_count;
                continue;
            }

            // If we get here, the source file exists, but the header file does not.
            retval = h_cw->WriteFile();
            if (retval == result::fail)
            {
                results.msgs.emplace_back() << "Cannot create or write to the file " << path << '\n';
            }
            else if (retval == result::exists)
            {
                ++results.file_count;
            }
            else
            {
                results.updated_files.emplace_back(path);
            }
            continue;
        }
        else if (retval == result::ignored)
        {
            // Completely ignore this file
            continue;
        }

        path.replace_extension(header_ext);
        if (path.file_exists())
            retval = result::exists;
        else
            retval = h_cw->WriteFile();

        if (retval == result::fail)
        {
            results.msgs.emplace_back() << "Cannot create or write to the file " << path << '\n';
        }
        else if (retval == result::exists)
        {
            ++results.file_count;
        }
        else
        {
            results.updated_files.emplace_back(path);
        }

        path.replace_extension(source_ext);
        retval = cpp_cw->WriteFile();
        if (retval == result::fail)
        {
            results.msgs.emplace_back() << "Cannot create or write to the file " << path << '\n';
        }
        else if (retval == result::exists)
        {
            ++results.file_count;
        }
        else
        {
            results.updated_files.emplace_back(path);
        }
    }
}

#if defined(INTERNAL_TESTING)

    #include "pugixml.hpp"

void GenerateTmpFiles(const std::vector<ttlib::cstr>& ClassList, pugi::xml_node root, int language)
{
    auto project = GetProject();

    ttSaveCwd cwd;
    ttlib::ChangeDir(GetProject()->getProjectPath());
    ttlib::cstr path;
    std::vector<ttlib::cstr> results;

    ttlib::cstr source_ext(".cpp");
    ttlib::cstr header_ext(".h");

    if (language == GEN_LANG_CPLUSPLUS)
    {
        if (auto& extProp = project->prop_as_string(prop_source_ext); extProp.size())
        {
            source_ext = extProp;
        }
        else
        {
            source_ext = ".cpp";
        }
        if (auto& extProp = project->prop_as_string(prop_header_ext); extProp.size())
        {
            header_ext = extProp;
        }
        else
        {
            header_ext = ".h";
        }
    }
    else if (language == GEN_LANG_PYTHON)
    {
        source_ext = ".py";
        header_ext = ".py";
    }

    std::vector<Node*> forms;
    project->CollectForms(forms);

    for (auto& iter_class: ClassList)
    {
        for (const auto& form: forms)
        {
            // The Images class doesn't have a prop_class_name, so use "Images". Note that this will fail if there is a real
            // form where the user set the class name to "Images". If this wasn't an Internal function, then we would need to
            // store nodes rather than class names.

            ttlib::cstr class_name(form->prop_as_string(prop_class_name));
            if (form->isGen(gen_Images))
            {
                if (language != GEN_LANG_CPLUSPLUS)
                    continue;
                class_name = "Images";
            }

            if (class_name.is_sameas(iter_class))
            {
                path.clear();
                if (language == GEN_LANG_CPLUSPLUS)
                {
                    if (auto& base_file = form->prop_as_string(prop_base_file); base_file.size())
                    {
                        path = base_file;
                        // "filename_base" is the default filename given to all form files. Unless it's changed, no code will
                        // be generated.
                        if (path == "filename_base")
                            continue;
                        if (auto* node_folder = form->get_folder();
                            node_folder && node_folder->HasValue(prop_folder_base_directory))
                        {
                            path = node_folder->as_string(prop_folder_base_directory);
                            path.append_filename(base_file.filename());
                        }
                        else if (GetProject()->HasValue(prop_base_directory) && !path.contains("/"))
                        {
                            path = GetProject()->GetBaseDirectory().utf8_string();
                            path.append_filename(base_file);
                        }
                        path.backslashestoforward();
                    }
                }
                else if (language == GEN_LANG_PYTHON)
                {
                    if (auto& base_file = form->prop_as_string(prop_python_file); base_file.size())
                    {
                        path = base_file;
                        if (path.empty())
                            continue;

                        if (auto* node_folder = form->get_folder();
                            node_folder && node_folder->HasValue(prop_folder_python_output_folder))
                        {
                            path = node_folder->as_string(prop_folder_python_output_folder);
                            path.append_filename(base_file.filename());
                        }
                        else if (GetProject()->HasValue(prop_python_output_folder) && !path.contains("/"))
                        {
                            path = GetProject()->GetBaseDirectory(GEN_LANG_PYTHON).utf8_string();
                            path.append_filename(base_file);
                        }
                        path.backslashestoforward();
                    }
                }

                if (path.empty())
                    continue;

                BaseCodeGenerator codegen(language);

                path.replace_extension(header_ext);
                auto h_cw = std::make_unique<FileCodeWriter>(path.wx_str());
                codegen.SetHdrWriteCode(h_cw.get());

                path.replace_extension(source_ext);
                auto cpp_cw = std::make_unique<FileCodeWriter>(path.wx_str());
                codegen.SetSrcWriteCode(cpp_cw.get());

                if (language == GEN_LANG_CPLUSPLUS)
                {
                    codegen.GenerateBaseClass(form);
                }
                else if (language == GEN_LANG_PYTHON)
                {
                    codegen.GeneratePythonClass(form);
                }

                bool new_hdr = false;
                if (language == GEN_LANG_CPLUSPLUS)
                {
                    // Currently, only C++ generates code from h_cw
                    new_hdr = (h_cw->WriteFile(true) > 0);
                }

                bool new_src = (cpp_cw->WriteFile(true) > 0);

                if (new_hdr || new_src)
                {
                    ttlib::cstr tmp_path(path);
                    if (auto pos_file = path.find_filename(); ttlib::is_found(pos_file))
                    {
                        tmp_path.insert(pos_file, "~wxue_");
                    }
                    else
                    {
                        ASSERT(tmp_path.size())
                        tmp_path.insert(0, "~wxue_");
                    }

                    tmp_path.replace_extension(header_ext);
                    h_cw = std::make_unique<FileCodeWriter>(tmp_path.wx_str());
                    codegen.SetHdrWriteCode(h_cw.get());

                    tmp_path.replace_extension(source_ext);
                    cpp_cw = std::make_unique<FileCodeWriter>(tmp_path.wx_str());
                    codegen.SetSrcWriteCode(cpp_cw.get());

                    if (language == GEN_LANG_CPLUSPLUS)
                    {
                        codegen.GenerateBaseClass(form);
                    }
                    else if (language == GEN_LANG_PYTHON)
                    {
                        codegen.GeneratePythonClass(form);
                    }

                    // WinMerge accepts an XML file the provides the left and right filenames
                    // to compare. After we write a file, we update the XML file with the
                    // name pair.

                    if (new_hdr)
                    {
                        auto paths = root.append_child("paths");
                        tmp_path.replace_extension(header_ext);
                        h_cw->WriteFile();
                        path.replace_extension(header_ext);
                        paths.append_child("left").text().set(path.c_str());
                        paths.append_child("left-readonly").text().set("0");

                        paths.append_child("right").text().set(tmp_path.c_str());
                        paths.append_child("right-readonly").text().set("1");
                    }
                    if (new_src)
                    {
                        auto paths = root.append_child("paths");
                        tmp_path.replace_extension(source_ext);
                        cpp_cw->WriteFile();
                        path.replace_extension(source_ext);
                        paths.append_child("left").text().set(path.c_str());
                        paths.append_child("left-readonly").text().set("0");

                        paths.append_child("right").text().set(tmp_path.c_str());
                        paths.append_child("right-readonly").text().set("1");
                    }
                }
            }
        }
    }
}

#endif  // defined(INTERNAL_TESTING)
