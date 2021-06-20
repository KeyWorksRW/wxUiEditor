/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate code files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "ttcwd.h"  // cwd -- Class for storing and optionally restoring the current directory

#include "mainframe.h"

#include "gen_base.h"    // BaseCodeGenerator -- Generate Base class
#include "mainapp.h"     // App -- Main application class
#include "node.h"        // Node class
#include "uifuncs.h"     // Miscellaneous functions for displaying UI
#include "write_code.h"  // Write code to Scintilla or file

bool GenerateCodeFiles(wxWindow* parent, bool NeedsGenerateCheck, std::vector<ttlib::cstr>* pClassList)
{
    auto project = wxGetApp().GetProject();
    if (project->GetChildCount() == 0)
    {
        if (NeedsGenerateCheck)
            return false;

        appMsgBox(_tt("You cannot generate any code until you have added a top level form."), _tt(strIdTitleCodeGeneration));
        return false;
    }
    ttSaveCwd cwd;
    ttlib::ChangeDir(wxGetApp().getProjectPath());
    ttlib::cstr path;
    std::vector<ttlib::cstr> results;

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

    size_t currentFiles = 0;

    for (size_t pos = 0; pos < project->GetChildCount(); ++pos)
    {
        auto form = project->GetChild(pos);
        if (auto& base_file = form->prop_as_string(prop_base_file); base_file.size())
        {
            path = base_file;
            // "filename_base" is the default filename given to all form files. Unless it's changed, no code will be
            // generated.
            if (path == "filename_base")
                continue;
            path.make_absolute();
            path.backslashestoforward();
        }
        else
        {
            results.emplace_back() << _tt("No filename specified for ") << form->prop_as_string(prop_class_name) << '\n';
        }

        try
        {
            BaseCodeGenerator codegen;

            path.replace_extension(header_ext);
            auto h_cw = std::make_unique<FileCodeWriter>(path.wx_str());
            codegen.SetHdrWriteCode(h_cw.get());

            path.replace_extension(source_ext);
            auto cpp_cw = std::make_unique<FileCodeWriter>(path.wx_str());
            codegen.SetSrcWriteCode(cpp_cw.get());

            codegen.GenerateBaseClass(project, form);

            path.replace_extension(header_ext);
            auto retval = h_cw->WriteFile(NeedsGenerateCheck);

            if (retval > 0)
            {
                if (!NeedsGenerateCheck)
                {
                    results.emplace_back() << path << _tt(strIdSaved) << '\n';
                }
                else
                {
                    if (pClassList)
                    {
                        pClassList->emplace_back(form->prop_as_string(prop_class_name));
                        continue;
                    }
                    else
                    {
                        return true;
                    }
                }
            }
            else if (retval < 0)
            {
                results.emplace_back() << _tt(strIdCantWrite) << path << '\n';
            }
            else  // retval == result::exists)
            {
                ++currentFiles;
            }

            path.replace_extension(source_ext);
            retval = cpp_cw->WriteFile(NeedsGenerateCheck);

            if (retval > 0)
            {
                if (!NeedsGenerateCheck)
                {
                    results.emplace_back() << path << _tt(strIdSaved) << '\n';
                }
                else
                {
                    if (pClassList)
                    {
                        pClassList->emplace_back(form->prop_as_string(prop_class_name));
                        continue;
                    }
                    else
                    {
                        return true;
                    }
                }
            }

            else if (retval < 0)
            {
                results.emplace_back() << _tt(strIdCantWrite) << path << '\n';
            }
            else  // retval == result::exists
            {
                ++currentFiles;
            }
        }
        catch (const std::exception& DBG_PARAM(e))
        {
            MSG_ERROR(e.what());
            appMsgBox(ttlib::cstr("An internal error occurred generating code files for ")
                          << form->prop_as_string(prop_base_file),
                      _tt("Code generation"));
            continue;
        }
    }

    if (NeedsGenerateCheck)
    {
        if (pClassList && pClassList->size())
            return true;
        else
            return false;
    }

    if (results.size())
    {
        ttlib::cstr msg;
        for (auto& iter: results)
        {
            msg += iter;
        }

        if (currentFiles)
        {
            msg << '\n' << _tt("The other ") << currentFiles << " generated files are current";
        }

        appMsgBox(msg, _tt(strIdTitleCodeGeneration), wxOK, parent);
    }
    else if (currentFiles && parent)
    {
        ttlib::cstr msg;
        msg << '\n' << _tt("All ") << currentFiles << " generated files are current";
        appMsgBox(msg, _tt(strIdTitleCodeGeneration), wxOK, parent);
    }
    return true;
}

void MainFrame::OnGenInhertedClass(wxCommandEvent& WXUNUSED(e))
{
    auto& project = wxGetApp().GetProjectPtr();
    ttlib::cwd cwd;
    ttlib::ChangeDir(wxGetApp().getProjectPath());
    ttlib::cstr path;
    std::vector<ttlib::cstr> results;

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

    size_t currentFiles = 0;

    for (size_t pos = 0; pos < project->GetChildCount(); ++pos)
    {
        auto form = project->GetChildPtr(pos);
        if (auto& file = form->prop_as_string(prop_derived_file); file.size())
        {
            path = file;
            if (path.empty())
                continue;
            path.make_relative(wxGetApp().getProjectPath());
            path.backslashestoforward();
            path.replace_extension(source_ext);
            if (path.file_exists())
            {
                // Count both source and header file
                currentFiles += 2;
                continue;
            }
            path.remove_extension();
        }
        else
        {
            continue;
        }

        BaseCodeGenerator codegen;

        path.replace_extension(header_ext);
        auto h_cw = std::make_unique<FileCodeWriter>(path.wx_str());
        codegen.SetHdrWriteCode(h_cw.get());

        path.replace_extension(source_ext);
        auto cpp_cw = std::make_unique<FileCodeWriter>(path.wx_str());
        codegen.SetSrcWriteCode(cpp_cw.get());

        auto retval = codegen.GenerateDerivedClass(project.get(), form.get());
        ASSERT_MSG(retval != result::exists, "this should be impossible since we checked above")
        if (retval == result::fail)
        {
            results.emplace_back() << _tt(strIdCantWrite) << path << '\n';
            continue;
        }
        else if (retval == result::exists)
        {
            ++currentFiles;
            continue;
        }
        else if (retval == result::ignored)
        {
            // Completely ignore this file
            continue;
        }

        path.replace_extension(header_ext);
        retval = h_cw->WriteFile();

        if (retval == result::fail)
        {
            results.emplace_back() << _tt(strIdCantWrite) << path << '\n';
        }
        else if (retval == result::exists)
        {
            ++currentFiles;
        }
        else
        {
            results.emplace_back() << path << _tt(strIdSaved) << '\n';
        }

        path.replace_extension(source_ext);
        retval = cpp_cw->WriteFile();
        if (retval == result::fail)
        {
            results.emplace_back() << _tt(strIdCantWrite) << path << '\n';
        }
        else if (retval == result::exists)
        {
            ++currentFiles;
        }
        else
        {
            results.emplace_back() << path << _tt(strIdSaved) << '\n';
        }
    }

    if (results.size())
    {
        ttlib::cstr msg;
        for (auto& iter: results)
        {
            msg += iter;
        }

        if (currentFiles)
        {
            msg << '\n' << _tt("The other ") << currentFiles << " derived files have already been created";
        }

        appMsgBox(msg, _tt(strIdTitleCodeGeneration), wxOK);
    }
    else if (currentFiles)
    {
        ttlib::cstr msg;
        msg << '\n' << _tt("All ") << currentFiles << " derived files have already been created";
        appMsgBox(msg, _tt(strIdTitleCodeGeneration), wxOK);
    }
    else
    {
        appMsgBox(_tt("There were no derived filenames specified -- nothing to generate.\n\nAdd a filename to the "
                      "derived_filename property to generate a derived file."),
                  _tt(strIdTitleCodeGeneration), wxOK);
    }
}

#if defined(_DEBUG)
    #include "../pugixml/pugixml.hpp"

void GenerateTmpFiles(const std::vector<ttlib::cstr>& ClassList, pugi::xml_node root)
{
    auto project = wxGetApp().GetProject();

    ttSaveCwd cwd;
    ttlib::ChangeDir(wxGetApp().getProjectPath());
    ttlib::cstr path;
    std::vector<ttlib::cstr> results;

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

    for (auto& iter_class: ClassList)
    {
        for (size_t pos = 0; pos < project->GetChildCount(); ++pos)
        {
            auto form = project->GetChild(pos);
            if (form->prop_as_string(prop_class_name).is_sameas(iter_class))
            {
                BaseCodeGenerator codegen;

                // At this point we know which form has changes, but we don't know if it's the src file, the header file, or
                // both, so we need to check again.
                ttlib::cstr base_file(form->prop_as_string(prop_base_file));
                base_file.replace_extension(header_ext);
                base_file.make_absolute();

                base_file.replace_extension(header_ext);
                auto h_cw = std::make_unique<FileCodeWriter>(base_file.wx_str());
                codegen.SetHdrWriteCode(h_cw.get());

                base_file.replace_extension(source_ext);
                auto cpp_cw = std::make_unique<FileCodeWriter>(base_file.wx_str());
                codegen.SetSrcWriteCode(cpp_cw.get());

                codegen.GenerateBaseClass(project, form);

                base_file.replace_extension(header_ext);
                bool new_hdr = (h_cw->WriteFile(true) > 0);

                base_file.replace_extension(source_ext);
                bool new_src = (cpp_cw->WriteFile(true) > 0);

                if (new_hdr)
                {
                    path = "~wxue_";
                    path << form->prop_as_string(prop_base_file);
                    path.make_absolute();

                    path.replace_extension(header_ext);
                    h_cw = std::make_unique<FileCodeWriter>(path.wx_str());
                    codegen.SetHdrWriteCode(h_cw.get());

                    path.replace_extension(source_ext);
                    cpp_cw = std::make_unique<FileCodeWriter>(path.wx_str());
                    codegen.SetSrcWriteCode(cpp_cw.get());

                    codegen.GenerateBaseClass(project, form);

                    path.replace_extension(header_ext);
                    h_cw->WriteFile();

                    auto paths = root.append_child("paths");
                    base_file.replace_extension(header_ext);
                    paths.append_child("left").text().set(base_file.c_str());
                    paths.append_child("left-readonly").text().set("0");

                    paths.append_child("right").text().set(path.c_str());
                    paths.append_child("right-readonly").text().set("1");
                }

                if (new_src)
                {
                    path = "~wxue_";
                    path << form->prop_as_string(prop_base_file);
                    path.make_absolute();

                    path.replace_extension(header_ext);
                    h_cw = std::make_unique<FileCodeWriter>(path.wx_str());
                    codegen.SetHdrWriteCode(h_cw.get());

                    path.replace_extension(source_ext);
                    cpp_cw = std::make_unique<FileCodeWriter>(path.wx_str());
                    codegen.SetSrcWriteCode(cpp_cw.get());

                    codegen.GenerateBaseClass(project, form);

                    path.replace_extension(source_ext);
                    cpp_cw->WriteFile();

                    auto paths = root.append_child("paths");
                    base_file.replace_extension(source_ext);
                    paths.append_child("left").text().set(base_file.c_str());
                    paths.append_child("left-readonly").text().set("0");

                    paths.append_child("right").text().set(path.c_str());
                    paths.append_child("right-readonly").text().set("1");
                }
            }
        }
    }
}

#endif  // _DEBUG
