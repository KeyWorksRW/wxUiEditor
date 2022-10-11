/////////////////////////////////////////////////////////////////////////////
// Purpose:   Preview Mockup
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/bookctrl.h>     // wxBookCtrlBase: common base class for wxList/Tree/Notebook
#include <wx/choicebk.h>     // wxChoicebook: wxChoice and wxNotebook combination
#include <wx/collpane.h>     // wxCollapsiblePane
#include <wx/gbsizer.h>      // wxGridBagSizer:  A sizer that can lay out items in a grid,
#include <wx/ribbon/bar.h>   // Top-level component of the ribbon-bar-style interface
#include <wx/sizer.h>        // provide wxSizer class for layout
#include <wx/statbox.h>      // wxStaticBox base header
#include <wx/statline.h>     // wxStaticLine class interface
#include <wx/wizard.h>       // wxWizard class: a GUI control presenting the user with a

#include "../mockup/mockup_content.h"  // MockupContent -- Mockup of a form's contents
#include "base_generator.h"            // BaseGenerator -- Base widget generator class
#include "gen_common.h"                // Common component functions
#include "mainframe.h"                 // MainFrame -- Main window frame
#include "node.h"                      // Node class
#include "project_class.h"             // Project class
#include "utils.h"                     // Utility functions that work with properties

// This function is almost identical to MockupContent::CreateChildren. However, the Mockup version assumes the top window is
// a wxPanel, whereas this version assumes the top version is a form.

void CreateMockupChildren(Node* node, wxWindow* parent, wxObject* parent_object, wxSizer* parent_sizer,
                          wxWindow* form_window)
{
#if defined(__WINDOWS__)
    if (node->HasValue(prop_platforms) && !node->value(prop_platforms).contains("Windows"))
        return;
#elif defined(__UNIX__)
    if (node->HasValue(prop_platforms) && !node->value(prop_platforms).contains("Unix"))
        return;
#elif defined(__WXOSX__)
    if (node->HasValue(prop_platforms) && !node->value(prop_platforms).contains("Mac"))
        return;
#endif

    auto generator = node->GetGenerator();
    ASSERT_MSG(generator, ttlib::cstr() << "Missing component for " << node->DeclName());
    if (!generator)
        return;

    auto created_object = generator->CreateMockup(node, parent);
    if (!created_object)
    {
        if (node->IsSpacer() && parent_object)
        {
            if (node->GetParent()->isGen(gen_wxGridBagSizer))
            {
                auto flags = node->GetSizerFlags();
                wxStaticCast(parent_object, wxGridBagSizer)
                    ->Add(node->prop_as_int(prop_width), node->prop_as_int(prop_height),
                          wxGBPosition(node->prop_as_int(prop_row), node->prop_as_int(prop_column)),
                          wxGBSpan(node->prop_as_int(prop_rowspan), node->prop_as_int(prop_colspan)), flags.GetFlags(),
                          node->prop_as_int(prop_border_size));
            }
            else
            {
                if (node->prop_as_int(prop_proportion) != 0)
                {
                    wxStaticCast(parent_object, wxSizer)->AddStretchSpacer(node->prop_as_int(prop_proportion));
                }
                else
                {
                    auto width = node->prop_as_int(prop_width);
                    auto height = node->prop_as_int(prop_height);
                    if (node->prop_as_bool(prop_add_default_border))
                    {
                        width += wxSizerFlags::GetDefaultBorder();
                        height += wxSizerFlags::GetDefaultBorder();
                    }
                    wxStaticCast(parent_object, wxSizer)->Add(width, height);
                }
            }
        }

        return;  // means the component doesn't create any UI element, and cannot have children
    }
    node->SetMockupObject(created_object);

    wxWindow* created_window { nullptr };
    wxSizer* created_sizer { nullptr };

    if (node->isGen(gen_wxMenuBar) || node->isGen(gen_MenuBar))
    {
        if (parent_sizer)
        {
            parent_sizer->Add((wxWindow*) created_object, wxSizerFlags().Expand().Border(0));
            parent_sizer->Add(new wxStaticLine(parent, wxID_ANY), wxSizerFlags().Border(0));
        }

        // REVIEW: [KeyWorks - 06-09-2022] MockupContent::CreateChildren returns here because there is no form,
        // whereas we need to continue processing children

        // BUGBUG: [Randalphwa - 09-09-2022] Er, why are we returning?
        return;
    }
    else if (node->IsSizer() || node->isGen(gen_wxStdDialogButtonSizer) || node->isGen(gen_TextSizer))
    {
        if (node->IsStaticBoxSizer())
        {
            auto staticBoxSizer = wxStaticCast(created_object, wxStaticBoxSizer);
            created_window = staticBoxSizer->GetStaticBox();
            created_sizer = staticBoxSizer;
        }
        else
        {
            created_sizer = wxStaticCast(created_object, wxSizer);
        }

        if (auto minsize = node->prop_as_wxSize(prop_minimum_size); minsize != wxDefaultSize)
        {
            created_sizer->SetMinSize(minsize);
            created_sizer->Layout();
        }
    }
    else
    {
        created_window = wxStaticCast(created_object, wxWindow);
        if (!node->isType(type_images))
        {
            MockupContent::SetWindowProperties(node, created_window, form_window);
        }
    }

    wxWindow* new_wxparent = (created_window ? created_window : parent);

    if (node->isGen(gen_wxCollapsiblePane))
    {
        auto collpane = wxStaticCast(created_object, wxCollapsiblePane);
        new_wxparent = collpane->GetPane();
    }

    if (node->isGen(gen_PageCtrl) && node->GetChildCount())
    {
        auto page_child = node->GetChild(0);
        if (page_child)
        {
            for (const auto& child: page_child->GetChildNodePtrs())
            {
                CreateMockupChildren(child.get(), parent, parent_object, nullptr, form_window);
            }
        }
    }
    else
    {
        for (const auto& child: node->GetChildNodePtrs())
        {
            CreateMockupChildren(child.get(), new_wxparent, created_object, nullptr, form_window);
        }
    }

    if (parent && (created_window || created_sizer))
    {
        if (auto node_parent = node->GetParent(); node_parent)
        {
            if (node_parent->isGen(gen_wxChoicebook) && node->isType(type_widget))
            {
                wxStaticCast(parent_object, wxChoicebook)
                    ->GetControlSizer()
                    ->Add(created_window, wxSizerFlags().Expand().Border(wxALL));
            }
            else if (node_parent->IsSizer())
            {
                auto sizer_flags = node->GetSizerFlags();
                if (node_parent->isGen(gen_wxGridBagSizer))
                {
                    auto sizer = wxStaticCast(parent_object, wxGridBagSizer);
                    wxGBPosition position(node->prop_as_int(prop_row), node->prop_as_int(prop_column));
                    wxGBSpan span(node->prop_as_int(prop_rowspan), node->prop_as_int(prop_colspan));

                    if (created_window)
                        sizer->Add(created_window, position, span, sizer_flags.GetFlags(), sizer_flags.GetBorderInPixels());
                    else
                        sizer->Add(created_sizer, position, span, sizer_flags.GetFlags(), sizer_flags.GetBorderInPixels());
                }
                else
                {
                    auto sizer = wxStaticCast(parent_object, wxSizer);
                    if (created_window && !node->IsStaticBoxSizer())
                    {
                        sizer->Add(created_window, sizer_flags.GetProportion(), sizer_flags.GetFlags(),
                                   sizer_flags.GetBorderInPixels());
                    }
                    else
                    {
                        sizer->Add(created_sizer, sizer_flags.GetProportion(), sizer_flags.GetFlags(),
                                   sizer_flags.GetBorderInPixels());
                    }
                }
            }
        }
    }
    generator->AfterCreation(created_object, parent, node, true);

    if (parent_sizer)
    {
        if (parent_sizer->IsKindOf(wxCLASSINFO(wxGridBagSizer)))
        {
            auto* gb_sizer = wxStaticCast(parent_sizer, wxGridBagSizer);
            if (created_window && !node->IsStaticBoxSizer())
                gb_sizer->Add(created_window, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALL, 5);
            else if (created_sizer)
                gb_sizer->Add(created_sizer, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALL, 5);
        }
        else
        {
            if (created_window && !node->IsStaticBoxSizer())
                parent_sizer->Add(created_window, wxSizerFlags().Expand());
            else if (created_sizer)
                parent_sizer->Add(created_sizer, wxSizerFlags(1).Expand());
        }
    }

    else if ((created_sizer && wxDynamicCast(parent_object, wxWindow)) || (!parent_object && created_sizer))
    {
        parent->SetSizer(created_sizer);
        parent->Fit();
    }
}

void MainFrame::OnMockupPreview(wxCommandEvent& /* event */)
{
    if (!m_selected_node)
    {
        wxMessageBox("You need to select a form first.", "Preview");
        return;
    }

    auto form_node = m_selected_node.get();
    if (!form_node->IsForm())
    {
        if (form_node->isGen(gen_Project) && form_node->GetChildCount())
        {
            form_node = GetProject()->GetFirstFormChild();
        }
        else
        {
            form_node = form_node->get_form();
        }
    }

    if (!form_node->isGen(gen_wxDialog) && !form_node->isGen(gen_PanelForm) && !form_node->isGen(gen_wxFrame) &&
        !form_node->isGen(gen_wxWizard))
    {
        wxMessageBox("This type of form is not available for previewing", "Preview");
        return;
    }

    PreviewCpp(form_node);
}

void MainFrame::PreviewCpp(Node* form_node)
{
    if (form_node->isGen(gen_wxDialog))
    {
        if (!form_node->GetChildCount())
        {
            wxMessageBox("You can't display a dialog without any children", "Preview");
            return;
        }
    }

    ttlib::cstr style = form_node->prop_as_string(prop_style);

    try
    {
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

        switch (form_node->gen_name())
        {
            case gen_PanelForm:
                {
                    wxDialog dlg;
                    if (!dlg.Create(GetWindow(), wxID_ANY, "Mockup Preview", wxDefaultPosition, wxDefaultSize,
                                    wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER))
                    {
                        wxMessageBox("Unable to create mockup dialog", "Mockup Preview");
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
                    if (!dlg.Create(GetWindow(), wxID_ANY, form_node->value(prop_title), DlgPoint(this, form_node, prop_pos),
                                    DlgSize(this, form_node, prop_size), GetStyleInt(form_node)))
                    {
                        wxMessageBox("Unable to create mockup dialog", "Mockup Preview");
                        return;
                    }
                    if (form_node->HasValue(prop_extra_style))
                    {
                        int ex_style = 0;
                        // Can't use multiview because GetConstantAsInt() searches an unordered_map which requires a
                        // std::string to pass to it
                        ttlib::multistr mstr(form_node->value(prop_extra_style), '|');
                        for (auto& iter: mstr)
                        {
                            // Friendly names will have already been converted, so normal lookup works fine.
                            ex_style |= g_NodeCreator.GetConstantAsInt(iter);
                        }

                        dlg.SetExtraStyle(dlg.GetExtraStyle() | ex_style);
                    }

                    CreateMockupChildren(form_node->GetChild(0), &dlg, &dlg, nullptr, &dlg);
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

            case gen_wxWizard:
                {
                    wxWizard wizard;
                    if (form_node->HasValue(prop_bitmap))
                    {
                        auto bundle = form_node->as_image_bundle(prop_bitmap);
                        if (!wizard.Create(GetWindow(), wxID_ANY, form_node->value(prop_title), bundle->bundle,
                                           DlgPoint(this, form_node, prop_pos), GetStyleInt(form_node)))
                        {
                            wxMessageBox("Unable to create mockup dialog", "Mockup Preview");
                            return;
                        }
                    }
                    else if (!wizard.Create(GetWindow(), wxID_ANY, form_node->value(prop_title), wxNullBitmap,
                                            DlgPoint(this, form_node, prop_pos), GetStyleInt(form_node)))
                    {
                        wxMessageBox("Unable to create mockup dialog", "Mockup Preview");
                        return;
                    }

                    if (form_node->HasValue(prop_extra_style))
                    {
                        int ex_style = 0;
                        // Can't use multiview because GetConstantAsInt() searches an unordered_map which requires a
                        // std::string to pass to it
                        ttlib::multistr mstr(form_node->value(prop_extra_style), '|');
                        for (auto& iter: mstr)
                        {
                            // Friendly names will have already been converted, so normal lookup works fine.
                            ex_style |= g_NodeCreator.GetConstantAsInt(iter);
                        }
                        wizard.SetExtraStyle(ex_style);
                    }

                    if (form_node->as_int(prop_border) != 5)
                        wizard.SetBorder(form_node->as_int(prop_border));
                    if (form_node->HasValue(prop_bmp_placement))
                    {
                        int placement = 0;
                        // Can't use multiview because GetConstantAsInt() searches an unordered_map which requires a
                        // std::string to pass to it
                        ttlib::multistr mstr(form_node->value(prop_bmp_placement), '|');
                        for (auto& iter: mstr)
                        {
                            // Friendly names will have already been converted, so normal lookup works fine.
                            placement |= g_NodeCreator.GetConstantAsInt(iter);
                        }
                        wizard.SetBitmapPlacement(placement);

                        if (form_node->as_int(prop_bmp_min_width) > 0)
                            wizard.SetMinimumBitmapWidth(form_node->as_int(prop_bmp_min_width));
                        if (form_node->HasValue(prop_bmp_background_colour))
                            wizard.SetBitmapBackgroundColour(form_node->as_wxColour(prop_bmp_background_colour));
                    }

                    std::vector<wxWizardPageSimple*> pages;
                    for (auto& page: form_node->GetChildNodePtrs())
                    {
                        auto wiz_page = new wxWizardPageSimple;
                        pages.emplace_back(wiz_page);
                        if (page->HasValue(prop_bitmap))
                        {
                            auto bundle = page->as_image_bundle(prop_bitmap);
                            wiz_page->Create(&wizard, nullptr, nullptr, bundle->bundle);
                        }
                        else
                            wiz_page->Create(&wizard);

                        if (page->GetChildCount())
                            CreateMockupChildren(page->GetChild(0), wiz_page, nullptr, nullptr, &wizard);
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
                wxMessageBox("This form is not supported yet...", "Mockup Preview");
                break;
        }
    }
    catch (const std::exception& TESTING_PARAM(e))
    {
        MSG_ERROR(e.what());
        wxMessageBox("An internal error occurred while creating the preview", "Preview");
    }

    // Restore the original style if it was temporarily changed.
    if (form_node->prop_as_string(prop_style) != style)
        form_node->prop_set_value(prop_style, style);
}
