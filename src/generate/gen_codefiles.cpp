/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate C++ Base code files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// #define THREADED_CODE_GEN

#if defined(THREADED_CODE_GEN)
    #include <future>
    #include <mutex>
    #include <thread>
#endif

#include "mainframe.h"

#include "file_codewriter.h"  // FileCodeWriter -- Classs to write code to disk
#include "gen_base.h"         // BaseCodeGenerator -- Generate Base class
#include "gen_results.h"      // Code generation file writing functions
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "write_code.h"       // Write code to Scintilla or file

using namespace code;

struct GenData
{
    GenData(GenResults& results, std::vector<tt_string>* pClassList)
    {
        this->presults = &results;
        this->pClassList = pClassList;
    }

    GenResults* presults { nullptr };
    std::vector<tt_string>* pClassList { nullptr };
    tt_string source_ext;
    tt_string header_ext;

#if !defined(THREADED_CODE_GEN)
    void AddUpdateFilename(tt_string& path)
    {
        presults->updated_files.emplace_back(path);
    };

    void AddResultMsg(tt_string& msg)
    {
        presults->msgs.emplace_back(msg);
    };

    void UpdateFileCount()
    {
        presults->file_count += 1;
    };

    void AddClassName(const tt_string& class_name)
    {
        if (pClassList)
        {
            pClassList->emplace_back(class_name);
        }
    };
#else
    std::mutex mutex_results;
    std::mutex mutex_class_list;

    void AddUpdateFilename(tt_string& path)
    {
        mutex_results.lock();
        presults->updated_files.emplace_back(path);
        mutex_results.unlock();
    };

    void AddResultMsg(tt_string& msg)
    {
        mutex_results.lock();
        presults->msgs.emplace_back(msg);
        mutex_results.unlock();
    };

    void UpdateFileCount()
    {
        mutex_results.lock();
        presults->file_count += 1;
        mutex_results.unlock();
    };

    void AddClassName(const tt_string& class_name)
    {
        if (pClassList)
        {
            mutex_class_list.lock();
            pClassList->emplace_back(class_name);
            mutex_class_list.unlock();
        }
    };
#endif
};

// While not required, this function can be run from a thread.
void GenThreadCpp(GenData& gen_data, Node* form)
{
    // These are just defined for convenience.
    tt_string& source_ext = gen_data.source_ext;
    tt_string& header_ext = gen_data.header_ext;

    tt_string path;

    if (auto& base_file = form->prop_as_string(prop_base_file); base_file.size())
    {
        path = base_file;
        // "filename_base" is the default filename given to all form files. Unless it's changed, no code will be
        // generated.
        if (path == "filename_base")
            return;
        if (auto* node_folder = form->get_folder(); node_folder && node_folder->HasValue(prop_folder_base_directory))
        {
            path = node_folder->as_string(prop_folder_base_directory);
            path.append_filename(base_file.filename());
        }
        else if (Project.HasValue(prop_base_directory) && !path.contains("/"))
        {
            path = Project.BaseDirectory().utf8_string();
            path.append_filename(base_file);
        }
        path.make_absolute();
        path.backslashestoforward();
    }
    else
    {
        gen_data.AddResultMsg(tt_string() << "No filename specified for " << form->prop_as_string(prop_class_name) << '\n');
        return;
    }

    BaseCodeGenerator codegen(GEN_LANG_CPLUSPLUS);

    path.replace_extension(header_ext);
    auto h_cw = std::make_unique<FileCodeWriter>(path.wx_str());
    codegen.SetHdrWriteCode(h_cw.get());

    path.replace_extension(source_ext);
    auto cpp_cw = std::make_unique<FileCodeWriter>(path.wx_str());
    codegen.SetSrcWriteCode(cpp_cw.get());

    codegen.GenerateCppClass(form);

    path.replace_extension(header_ext);

    int flags = flag_no_ui;
    if (gen_data.pClassList)
        flags |= flag_test_only;
    auto retval = h_cw->WriteFile(GEN_LANG_CPLUSPLUS, flags);

    if (retval > 0)
    {
        if (!gen_data.pClassList)
        {
            gen_data.AddUpdateFilename(path);
        }
        else
        {
            if (form->isGen(gen_Images))
            {
                // While technically this is a "form" it doesn't have the usual properties set
                gen_data.AddClassName(GenEnum::map_GenNames[gen_Images]);
            }
            else
            {
                gen_data.AddClassName(form->prop_as_string(prop_class_name));
            }
            return;
        }
    }
    else if (retval < 0)
    {
        gen_data.AddResultMsg(tt_string() << "Cannot create or write to the file " << path << '\n');
    }
    else  // retval == result::exists)
    {
        gen_data.UpdateFileCount();
    }

    path.replace_extension(source_ext);
    retval = cpp_cw->WriteFile(GEN_LANG_CPLUSPLUS, flags);

    if (retval > 0)
    {
        if (!gen_data.pClassList)
        {
            gen_data.AddUpdateFilename(path);
        }
        else
        {
            gen_data.AddClassName(form->prop_as_string(prop_class_name));
            return;
        }
    }

    else if (retval < 0)
    {
        gen_data.AddResultMsg(tt_string() << "Cannot create or write to the file " << path << '\n');
    }
    else  // retval == result::exists
    {
        gen_data.UpdateFileCount();
    }
}

bool GenerateCodeFiles(GenResults& results, std::vector<tt_string>* pClassList)
{
    if (Project.ChildCount() == 0)
    {
        wxMessageBox("You cannot generate any code until you have added a top level form.", "Code Generation");
        return false;
    }
    tt_cwd cwd(true);
    Project.ChangeDir();

    if (Project.as_bool(prop_generate_cmake) && !pClassList)
    {
        for (auto& iter: Project.ChildNodePtrs())
        {
            if (iter->isGen(gen_folder) && iter->HasValue(prop_folder_cmake_file))
            {
                if (WriteCMakeFile(iter.get(), results.updated_files, results.msgs) == result::created)
                {
                    ++results.file_count;
                }
            }
        }
        if (Project.HasValue(prop_cmake_file))
        {
            if (WriteCMakeFile(Project.ProjectNode(), results.updated_files, results.msgs) == result::created)
            {
                ++results.file_count;
            }
        }
    }

    tt_string path;

    tt_string source_ext(".cpp");
    tt_string header_ext(".h");

    if (auto& extProp = Project.value(prop_source_ext); extProp.size())
    {
        source_ext = extProp;
    }

    if (auto& extProp = Project.value(prop_header_ext); extProp.size())
    {
        header_ext = extProp;
    }

    std::vector<Node*> forms;
    Project.CollectForms(forms);

    GenData gen_data(results, pClassList);
    gen_data.source_ext = source_ext;
    gen_data.header_ext = header_ext;

#if defined(THREADED_CODE_GEN)
    auto num_cpus = std::thread::hardware_concurrency();

    // Keep in mind that GenThreadCpp() will itself create two threads when it calls
    // codegen.GenerateBaseClass(). These additional threads are very short lived, and the
    // calling thread will often block until they are done. GenThreadCpp() itself can block
    // any time it needs to update results. We don't want to create too many threads, but we
    // can rely on some blocking and therefore have more total threads created then there are
    // CPUs to run them. That means that in theory, setting max_threads = 10 could result in
    // 30 threads. In practice, threads will block or be deleted before that happens.

    // Also keep in mind that some Intel processors have a few threads that run slower than
    // normal threads. So even if you get a value of 24 cpus, only 20 of them will run at
    // full speed. Just another reason to keep the max_threads count below the maximum number
    // of CPUS.

    size_t max_threads = 2;
    if (num_cpus > 3)
        max_threads = num_cpus / 3;

    std::vector<std::thread> threads;
    size_t thread_idx = 0;
#endif

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    results.StartClock();
#endif
    for (const auto& form: forms)
    {
#if defined(THREADED_CODE_GEN)
        if (threads.size() < max_threads)
        {
            threads.emplace_back(GenThreadCpp, std::ref(gen_data), form);
        }
        else
        {
            threads[thread_idx].join();
            threads[thread_idx] = std::thread(GenThreadCpp, std::ref(gen_data), form);
            ++thread_idx;
            if (thread_idx >= threads.size())
                thread_idx = 0;
        }
#else   // not defined(THREADED_CODE_GEN)
        GenThreadCpp(gen_data, form);
#endif  // THREADED_CODE_GEN
    }

#if defined(THREADED_CODE_GEN)
    for (auto& thread: threads)
    {
        thread.join();
    }
#endif

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    results.EndClock();
#endif

    if (pClassList)
        return pClassList->size() > 0;
    else
        return results.updated_files.size() > 0;
}

void GenInhertedClass(GenResults& results)
{
    tt_cwd cwd;
    Project.ChangeDir();
    tt_string path;

    tt_string source_ext(".cpp");
    tt_string header_ext(".h");

    if (auto& extProp = Project.value(prop_source_ext); extProp.size())
    {
        source_ext = extProp;
    }

    if (auto extProp = Project.value(prop_header_ext); extProp.size())
    {
        header_ext = extProp;
    }

    std::vector<Node*> forms;
    Project.CollectForms(forms);

    for (const auto& form: forms)
    {
        if (auto& file = form->prop_as_string(prop_derived_file); file.size())
        {
            path = file;
            if (path.empty())
                continue;
            path.backslashestoforward();
            if (Project.HasValue(prop_derived_directory) && !path.contains("/"))
            {
                path = Project.value(prop_derived_directory);
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

        auto retval = codegen.GenerateDerivedClass(Project.ProjectNode(), form);
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
            retval = h_cw->WriteFile(GEN_LANG_CPLUSPLUS, flag_no_ui);
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
            retval = h_cw->WriteFile(GEN_LANG_CPLUSPLUS, flag_no_ui);

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
        retval = cpp_cw->WriteFile(GEN_LANG_CPLUSPLUS, flag_no_ui);
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

void GenerateTmpFiles(const std::vector<tt_string>& ClassList, pugi::xml_node root, int language)
{
    tt_cwd cwd(true);
    Project.ChangeDir();
    tt_string path;
    std::vector<tt_string> results;

    tt_string source_ext(".cpp");
    tt_string header_ext(".h");

    if (language == GEN_LANG_CPLUSPLUS)
    {
        if (auto& extProp = Project.value(prop_source_ext); extProp.size())
        {
            source_ext = extProp;
        }
        else
        {
            source_ext = ".cpp";
        }
        if (auto& extProp = Project.value(prop_header_ext); extProp.size())
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
    Project.CollectForms(forms);

    for (auto& iter_class: ClassList)
    {
        for (const auto& form: forms)
        {
            // The Images class doesn't have a prop_class_name, so use "Images". Note that this will fail if there is a real
            // form where the user set the class name to "Images". If this wasn't an Internal function, then we would need to
            // store nodes rather than class names.

            tt_string class_name(form->prop_as_string(prop_class_name));
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
                        else if (Project.HasValue(prop_base_directory) && !path.contains("/"))
                        {
                            path = Project.BaseDirectory().utf8_string();
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
                        else if (Project.HasValue(prop_python_output_folder) && !path.contains("/"))
                        {
                            path = Project.BaseDirectory(GEN_LANG_PYTHON).utf8_string();
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
                    codegen.GenerateCppClass(form);
                }
                else if (language == GEN_LANG_PYTHON)
                {
                    codegen.GeneratePythonClass(form);
                }

                bool new_hdr = false;
                if (language == GEN_LANG_CPLUSPLUS)
                {
                    // Currently, only C++ generates code from h_cw
                    new_hdr = (h_cw->WriteFile(GEN_LANG_CPLUSPLUS, flag_test_only) > 0);
                }

                bool new_src = (cpp_cw->WriteFile(GEN_LANG_CPLUSPLUS, flag_test_only) > 0);

                if (new_hdr || new_src)
                {
                    tt_string tmp_path(path);
                    if (auto pos_file = path.find_filename(); tt::is_found(pos_file))
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
                        codegen.GenerateCppClass(form);
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
                        h_cw->WriteFile(language, flag_no_ui);
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
                        cpp_cw->WriteFile(language, flag_no_ui);
                        path.replace_extension(source_ext);
                        path.make_relative(Project.ProjectPath().utf8_string());
                        paths.append_child("left").text().set(path.c_str());
                        paths.append_child("left-readonly").text().set("0");

                        tmp_path.make_relative(Project.ProjectPath().utf8_string());
                        paths.append_child("right").text().set(tmp_path.c_str());
                        paths.append_child("right-readonly").text().set("1");
                    }
                }
            }
        }
    }
}

#endif  // defined(INTERNAL_TESTING)
