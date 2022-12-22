/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Python code files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <set>

#include "ttcwd_wx.h"  // cwd -- Class for storing and optionally restoring the current directory

#include "mainframe.h"

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "code.h"             // Code -- Helper class for generating code
#include "file_codewriter.h"  // FileCodeWriter -- Classs to write code to disk
#include "gen_base.h"         // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "gen_common.h"       // Common component functions
#include "generate_dlg.h"     // GenerateDlg -- Dialog for choosing and generating specific language file(s)
#include "node.h"             // Node class
#include "project_class.h"    // Project class
#include "utils.h"            // Miscellaneous utilities
#include "write_code.h"       // Write code to Scintilla or file

#include "pugixml.hpp"

// defined in gen_xrc.cpp
int GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags);

#if defined(_DEBUG) || defined(INTERNAL_TESTING)

void MainFrame::OnGeneratePython(wxCommandEvent& WXUNUSED(event))
{
    GenResults results;
    GeneratePythonFiles(results);

    ttlib::cstr msg;
    if (results.updated_files.size() || results.msgs.size())
    {
        if (results.updated_files.size())
        {
            if (results.updated_files.size() == 1)
                msg << "1 file was updated";
            else
                msg << " files were updated";
        }
    }
    else if (results.file_count)
    {
        msg << "All " << results.file_count << " generated files are current";
    }

    SetStatusText(msg);
}

#endif

static void GatherImportModules(std::set<std::string>& imports, Node* node)
{
    if (auto* gen = node->GetGenerator(); gen)
    {
        gen->GetPythonImports(node, imports);
    }
    for (auto& child: node->GetChildNodePtrs())
    {
        GatherImportModules(imports, child.get());
    }
}

bool GeneratePythonFiles(GenResults& results, std::vector<ttlib::cstr>* pClassList)
{
    auto project = GetProject();
    if (project->GetChildCount() == 0)
    {
        wxMessageBox("You cannot generate any code until you have added a top level form.", "Code Generation");
        return false;
    }
    ttSaveCwd cwd;
    GetProject()->GetProjectPath().ChangeDir();
    ttlib::cstr path;

    bool generate_result = true;
    std::vector<Node*> forms;
    project->CollectForms(forms);
    for (const auto& form: forms)
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
        else
        {
#if !defined(_DEBUG)
            // For a lot of wxPython testing of projects with multiple dialogs, there may
            // only be a few forms where wxPython generation is being tested, so don't nag in
            // Debug builds. :-)
            results.msgs.emplace_back() << "No Python filename specified for " << form->prop_as_string(prop_class_name)
                                        << '\n';
#endif  // _DEBUG
            continue;
        }

        try
        {
            BaseCodeGenerator codegen(GEN_LANG_PYTHON);

            auto h_cw = std::make_unique<FileCodeWriter>(path.wx_str());
            codegen.SetHdrWriteCode(h_cw.get());

            path.replace_extension(".py");
            auto cpp_cw = std::make_unique<FileCodeWriter>(path.wx_str());
            codegen.SetSrcWriteCode(cpp_cw.get());

            codegen.GeneratePythonClass(form);

            auto retval = cpp_cw->WriteFile(pClassList ? true : false);

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

            if (retval >= 0 && form->as_bool(prop_python_use_xrc) && form->HasValue(prop_python_xrc_file))
            {
                pugi::xml_document doc_new;
                doc_new.append_child(pugi::node_comment)
                    .set_value("Generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor ");
                doc_new.append_child(pugi::node_comment)
                    .set_value("DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!");

                auto root = doc_new.append_child("resource");
                root.append_attribute("xmlns") = "http://www.wxwidgets.org/wxxrc";
                root.append_attribute("version") = "2.5.3.0";

                auto form_object = root.append_child("object");
                GenXrcObject(form, form_object, false);

                auto xrc_base_file = form->prop_as_string(prop_python_xrc_file);
                path = xrc_base_file;
                if (auto* node_folder = form->get_folder();
                    node_folder && node_folder->HasValue(prop_folder_python_output_folder))
                {
                    path = node_folder->as_string(prop_folder_python_output_folder);
                    path.append_filename(xrc_base_file.filename());
                }
                else if (GetProject()->HasValue(prop_python_output_folder) && !path.contains("/"))
                {
                    path = GetProject()->GetBaseDirectory(GEN_LANG_PYTHON).utf8_string();
                    path.append_filename(xrc_base_file);
                }
                path.backslashestoforward();
                path.replace_extension(".xrc");

                // TODO: [Randalphwa - 12-05-2022] We need to check if the file exists and if
                // it does, we need to compare the contents to see if it has changed and only
                // save the file if it has.

                if (!doc_new.save_file(path.c_str(), "\t"))
                {
                    results.msgs.emplace_back() << "Cannot create or write to the file " << path << '\n';
                }
                else
                {
                    results.updated_files.emplace_back(path);
                }
            }
        }
        catch (const std::exception& TESTING_PARAM(e))
        {
            MSG_ERROR(e.what());
            wxMessageBox(ttlib::cstr("An internal error occurred generating code files for ")
                             << form->prop_as_string(prop_python_file),
                         "Code generation");
            continue;
        }
    }
    return generate_result;
}

using namespace GenEnum;

// clang-format off

inline constexpr const auto txt_PythonCmtBlock =
R"===(###############################################################################
# Code generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor/
#
# DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!
###############################################################################

import wx
)===";

// clang-format on

// Equivalent to GenerateBaseClass in gen_base.cpp

void BaseCodeGenerator::GeneratePythonClass(Node* form_node, PANEL_PAGE panel_type)
{
    if (form_node->isGen(gen_Images))
    {
        return;
    }

    m_project = GetProject();
    m_form_node = form_node;
    m_ImagesForm = nullptr;

    EventVector events;
    std::thread thrd_get_events(&BaseCodeGenerator::CollectEventHandlers, this, form_node, std::ref(events));
    std::vector<Node*> forms;
    m_project->CollectForms(forms);

    for (const auto& form: forms)
    {
        if (form->isGen(gen_Images))
        {
            m_ImagesForm = form;
            break;
        }
    }

    m_panel_type = panel_type;

    m_header->Clear();
    m_source->Clear();

    m_source->writeLine(txt_PythonCmtBlock);
    m_header->writeLine(ttlib::cstr("# Sample inherited class from ") << form_node->as_string(prop_class_name));
    m_header->writeLine();
    m_header->writeLine("import wx");
    std::set<std::string> imports;
    GatherImportModules(imports, form_node);
    for (const auto& import: imports)
    {
        m_source->writeLine(import);
        m_header->writeLine(import);
    }
    m_source->writeLine();
    m_header->writeLine();
    m_header->writeLine(ttlib::cstr("import ") << form_node->as_string(prop_python_file) << "\n");
    m_header->writeLine();

    if (form_node->HasValue(prop_python_inherit_name))
    {
        ttlib::cstr inherit("class ");
        inherit << form_node->prop_as_string(prop_python_inherit_name) << "(";
        inherit << form_node->as_string(prop_python_file) << "." << form_node->as_string(prop_class_name) << "):";

        m_header->writeLine(inherit);
        m_header->Indent();
        m_header->writeLine("def __init__(self, parent):");
        m_header->Indent();
        m_header->writeLine("super().__init__(parent)");
    }

    thrd_get_events.join();

    auto generator = form_node->GetNodeDeclaration()->GetGenerator();
    Code code(form_node, GEN_LANG_PYTHON);

    if (generator->GenPythonForm(code))
    {
        m_source->writeLine(code.m_code, indent::auto_keep_whitespace);
        m_source->writeLine();
        m_source->Indent();
        m_source->Indent();
    }

    size_t auto_indent = indent::auto_no_whitespace;
    if (auto result = generator->GenSettings(form_node, auto_indent, GEN_LANG_PYTHON); result)
    {
        if (result.value().size())
        {
            m_source->writeLine(result.value(), indent::auto_keep_whitespace);
            m_source->writeLine();
        }
    }

    if (form_node->get_prop_ptr(prop_window_extra_style))
    {
        Code win_code(form_node, GEN_LANG_PYTHON);
        win_code.GenWindowSettings();
        if (win_code.size())
        {
            // GenerateWindowSettings() can result in code within braces, so keep any leading whitespace.
            m_source->writeLine(win_code.m_code, indent::auto_keep_whitespace);
        }
    }

    m_source->SetLastLineBlank();
    for (const auto& child: form_node->GetChildNodePtrs())
    {
        if (child->isGen(gen_wxContextMenuEvent))
            continue;
        GenConstruction(child.get());
    }

    code.clear();

    if (auto result = generator->CommonAdditionalCode(code, code_after_children); result)
    {
        if (result.value().size())
        {
            m_source->writeLine();
            m_source->writeLine(result.value(), indent::auto_keep_whitespace);
        }
    }

    // TODO: [Randalphwa - 12-04-2022] Python supports persistence, though it's not as easy as it is in C++.
    // See https://docs.wxpython.org/wx.lib.agw.persist.html?highlight=persist#module-wx.lib.agw.persist

    if (events.size())
    {
        m_source->writeLine();
        if (m_form_node->HasValue(prop_python_inherit_name))
            m_source->writeLine(ttlib::cstr("# Bind Event handlers to inherited ")
                                << m_form_node->as_string(prop_python_inherit_name) << " class functions");
        else
            m_source->writeLine("# Bind Event handlers");
        GenSrcEventBinding(form_node, events);

        m_source->ResetIndent();
        m_source->writeLine();
        m_source->Indent();
        GenPythonEventHandlers(events);
    }

    // Make certain indentation is reset after all construction code is written
    m_source->ResetIndent();
    m_header->ResetIndent();
}

void BaseCodeGenerator::GenPythonEventHandlers(const EventVector& events)
{
    bool inherited_class = m_form_node->HasValue(prop_python_inherit_name);

    // Multiple events can be bound to the same function, so use a set to make sure we only generate each function once.
    std::set<ttlib::cstr> code_lines;

    for (auto& event: events)
    {
        // Ignore lambda's and functions in another class
        if (event->get_value().contains("[") || event->get_value().contains("::"))
            continue;

        ttlib::cstr code("\tdef ");
        code << event->get_value() << "(self, event):\n\t\t";
        if (inherited_class || event->GetNode()->get_form()->prop_as_bool(prop_python_call_skip))
            code << "event.Skip()";
        else
            code << "pass";
        code_lines.emplace(code);
    }

    if (code_lines.size())
    {
        if (inherited_class)
        {
            m_header->Unindent();
            m_header->writeLine();
            m_header->writeLine("# Event handler functions");
        }
        else
        {
            m_source->writeLine();
            m_source->writeLine("# Event handler functions");
        }
        for (auto code: code_lines)
        {
            if (!inherited_class)
            {
                m_source->writeLine(code, false);
                m_source->writeLine();
            }
            else
            {
                m_header->writeLine(code, false);
                m_header->writeLine();
            }
        }
    }
}

bool PythonBitmapList(Code& code, GenEnum::PropName prop)
{
    auto& description = code.node()->as_string(prop);
    ttlib::multiview parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    if (parts[IndexImage].empty() || parts[IndexType].contains("Art") || parts[IndexType].contains("SVG"))
    {
        return false;
    }

    auto bundle = GetProject()->GetPropertyImageBundle(description);

    if (!bundle || bundle->lst_filenames.size() < 3)
    {
        return false;
    }

    bool is_xpm = (parts[IndexType].is_sameas("XPM"));
    auto path = MakePythonPath(code.node());

    code += "bitmaps = [ ";
    bool needs_comma = false;
    for (auto& iter: bundle->lst_filenames)
    {
        if (needs_comma)
        {
            code.UpdateBreakAt();
            code.Comma(false).Eol().Tab(3);
        }
        ttlib::cstr name(iter);
        name.make_absolute();
        name.make_relative(path);
        name.backslashestoforward();

        code.Add("wxBitmap(\'") << name << "\'";
        if (is_xpm)
            code.Add(", wx.BITMAP_TYPE_XPM");
        code += ")";
        needs_comma = true;
    }
    code += " ]\n";
    code.UpdateBreakAt();

    return true;
}
bool PythonBundleCode(Code& code, GenEnum::PropName prop)
{
    auto& description = code.node()->as_string(prop);
    if (description.empty())
    {
        code.Add("wxNullBitmap");
        return false;
    }

    ttlib::multiview parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    if (parts.size() <= 1 || parts[IndexImage].empty())
    {
        code.Add("wxNullBitmap");
        return false;
    }

    if (parts[IndexType].contains("Art"))
    {
        ttlib::cstr art_id(parts[IndexArtID]);
        ttlib::cstr art_client;
        if (auto pos = art_id.find('|'); ttlib::is_found(pos))
        {
            art_client = art_id.subview(pos + 1);
            art_id.erase(pos);
        }

        code.Add("wxArtProvider.GetBitmapBundle(").Add(art_id);

        // Note that current documentation states that the client is required, but the header file says otherwise
        if (art_client.size())
            code.Comma().Add(art_client);
        code << ')';
        return true;
    }

    auto path = MakePythonPath(code.node());

    if (auto bundle = GetProject()->GetPropertyImageBundle(description); bundle && bundle->lst_filenames.size())
    {
        ttlib::cstr name(bundle->lst_filenames[0]);
        name.make_absolute();
        name.make_relative(path);
        name.backslashestoforward();

        if (description.starts_with("SVG"))
        {
            code += "wx.BitmapBundle.";
            code << "FromSVGFile(\'" << name << "\'";
            wxSize svg_size { -1, -1 };
            if (parts[IndexSize].size())
            {
                GetSizeInfo(svg_size, parts[IndexSize]);
            }
            code.Comma().Add("wxSize(").itoa(svg_size.x).Comma().itoa(svg_size.y) += "))";
        }

        else if (bundle->lst_filenames.size() == 1)
        {
            code += "wx.BitmapBundle.";
            code << "FromBitmap(wx.Bitmap(\'" << name << "\'))";
        }
        else if (bundle->lst_filenames.size() == 2)
        {
            ttlib::cstr name2(bundle->lst_filenames[0]);
            name2.make_absolute();
            name2.make_relative(path);
            name2.backslashestoforward();
            code += "wx.BitmapBundle.";
            code << "FromBitmap(wx.Bitmap(\'" << name << "\', \'" << name2 << "\'))";
        }
        else
        {
            FAIL_MSG("Unexpected number of images in bundle -- should be <= 2");
            code.Add("wxNullBitmap");
            return false;
        }
    }
    else
    {
        FAIL_MSG("Missing bundle description");
        code.Add("wxNullBitmap");
        return false;
    }

    return true;
}

ttlib::cstr MakePythonPath(Node* node)
{
    ttlib::cstr path;
    Node* form = node->get_form();

    if (auto& base_file = form->prop_as_string(prop_python_file); base_file.size())
    {
        path = base_file;

        if (auto* node_folder = form->get_folder(); node_folder && node_folder->HasValue(prop_folder_python_output_folder))
        {
            path = node_folder->as_string(prop_folder_python_output_folder);
            path.append_filename(base_file.filename());
        }
        else if (GetProject()->HasValue(prop_python_output_folder) && !path.contains("/"))
        {
            path = GetProject()->GetBaseDirectory(GEN_LANG_PYTHON).utf8_string();
            path.append_filename(base_file);
        }
    }
    if (path.empty())
        path = "./z";
    path.make_absolute();
    path.remove_filename();
    return path;
}
