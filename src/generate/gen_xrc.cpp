/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate XRC file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <cstring>  // for std::memcmp
#include <future>
#include <sstream>
#include <thread>
#include <unordered_set>

#include <wx/filedlg.h>     // wxFileDialog base header
#include <wx/filename.h>    // wxFileName - encapsulates a file path
#include <wx/mstream.h>     // Memory stream classes
#include <wx/wizard.h>      // wxWizard class: a GUI control presenting the user with a
#include <wx/xml/xml.h>     // wxXmlDocument - XML parser & data holder class
#include <wx/xrc/xmlres.h>  // XML resources

// The following handlers must be explicitly added

#include <wx/xrc/xh_aui.h>             // XRC resource handler for wxAUI
#include <wx/xrc/xh_auitoolb.h>        // XML resource handler for wxAuiToolBar
#include <wx/xrc/xh_ribbon.h>          // XML resource handler for wxRibbon related classes
#include <wx/xrc/xh_richtext.h>        // XML resource handler for wxRichTextCtrl
#include <wx/xrc/xh_styledtextctrl.h>  // XML resource handler for wxStyledTextCtrl

#include "gen_base.h"  // BaseCodeGenerator -- Generate Src and Hdr files for Base Class

#include "../panels/propgrid_panel.h"   // PropGridPanel -- Node inspector class
#include "../tools/preview_settings.h"  // PreviewSettings
#include "../ui/xrccompare.h"           // C++/XRC UI Comparison dialog
#include "cstm_event.h"                 // CustomEvent -- Custom Event class
#include "gen_common.h"                 // GeneratorLibrary -- Generator classes
#include "gen_results.h"                // Code generation file writing functions
#include "gen_xrc_utils.h"              // Common XRC generating functions
#include "generate_xrc_dlg.h"           // GenerateXrcDlg -- Dialog for generating XRC file(s)
#include "mainframe.h"                  // MainFrame -- Main window frame
#include "node.h"                       // Node class
#include "node_creator.h"               // NodeCreator -- Class used to create nodes
#include "project_handler.h"            // ProjectHandler class
#include "utils.h"                      // Utility functions that work with properties
#include "write_code.h"                 // Write code to Scintilla or file

#include "pugixml.hpp"

const char* txt_dlg_name = "_wxue_temp_dlg";

int GenerateXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto generator = node->getNodeDeclaration()->getGenerator();
    auto result = generator->GenXrcObject(node, object, xrc_flags);
    if (result == BaseGenerator::xrc_not_supported)
    {
        if (node->isGen(gen_Project))
        {
            result = BaseGenerator::xrc_updated;
        }
        else
        {
            auto item = InitializeXrcObject(node, object);
            auto comment = generator->GetWarning(node, GEN_LANG_XRC);
            if (comment)
            {
                // We need a dummy item to hold the comment but which will not show up in the UI
                GenXrcObjectAttributes(node, item, "wxBoxSizer");

                object.append_child(pugi::node_comment).set_value(comment->c_str());
            }

            return BaseGenerator::xrc_form_not_supported;
        }
    }

    if (result == BaseGenerator::xrc_sizer_item_created)
    {
        auto actual_object = object.child("object");
        if (node->isGen(gen_wxCollapsiblePane))
        {
            // XRC wants a panewindow object as the sole child of wxCollapsiblePane, and all node children
            // must be added as children of this panewindow.

            actual_object = actual_object.append_child("object");
            actual_object.append_attribute("class").set_value("panewindow");
        }

        for (const auto& child: node->getChildNodePtrs())
        {
            // Normally, the XRC heirarchy matches our node heirarchy with the exception of XRC needing
            // a sizeritem as the immediate parent of a widget node. The exception is wxTreebook -- while
            // our nodes have BookPages as children of BookPages, XRC expects all BookPages to be children
            // of the wxTreebook with a depth parameter indicating if it is a sub-page or not.

            if (child->isGen(gen_BookPage) && child->getParent()->isGen(gen_BookPage))
            {
                int depth = 0;
                actual_object = object;
                for (;;)
                {
                    auto class_attr = actual_object.attribute("class");
                    if (class_attr.value() != "wxTreebook")
                    {
                        if (class_attr.value() == "treebookpage")
                            ++depth;
                        actual_object = actual_object.parent();
                        ASSERT(!actual_object.empty())
                    }
                    else
                    {
                        break;
                    }
                }
                auto child_object = actual_object.append_child("object");
                child_object.append_child("depth").text().set(depth);
                GenerateXrcObject(child.get(), child_object, xrc_flags);
                continue;
            }

            auto child_object = actual_object.append_child("object");
            auto child_result = GenerateXrcObject(child.get(), child_object, xrc_flags);
            if (child_result == BaseGenerator::xrc_not_supported)
            {
                actual_object.remove_child(child_object);
            }
        }
        return result;
    }
    else if (result == BaseGenerator::xrc_updated)
    {
        if (node->isGen(gen_tool_dropdown))
        {
            return result;  // The dropdown tool will already have handled it's children.
        }

        for (const auto& child: node->getChildNodePtrs())
        {
            auto child_object = object.append_child("object");
            auto child_result = GenerateXrcObject(child.get(), child_object, xrc_flags);
            if (child_result == BaseGenerator::xrc_not_supported)
            {
                object.remove_child(child_object);
                // REVIEW: [Randalphwa - 09-02-2022] In most cases, we can simply skip over the unsupported node. If not, we
                // need to special-case it rather than just breaking out of the loop.
            }
        }
        return result;
    }
    else if (result == BaseGenerator::xrc_form_not_supported)
    {
        if (xrc_flags & xrc::add_comments)
        {
            return result;
        }
        else
        {
            return BaseGenerator::xrc_not_supported;
        }
    }
    else
    {
        return BaseGenerator::xrc_not_supported;
    }
}

void CollectHandlers(Node* node, std::set<std::string>& handlers)
{
    auto generator = node->getNodeDeclaration()->getGenerator();
    generator->RequiredHandlers(node, handlers);
    for (const auto& child: node->getChildNodePtrs())
    {
        generator = child->getNodeDeclaration()->getGenerator();
        generator->RequiredHandlers(child.get(), handlers);
        if (child->getChildCount())
        {
            CollectHandlers(child.get(), handlers);
        }
    }
}

std::string GenerateXrcStr(Node* node_start, size_t xrc_flags)
{
    pugi::xml_document doc;
    auto root = doc.append_child("resource");
    root.append_attribute("xmlns") = "http://www.wxwidgets.org/wxxrc";
    root.append_attribute("version") = "2.5.3.0";

    if (node_start->isGen(gen_MenuBar) || node_start->isGen(gen_RibbonBar) || node_start->isGen(gen_ToolBar))
    {
        if (auto temp_form = NodeCreation.createNode(gen_PanelForm, nullptr).first; temp_form)
        {
            auto sizer = NodeCreation.createNode(gen_VerticalBoxSizer, temp_form.get()).first;
            temp_form->adoptChild(sizer);
            auto node_copy = NodeCreation.makeCopy(node_start, sizer.get());
            sizer->adoptChild(node_copy);
            node_start = temp_form.get();
        }
    }

    if (!node_start)
    {
        root.append_child("object");
    }
    else if (node_start->isGen(gen_Project))
    {
        GenerateXrcObject(node_start, root, xrc_flags);
    }
    else if ((xrc_flags & xrc::previewing) && node_start->isGen(gen_PanelForm))
    {
        auto object = root.append_child("object");
        object.append_attribute("class").set_value("wxDialog");
        object.append_attribute("name").set_value(txt_dlg_name);
        object.append_child("style").text().set("wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER");
        object.append_child("centered").text().set("1");
        object.append_child("title").text().set(node_start->as_string(prop_class_name));
        auto item = object.append_child("object");
        item.append_attribute("class").set_value("wxBoxSizer");
        item.append_attribute("name").set_value("_wxue_temp_sizer");
        item.append_child("orient").text().set("wxVERTICAL");
        auto sizer_item = item.append_child("object");
        sizer_item.append_attribute("class").set_value("sizeritem");
        object = sizer_item.append_child("object");

        GenerateXrcObject(node_start, object, xrc_flags);
    }
    else if ((xrc_flags & xrc::previewing) && node_start->isGen(gen_wxDialog))
    {
        auto object = root.append_child("object");
        object.append_attribute("class").set_value("wxPanel");
        object.append_attribute("name").set_value(txt_dlg_name);
        object = object.append_child("object");
        GenerateXrcObject(node_start->getChild(0), object, xrc_flags);
    }
    else
    {
        auto object = root.append_child("object");
        GenerateXrcObject(node_start, object, xrc_flags);
    }

    std::ostringstream xml_stream;
    auto format_flags = pugi::format_default;
    if (xrc_flags & xrc::format_no_indent_nodes)
    {
        format_flags = 0;
    }
    else if (xrc_flags & xrc::format_indent_attributes)
    {
        format_flags = pugi::format_indent_attributes;
    }
    std::string indentation(xrc_flags & xrc::format_indent_with_spaces ? "  " : "\t");
    doc.save(xml_stream, indentation.c_str(), format_flags);
    return xml_stream.str();
}

void BaseCodeGenerator::GenerateXrcClass(PANEL_PAGE panel_type)
{
    m_panel_type = panel_type;

    m_header->Clear();
    m_source->Clear();

    if (!m_form_node)
        return;

    if (m_panel_type != HDR_PANEL)
    {
        XrcGenerator xrc_gen;
        xrc_gen.AddProjectFlags();
        if (m_panel_type == CPP_PANEL)
        {
            xrc_gen.AddXrcFlags(xrc::add_comments);
        }
        xrc_gen.AddNode(m_form_node);
        auto doc_str = xrc_gen.getXmlString();
        m_source->doWrite(doc_str);
    }

    else  // Info panel
    {
        if (m_form_node != Project.getProjectNode())
        {
            m_header->writeLine(tt_string("Resource name is ") << m_form_node->as_string(prop_class_name));
            m_header->writeLine();
        }
        m_header->writeLine("Required handlers:");
        m_header->writeLine();
        m_header->Indent();

        std::set<std::string> handlers;
        CollectHandlers(m_form_node, handlers);
        for (auto& iter: handlers)
        {
            m_header->writeLine(iter);
        }
    }
}

bool GenerateXrcFiles(GenResults& results, std::vector<tt_string>* pClassList)
{
    XrcGenerator xrc_gen;
    xrc_gen.AddProjectFlags();
    xrc_gen.GenerateAllXrcForms(results, pClassList);
    return true;
}

void MainFrame::OnGenSingleXRC(wxCommandEvent& WXUNUSED(event))
{
    auto form = wxGetMainFrame()->getSelectedNode();
    if (form && !form->isForm())
    {
        form = form->getForm();
    }
    if (!form)
    {
        wxMessageBox("You must select a form before you can generate code.", "Code Generation");
        return;
    }
    else if (form->isGen(gen_Images) || form->isGen(gen_Data) || form->isGen(gen_wxPopupTransientWindow))
    {
        wxMessageBox("You cannot generate an XRC file for this type of form.", "Code Generation");
        return;
    }

    GenResults results;
    XrcGenerator xrc_gen;
    xrc_gen.AddProjectFlags();
    auto [path, has_base_file] = Project.GetOutputPath(form, GEN_LANG_XRC);
    if (path.empty())
    {
        wxMessageBox("No XRC filename specified for " + form->as_string(prop_class_name), "Code Generation");
        return;
    }
    if (path.extension().empty())
    {
        path.replace_extension(".xrc");
    }

    xrc_gen.AddNode(form);

    if (path.file_exists())
    {
        // Compare the new document with the existing file, and only write it if it has changed
        wxFile file_original(path.make_wxString(), wxFile::read_write);
        if (file_original.IsOpened())
        {
            // Check to see if the file would be changed. If not, we don't need to update it.
            auto new_str = xrc_gen.getXmlString();

            auto in_size = file_original.Length();
            if (new_str.size() == (to_size_t) in_size)
            {
                auto buffer = std::make_unique<unsigned char[]>(in_size);
                if (file_original.Read(buffer.get(), in_size) == in_size)
                {
                    if (std::memcmp(buffer.get(), new_str.data(), in_size) == 0)
                    {
                        ++results.file_count;
                    }
                }
            }
            else
            {
                // The document differs from the file, so write the document to the file.
                file_original.Close();
                if (!file_original.Create(path.make_wxString(), true))
                {
                    results.msgs.emplace_back() << "Cannot create the file " << path << '\n';
                }
                else
                {
                    if (file_original.Write(new_str.c_str(), new_str.length()) != new_str.length())
                    {
                        results.msgs.emplace_back() << "Cannot write to the file " << path << '\n';
                    }
                    else
                    {
                        results.updated_files.emplace_back(path);
                        if (results.updated_files.size())
                        {
                            results.file_count += results.updated_files.size();
                        }
                    }
                }
            }
        }
    }
    else  // file doesn't exist, so write it out
    {
        if (!xrc_gen.getDocument().save_file(path))
        {
            results.msgs.emplace_back() << "Cannot create or write to the file " << path << '\n';
        }
        else
        {
            results.updated_files.emplace_back(path);
            if (results.updated_files.size())
            {
                results.file_count += results.updated_files.size();
            }
        }
    }

    tt_string msg;
    if (results.updated_files.size())
    {
        if (results.updated_files.size() == 1)
            msg << "1 file was updated";
        else
            msg << results.updated_files.size() << " files were updated";
        msg << '\n';
    }
    else
    {
        msg << "Generated file is current";
    }

    if (results.msgs.size())
    {
        for (auto& iter: results.msgs)
        {
            msg << '\n';
            msg << iter;
        }
    }

    wxMessageBox(msg, "XRC Code Generation", wxOK | wxICON_INFORMATION);
}

XrcGenerator::XrcGenerator() {};

void XrcGenerator::AddProjectFlags()
{
    if (Project.as_bool(prop_xrc_no_whitespace))
    {
        m_xrc_flags |= xrc::format_no_indent_nodes;
    }
    else
    {
        if (Project.as_bool(prop_xrc_indent_attributes))
        {
            m_xrc_flags |= xrc::format_indent_attributes;
        }
        if (Project.as_bool(prop_xrc_indent_with_spaces))
        {
            m_xrc_flags |= xrc::format_indent_with_spaces;
        }
    }

    if (Project.as_bool(prop_xrc_add_var_comments))
    {
        m_xrc_flags |= xrc::add_comments;
    }
}

std::string XrcGenerator::getXmlString()
{
    std::ostringstream xml_stream;
    auto format_flags = pugi::format_default;
    if (m_xrc_flags & xrc::format_no_indent_nodes)
    {
        format_flags = 0;
    }
    else if (m_xrc_flags & xrc::format_indent_attributes)
    {
        format_flags = pugi::format_indent_attributes;
    }
    std::string indentation(m_xrc_flags & xrc::format_indent_with_spaces ? "  " : "\t");
    m_doc.save(xml_stream, indentation.c_str(), format_flags);
    return xml_stream.str();
}

void XrcGenerator::GenerateAllXrcForms(GenResults& results, std::vector<tt_string>* pClassList)
{
    bool combine_forms = Project.as_bool(prop_combine_all_forms);
    if (combine_forms)
    {
        auto path = Project.as_string(prop_combined_xrc_file);
        if (path.empty())
        {
            results.msgs.emplace_back() << "No combined XRC filename specified for the project.\n";
            return;
        }
        ClearDocument();
        AddGeneratedComments();
    }

    std::vector<Node*> forms;
    Project.CollectForms(forms);
    for (auto& form: forms)
    {
        if (!combine_forms)
        {
            ClearDocument();
            AddGeneratedComments();
            auto [path, has_base_file] = Project.GetOutputPath(form, GEN_LANG_XRC);
            if (path.empty())
            {
                // If the form type is supported, warn the user about not having an XRC file for it.
                if (!form->isGen(gen_Images) && !form->isGen(gen_Data) && !form->isGen(gen_wxPopupTransientWindow))
                    results.msgs.emplace_back()
                        << "No XRC filename specified for " << form->as_string(prop_class_name) << '\n';
                continue;
            }
            if (path.extension().empty())
            {
                path.replace_extension(".xrc");
            }

            AddNode(form);

            if (path.file_exists())
            {
                // Compare the new document with the existing file, and only write it if it has changed
                wxFile file_original(path.make_wxString(), wxFile::read_write);
                if (file_original.IsOpened())
                {
                    // Check to see if the file would be changed. If not, we don't need to update it.
                    std::ostringstream xml_stream;
                    m_doc.save(xml_stream, getIndentationString().c_str());
                    auto new_str = xml_stream.str();

                    auto in_size = file_original.Length();
                    if (new_str.size() == (to_size_t) in_size)
                    {
                        auto buffer = std::make_unique<unsigned char[]>(in_size);
                        if (file_original.Read(buffer.get(), in_size) == in_size)
                        {
                            if (std::memcmp(buffer.get(), new_str.data(), in_size) == 0)
                            {
                                if (pClassList)
                                {
                                    pClassList->emplace_back(form->as_string(prop_class_name));
                                }
                                else
                                {
                                    ++results.file_count;
                                }
                                continue;
                            }
                        }
                    }
                    else
                    {
                        // The document differs from the file, so write the document to the file.
                        file_original.Close();
                        if (!file_original.Create(path.make_wxString(), true))
                        {
                            results.msgs.emplace_back() << "Cannot create the file " << path << '\n';
                        }
                        else
                        {
                            if (file_original.Write(new_str.c_str(), new_str.length()) != new_str.length())
                            {
                                results.msgs.emplace_back() << "Cannot write to the file " << path << '\n';
                            }
                            else
                            {
                                results.updated_files.emplace_back(path);
                                if (results.updated_files.size())
                                {
                                    results.file_count += results.updated_files.size();
                                }
                            }
                        }
                    }
                }
            }
            else  // file doesn't exist, so write it out
            {
                if (!m_doc.save_file(path))
                {
                    results.msgs.emplace_back() << "Cannot create or write to the file " << path << '\n';
                }
                else
                {
                    results.updated_files.emplace_back(path);
                    if (results.updated_files.size())
                    {
                        results.file_count += results.updated_files.size();
                    }
                }
            }
        }
        else
        {
            AddNode(form);
        }
    }

    if (combine_forms)
    {
        auto path = Project.as_string(prop_xrc_directory);
        if (path.empty())
        {
            path = Project.as_string(prop_combined_xrc_file);
        }
        else
        {
            auto base_file = Project.as_string(prop_combined_xrc_file);
            base_file.backslashestoforward();
            if (base_file.contains("/"))
            {
                path.backslashestoforward();
                if (path.back() == '/')
                    path.pop_back();

                // If the first part of the base_file is a folder and it matches the last folder in
                // result, then assume the folder name is duplicated in base_file. Remove the folder
                // from result before adding the base_file path.
                if (auto end_folder = base_file.find('/'); end_folder != tt::npos)
                {
                    if (path.ends_with(base_file.substr(0, end_folder)))
                    {
                        path.erase(path.size() - end_folder, end_folder);
                    }
                }
            }
            path.append_filename(base_file);
            path.make_absolute();
            path.backslashestoforward();
        }
        if (path.extension().empty())
        {
            path.replace_extension(".xrc");
        }

        if (!m_doc.save_file(path))
        {
            results.msgs.emplace_back() << "Cannot create or write to the file " << path << '\n';
            return;
        }
        results.updated_files.emplace_back(path);
    }
}

void XrcGenerator::AddNode(Node* node_start)
{
    if (!m_root)
    {
        m_root = m_doc.append_child("resource");
        m_root.append_attribute("xmlns") = "http://www.wxwidgets.org/wxxrc";
        m_root.append_attribute("version") = "2.5.3.0";
    }

    if (node_start->isGen(gen_MenuBar) || node_start->isGen(gen_RibbonBar) || node_start->isGen(gen_ToolBar))
    {
        if (auto temp_form = NodeCreation.createNode(gen_PanelForm, nullptr).first; temp_form)
        {
            auto sizer = NodeCreation.createNode(gen_VerticalBoxSizer, temp_form.get()).first;
            temp_form->adoptChild(sizer);
            auto node_copy = NodeCreation.makeCopy(node_start, sizer.get());
            sizer->adoptChild(node_copy);
            node_start = temp_form.get();
        }
    }

    if (!node_start)
    {
        m_root.append_child("object");
    }
    else if (node_start->isGen(gen_Project))
    {
        GenerateXrcObject(node_start, m_root, m_xrc_flags);
    }
    else if ((m_xrc_flags & xrc::previewing) && node_start->isGen(gen_PanelForm))
    {
        auto object = m_root.append_child("object");
        object.append_attribute("class").set_value("wxDialog");
        object.append_attribute("name").set_value(txt_dlg_name);
        object.append_child("style").text().set("wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER");
        object.append_child("centered").text().set("1");
        object.append_child("title").text().set(node_start->as_string(prop_class_name));
        auto item = object.append_child("object");
        item.append_attribute("class").set_value("wxBoxSizer");
        item.append_attribute("name").set_value("_wxue_temp_sizer");
        item.append_child("orient").text().set("wxVERTICAL");
        auto sizer_item = item.append_child("object");
        sizer_item.append_attribute("class").set_value("sizeritem");
        object = sizer_item.append_child("object");

        GenerateXrcObject(node_start, object, m_xrc_flags);
    }
    else if ((m_xrc_flags & xrc::previewing) && node_start->isGen(gen_wxDialog))
    {
        auto object = m_root.append_child("object");
        object.append_attribute("class").set_value("wxPanel");
        object.append_attribute("name").set_value(txt_dlg_name);
        object = object.append_child("object");
        GenerateXrcObject(node_start->getChild(0), object, m_xrc_flags);
    }
    else
    {
        auto object = m_root.append_child("object");
        GenerateXrcObject(node_start, object, m_xrc_flags);
    }
}

void XrcGenerator::AddGeneratedComments()
{
    m_doc.append_child(pugi::node_comment)
        .set_value("Generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor ");
    m_doc.append_child(pugi::node_comment)
        .set_value("DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!");
}

std::string XrcGenerator::getIndentationString()
{
    if (Project.as_bool(prop_xrc_indent_with_spaces))
    {
        return "  ";  // Two spaces
    }
    else
    {
        return "\t";  // Tab character
    }
}
