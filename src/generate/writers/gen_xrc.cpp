/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate XRC file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <cstring>  // for std::memcmp
#include <sstream>

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

#include "gen_xrc.h"  // BaseCodeGenerator -- Generate Src and Hdr files for Base Class

#include "../panels/propgrid_panel.h"  // PropGridPanel -- Node inspector class
#include "../ui/xrccompare.h"          // C++/XRC UI Comparison dialog
#include "gen_common.h"                // GeneratorLibrary -- Generator classes
#include "gen_results.h"               // Code generation file writing functions
#include "gen_xrc_utils.h"             // Common XRC generating functions
#include "mainframe.h"                 // MainFrame -- Main window frame
#include "node.h"                      // Node class
#include "node_creator.h"              // NodeCreator -- Class used to create nodes
#include "project_handler.h"           // ProjectHandler class
#include "utils.h"                     // Utility functions that work with properties
#include "write_code.h"                // Write code to Scintilla or file

#include "pugixml.hpp"

const char* txt_dlg_name = "_wxue_temp_dlg";

int GenerateXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto* generator = node->get_NodeDeclaration()->get_Generator();
    auto result = generator->GenXrcObject(node, object, xrc_flags);
    if (result == BaseGenerator::xrc_not_supported)
    {
        if (node->is_Gen(gen_Project))
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
        if (node->is_Gen(gen_wxCollapsiblePane))
        {
            // XRC wants a panewindow object as the sole child of wxCollapsiblePane, and all node
            // children must be added as children of this panewindow.

            actual_object = actual_object.append_child("object");
            actual_object.append_attribute("class").set_value("panewindow");
        }

        for (const auto& child: node->get_ChildNodePtrs())
        {
            // Normally, the XRC hierarchy matches our node hierarchy with the exception of XRC
            // needing a sizeritem as the immediate parent of a widget node. The exception is
            // wxTreebook -- while our nodes have BookPages as children of BookPages, XRC expects
            // all BookPages to be children of the wxTreebook with a depth parameter indicating if
            // it is a sub-page or not.

            if (child->is_Gen(gen_BookPage) && child->get_Parent()->is_Gen(gen_BookPage))
            {
                int depth = 0;
                actual_object = object;
                for (;;)
                {
                    auto class_attr = actual_object.attribute("class");
                    if (class_attr.value() != "wxTreebook")
                    {
                        if (class_attr.value() == "treebookpage")
                        {
                            ++depth;
                        }
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
    if (result == BaseGenerator::xrc_updated)
    {
        if (node->is_Gen(gen_tool_dropdown))
        {
            return result;  // The dropdown tool will already have handled it's children.
        }

        for (const auto& child: node->get_ChildNodePtrs())
        {
            auto child_object = object.append_child("object");
            auto child_result = GenerateXrcObject(child.get(), child_object, xrc_flags);
            if (child_result == BaseGenerator::xrc_not_supported)
            {
                object.remove_child(child_object);
                // REVIEW: [Randalphwa - 09-02-2022] In most cases, we can simply skip over the
                // unsupported node. If not, we need to special-case it rather than just breaking
                // out of the loop.
            }
        }
        return result;
    }
    if (result == BaseGenerator::xrc_form_not_supported)
    {
        if (xrc_flags & xrc::add_comments)
        {
            return result;
        }
        return BaseGenerator::xrc_not_supported;
    }
    return BaseGenerator::xrc_not_supported;
}

namespace
{

    void CollectHandlers(Node* node, std::set<std::string>& handlers)
    {
        auto* generator = node->get_NodeDeclaration()->get_Generator();
        generator->RequiredHandlers(node, handlers);
        for (const auto& child: node->get_ChildNodePtrs())
        {
            auto* child_generator = child->get_NodeDeclaration()->get_Generator();
            child_generator->RequiredHandlers(child.get(), handlers);
            if (child->get_ChildCount())
            {
                CollectHandlers(child.get(), handlers);
            }
        }
    }

}  // anonymous namespace

std::string GenerateXrcStr(Node* node_start, size_t xrc_flags)
{
    pugi::xml_document doc;
    auto root = doc.append_child("resource");
    root.append_attribute("xmlns") = "http://www.wxwidgets.org/wxxrc";
    root.append_attribute("version") = "2.5.3.0";

    if (node_start->is_Gen(gen_MenuBar) || node_start->is_Gen(gen_RibbonBar) ||
        node_start->is_Gen(gen_ToolBar))
    {
        if (auto temp_form = NodeCreation.CreateNode(gen_PanelForm, nullptr).first; temp_form)
        {
            auto sizer = NodeCreation.CreateNode(gen_VerticalBoxSizer, temp_form.get()).first;
            temp_form->AdoptChild(sizer);
            auto node_copy = NodeCreation.MakeCopy(node_start, sizer.get());
            sizer->AdoptChild(node_copy);
            node_start = temp_form.get();
        }
    }

    if (!node_start)
    {
        root.append_child("object");
    }
    else if (node_start->is_Gen(gen_Project))
    {
        GenerateXrcObject(node_start, root, xrc_flags);
    }
    else if ((xrc_flags & xrc::previewing) && node_start->is_Gen(gen_PanelForm))
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
    else if ((xrc_flags & xrc::previewing) && node_start->is_Gen(gen_wxDialog))
    {
        auto object = root.append_child("object");
        object.append_attribute("class").set_value("wxPanel");
        object.append_attribute("name").set_value(txt_dlg_name);
        object = object.append_child("object");
        GenerateXrcObject(node_start->get_Child(0), object, xrc_flags);
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

XrcCodeGenerator::XrcCodeGenerator(Node* form_node) : BaseCodeGenerator(GEN_LANG_XRC, form_node) {}

void XrcCodeGenerator::GenerateClass(GenLang language, PANEL_PAGE panel_type,
                                     wxProgressDialog* /* progress */)
{
    m_language = language;
    ASSERT(m_language == GEN_LANG_XRC);
    m_panel_type = panel_type;

    if (m_header)
    {
        m_header->Clear();
    }
    m_source->Clear();

    if (!m_form_node)
    {
        return;
    }

    if (m_panel_type != PANEL_PAGE::HDR_INFO_PANEL)
    {
        XrcGenerator xrc_gen;
        xrc_gen.AddProjectFlags();
        if (m_panel_type == PANEL_PAGE::SOURCE_PANEL)
        {
            xrc_gen.AddXrcFlags(xrc::add_comments);
        }
        xrc_gen.AddNode(m_form_node);
        auto doc_str = xrc_gen.getXmlString();
        m_source->doWrite(doc_str);
    }

    // Info panel - only used for display mode, m_header must be valid
    else if (m_header)
    {
        if (m_form_node != Project.get_ProjectNode())
        {
            m_header->writeLine(
                (wxString("Resource name is ") << m_form_node->as_string(prop_class_name))
                    .ToStdString());
            m_header->writeLine();
        }
        m_header->writeLine("Required handlers:");
        m_header->writeLine();
        m_header->Indent();

        std::set<std::string> handlers;
        CollectHandlers(m_form_node, handlers);
        for (const auto& iter: handlers)
        {
            m_header->writeLine(iter);
        }
    }
}

void MainFrame::OnGenSingleXRC(wxCommandEvent& /* event unused */)
{
    auto* form = wxGetMainFrame()->getSelectedNode();
    if (form && !form->is_Form())
    {
        form = form->get_Form();
    }
    if (!form)
    {
        wxMessageBox("You must select a form before you can generate code.", "Code Generation");
        return;
    }
    if (form->is_Gen(gen_Images) || form->is_Gen(gen_Data) ||
        form->is_Gen(gen_wxPopupTransientWindow))
    {
        wxMessageBox("You cannot generate an XRC file for this type of form.", "Code Generation");
        return;
    }

    GenResults results;
    results.SetNodes(form);
    results.SetLanguages(GEN_LANG_XRC);
    results.SetMode(GenResults::Mode::generate_and_write);
    std::ignore = results.Generate();

    wxString msg;
    if (results.GetUpdatedFiles().size())
    {
        if (results.GetUpdatedFiles().size() == 1)
        {
            msg << "1 file was updated";
        }
        else
        {
            msg << results.GetUpdatedFiles().size() << " files were updated";
        }
        msg << '\n';
    }
    else
    {
        msg << "Generated file is current";
    }

    if (results.GetMsgs().size())
    {
        for (auto& iter: results.GetMsgs())
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

void XrcGenerator::AddNode(Node* node_start)
{
    if (!m_root)
    {
        m_root = m_doc.append_child("resource");
        m_root.append_attribute("xmlns") = "http://www.wxwidgets.org/wxxrc";
        m_root.append_attribute("version") = "2.5.3.0";
    }

    if (node_start->is_Gen(gen_MenuBar) || node_start->is_Gen(gen_RibbonBar) ||
        node_start->is_Gen(gen_ToolBar))
    {
        if (auto temp_form = NodeCreation.CreateNode(gen_PanelForm, nullptr).first; temp_form)
        {
            auto sizer = NodeCreation.CreateNode(gen_VerticalBoxSizer, temp_form.get()).first;
            temp_form->AdoptChild(sizer);
            auto node_copy = NodeCreation.MakeCopy(node_start, sizer.get());
            sizer->AdoptChild(node_copy);
            node_start = temp_form.get();
        }
    }

    if (!node_start)
    {
        m_root.append_child("object");
    }
    else if (node_start->is_Gen(gen_Project))
    {
        GenerateXrcObject(node_start, m_root, m_xrc_flags);
    }
    else if ((m_xrc_flags & xrc::previewing) && node_start->is_Gen(gen_PanelForm))
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
    else if ((m_xrc_flags & xrc::previewing) && node_start->is_Gen(gen_wxDialog))
    {
        auto object = m_root.append_child("object");
        object.append_attribute("class").set_value("wxPanel");
        object.append_attribute("name").set_value(txt_dlg_name);
        object = object.append_child("object");
        GenerateXrcObject(node_start->get_Child(0), object, m_xrc_flags);
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

auto XrcGenerator::getIndentationString() -> std::string
{
    if (Project.as_bool(prop_xrc_indent_with_spaces))
    {
        return "  ";  // Two spaces
    }
    return "\t";  // Tab character
}
