/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate XRC file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

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
#include "../ui/xrccompare.h"           // auto-generated: xrccompare_base.h and xrccompare_base.cpp
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
    auto generator = node->GetNodeDeclaration()->GetGenerator();
    auto result = generator->GenXrcObject(node, object, xrc_flags);
    if (result == BaseGenerator::xrc_not_supported && node->isGen(gen_Project))
    {
        result = BaseGenerator::xrc_updated;
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

        for (const auto& child: node->GetChildNodePtrs())
        {
            // Normally, the XRC heirarchy matches our node heirarchy with the exception of XRC needing
            // a sizeritem as the immediate parent of a widget node. The exception is wxTreebook -- while
            // our nodes have BookPages as children of BookPages, XRC expects all BookPages to be children
            // of the wxTreebook with a depth parameter indicating if it is a sub-page or not.

            if (child->isGen(gen_BookPage) && child->GetParent()->isGen(gen_BookPage))
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

        for (const auto& child: node->GetChildNodePtrs())
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
    auto generator = node->GetNodeDeclaration()->GetGenerator();
    generator->RequiredHandlers(node, handlers);
    for (const auto& child: node->GetChildNodePtrs())
    {
        generator = child->GetNodeDeclaration()->GetGenerator();
        generator->RequiredHandlers(child.get(), handlers);
        if (child->GetChildCount())
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

    NodeSharedPtr temp_form = nullptr;
    if (node_start->isGen(gen_MenuBar) || node_start->isGen(gen_RibbonBar) || node_start->isGen(gen_ToolBar))
    {
        temp_form = NodeCreation.CreateNode(gen_PanelForm, nullptr);
        if (temp_form)
        {
            auto sizer = NodeCreation.CreateNode(gen_VerticalBoxSizer, temp_form.get());
            temp_form->Adopt(sizer);
            auto node_copy = NodeCreation.MakeCopy(node_start, sizer.get());
            sizer->Adopt(node_copy);
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
#if 0
    else if ((xrc_flags & xrc::previewing) && node_start->isGen(gen_wxDialog))
    {
        auto object = root.append_child("object");
        object.append_attribute("class").set_value("wxPanel");
        object.append_attribute("name").set_value(txt_dlg_name);
        object = object.append_child("object");
        GenXrcObject(node_start->GetChild(0), object, xrc_flags);
    }
#endif
    else
    {
        auto object = root.append_child("object");
        GenXrcObject(node_start, object, xrc_flags);
    }

    std::ostringstream xml_stream;
    doc.save(xml_stream, "\t");
    return xml_stream.str();
}

void BaseCodeGenerator::GenerateXrcClass(Node* form_node, PANEL_PAGE panel_type)
{
    m_panel_type = panel_type;

    m_header->Clear();
    m_source->Clear();

    if (form_node)
    {
        m_form_node = form_node;
    }
    else
    {
        m_form_node = Project.GetFirstFormChild();
    }

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
        if (form_node != Project.ProjectNode())
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

bool GenerateXrcFiles(GenResults& results, tt_string out_file, std::vector<tt_string>* /* pClassList */)
{
    if (Project.ChildCount() == 0)
    {
        wxMessageBox("This project does not yet contain any forms -- nothing to save!", "Export XRC");
        return false;
    }

    tt_cwd cwd(true);
    Project.ProjectPath().ChangeDir();

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

        GenXrcObject(Project.ProjectNode(), root, false);
        if (!doc.save_file(out_file.c_str(), "\t"))
        {
            wxMessageBox(wxString("An unexpected error occurred exporting ") << out_file, "Export XRC");
        }
        return true;
    }
    std::vector<Node*> forms;
    Project.CollectForms(forms);
    tt_string path;

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    results.EndClock();
#endif

    for (auto& form: forms)
    {
        if (auto& base_file = form->as_string(prop_xrc_file); base_file.size())
        {
            path = Project.BaseDirectory(form, GEN_LANG_XRC);
            if (path.size())
            {
                path.append_filename(base_file);
            }
            else
            {
                path = base_file;
            }

            path.make_absolute();
            path.backslashestoforward();
        }
        else
        {
            // If the form type is supported, warn the user about not having an XRC file for it.
            if (!form->isGen(gen_Images) && !form->isGen(gen_wxPopupTransientWindow))
                results.msgs.emplace_back() << "No XRC filename specified for " << form->as_string(prop_class_name) << '\n';
            continue;
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
            wxFile file_original(path.make_wxString(), wxFile::read);
            if (file_original.IsOpened())
            {
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
                            ++results.file_count;
                            continue;
                        }
                    }
                }
            }
        }

        if (!doc_new.save_file(path.c_str(), "\t"))
        {
            results.msgs.emplace_back() << "Cannot create or write to the file " << path << '\n';
        }
        else
        {
            results.updated_files.emplace_back(path);
        }
    }

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    results.EndClock();
#endif

    return true;
}
