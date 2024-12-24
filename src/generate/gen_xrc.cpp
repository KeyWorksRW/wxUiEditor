/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate XRC file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2024 KeyWorks Software (Ralph Walden)
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

int GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
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
                GenXrcObject(child.get(), child_object, xrc_flags);
                continue;
            }

            auto child_object = actual_object.append_child("object");
            auto child_result = GenXrcObject(child.get(), child_object, xrc_flags);
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
            auto child_result = GenXrcObject(child.get(), child_object, xrc_flags);
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
        GenXrcObject(node_start, root, xrc_flags);
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

        GenXrcObject(node_start, object, xrc_flags);
    }
    else if ((xrc_flags & xrc::previewing) && node_start->isGen(gen_wxDialog))
    {
        auto object = root.append_child("object");
        object.append_attribute("class").set_value("wxPanel");
        object.append_attribute("name").set_value(txt_dlg_name);
        object = object.append_child("object");
        GenXrcObject(node_start->getChild(0), object, xrc_flags);
    }
    else
    {
        auto object = root.append_child("object");
        GenXrcObject(node_start, object, xrc_flags);
    }

    std::ostringstream xml_stream;
    doc.save(xml_stream, "\t");
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
        size_t xrc_flags = xrc::use_xrc_dir;
        if (m_panel_type == CPP_PANEL)
            xrc_flags |= xrc::add_comments;
        auto doc_str = GenerateXrcStr(m_form_node, xrc_flags);
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

// This assumes each form is being generated to a single file.
static bool GenerateXrcForm(Node* form, GenResults& results, std::vector<tt_string>* pClassList)
{
    auto [path, has_base_file] = Project.GetOutputPath(form, GEN_LANG_XRC);
    if (path.empty())
    {
        // If the form type is supported, warn the user about not having an XRC file for it.
        if (!form->isGen(gen_Images) && !form->isGen(gen_Data) && !form->isGen(gen_wxPopupTransientWindow))
            results.msgs.emplace_back() << "No XRC filename specified for " << form->as_string(prop_class_name) << '\n';
        return false;
    }
    if (path.extension().empty())
    {
        path.replace_extension(".xrc");
    }

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

    if (path.file_exists())
    {
        wxFile file_original(path.make_wxString(), wxFile::read_write);
        if (file_original.IsOpened())
        {
            // Check to see if the file would be changed. If not, we don't need to update it.
            std::ostringstream xml_stream;
            doc_new.save(xml_stream, "\t");
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
                        return true;
                    }
                }
            }
            else
            {
                file_original.Close();
                if (!file_original.Create(path.make_wxString(), true))
                {
                    results.msgs.emplace_back() << "Cannot create the file " << path << '\n';
                    return false;
                }

                if (file_original.Write(new_str.c_str(), new_str.length()) != new_str.length())
                {
                    results.msgs.emplace_back() << "Cannot write to the file " << path << '\n';
                    return false;
                }

                results.updated_files.emplace_back(path);
                return true;
            }
        }

        if (!doc_new.save_file(path))
        {
            results.msgs.emplace_back() << "Cannot create or write to the file " << path << '\n';
        }
        else
        {
            results.updated_files.emplace_back(path);
        }
        return true;
    }
    return false;
}

bool GenerateXrcFiles(GenResults& results, tt_string out_file, std::vector<tt_string>* pClassList)
{
    if (Project.getChildCount() == 0)
    {
        wxMessageBox("This project does not yet contain any forms -- nothing to save!", "Export XRC");
        return false;
    }

    tt_cwd cwd(true);
    Project.getProjectPath().ChangeDir();

    if (out_file.size())
    {
        if (out_file.extension().empty())
        {
            out_file.replace_extension(".xrc");
        }

        pugi::xml_document doc;
        doc.append_child(pugi::node_comment)
            .set_value("Generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor ");
        doc.append_child(pugi::node_comment)
            .set_value("DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!");

        auto root = doc.append_child("resource");
        root.append_attribute("xmlns") = "http://www.wxwidgets.org/wxxrc";
        root.append_attribute("version") = "2.5.3.0";

        root.append_child(pugi::node_comment)
            .set_value("Generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor ");
        root.append_child(pugi::node_comment)
            .set_value("DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!");

        GenXrcObject(Project.getProjectNode(), root, false);
        if (!doc.save_file(out_file))
        {
            std::string msg("Unable to save\n    \"" + out_file + "\"\n");
            wxMessageDialog dlg(nullptr, msg, "", wxICON_ERROR | wxOK);
            dlg.ShowModal();
        }
        return true;
    }

    bool generate_result = false;
    std::vector<Node*> forms;
    Project.CollectForms(forms);

    if (wxGetApp().isTestingMenuEnabled())
        results.EndClock();

    for (auto& form: forms)
    {
        GenerateXrcForm(form, results, pClassList);
        if (results.updated_files.size())
        {
            generate_result = true;
        }
    }

    if (wxGetApp().isTestingMenuEnabled())
        results.EndClock();

    return generate_result;
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

    GenResults results;
    GenerateXrcForm(form, results, nullptr);

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
