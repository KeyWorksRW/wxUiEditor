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
#include "utils.h"                     // Utility functions that work with properties

// This function is almost identical to MockupContent::CreateChildren. However, the Mockup version assumes the top window is
// a wxPanel, whereas this version assumes the top version is a form.

void CreateMockupChildren(Node* node, wxWindow* parent, wxObject* parent_object, wxSizer* parent_sizer,
                          wxWindow* form_window)
{
#if defined(__WINDOWS__)
    if (node->hasValue(prop_platforms) && !node->as_string(prop_platforms).contains("Windows"))
        return;
#elif defined(__UNIX__)
    if (node->hasValue(prop_platforms) && !node->as_string(prop_platforms).contains("Unix"))
        return;
#elif defined(__WXOSX__)
    if (node->hasValue(prop_platforms) && !node->as_string(prop_platforms).contains("Mac"))
        return;
#endif

    auto generator = node->getGenerator();
    ASSERT_MSG(generator, tt_string() << "Missing component for " << node->declName());
    if (!generator)
        return;

    auto created_object = generator->CreateMockup(node, parent);
    if (!created_object)
    {
        if (node->isSpacer() && parent_object)
        {
            if (node->getParent()->isGen(gen_wxGridBagSizer))
            {
                auto flags = node->getSizerFlags();
                wxStaticCast(parent_object, wxGridBagSizer)
                    ->Add(node->as_int(prop_width), node->as_int(prop_height),
                          wxGBPosition(node->as_int(prop_row), node->as_int(prop_column)),
                          wxGBSpan(node->as_int(prop_rowspan), node->as_int(prop_colspan)), flags.GetFlags(),
                          parent->FromDIP(wxSize(node->as_int(prop_border_size), -1)).x);
            }
            else
            {
                if (node->as_int(prop_proportion) != 0)
                {
                    wxStaticCast(parent_object, wxSizer)->AddStretchSpacer(node->as_int(prop_proportion));
                }
                else
                {
                    auto width = node->as_int(prop_width);
                    auto height = node->as_int(prop_height);
                    if (node->as_bool(prop_add_default_border))
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
    node->setMockupObject(created_object);

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
    else if (node->isSizer() || node->isGen(gen_wxStdDialogButtonSizer) || node->isGen(gen_TextSizer))
    {
        if (node->isStaticBoxSizer())
        {
            auto staticBoxSizer = wxStaticCast(created_object, wxStaticBoxSizer);
            created_window = staticBoxSizer->GetStaticBox();
            created_sizer = staticBoxSizer;
        }
        else
        {
            created_sizer = wxStaticCast(created_object, wxSizer);
        }

        if (auto minsize = node->as_wxSize(prop_minimum_size); minsize != wxDefaultSize)
        {
            created_sizer->SetMinSize(minsize);
            created_sizer->Layout();
        }
    }
    else
    {
        created_window = wxStaticCast(created_object, wxWindow);
        if (!node->isType(type_images) && !node->isType(type_data_list))
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

    if (node->isGen(gen_PageCtrl) && node->getChildCount())
    {
        auto page_child = node->getChild(0);
        if (page_child)
        {
            for (const auto& child: page_child->getChildNodePtrs())
            {
                CreateMockupChildren(child.get(), parent, parent_object, nullptr, form_window);
            }
        }
    }
    else
    {
        for (const auto& child: node->getChildNodePtrs())
        {
            CreateMockupChildren(child.get(), new_wxparent, created_object, nullptr, form_window);
        }
    }

    if (parent && (created_window || created_sizer))
    {
        if (auto node_parent = node->getParent(); node_parent)
        {
            if (node_parent->isGen(gen_wxChoicebook) && node->isType(type_widget))
            {
                wxStaticCast(parent_object, wxChoicebook)
                    ->GetControlSizer()
                    ->Add(created_window, wxSizerFlags().Expand().Border(wxALL));
            }
            else if (node_parent->isSizer())
            {
                auto sizer_flags = node->getSizerFlags();
                if (node_parent->isGen(gen_wxGridBagSizer))
                {
                    auto sizer = wxStaticCast(parent_object, wxGridBagSizer);
                    wxGBPosition position(node->as_int(prop_row), node->as_int(prop_column));
                    wxGBSpan span(node->as_int(prop_rowspan), node->as_int(prop_colspan));

                    if (created_window)
                        sizer->Add(created_window, position, span, sizer_flags.GetFlags(), sizer_flags.GetBorderInPixels());
                    else
                        sizer->Add(created_sizer, position, span, sizer_flags.GetFlags(), sizer_flags.GetBorderInPixels());
                }
                else
                {
                    auto sizer = wxStaticCast(parent_object, wxSizer);
                    if (created_window && !node->isStaticBoxSizer())
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
            if (created_window && !node->isStaticBoxSizer())
                gb_sizer->Add(created_window, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALL, 5);
            else if (created_sizer)
                gb_sizer->Add(created_sizer, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALL, 5);
        }
        else
        {
            if (created_window && !node->isStaticBoxSizer())
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
