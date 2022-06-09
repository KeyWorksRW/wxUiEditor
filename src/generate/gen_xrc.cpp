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

#include "tttextfile.h"  // textfile -- Classes for reading and writing line-oriented files

#include "gen_base.h"

#include "gen_common.h"   // GeneratorLibrary -- Generator classes
#include "mainapp.h"      // App -- Main application class
#include "mainframe.h"    // MainFrame -- Main window frame
#include "node.h"         // Node class
#include "pjtsettings.h"  // ProjectSettings -- Hold data for currently loaded project
#include "utils.h"        // Utility functions that work with properties
#include "write_code.h"   // Write code to Scintilla or file

#include "pugixml.hpp"

namespace xrc_gen
{
    enum
    {
        no_comments = false,
        comments = true,
        preview = true
    };
}

inline constexpr const auto txt_XRC_HEADER = R"===(<?xml version="1.0"?>
<resource xmlns="http://www.wxwidgets.org/wxxrc" version="2.5.3.0">
)===";

inline constexpr const auto txt_XRC_FOOTER = R"===(</resource>
)===";

const char* txt_dlg_name = "_wxue_temp_dlg";

int GenXrcObject(Node* node, pugi::xml_node& object, bool add_comments);

void MainFrame::OnPreviewXrc(wxCommandEvent& /* event */)
{
    m_pxrc_dlg = nullptr;

    if (!m_selected_node)
    {
        wxMessageBox("You need to select a dialog first.", "XRC Dialog Preview");
        return;
    }

    auto form_node = m_selected_node.get();
    if (!form_node->IsForm())
    {
        if (form_node->isGen(gen_Project) && form_node->GetChildCount())
        {
            form_node = form_node->GetChild(0);
        }
        else
        {
            form_node = form_node->get_form();
        }
    }

    if (!form_node->isGen(gen_wxDialog) && !form_node->isGen(gen_PanelForm) && !form_node->isGen(gen_wxFrame) &&
        !form_node->isGen(gen_wxWizard))
    {
        wxMessageBox("This type of form is not available in XRC.", "XRC Preview");
        return;
    }

    auto xrc_resource = wxXmlResource::Get();

    if (!m_isXrcResourceInitalized)
    {
        m_isXrcResourceInitalized = true;

        xrc_resource->InitAllHandlers();
        xrc_resource->AddHandler(new wxRichTextCtrlXmlHandler);
        xrc_resource->AddHandler(new wxAuiXmlHandler);
        xrc_resource->AddHandler(new wxAuiToolBarXmlHandler);
        xrc_resource->AddHandler(new wxRibbonXmlHandler);
        xrc_resource->AddHandler(new wxStyledTextCtrlXmlHandler);
    }

    ttlib::cstr style = form_node->prop_as_string(prop_style);

    wxString res_name("wxuiPreview");
    try
    {
        // Our directory is probably already set correctly, but this will make certain that it is.

        ttSaveCwd save_cwd;
        wxSetWorkingDirectory(wxGetApp().GetProjectPath());

        if (form_node->isGen(gen_wxDialog) &&
            (style.empty() || (!style.contains("wxDEFAULT_DIALOG_STYLE") && !style.contains("wxCLOSE_BOX"))))
        {
            ttlib::cstr modified_style("wxCLOSE_BOX|wxCAPTION");
            if (style.size())
                modified_style << '|' << style;
            form_node->prop_set_value(prop_style, modified_style);
            wxMessageBox("Caption and Close box temporarily added so that you can close the preview dialog.",
                         "wxDialog Preview", wxICON_INFORMATION);
        }

        auto doc_str = GenerateXrcStr(form_node, xrc_gen::no_comments, xrc_gen::preview);
        wxMemoryInputStream stream(doc_str.c_str(), doc_str.size());
        wxScopedPtr<wxXmlDocument> xmlDoc(new wxXmlDocument(stream, "UTF-8"));
        if (!xmlDoc->IsOk())
        {
            wxMessageBox("Invalid XRC file generated -- it cannot be loaded.", "XRC Dialog Preview");
            return;
        }

        if (!xrc_resource->LoadDocument(xmlDoc.release(), res_name))
        {
            wxMessageBox("wxWidgets could not parse the XRC data.", "XRC Dialog Preview");
            return;
        }

        // If it's a form, then the xml-generator has already created a parent dialog.
        if (form_node->isGen(gen_wxDialog) || form_node->isGen(gen_PanelForm))
        {
            wxDialog dlg;
            m_pxrc_dlg = &dlg;  // so event handlers can access it
            dlg.Bind(wxEVT_KEY_UP, &MainFrame::OnXrcKeyUp, this);

            wxString dlg_name =
                form_node->isGen(gen_wxDialog) ? form_node->prop_as_wxString(prop_class_name) : wxString(txt_dlg_name);
            if (xrc_resource->LoadDialog(&dlg, wxGetFrame().GetWindow(), dlg_name))
            {
                dlg.ShowModal();
            }
            else
            {
                wxMessageBox(ttlib::cstr("Could not load ") << form_node->prop_as_string(prop_class_name) << " resource.",
                             "XRC wxDialog Preview");
            }
            m_pxrc_dlg = nullptr;
        }
        else if (form_node->isGen(gen_wxFrame))
        {
            m_pxrc_win = new wxFrame;
            if (xrc_resource->LoadFrame(m_pxrc_win, wxGetFrame().GetWindow(), form_node->prop_as_wxString(prop_class_name)))
            {
                m_pxrc_win->Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnXrcClose, this);
                m_pxrc_win->Bind(wxEVT_ACTIVATE, &MainFrame::OnXrcActivate, this);
                m_pxrc_win->Show();
            }
            else
            {
                wxMessageBox(ttlib::cstr("Could not load ") << form_node->prop_as_string(prop_class_name) << " resource.",
                             "XRC wxFrame Preview");
            }
        }
        else if (form_node->isGen(gen_wxWizard))
        {
            if (auto object = xrc_resource->LoadObject(NULL, form_node->prop_as_string(prop_class_name), "wxWizard"); object)
            {
                auto wizard = wxStaticCast(object, wxWizard);
                if (form_node->GetChildCount())
                {
                    auto first_page = wizard->FindWindow(form_node->GetChild(0)->prop_as_wxString(prop_var_name));
                    wizard->RunWizard(wxStaticCast(first_page, wxWizardPageSimple));
                    wizard->Destroy();
                }
                else
                {
                    wxMessageBox("You can't run a wizard that doesn't have any pages.", "XRC wxWizard Preview");
                }
            }
            else
            {
                wxMessageBox(ttlib::cstr("Could not load ") << form_node->prop_as_string(prop_class_name) << " resource.",
                             "XRC wxWizard Preview");
            }
        }
    }
    catch (const std::exception& TESTING_PARAM(e))
    {
        MSG_ERROR(e.what());
        wxMessageBox("An internal error occurred generating XRC code", "XRC Dialog Preview");
    }

    // Restore the original style if it was temporarily changed.
    if (form_node->prop_as_string(prop_style) != style)
        form_node->prop_set_value(prop_style, style);

    xrc_resource->Unload(res_name);
}

void MainFrame::OnXrcKeyUp(wxKeyEvent& event)
{
    if (event.GetKeyCode() != WXK_ESCAPE)
        return;

    if (m_pxrc_dlg)
    {
        m_pxrc_dlg->EndModal(wxID_OK);
    }
}

// clang-format off
void MainFrame::OnXrcClose(wxCloseEvent& /* event */)
{
    m_pxrc_win->Destroy();
}

void MainFrame::OnXrcActivate(wxActivateEvent& event)
{
    if (!event.GetActive())
        m_pxrc_win->Destroy();
    else
        event.Skip();
}
// clang-format on

void MainFrame::OnExportXRC(wxCommandEvent& WXUNUSED(event))
{
    auto filename = wxGetApp().GetProjectFileName().filename();
    if (filename.is_sameas(txtEmptyProject))
    {
        filename = "MyProject";
    }
    filename.replace_extension(".xrc");

    wxFileDialog dialog(this, "Export Project As XRC", wxGetApp().GetProjectPath(), filename, "XRC File (*.xrc)|*.xrc",
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_OK)
    {
        filename = dialog.GetPath();
        if (filename.extension().empty())
        {
            filename.replace_extension(".xrc");
        }

        pugi::xml_document doc;
        auto root = doc.append_child("resource");
        root.append_attribute("xmlns") = "http://www.wxwidgets.org/wxxrc";
        root.append_attribute("version") = "2.5.3.0";

        GenXrcObject(wxGetApp().GetProject(), root, false);
        if (!doc.save_file(filename.sub_cstr().c_str(), "\t"))
        {
            wxMessageBox(wxString("An unexpected error occurred exportin ") << filename, "Export XRC");
        }
    }
}

int GenXrcObject(Node* node, pugi::xml_node& object, bool add_comments)
{
    auto generator = node->GetNodeDeclaration()->GetGenerator();
    auto result = generator->GenXrcObject(node, object, add_comments);
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
                GenXrcObject(child.get(), child_object, add_comments);
                continue;
            }

            auto child_object = actual_object.append_child("object");
            auto child_result = GenXrcObject(child.get(), child_object, add_comments);
            if (child_result == BaseGenerator::xrc_not_supported)
            {
                actual_object.remove_child(child_object);
            }
        }
        return result;
    }
    else if (result == BaseGenerator::xrc_updated)
    {
        for (const auto& child: node->GetChildNodePtrs())
        {
            auto child_object = object.append_child("object");
            auto child_result = GenXrcObject(child.get(), child_object, add_comments);
            if (child_result == BaseGenerator::xrc_not_supported)
            {
                object.remove_child(child_object);
                break;
            }
        }
        return result;
    }
    else if (result == BaseGenerator::xrc_form_not_supported)
    {
        if (add_comments)
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

std::string GenerateXrcStr(Node* node_start, bool add_comments, bool is_preview)
{
    pugi::xml_document doc;
    auto root = doc.append_child("resource");
    root.append_attribute("xmlns") = "http://www.wxwidgets.org/wxxrc";
    root.append_attribute("version") = "2.5.3.0";

    if (!node_start)
    {
        root.append_child("object");
    }
    else if (node_start->isGen(gen_Project))
    {
        GenXrcObject(node_start, root, add_comments);
    }
    else if (is_preview && node_start->isGen(gen_PanelForm))
    {
        auto object = root.append_child("object");
        object.append_attribute("class").set_value("wxDialog");
        object.append_attribute("name").set_value(txt_dlg_name);
        object.append_child("style").text().set("wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER");
        object.append_child("centered").text().set("1");
        object.append_child("title").text().set(node_start->prop_as_string(prop_class_name));
        auto item = object.append_child("object");
        item.append_attribute("class").set_value("wxBoxSizer");
        item.append_attribute("name").set_value("_wxue_temp_sizer");
        item.append_child("orient").text().set("wxVERTICAL");
        auto sizer_item = item.append_child("object");
        sizer_item.append_attribute("class").set_value("sizeritem");
        object = sizer_item.append_child("object");

        GenXrcObject(node_start, object, add_comments);
    }
    else if (is_preview && node_start->isGen(gen_wxDialog))
    {
        auto object = root.append_child("object");
        object.append_attribute("class").set_value("wxPanel");
        object.append_attribute("name").set_value(txt_dlg_name);
        object = object.append_child("object");
        GenXrcObject(node_start->GetChild(0), object, add_comments);
    }
    else
    {
        auto object = root.append_child("object");
        GenXrcObject(node_start, object, add_comments);
    }

    std::ostringstream xml_stream;
    doc.save(xml_stream, "\t");
    return xml_stream.str();
}

void BaseCodeGenerator::GenerateXrcClass(Node* form_node, PANEL_TYPE panel_type)
{
    m_project = wxGetApp().GetProject();
    m_form_node = form_node;

    m_panel_type = panel_type;

    m_header->Clear();
    m_source->Clear();

    if (m_panel_type != HDR_PANEL)
    {
        auto doc_str = GenerateXrcStr(form_node ? form_node : m_project, m_panel_type == CPP_PANEL);
        m_source->doWrite(doc_str);
    }

    else  // Info panel
    {
        if (form_node != m_project)
        {
            m_header->writeLine(ttlib::cstr("Resource name is ") << form_node->prop_as_string(prop_class_name));
            m_header->writeLine();
        }
        m_header->writeLine("Required handlers:");
        m_header->writeLine();
        m_header->Indent();

        std::set<std::string> handlers;
        CollectHandlers(form_node, handlers);
        for (auto& iter: handlers)
        {
            m_header->writeLine(iter);
        }
    }
}
