/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate C++ Base code files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "file_codewriter.h"  // FileCodeWriter -- Classs to write code to disk
#include "gen_results.h"      // Code generation file writing functions
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class

#include "gen_cpp.h"     // CppCodeGenerator -- Generate C++ code
#include "gen_perl.h"    // PerlCodeGenerator class
#include "gen_python.h"  // PythonCodeGenerator -- Generate wxPython code
#include "gen_ruby.h"    // RubyCodeGenerator -- Generate wxRuby code
#include "gen_xrc.h"     // XrcGenerator -- Generate XRC code

#include "pugixml.hpp"
#include <memory>

#include "wxue_namespace/wxue_string.h"  // wxue::string, wxue::SaveCwd

using namespace code;

auto GenInhertedClass(GenResults& results) -> void
{
    wxue::SaveCwd cwd(wxue::restore_cwd);
    Project.ChangeDir();
    wxue::string path;

    wxue::string source_ext(".cpp");
    wxue::string header_ext(".h");

    if (const auto& extProp = Project.as_string(prop_source_ext); extProp.size())
    {
        source_ext = extProp;
    }

    if (const auto& extProp = Project.as_string(prop_header_ext); extProp.size())
    {
        header_ext = extProp;
    }

    std::vector<Node*> forms;
    Project.CollectForms(forms);

    for (const auto& form: forms)
    {
        if (const auto& file = form->as_string(prop_derived_file); !file.size())
        {
            continue;
        }

        path = Project.get_DerivedFilename(form);
        if (path.empty())
        {
            continue;
        }
        if (path.file_exists())
        {
            // Count both source and header file
            path.replace_extension(header_ext);
            if (path.file_exists())
            {
                results.SetFileCount(results.GetFileCount() + 2);
                continue;
            }
            results.IncrementFileCount();
        }

        CppCodeGenerator codegen(form);

        path.replace_extension(header_ext);
        auto h_cw = std::make_unique<FileCodeWriter>(path);
        codegen.SetHdrWriteCode(h_cw.get());

        path.replace_extension(source_ext);
        auto cpp_cw = std::make_unique<FileCodeWriter>(path);
        codegen.SetSrcWriteCode(cpp_cw.get());

        auto retval = codegen.GenerateDerivedClass(Project.get_ProjectNode(), form);
        if (retval == result::fail)
        {
            results.GetMsgs().emplace_back(
                std::format("Cannot create or write to the file {}\n", path.c_str()));
            continue;
        }
        else if (retval == result::exists)
        {
            path.replace_extension(header_ext);
            if (path.file_exists())
            {
                results.IncrementFileCount();
                continue;
            }

            // If we get here, the source file exists, but the header file does not.
            int flags = flag_no_ui;
            if (form->as_bool(prop_no_closing_brace))
            {
                flags |= flag_add_closing_brace;
            }
            retval = h_cw->WriteFile(GEN_LANG_CPLUSPLUS, flags);
            if (retval == result::fail)
            {
                results.GetMsgs().emplace_back(
                    std::format("Cannot create or write to the file {}\n", path.c_str()));
            }
            if (retval == result::exists)
            {
                results.IncrementFileCount();
            }
            if (retval != result::fail && retval != result::exists)
            {
                results.GetUpdatedFiles().emplace_back(path);
            }
            continue;
        }
        if (retval == result::ignored)
        {
            // Completely ignore this file
            continue;
        }

        path.replace_extension(header_ext);
        if (path.file_exists())
        {
            retval = result::exists;
        }
        else
        {
            int flags = flag_no_ui;
            if (form->as_bool(prop_no_closing_brace))
            {
                flags |= flag_add_closing_brace;
            }

            retval = h_cw->WriteFile(GEN_LANG_CPLUSPLUS, flags);
        }

        if (retval == result::fail)
        {
            results.GetMsgs().emplace_back(std::format("Cannot create or write to the file {}\n",
                                                       static_cast<std::string>(path)));
        }
        if (retval == result::exists)
        {
            results.IncrementFileCount();
        }
        if (retval != result::fail && retval != result::exists)
        {
            results.GetUpdatedFiles().emplace_back(path);
        }

        path.replace_extension(source_ext);
        retval = cpp_cw->WriteFile(GEN_LANG_CPLUSPLUS, flag_no_ui);
        if (retval == result::fail)
        {
            results.GetMsgs().emplace_back(
                std::format("Cannot create or write to the file {}\n", path.c_str()));
        }
        if (retval == result::exists)
        {
            results.IncrementFileCount();
        }
        if (retval != result::fail && retval != result::exists)
        {
            results.GetUpdatedFiles().emplace_back(path);
        }
    }
}
