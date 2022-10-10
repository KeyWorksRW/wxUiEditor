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

#include <tttextfile_wx.h>  // textfile -- Classes for reading and writing line-oriented files

#include "gen_base.h"  // BaseCodeGenerator -- Generate Src and Hdr files for Base Class

#include "../panels/propgrid_panel.h"  // PropGridPanel -- Node inspector class
#include "../ui/xrccompare.h"          // auto-generated: xrccompare_base.h and xrccompare_base.cpp
#include "cstm_event.h"                // CustomEvent -- Custom Event class
#include "gen_common.h"                // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"             // Common XRC generating functions
#include "generate_xrc_dlg.h"          // GenerateXrcDlg -- Dialog for generating XRC file(s)
#include "mainframe.h"                 // MainFrame -- Main window frame
#include "node.h"                      // Node class
#include "preview_settings.h"          // PreviewSettings
#include "project_class.h"             // Project class
#include "utils.h"                     // Utility functions that work with properties
#include "write_code.h"                // Write code to Scintilla or file

#include "pugixml.hpp"

const char* txt_dlg_name = "_wxue_temp_dlg";

void MainFrame::OnPreviewXrc(wxCommandEvent& /* event */)
{
    m_pxrc_dlg = nullptr;

    if (!m_selected_node)
    {
        wxMessageBox("You need to select a top level form first.", "Preview");
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
        wxMessageBox("This type of form cannot be previewed.", "Preview");
        return;
    }

    PreviewSettings dlg_preview_settings(this);
    auto& prefs = wxGetApp().GetPrefs();
    if (prefs.preview_type == App::PREVIEW_TYPE_XRC)
        dlg_preview_settings.set_type_xrc(true);
    else if (prefs.preview_type == App::PREVIEW_TYPE_BOTH)
        dlg_preview_settings.set_type_both(true);
    else
        dlg_preview_settings.set_type_cpp(true);

    if (dlg_preview_settings.ShowModal() == wxID_CANCEL)
        return;

    if (dlg_preview_settings.is_type_xrc())
        prefs.preview_type = App::PREVIEW_TYPE_XRC;
    else if (dlg_preview_settings.is_type_both())
        prefs.preview_type = App::PREVIEW_TYPE_BOTH;
    else
        prefs.preview_type = App::PREVIEW_TYPE_CPP;

    if (prefs.preview_type == App::PREVIEW_TYPE_BOTH)
    {
        if (!form_node->isGen(gen_wxDialog) && !form_node->isGen(gen_PanelForm))
        {
            wxMessageBox("You can only compare dialogs and panels", "Compare");
            return;
        }

        XrcCompare dlg_compare;
        if (!dlg_compare.DoCreate(this, form_node))
        {
            wxMessageBox("Unable to create the XrcCompare dialog box!", "Compare");
            return;
        }

        dlg_compare.ShowModal();
        return;
    }
    else if (prefs.preview_type == App::PREVIEW_TYPE_CPP)
    {
        PreviewCpp(form_node);
        return;
    }

    // If we get here, it's a normal XRC preview

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
        wxSetWorkingDirectory(GetProject()->GetProjectPath());

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

        auto doc_str = GenerateXrcStr(form_node, form_node->isGen(gen_PanelForm) ? xrc::previewing : 0);
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

void MainFrame::OnExportXRC(wxCommandEvent& WXUNUSED(event))
{
    auto project = GetProject();
    if (project->GetChildCount() == 0)
    {
        wxMessageBox("This project does not yet contain any forms -- nothing to save!", "Export XRC");
        return;
    }

    GenerateXrcDlg dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        if (!dlg.CombineFiles())
        {
            GenerateXrcFiles();
        }
        else
        {
            ttlib::cstr out_file = dlg.GetCombinedFilename();
            if (out_file.size() && out_file.extension().empty())
            {
                out_file.replace_extension(".xrc");
            }
            if (GenerateXrcFiles(out_file))
            {
                wxMessageBox(wxString() << out_file.wx_str() << " saved.", "Export XRC", wxOK | wxCENTRE);
            }

            if (!project->HasValue(prop_combined_xrc_file))
            {
                project->set_value(prop_combined_xrc_file, out_file);
                CustomEvent node_event(EVT_NodePropChange, project->get_prop_ptr(prop_combined_xrc_file));
                m_property_panel->OnNodePropChange(node_event);
                m_isProject_modified = true;
            }
        }
    }

    UpdateWakaTime();
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
        object.append_child("title").text().set(node_start->prop_as_string(prop_class_name));
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
        GenXrcObject(node_start->GetChild(0), object, xrc_flags);
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

void BaseCodeGenerator::GenerateXrcClass(Node* form_node, PANEL_PAGE panel_type)
{
    m_project = GetProject();
    m_panel_type = panel_type;

    m_header->Clear();
    m_source->Clear();

    if (form_node)
    {
        m_form_node = form_node;
    }
    else
    {
        m_form_node = GetProject()->GetFirstFormChild();
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
        if (form_node != m_project)
        {
            m_header->writeLine(ttlib::cstr("Resource name is ") << m_form_node->prop_as_string(prop_class_name));
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

bool GenerateXrcFiles(ttlib::cstr out_file, bool NeedsGenerateCheck)
{
    auto project = GetProject();
    if (project->GetChildCount() == 0)
    {
        if (NeedsGenerateCheck)
            return false;

        wxMessageBox("This project does not yet contain any forms -- nothing to save!", "Export XRC");
        return false;
    }

    ttSaveCwd cwd;
    GetProject()->GetProjectPath().ChangeDir();

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

        GenXrcObject(GetProject(), root, false);
        if (!doc.save_file(out_file.c_str(), "\t"))
        {
            wxMessageBox(wxString("An unexpected error occurred exporting ") << out_file, "Export XRC");
        }
        return true;
    }

    std::vector<ttlib::cstr> results;
    size_t currentFiles = 0;

    for (auto& form: project->GetChildNodePtrs())
    {
        if (!form->HasValue(prop_xrc_file))
        {
            // If the form type is supported, warn the user about not having an XRC file for it.
            if (!form->isGen(gen_Images) && !form->isGen(gen_wxPopupTransientWindow))
                results.emplace_back() << "No filename specified for " << form->prop_as_string(prop_class_name) << '\n';
            continue;
        }
        out_file = form->value(prop_xrc_file);
        if (out_file.extension().empty())
        {
            out_file.replace_extension(".xrc");
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
        GenXrcObject(form.get(), form_object, false);

        if (out_file.file_exists())
        {
            std::ostringstream xml_stream;
            doc_new.save(xml_stream, "\t");
            auto new_str = xml_stream.str();

            pugi::xml_document doc_old;
            if (doc_old.load_file(out_file.c_str()))
            {
                std::ostringstream xml_old_stream;
                doc_old.save(xml_old_stream, "\t");
                auto old_str = xml_old_stream.str();
                if (old_str == new_str)
                {
                    ++currentFiles;
                    continue;
                }
            }
        }

        if (NeedsGenerateCheck)
            return true;

        if (!doc_new.save_file(out_file.c_str(), "\t"))
        {
            results.emplace_back() << "Cannot create or write to the file " << out_file << '\n';
        }
        else
        {
            results.emplace_back() << out_file.filename() << " saved" << '\n';
        }
    }

    if (NeedsGenerateCheck)
        return false;

    if (results.size())
    {
        ttlib::cstr msg;
        for (auto& iter: results)
        {
            msg += iter;
        }

        if (currentFiles)
        {
            msg << '\n' << "The other " << currentFiles << " generated files are current";
        }

        wxMessageBox(msg.wx_str(), "Code Generation", wxOK);
    }
    else if (currentFiles)
    {
        ttlib::cstr msg;
        msg << '\n' << "All " << currentFiles << " XRC file(s) are current";
        wxMessageBox(msg, "Code Generation", wxOK);
    }

    return true;
}
