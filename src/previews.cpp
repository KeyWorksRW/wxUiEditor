/////////////////////////////////////////////////////////////////////////////
// Purpose:   Top level Preview functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#if __has_include(<format>)
    #include <format>
#endif

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/mstream.h>      // Memory stream classes
#include <wx/sizer.h>        // provide wxSizer class for layout
#include <wx/wizard.h>       // wxWizard class: a GUI control presenting the user with a
#include <wx/xml/xml.h>      // wxXmlDocument - XML parser & data holder class

// The following handlers must be explicitly added

#include <wx/xrc/xh_aui.h>             // XRC resource handler for wxAUI
#include <wx/xrc/xh_auitoolb.h>        // XML resource handler for wxAuiToolBar
#include <wx/xrc/xh_ribbon.h>          // XML resource handler for wxRibbon related classes
#include <wx/xrc/xh_richtext.h>        // XML resource handler for wxRichTextCtrl
#include <wx/xrc/xh_styledtextctrl.h>  // XML resource handler for wxStyledTextCtrl

#include "../tools/preview_settings.h"  // PreviewSettings
#include "../ui/xrccompare.h"           // C++/XRC UI Comparison dialog
#include "dlg_msgs.h"                   // wxMessageDialog dialogs
#include "gen_common.h"                 // GeneratorLibrary -- Generator classes
#include "generate/gen_xrc.h"           // Generate XRC file
#include "mainapp.h"                    // App -- Main application class
#include "mainframe.h"                  // MainFrame -- Main window frame
#include "node.h"                       // Node class
#include "panels/base_panel.h"          // BasePanel -- Code generation panel
#include "preferences.h"                // Preferences -- Stores user preferences
#include "project_handler.h"            // ProjectHandler class
#include "utils.h"                      // Utility functions that work with properties

#include "pugixml.hpp"

// Defined in mockup/mockup_preview.cpp
void CreateMockupChildren(Node* node, wxWindow* parent, wxObject* parent_object, wxSizer* parent_sizer,
                          wxWindow* form_window);
void PreviewXrc(Node* form_node);
void Preview(Node* form_node);

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

    auto form_node = m_selected_node.get();
    if (!form_node->isForm())
    {
        if (form_node->isGen(gen_Project) && form_node->getChildCount())
        {
            form_node = form_node->getChild(0);
        }
        else
        {
            form_node = form_node->getForm();
        }
    }

    switch (form_node->getGenName())
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

    if (static_cast<BasePanel*>(m_notebook->GetCurrentPage()) == m_xrcPanel)
    {
        PreviewXrc(form_node);
        return;
    }
    else if (static_cast<BasePanel*>(m_notebook->GetCurrentPage()) == m_cppPanel)
    {
        PreviewCpp(form_node);
        return;
    }

    Preview(form_node);
}

////////////////////////////// Bind Events //////////////////////////////

// These handlers work with the Preview dialogs and windows.

void MainFrame::OnXrcKeyUp(wxKeyEvent& event)
{
    if (event.GetKeyCode() != WXK_ESCAPE)
        return;

    if (m_pxrc_dlg)
    {
        m_pxrc_dlg->EndModal(wxID_OK);
        m_pxrc_dlg = nullptr;
    }
}

void MainFrame::OnPreviewWinClose(wxCloseEvent& /* event */)
{
    if (m_pxrc_win)
        m_pxrc_win->Destroy();
    m_pxrc_win = nullptr;
}

void MainFrame::OnPreviewWinActivate(wxActivateEvent& event)
{
    if (!event.GetActive())
    {
        if (m_pxrc_win)
            m_pxrc_win->Destroy();
        m_pxrc_win = nullptr;
    }
    else
        event.Skip();
}

////////////////////////////// Top level Preview function //////////////////////////////

void Preview(Node* form_node)
{
    PreviewSettings dlg_preview_settings(wxGetMainFrame());
    if (UserPrefs.GetPreviewType() == Prefs::PREVIEW_TYPE_XRC)
        dlg_preview_settings.set_type_xrc(true);
    else if (UserPrefs.GetPreviewType() == Prefs::PREVIEW_TYPE_BOTH)
        dlg_preview_settings.set_type_both(true);
    else
        dlg_preview_settings.set_type_cpp(true);

    if (dlg_preview_settings.ShowModal() == wxID_CANCEL)
        return;

    if (dlg_preview_settings.is_type_xrc())
        UserPrefs.SetPreviewType(Prefs::PREVIEW_TYPE_XRC);
    else if (dlg_preview_settings.is_type_both())
        UserPrefs.SetPreviewType(Prefs::PREVIEW_TYPE_BOTH);
    else
        UserPrefs.SetPreviewType(Prefs::PREVIEW_TYPE_CPP);

    if (UserPrefs.GetPreviewType() == Prefs::PREVIEW_TYPE_BOTH)
    {
        if (!form_node->isGen(gen_wxDialog) && !form_node->isGen(gen_PanelForm))
        {
            wxMessageBox("You can only compare dialogs and panels", "Compare");
            return;
        }

        tt_cwd cwd(true);
        wxSetWorkingDirectory(Project.ArtDirectory().make_wxString());

        XrcCompare dlg_compare;
        if (!dlg_compare.DoCreate(wxGetMainFrame(), form_node))
        {
            wxMessageBox("Unable to create the XrcCompare dialog box!", "Compare");
            return;
        }

        dlg_compare.ShowModal();
        return;
    }
    else if (UserPrefs.GetPreviewType() == Prefs::PREVIEW_TYPE_CPP)
    {
        wxGetMainFrame()->PreviewCpp(form_node);
        return;
    }
    else if (UserPrefs.GetPreviewType() == Prefs::PREVIEW_TYPE_XRC)
    {
        PreviewXrc(form_node);
        return;
    }
}

void PreviewXrc(Node* form_node)
{
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

    // Our directory is probably already set correctly, but this will make certain that it is.
    tt_cwd save_cwd(true);
    Project.ChangeDir();

    tt_string style = form_node->as_string(prop_style);
    if (form_node->isGen(gen_wxDialog) &&
        (style.empty() || (!style.contains("wxDEFAULT_DIALOG_STYLE") && !style.contains("wxCLOSE_BOX"))))
    {
        tt_string modified_style("wxCLOSE_BOX|wxCAPTION");
        if (style.size())
            modified_style << '|' << style;
        form_node->set_value(prop_style, modified_style);
        wxMessageBox("Caption and Close box temporarily added so that you can close the preview dialog.", "XRC Preview",
                     wxICON_INFORMATION);
    }

    // This needs to be outside of the try block so that xrc_resource->Unload(res_name) can
    // be called after the catch block.
    wxString res_name("wxuiPreview");

    try
    {
        auto doc_str = GenerateXrcStr(form_node, form_node->isGen(gen_PanelForm) ? xrc::previewing : 0);
        wxMemoryInputStream stream(doc_str.c_str(), doc_str.size());
        wxXmlParseError err_details;
        auto xmlDoc = std::make_unique<wxXmlDocument>(wxXmlDocument());
        if (auto result = xmlDoc->Load(stream, wxXMLDOC_NONE, &err_details); !result)
        {
#if __has_include(<format>)
            std::string msg =
                std::format(std::locale(""), "Parsing error: {} at line: {}, column: {}, offset: {:L}\n",
                            err_details.message.ToStdString(), err_details.line, err_details.column, err_details.offset);
#else
            wxString msg;
            msg.Format("Parsing error: %s at line: %d, column: %d, offset: %ld\n", err_details.message, err_details.line,
                       err_details.column, err_details.offset);
#endif
            wxMessageDialog(wxGetMainFrame()->getWindow(), msg, "Parsing Error", wxOK | wxICON_ERROR).ShowModal();
            return;
        }
        if (!xmlDoc->IsOk())
        {
            wxMessageBox("Invalid XRC file generated -- it cannot be loaded.", "XRC Preview");
            return;
        }

        if (!xrc_resource->LoadDocument(xmlDoc.release(), res_name))
        {
            wxMessageBox("wxWidgets could not parse the XRC data.", "XRC Preview");
            return;
        }

        tt_cwd cwd(true);
        wxSetWorkingDirectory(Project.ArtDirectory().make_wxString());

        switch (form_node->getGenName())
        {
            case gen_wxDialog:
            case gen_PanelForm:
            case gen_MenuBar:
            case gen_RibbonBar:
            case gen_ToolBar:
                {
                    wxString dlg_name =
                        form_node->isGen(gen_wxDialog) ? form_node->as_wxString(prop_class_name) : wxString(txt_dlg_name);

                    if (auto* dlg = xrc_resource->LoadDialog(wxGetMainFrame(), dlg_name); dlg)
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
                        wxMessageBox(tt_string("Could not load ") << form_node->as_string(prop_class_name) << " resource.",
                                     "XRC Preview");
                    }
                }
                break;

            case gen_wxFrame:
                if (auto* frame = xrc_resource->LoadFrame(wxGetMainFrame(), form_node->as_wxString(prop_class_name)); frame)
                {
                    wxGetMainFrame()->setPreviewWinPtr(frame);
                    frame->Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnPreviewWinClose, wxGetMainFrame());
                    frame->Bind(wxEVT_ACTIVATE, &MainFrame::OnPreviewWinActivate, wxGetMainFrame());
                    frame->Centre(wxBOTH);
                    frame->Show();
                    // The wxFrame will be deleted when the window is deactivated or closed
                }
                else
                {
                    wxMessageBox(tt_string("Could not load ") << form_node->as_string(prop_class_name) << " resource.",
                                 "XRC Preview");
                }
                break;

            case gen_wxWizard:
                if (auto* object = xrc_resource->LoadObject(NULL, form_node->as_string(prop_class_name), "wxWizard"); object)
                {
                    auto* wizard = wxStaticCast(object, wxWizard);
                    if (form_node->getChildCount())
                    {
                        auto first_page = wizard->FindWindow(form_node->getChild(0)->as_wxString(prop_var_name));
                        wizard->RunWizard(wxStaticCast(first_page, wxWizardPageSimple));
                        wizard->Destroy();
                    }
                    else
                    {
                        wxMessageBox("You can't run a wizard that doesn't have any pages.", "XRC Preview");
                    }
                }
                else
                {
                    wxMessageBox(tt_string("Could not load ") << form_node->as_string(prop_class_name) << " resource.",
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

    // Restore the original style if it was temporarily changed.
    if (form_node->as_string(prop_style) != style)
        form_node->set_value(prop_style, style);

    xrc_resource->Unload(res_name);
}

void MainFrame::PreviewCpp(Node* form_node)
{
    if (form_node->isGen(gen_wxDialog))
    {
        if (!form_node->getChildCount())
        {
            wxMessageBox("You can't display a dialog without any children", "Preview");
            return;
        }
    }

    tt_string style = form_node->as_string(prop_style);
    if (form_node->isGen(gen_wxDialog) &&
        (style.empty() || (!style.contains("wxDEFAULT_DIALOG_STYLE") && !style.contains("wxCLOSE_BOX"))))
    {
        tt_string modified_style("wxCLOSE_BOX|wxCAPTION");
        if (style.size())
            modified_style << '|' << style;
        form_node->set_value(prop_style, modified_style);
        wxMessageBox("Caption and Close box temporarily added so that you can close the preview dialog.", "C++ Preview",
                     wxICON_INFORMATION);
    }

    try
    {
        switch (form_node->getGenName())
        {
            case gen_PanelForm:
                {
                    wxDialog dlg;
                    if (!dlg.Create(wxGetMainFrame(), wxID_ANY, "C++ Preview", wxDefaultPosition, wxDefaultSize,
                                    wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER))
                    {
                        wxMessageBox("Unable to create preview dialog", "C++ Preview");
                        return;
                    }

                    m_pxrc_dlg = &dlg;  // so event handlers can access it
                    dlg.Bind(wxEVT_KEY_UP, &MainFrame::OnXrcKeyUp, this);

                    auto dlg_sizer = new wxBoxSizer(wxVERTICAL);
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
                                    DlgPoint(form_node, prop_pos), DlgSize(form_node, prop_size), GetStyleInt(form_node)))
                    {
                        wxMessageBox("Unable to create dialog", "C++ Preview");
                        return;
                    }
                    if (form_node->hasValue(prop_extra_style))
                    {
                        int ex_style = 0;
                        // Can't use multiview because getConstantAsInt() searches an unordered_map which requires a
                        // std::string to pass to it
                        tt_string_vector mstr(form_node->as_string(prop_extra_style), '|');
                        for (auto& iter: mstr)
                        {
                            // Friendly names will have already been converted, so normal lookup works fine.
                            ex_style |= NodeCreation.getConstantAsInt(iter);
                        }

                        dlg.SetExtraStyle(dlg.GetExtraStyle() | ex_style);
                    }

                    CreateMockupChildren(form_node->getChild(0), &dlg, &dlg, nullptr, &dlg);
                    if (auto btn = dlg.FindWindowById(dlg.GetAffirmativeId()); btn)
                    {
                        btn->Bind(wxEVT_BUTTON,
                                  [&dlg](wxCommandEvent&)
                                  {
                                      dlg.EndModal(wxID_OK);
                                  });
                    }

                    if (auto btn = dlg.FindWindowById(dlg.GetEscapeId()); btn)
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
                        new wxFrame(nullptr, wxID_ANY, form_node->as_string(prop_title), DlgPoint(form_node, prop_pos),
                                    DlgSize(form_node, prop_size), GetStyleInt(form_node));
                    frame)
                {
                    for (auto& iter: form_node->getChildNodePtrs())
                    {
                        CreateMockupChildren(iter.get(), frame, nullptr, nullptr, frame);
                    }
                    // CreateMockupChildren(form_node->getChild(0), frame, frame, nullptr, frame);

                    wxGetMainFrame()->setPreviewWinPtr(frame);
                    frame->Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnPreviewWinClose, wxGetMainFrame());
                    frame->Bind(wxEVT_ACTIVATE, &MainFrame::OnPreviewWinActivate, wxGetMainFrame());
                    frame->Centre(wxBOTH);
                    frame->Show();
                    // The wxFrame will be deleted when the window is deactivated or closed
                }
                break;

            case gen_wxWizard:
                {
                    wxWizard wizard;
                    if (form_node->hasValue(prop_bitmap))
                    {
                        auto bundle = form_node->as_wxBitmapBundle(prop_bitmap);
                        if (!wizard.Create(wxGetMainFrame(), wxID_ANY, form_node->as_string(prop_title), bundle,
                                           DlgPoint(form_node, prop_pos), GetStyleInt(form_node)))
                        {
                            wxMessageBox("Unable to create wizard", "C++ Preview");
                            return;
                        }
                    }
                    else if (!wizard.Create(wxGetMainFrame(), wxID_ANY, form_node->as_string(prop_title), wxNullBitmap,
                                            DlgPoint(form_node, prop_pos), GetStyleInt(form_node)))
                    {
                        wxMessageBox("Unable to create wizard", "C++ Preview");
                        return;
                    }

                    if (form_node->hasValue(prop_extra_style))
                    {
                        int ex_style = 0;
                        // Can't use multiview because getConstantAsInt() searches an unordered_map which requires a
                        // std::string to pass to it
                        tt_string_vector mstr(form_node->as_string(prop_extra_style), '|');
                        for (auto& iter: mstr)
                        {
                            // Friendly names will have already been converted, so normal lookup works fine.
                            ex_style |= NodeCreation.getConstantAsInt(iter);
                        }
                        wizard.SetExtraStyle(ex_style);
                    }

                    if (form_node->as_int(prop_border) != 5)
                        wizard.SetBorder(form_node->as_int(prop_border));
                    if (form_node->hasValue(prop_bmp_placement))
                    {
                        int placement = 0;
                        // Can't use multiview because getConstantAsInt() searches an unordered_map which requires a
                        // std::string to pass to it
                        tt_string_vector mstr(form_node->as_string(prop_bmp_placement), '|');
                        for (auto& iter: mstr)
                        {
                            // Friendly names will have already been converted, so normal lookup works fine.
                            placement |= NodeCreation.getConstantAsInt(iter);
                        }
                        wizard.SetBitmapPlacement(placement);

                        if (form_node->as_int(prop_bmp_min_width) > 0)
                            wizard.SetMinimumBitmapWidth(form_node->as_int(prop_bmp_min_width));
                        if (form_node->hasValue(prop_bmp_background_colour))
                            wizard.SetBitmapBackgroundColour(form_node->as_wxColour(prop_bmp_background_colour));
                    }

                    std::vector<wxWizardPageSimple*> pages;
                    for (auto& page: form_node->getChildNodePtrs())
                    {
                        auto wiz_page = new wxWizardPageSimple;
                        pages.emplace_back(wiz_page);
                        if (page->hasValue(prop_bitmap))
                        {
                            auto bundle = page->as_wxBitmapBundle(prop_bitmap);
                            wiz_page->Create(&wizard, nullptr, nullptr, bundle);
                        }
                        else
                            wiz_page->Create(&wizard);

                        if (page->getChildCount())
                            CreateMockupChildren(page->getChild(0), wiz_page, nullptr, nullptr, &wizard);
                    }

                    for (size_t idx = 0; idx < pages.size(); ++idx)
                    {
                        if (idx > 0)
                            pages[idx]->SetPrev(pages[idx - 1]);
                        if (idx + 1 < pages.size())
                            pages[idx]->SetNext(pages[idx + 1]);
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
        form_node->set_value(prop_style, style);
}
