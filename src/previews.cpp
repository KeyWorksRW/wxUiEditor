/////////////////////////////////////////////////////////////////////////////
// Purpose:   Top level Preview functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "previews.h"

#include <format>

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/mstream.h>      // Memory stream classes
#include <wx/sizer.h>        // provide wxSizer class for layout
#include <wx/wizard.h>       // wxWizard class: a GUI control presenting the user with a
#include <wx/xml/xml.h>      // wxXmlDocument - XML parser & data holder class

#include "wxue_namespace/wxue_string.h"         // wxue::string
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector

// The following handlers must be explicitly added

#include <wx/xrc/xh_aui.h>             // XRC resource handler for wxAUI
#include <wx/xrc/xh_auitoolb.h>        // XML resource handler for wxAuiToolBar
#include <wx/xrc/xh_ribbon.h>          // XML resource handler for wxRibbon related classes
#include <wx/xrc/xh_richtext.h>        // XML resource handler for wxRichTextCtrl
#include <wx/xrc/xh_styledtextctrl.h>  // XML resource handler for wxStyledTextCtrl

#include "../tools/preview_settings.h"  // PreviewSettings
#include "../ui/xrccompare.h"           // C++/XRC UI Comparison dialog

#include "dlg_msgs.h"    // wxMessageDialog dialogs
#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "gen_enums.h"
#include "gen_xrc.h"            // Generate XRC file
#include "mainapp.h"            // App -- Main application class
#include "mainframe.h"          // MainFrame -- Main window frame
#include "node.h"               // Node class
#include "node_creator.h"       // NodeCreator class
#include "panels/base_panel.h"  // BasePanel -- Code generation panel
#include "preferences.h"        // Preferences -- Stores user preferences
#include "project_handler.h"    // ProjectHandler class
#include "utils.h"              // Utility functions that work with properties

// Defined in mockup/mockup_preview.cpp
void CreateMockupChildren(Node* node, wxWindow* parent, wxObject* parent_object,
                          wxSizer* parent_sizer, wxWindow* form_window);

bool g_isXrcResourceInitalized { false };

extern const char* txt_dlg_name;  // Defined in gen_xrc.cpp ("_wxue_temp_dlg")

void MainFrame::OnPreviewXrc(wxCommandEvent& /* event */)
{
    m_pxrc_dlg = nullptr;

    if (!m_selected_node)
    {
        wxMessageBox("You need to select a top level form first.", "Preview");
        return;
    }

    auto* form_node = m_selected_node.get();
    if (!form_node->is_Form())
    {
        if (form_node->is_Gen(gen_Project) && form_node->get_ChildCount())
        {
            form_node = form_node->get_Child(0);
        }
        else
        {
            form_node = form_node->get_Form();
        }
    }

    switch (form_node->get_GenName())
    {
        case gen_wxDialog:
        case gen_wxFrame:
        case gen_PanelForm:
        case gen_wxWizard:
        case gen_MenuBar:
        case gen_RibbonBar:
        case gen_ToolBar:
            break;

        default:
            wxMessageBox("This type of form cannot be previewed.", "Preview");
            return;
    }

    if (dynamic_cast<BasePanel*>(m_notebook->GetCurrentPage()) == m_xrcPanel)
    {
        PreviewXrc(form_node);
        return;
    }
    if (dynamic_cast<BasePanel*>(m_notebook->GetCurrentPage()) == m_cppPanel)
    {
        PreviewCpp(form_node);
        return;
    }

    Preview(form_node);
}

////////////////////////////// Bind Events //////////////////////////////

// These handlers work with the Preview dialogs and windows.

auto MainFrame::OnXrcKeyUp(wxKeyEvent& event) -> void
{
    if (event.GetKeyCode() != WXK_ESCAPE)
    {
        return;
    }

    if (m_pxrc_dlg)
    {
        m_pxrc_dlg->EndModal(wxID_OK);
        m_pxrc_dlg = nullptr;
    }
}

auto MainFrame::OnPreviewWinClose(wxCloseEvent& /* event */) -> void
{
    if (m_pxrc_win)
    {
        m_pxrc_win->Destroy();
    }
    m_pxrc_win = nullptr;
}

auto MainFrame::OnPreviewWinActivate(wxActivateEvent& event) -> void
{
    if (!event.GetActive())
    {
        if (m_pxrc_win)
        {
            m_pxrc_win->Destroy();
        }
        m_pxrc_win = nullptr;
    }
    else
    {
        event.Skip();
    }
}

////////////////////////////// Top level Preview function //////////////////////////////

auto Preview(Node* form_node) -> void
{
    PreviewSettings dlg_preview_settings(wxGetMainFrame());
    if (UserPrefs.GetPreviewType() == Prefs::PREVIEW_TYPE::xrc)
    {
        dlg_preview_settings.set_type_xrc(true);
    }
    else if (UserPrefs.GetPreviewType() == Prefs::PREVIEW_TYPE::both)
    {
        dlg_preview_settings.set_type_both(true);
    }
    else
    {
        dlg_preview_settings.set_type_cpp(true);
    }

    if (dlg_preview_settings.ShowModal() == wxID_CANCEL)
    {
        return;
    }

    if (dlg_preview_settings.is_type_xrc())
    {
        UserPrefs.SetPreviewType(Prefs::PREVIEW_TYPE::xrc);
    }
    else if (dlg_preview_settings.is_type_both())
    {
        UserPrefs.SetPreviewType(Prefs::PREVIEW_TYPE::both);
    }
    else
    {
        UserPrefs.SetPreviewType(Prefs::PREVIEW_TYPE::cpp);
    }

    if (UserPrefs.GetPreviewType() == Prefs::PREVIEW_TYPE::both)
    {
        if (!form_node->is_Gen(gen_wxDialog) && !form_node->is_Gen(gen_PanelForm))
        {
            wxMessageBox("You can only compare dialogs and panels", "Compare");
            return;
        }

        wxue::SaveCwd cwd(wxue::restore_cwd);
        wxSetWorkingDirectory(Project.ArtDirectory().wx());

        XrcCompare dlg_compare;
        if (!dlg_compare.DoCreate(wxGetMainFrame(), form_node))
        {
            wxMessageBox("Unable to create the XrcCompare dialog box!", "Compare");
            return;
        }

        dlg_compare.ShowModal();
        return;
    }
    if (UserPrefs.GetPreviewType() == Prefs::PREVIEW_TYPE::cpp)
    {
        wxGetMainFrame()->PreviewCpp(form_node);
        return;
    }
    if (UserPrefs.GetPreviewType() == Prefs::PREVIEW_TYPE::xrc)
    {
        PreviewXrc(form_node);
        return;
    }
}

auto PreviewXrc(Node* form_node) -> void
{
    // Our directory is probably already set correctly, but this will make certain that it is.
    wxue::SaveCwd save_cwd(wxue::restore_cwd);
    Project.ChangeDir();

    wxue::string style = form_node->as_string(prop_style);
    if (form_node->is_Gen(gen_wxDialog) &&
        (style.empty() ||
         (!style.contains("wxDEFAULT_DIALOG_STYLE") && !style.contains("wxCLOSE_BOX"))))
    {
        wxue::string modified_style("wxCLOSE_BOX|wxCAPTION");
        if (style.size())
        {
            modified_style << '|' << style;
        }
        form_node->set_value(prop_style, modified_style);
        wxMessageBox(
            "Caption and Close box temporarily added so that you can close the preview dialog.",
            "XRC Preview", wxICON_INFORMATION);
    }

    auto doc_str =
        GenerateXrcStr(form_node, form_node->is_Gen(gen_PanelForm) ? xrc::previewing : 0);

    // Restore the original style if it was temporarily changed.
    if (form_node->as_string(prop_style) != style)
    {
        form_node->set_value(prop_style, style);
    }

    PreviewXrc(doc_str, form_node->get_GenName(), form_node);
}

auto PreviewXrc(std::string& doc_str, GenEnum::GenName gen_name, Node* form_node) -> void
{
    pugi::xml_document doc;
    if (auto result = doc.load_string(doc_str); !result)
    {
        std::string msg =
            std::format(std::locale(""), "Parsing error: {}\n Line: {}, Column: {}, Offset: {:L}\n",
                        result.description(), result.line, result.column, result.offset);
        wxMessageDialog(wxGetMainFrame()->getWindow(), msg, "Parsing Error", wxOK | wxICON_ERROR)
            .ShowModal();
        return;
    }

    auto* xrc_resource = wxXmlResource::Get();

    if (!g_isXrcResourceInitalized)
    {
        g_isXrcResourceInitalized = true;

        xrc_resource->InitAllHandlers();
        xrc_resource->AddHandler(new wxRichTextCtrlXmlHandler);
        xrc_resource->AddHandler(new wxAuiXmlHandler);
        xrc_resource->AddHandler(new wxAuiToolBarXmlHandler);
        xrc_resource->AddHandler(new wxRibbonXmlHandler);
        xrc_resource->AddHandler(new wxStyledTextCtrlXmlHandler);
    }

    // This needs to be outside of the try block so that xrc_resource->Unload(res_name) can
    // be called after the catch block.
    wxString res_name("wxuiPreview");

    try
    {
        wxMemoryInputStream stream(doc_str.c_str(), doc_str.size());
        wxXmlParseError err_details;
        auto xmlDoc = std::make_unique<wxXmlDocument>(wxXmlDocument());

        // At this point, there really shouldn't be any errors since pugi::xml has already parsed
        // the XML string and would would have exited this function if there were any errors.
        if (auto result = xmlDoc->Load(stream, wxXMLDOC_NONE, &err_details); !result)
        {
            std::string msg = std::format(
                std::locale(""), "Parsing error: {} at line: {}, column: {}, offset: {:L}\n",
                err_details.message.ToStdString(), err_details.line, err_details.column,
                err_details.offset);
            wxMessageDialog(wxGetMainFrame()->getWindow(), msg, "Parsing Error",
                            wxOK | wxICON_ERROR)
                .ShowModal();
            return;
        }

        if (!xrc_resource->LoadDocument(xmlDoc.release(), res_name))
        {
            wxMessageBox("wxXmlResource could not parse the XRC data.", "XRC Preview");
            return;
        }

        wxue::SaveCwd cwd(wxue::restore_cwd);
        wxSetWorkingDirectory(Project.ArtDirectory().wx());

        wxString form_class_name =
            form_node ? form_node->as_string(GenEnum::prop_class_name).c_str() : txt_dlg_name;

        switch (gen_name)
        {
            case gen_wxDialog:
            case gen_PanelForm:
            case gen_MenuBar:
            case gen_RibbonBar:
            case gen_ToolBar:
                {
                    if (auto* dlg = xrc_resource->LoadDialog(wxGetMainFrame(), form_class_name);
                        dlg)
                    {
                        wxGetMainFrame()->setPreviewDlgPtr(dlg);  // so event handlers can access it
                        dlg->Bind(wxEVT_KEY_UP, &MainFrame::OnXrcKeyUp, wxGetMainFrame());
                        dlg->Centre(wxBOTH);
                        dlg->ShowModal();
                        delete dlg;
                        wxGetMainFrame()->setPreviewDlgPtr(nullptr);
                    }
                    else
                    {
                        wxMessageBox(wxString("Could not load ") << form_class_name << " resource.",
                                     "XRC Preview");
                    }
                }
                break;

            case gen_wxFrame:
                if (auto* frame = xrc_resource->LoadFrame(wxGetMainFrame(), form_class_name); frame)
                {
                    wxGetMainFrame()->setPreviewWinPtr(frame);
                    frame->Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnPreviewWinClose,
                                wxGetMainFrame());
                    frame->Bind(wxEVT_ACTIVATE, &MainFrame::OnPreviewWinActivate, wxGetMainFrame());
                    frame->Centre(wxBOTH);
                    frame->Show();
                    // The wxFrame will be deleted when the window is deactivated or closed
                }
                else
                {
                    wxMessageBox(wxString("Could not load ") << form_class_name << " resource.",
                                 "XRC Preview");
                }
                break;

            case gen_wxWizard:
                if (!form_node)
                {
                    wxMessageBox("A wizard requires a form_node to preview it.", "XRC Preview");
                    return;
                }
                if (auto* object = xrc_resource->LoadObject(
                        NULL, form_node->as_string(prop_class_name), "wxWizard");
                    object)
                {
                    auto* wizard = wxStaticCast(object, wxWizard);
                    if (form_node->get_ChildCount())
                    {
                        auto* first_page =
                            wizard->FindWindow(form_node->get_Child(0)->as_wxString(prop_var_name));
                        wizard->RunWizard(wxStaticCast(first_page, wxWizardPageSimple));
                        wizard->Destroy();
                    }
                    else
                    {
                        wxMessageBox("You can't run a wizard that doesn't have any pages.",
                                     "XRC Preview");
                    }
                }
                else
                {
                    wxMessageBox(wxue::string("Could not load ")
                                     << form_node->as_string(prop_class_name) << " resource.",
                                 "XRC Preview");
                }
                break;

            default:
                wxMessageBox("This type of form cannot be previewed.", "XRC Preview");
                break;
        }
    }
    catch (const std::exception& err)
    {
        MSG_ERROR(err.what());
        dlgGenInternalError(err, "XRC code", "XRC Preview");
    }

    xrc_resource->Unload(res_name);
}

void MainFrame::PreviewCpp(Node* form_node)
{
    if (form_node->is_Gen(gen_wxDialog))
    {
        if (!form_node->get_ChildCount())
        {
            wxMessageBox("You can't display a dialog without any children", "Preview");
            return;
        }
    }

    wxue::string style = form_node->as_string(prop_style);
    if (form_node->is_Gen(gen_wxDialog) &&
        (style.empty() ||
         (!style.contains("wxDEFAULT_DIALOG_STYLE") && !style.contains("wxCLOSE_BOX"))))
    {
        wxue::string modified_style("wxCLOSE_BOX|wxCAPTION");
        if (style.size())
        {
            modified_style << '|' << style;
        }
        form_node->set_value(prop_style, modified_style);
        wxMessageBox(
            "Caption and Close box temporarily added so that you can close the preview dialog.",
            "C++ Preview", wxICON_INFORMATION);
    }

    try
    {
        switch (form_node->get_GenName())
        {
            case gen_PanelForm:
                {
                    wxDialog dlg;
                    if (!dlg.Create(wxGetMainFrame(), wxID_ANY, "C++ Preview", wxDefaultPosition,
                                    wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER))
                    {
                        wxMessageBox("Unable to create preview dialog", "C++ Preview");
                        return;
                    }

                    m_pxrc_dlg = &dlg;  // so event handlers can access it
                    dlg.Bind(wxEVT_KEY_UP, &MainFrame::OnXrcKeyUp, this);

                    auto* dlg_sizer = new wxBoxSizer(wxVERTICAL);
                    dlg_sizer->SetMinSize(wxSize(300, 400));
                    CreateMockupChildren(form_node, &dlg, nullptr, dlg_sizer, &dlg);
                    dlg.SetSizerAndFit(dlg_sizer);
                    dlg.Centre(wxBOTH);

                    dlg.ShowModal();
                }
                break;

            case gen_wxDialog:
                {
                    wxDialog dlg;
                    if (!dlg.Create(wxGetMainFrame(), wxID_ANY, form_node->as_string(prop_title),
                                    DlgPoint(form_node, prop_pos), DlgSize(form_node, prop_size),
                                    GetStyleInt(form_node)))
                    {
                        wxMessageBox("Unable to create dialog", "C++ Preview");
                        return;
                    }
                    if (form_node->HasValue(prop_extra_style))
                    {
                        int ex_style = 0;
                        // Can't use multiview because get_ConstantAsInt() searches an unordered_map
                        // which requires a std::string to pass to it
                        wxue::StringVector mstr(form_node->as_string(prop_extra_style), '|');
                        for (auto& iter: mstr)
                        {
                            // Friendly names will have already been converted, so normal lookup
                            // works fine.
                            ex_style |= NodeCreation.get_ConstantAsInt(iter);
                        }

                        dlg.SetExtraStyle(dlg.GetExtraStyle() | ex_style);
                    }

                    CreateMockupChildren(form_node->get_Child(0), &dlg, &dlg, nullptr, &dlg);
                    if (auto* btn = wxDialog::FindWindowById(dlg.GetAffirmativeId()); btn)
                    {
                        btn->Bind(wxEVT_BUTTON,
                                  [&dlg](wxCommandEvent&)
                                  {
                                      dlg.EndModal(wxID_OK);
                                  });
                    }

                    if (auto* btn = wxDialog::FindWindowById(dlg.GetEscapeId()); btn)
                    {
                        btn->Bind(wxEVT_BUTTON,
                                  [&dlg](wxCommandEvent&)
                                  {
                                      dlg.EndModal(wxID_CANCEL);
                                  });
                    }

                    dlg.Fit();
                    dlg.Centre(wxBOTH);

                    dlg.ShowModal();
                }
                break;

            case gen_wxFrame:
                if (auto* frame =
                        new wxFrame(nullptr, wxID_ANY, form_node->as_string(prop_title),
                                    DlgPoint(form_node, prop_pos), DlgSize(form_node, prop_size),
                                    GetStyleInt(form_node));
                    frame)
                {
                    for (auto& iter: form_node->get_ChildNodePtrs())
                    {
                        CreateMockupChildren(iter.get(), frame, nullptr, nullptr, frame);
                    }
                    // CreateMockupChildren(form_node->get_Child(0), frame, frame, nullptr, frame);

                    wxGetMainFrame()->setPreviewWinPtr(frame);
                    frame->Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnPreviewWinClose,
                                wxGetMainFrame());
                    frame->Bind(wxEVT_ACTIVATE, &MainFrame::OnPreviewWinActivate, wxGetMainFrame());
                    frame->Centre(wxBOTH);
                    frame->Show();
                    // The wxFrame will be deleted when the window is deactivated or closed
                }
                break;

            case gen_wxWizard:
                {
                    wxWizard wizard;
                    if (form_node->HasValue(prop_bitmap))
                    {
                        auto bundle = form_node->as_wxBitmapBundle(prop_bitmap);
                        if (!wizard.Create(wxGetMainFrame(), wxID_ANY,
                                           form_node->as_string(prop_title), bundle,
                                           DlgPoint(form_node, prop_pos), GetStyleInt(form_node)))
                        {
                            wxMessageBox("Unable to create wizard", "C++ Preview");
                            return;
                        }
                    }
                    else if (!wizard.Create(wxGetMainFrame(), wxID_ANY,
                                            form_node->as_string(prop_title), wxNullBitmap,
                                            DlgPoint(form_node, prop_pos), GetStyleInt(form_node)))
                    {
                        wxMessageBox("Unable to create wizard", "C++ Preview");
                        return;
                    }

                    if (form_node->HasValue(prop_extra_style))
                    {
                        int ex_style = 0;
                        // Can't use multiview because get_ConstantAsInt() searches an unordered_map
                        // which requires a std::string to pass to it
                        wxue::StringVector mstr(form_node->as_string(prop_extra_style), '|');
                        for (auto& iter: mstr)
                        {
                            // Friendly names will have already been converted, so normal lookup
                            // works fine.
                            ex_style |= NodeCreation.get_ConstantAsInt(iter);
                        }
                        wizard.SetExtraStyle(ex_style);
                    }

                    if (form_node->as_int(prop_border) != 5)
                    {
                        wizard.SetBorder(form_node->as_int(prop_border));
                    }
                    if (form_node->HasValue(prop_bmp_placement))
                    {
                        int placement = 0;
                        // Can't use multiview because get_ConstantAsInt() searches an unordered_map
                        // which requires a std::string to pass to it
                        wxue::StringVector mstr(form_node->as_string(prop_bmp_placement), '|');
                        for (auto& iter: mstr)
                        {
                            // Friendly names will have already been converted, so normal lookup
                            // works fine.
                            placement |= NodeCreation.get_ConstantAsInt(iter);
                        }
                        wizard.SetBitmapPlacement(placement);

                        if (form_node->as_int(prop_bmp_min_width) > 0)
                        {
                            wizard.SetMinimumBitmapWidth(form_node->as_int(prop_bmp_min_width));
                        }
                        if (form_node->HasValue(prop_bmp_background_colour))
                        {
                            wizard.SetBitmapBackgroundColour(
                                form_node->as_wxColour(prop_bmp_background_colour));
                        }
                    }

                    std::vector<wxWizardPageSimple*> pages;
                    for (auto& page: form_node->get_ChildNodePtrs())
                    {
                        auto* wiz_page = new wxWizardPageSimple;
                        pages.emplace_back(wiz_page);
                        if (page->HasValue(prop_bitmap))
                        {
                            auto bundle = page->as_wxBitmapBundle(prop_bitmap);
                            wiz_page->Create(&wizard, nullptr, nullptr, bundle);
                        }
                        else
                        {
                            wiz_page->Create(&wizard);
                        }

                        if (page->get_ChildCount())
                        {
                            CreateMockupChildren(page->get_Child(0), wiz_page, nullptr, nullptr,
                                                 &wizard);
                        }
                    }

                    for (size_t idx = 0; idx < pages.size(); ++idx)
                    {
                        if (idx > 0)
                        {
                            pages[idx]->SetPrev(pages[idx - 1]);
                        }
                        if (idx + 1 < pages.size())
                        {
                            pages[idx]->SetNext(pages[idx + 1]);
                        }
                    }

                    wizard.RunWizard(wxStaticCast(pages[0], wxWizardPageSimple));
                    wizard.Destroy();
                }
                break;

            default:
                wxMessageBox("This form is not supported yet...", "C++ Preview");
                break;
        }
    }
    catch (const std::exception& err)
    {
        MSG_ERROR(err.what());
        dlgGenInternalError(err, "preview", "Preview");
    }

    // Restore the original style if it was temporarily changed.
    if (form_node->as_string(prop_style) != style)
    {
        form_node->set_value(prop_style, style);
    }
}
