/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate code files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <ttcvector.h>  // cstrVector -- Vector of ttlib::cstr strings
#include <ttcwd.h>      // cwd -- Class for storing and optionally restoring the current directory

#include "mainframe.h"

#include "gen_base.h"    // BaseCodeGenerator -- Generate Base class
#include "mainapp.h"     // App -- Main application class
#include "node.h"        // Node class
#include "uifuncs.h"     // Miscellaneous functions for displaying UI
#include "write_code.h"  // Write code to Scintilla or file

bool GenerateCodeFiles(wxWindow* parent, bool NeedsGenerateCheck)
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
    ttlib::cstrVector results;

    ttlib::cstr source_ext(".cpp");
    ttlib::cstr header_ext(".h");

    if (auto extProp = project->get_value_ptr("source_ext"); extProp)
    {
        source_ext = *extProp;
    }

    if (auto extProp = project->get_value_ptr("header_ext"); extProp)
    {
        header_ext = *extProp;
    }

    size_t currentFiles = 0;

    for (size_t pos = 0; pos < project->GetChildCount(); ++pos)
    {
        auto form = project->GetChild(pos);
        if (auto base_file = form->get_value_ptr("base_file"); base_file)
        {
            path = *base_file;
            if (path == "filename_base")
                return false;
            path.make_absolute();
            path.backslashestoforward();
        }
        else
        {
            results.emplace_back() << _tt("No filename specified for ") << *form->get_value_ptr(txt_class_name) << '\n';
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
            if (NeedsGenerateCheck && retval > 0)
                return true;
            if (retval < 0)
            {
                results.emplace_back() << _tt(strIdCantWrite) << path << '\n';
            }
            else if (retval == 0)
            {
                ++currentFiles;
            }
            else
            {
                results.emplace_back() << path << _tt(strIdSaved) << '\n';
            }

            path.replace_extension(source_ext);
            retval = cpp_cw->WriteFile(NeedsGenerateCheck);
            if (NeedsGenerateCheck && retval > 0)
                return true;

            if (retval < 0)
            {
                results.emplace_back() << _tt(strIdCantWrite) << path << '\n';
            }
            else if (retval == 0)
            {
                ++currentFiles;
            }
            else
            {
                results.emplace_back() << path << _tt(strIdSaved) << '\n';
            }
        }
        catch (const std::exception& DBG_PARAM(e))
        {
            MSG_ERROR(e.what());
            appMsgBox(ttlib::cstr("An internal error occurred generating code files for ")
                          << form->prop_as_string("base_file"),
                      _tt("Code generation"));
            continue;
        }
    }

    if (!parent || NeedsGenerateCheck)
    {
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
    ttlib::cstrVector results;

    ttlib::cstr source_ext(".cpp");
    ttlib::cstr header_ext(".h");

    if (auto extProp = project->get_value_ptr("source_ext"); extProp)
    {
        source_ext = *extProp;
    }

    if (auto extProp = project->get_value_ptr("header_ext"); extProp)
    {
        header_ext = *extProp;
    }

    for (size_t pos = 0; pos < project->GetChildCount(); ++pos)
    {
        auto form = project->GetChildPtr(pos);
        if (auto file = form->get_value_ptr("derived_file"); file)
        {
            path = *file;
            if (path.empty())
                continue;
            path.make_relative(wxGetApp().getProjectPath());
            path.backslashestoforward();
            path.replace_extension(source_ext);
            if (path.file_exists())
            {
                results.emplace_back() << path << _tt(" already exists") << '\n';
                continue;
            }
            path.remove_extension();
        }
        else
        {
            results.emplace_back() << _tt("No filename specified for ") << form->get_value_ptr(txt_var_name)->c_str() << '\n';
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
        if (retval == result::fail)
        {
            results.emplace_back() << _tt(strIdCantWrite) << path << '\n';
            continue;
        }
        else if (retval == result::exists)
        {
            results.emplace_back() << path << _tt(" already exists") << '\n';
            continue;
        }
        else if (retval == result::ignored)
        {
            // Completely ignore this file -- either the name is empty, or it hasn't changed from the default "filename"
            continue;
        }

        path.replace_extension(header_ext);
        retval = h_cw->WriteFile();

        if (retval < 0)
        {
            results.emplace_back() << _tt(strIdCantWrite) << path << '\n';
        }
        else if (retval == 0)
        {
            results.emplace_back() << path << _tt(" is current") << '\n';
        }
        else
        {
            results.emplace_back() << path << _tt(strIdSaved) << '\n';
        }

        path.replace_extension(source_ext);
        retval = cpp_cw->WriteFile();
        if (retval < 0)
        {
            results.emplace_back() << _tt(strIdCantWrite) << path << '\n';
        }
        else if (retval == 0)
        {
            results.emplace_back() << path << _tt(" already exists") << '\n';
        }
        else
        {
            results.emplace_back() << path << _tt(strIdSaved) << '\n';
        }
    }

    if (results.size())
    {
        // TODO: [KeyWorks - 07-03-2020] Ultimately this should be put in a dialog with a list control.
        ttlib::cstr msg;
        for (auto& iter: results)
        {
            msg += iter;
        }
        appMsgBox(msg, _tt(strIdTitleCodeGeneration), wxOK);
    }
    else
    {
        appMsgBox(_tt("There were no derived filenames specified -- nothing to generate.\n\nAdd a filename to the "
                      "derived_filename "
                      "property to generate a derived file."),
                  _tt(strIdTitleCodeGeneration), wxOK);
    }
}
