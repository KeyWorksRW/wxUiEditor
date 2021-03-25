/////////////////////////////////////////////////////////////////////////////
// Purpose:   Mockup of a form's contents
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// Note that for most forms, this is the top level wxPanel and we create the contents of the form as if we were the form. The
// notable exception is a MockupWizard -- in this case we create a MockupWizard child which itself is a wxPanel that
// substitutes for the wxWizard form.

#include "pch.h"

#include <wx/bookctrl.h>    // wxBookCtrlBase: common base class for wxList/Tree/Notebook
#include <wx/collpane.h>    // wxCollapsiblePane
#include <wx/gbsizer.h>     // wxGridBagSizer:  A sizer that can lay out items in a grid,
#include <wx/ribbon/bar.h>  // Top-level component of the ribbon-bar-style interface
#include <wx/sizer.h>       // provide wxSizer class for layout
#include <wx/statbox.h>     // wxStaticBox base header
#include <wx/statline.h>    // wxStaticLine class interface

#include "mockup_content.h"

#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "mainframe.h"       // MainFrame -- Main window frame
#include "mockup_parent.h"   // Top-level MockUp Parent window
#include "mockup_wizard.h"   // MockupWizard Mock Up class
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "node_decl.h"       // NodeDeclaration class
#include "utils.h"           // Utility functions that work with properties

MockupContent::MockupContent(wxWindow* parent, MockupParent* mockupParent) : wxPanel(parent)
{
    m_mockupParent = mockupParent;
}

void MockupContent::RemoveNodes()
{
    if (m_wizard)
    {
        m_wizard->Destroy();
        m_wizard = nullptr;
    }

    m_obj_node_pair.clear();
    m_node_obj_pair.clear();

    DestroyChildren();
    SetSizer(nullptr);

    m_parent_sizer = nullptr;
}

// This is called by MockupParent in order to create all child components
void MockupContent::CreateAllGenerators()
{
    auto form = m_mockupParent->GetSelectedForm();
    if (form->GetClassName() == "MenuBar" || form->GetClassName() == "ToolBar")
    {
        m_parent_sizer = new wxBoxSizer(wxVERTICAL);
        CreateChildren(form, this, this, m_parent_sizer);
        SetSizerAndFit(m_parent_sizer);
        return;
    }

    if (form->GetClassName() == "wxFrame")
        m_parent_sizer = new wxBoxSizer(wxVERTICAL);

    for (size_t i = 0; i < form->GetChildCount(); i++)
    {
        auto child = form->GetChild(i);

        if (m_wizard)
        {
            CreateChildren(child, m_wizard, m_wizard);
        }

        else
        {
            CreateChildren(child, this, this, m_parent_sizer);
        }
    }

    if (m_wizard)
    {
        m_wizard->SetSelection(0);

        // A MockupWizard is added directly as a panel without an intervening sizer, so we need to calculate the size of the
        // window that we need to display it.
        auto sizer = m_wizard->GetSizer();
        if (sizer)
        {
            auto min_size = sizer->GetMinSize();
            SetMinClientSize(min_size);
        }
    }

    if (m_parent_sizer)
        SetSizerAndFit(m_parent_sizer);
    else
        Layout();
}

void MockupContent::CreateChildren(Node* node, wxWindow* parent, wxObject* parentNode, wxBoxSizer* parent_sizer)
{
    auto comp = node->GetGenerator();
    ASSERT_MSG(comp, ttlib::cstr() << "Missing component for " << node->GetClassName());
    if (!comp)
        return;

    auto created_object = comp->Create(node, parent);
    if (!created_object)
    {
        if (node->IsSpacer() && parentNode)
        {
            if (node->prop_as_int("proportion") != 0)
            {
                wxStaticCast(parentNode, wxSizer)->AddStretchSpacer(node->prop_as_int("proportion"));
            }
            else
            {
                auto width = node->prop_as_int("width");
                auto height = node->prop_as_int("height");
                if (node->prop_as_bool("add_default_border"))
                {
                    width += wxSizerFlags::GetDefaultBorder();
                    height += wxSizerFlags::GetDefaultBorder();
                }
                wxStaticCast(parentNode, wxSizer)->Add(width, height);
            }
        }
        return;  // means the component doesn't create any UI element, and cannot have children
    }

    wxWindow* created_window = nullptr;
    wxSizer* created_sizer = nullptr;

    if (node->GetClassName() == "wxMenuBar" || node->GetClassName() == "MenuBar")
    {
        // Store the wxObject/Node pair both ways
        m_obj_node_pair[created_object] = node;
        m_node_obj_pair[node] = created_object;

        if (parent_sizer)
        {
            parent_sizer->Add((wxWindow*) created_object, wxSizerFlags().Expand().Border(0));
            parent_sizer->Add(new wxStaticLine(this, wxID_ANY), wxSizerFlags().Border(0));
        }

        // We don't create any children because the only thing visible is the mock menu
        return;
    }
    else if (node->IsSizer() || node->GetClassName() == "wxStdDialogButtonSizer" || node->GetClassName() == "TextSizer")
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

        if (auto minsize = node->prop_as_wxSize(txt_minimum_size); minsize != wxDefaultSize)
        {
            created_sizer->SetMinSize(minsize);
            created_sizer->Layout();
        }
    }
    else
    {
        created_window = wxStaticCast(created_object, wxWindow);
        SetWindowProperties(node, created_window);
    }

    // Store the wxObject/Node pair both ways.
    m_obj_node_pair[created_object] = node;
    m_node_obj_pair[node] = created_object;

    wxWindow* new_wxparent = (created_window ? created_window : parent);

    if (node->GetClassName() == "wxCollapsiblePane")
    {
        auto collpane = wxStaticCast(created_object, wxCollapsiblePane);
        new_wxparent = collpane->GetPane();
    }

    for (const auto& child: node->GetChildNodePtrs())
    {
        CreateChildren(child.get(), new_wxparent, created_object);
    }

    if (node->GetParent()->GetNodeTypeName() == "wizard")
    {
        m_wizard->AddPage(wxStaticCast(created_window, wxPanel));
        return;
    }

    if (parent && (created_window || created_sizer))
    {
        auto obj_parent = GetNode(parentNode);
        if (obj_parent && obj_parent->IsSizer())
        {
            auto child_obj = GetNode(created_object);
            auto sizer_flags = child_obj->GetSizerFlags();
            if (obj_parent->GetClassName() == "wxGridBagSizer")
            {
                auto sizer = wxStaticCast(parentNode, wxGridBagSizer);
                wxGBPosition position(child_obj->prop_as_int(txt_row), child_obj->prop_as_int(txt_column));
                wxGBSpan span(child_obj->prop_as_int(txt_rowspan), child_obj->prop_as_int(txt_colspan));

                if (created_window)
                    sizer->Add(created_window, position, span, sizer_flags.GetFlags(), sizer_flags.GetBorderInPixels());
                else
                    sizer->Add(created_sizer, position, span, sizer_flags.GetFlags(), sizer_flags.GetBorderInPixels());
            }
            else
            {
                auto sizer = wxStaticCast(parentNode, wxSizer);
                if (created_window && !child_obj->IsStaticBoxSizer())
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
    comp->AfterCreation(created_object, parent);

    if (parent_sizer)
    {
        if (created_window)
            parent_sizer->Add(created_window, wxSizerFlags().Expand().Border(0));
        else if (created_sizer)
            parent_sizer->Add(created_sizer, wxSizerFlags(1).Expand().Border(0));
    }

    else if ((created_sizer && wxDynamicCast(parentNode, wxWindow)) || (!parentNode && created_sizer))
    {
        parent->SetSizer(created_sizer);
        if (parentNode)
            created_sizer->SetSizeHints(parent);

        parent->SetAutoLayout(true);
        parent->Layout();
    }
}

void MockupContent::SetWindowProperties(Node* node, wxWindow* window)
{
    if (auto size = node->prop_as_wxSize("size"); size != wxDefaultSize)
    {
        window->SetSize(size);
    }

    if (auto minsize = node->prop_as_wxSize(txt_minimum_size); minsize != wxDefaultSize)
    {
        window->SetMinSize(minsize);
    }

    if (auto maxsize = node->prop_as_wxSize("maximum_size"); maxsize != wxDefaultSize)
    {
        window->SetMaxSize(maxsize);
    }

    if (auto font = node->get_prop_ptr("font"); font && font->HasValue())
    {
        window->SetFont(node->prop_as_font("font"));
    }

    if (auto fg_colour = node->get_prop_ptr("foreground_colour"); fg_colour && fg_colour->HasValue())
    {
        window->SetForegroundColour(ConvertToColour(fg_colour->GetValue()));
    }

    if (auto bg_colour = node->get_prop_ptr("background_colour"); bg_colour && bg_colour->HasValue())
    {
        window->SetBackgroundColour(ConvertToColour(bg_colour->GetValue()));
    }

    if (auto extra_style = node->get_prop_ptr("window_extra_style"); extra_style && extra_style->as_int() != 0)
    {
        window->SetExtraStyle(extra_style->as_int());
    }

    if (auto disabled = node->get_prop_ptr(txt_disabled); disabled && disabled->as_bool())
    {
        window->Disable();
    }

    if (auto hidden = node->get_prop_ptr(txt_hidden); hidden && hidden->as_bool() && !m_mockupParent->IsShowingHidden())
    {
        window->Show(false);
    }

    if (auto tooltip = node->get_prop_ptr("tooltip"); tooltip && tooltip->as_string().size())
    {
        window->SetToolTip(tooltip->as_wxString());
    }
}

void MockupContent::CreateWizard()
{
    auto form = m_mockupParent->GetSelectedForm();
    m_wizard = new MockupWizard(this, form);
}

Node* MockupContent::GetNode(wxObject* wxobject)
{
    if (auto node = m_obj_node_pair.find(wxobject); node != m_obj_node_pair.end())
        return node->second;
    else
        return nullptr;
}

wxObject* MockupContent::Get_wxObject(Node* node)
{
    if (auto wxobject = m_node_obj_pair.find(node); wxobject != m_node_obj_pair.end())
        return wxobject->second;
    else
        return nullptr;
}

void MockupContent::OnNodeSelected(Node* node)
{
    if (m_wizard && node->GetClassName() == "wxWizardPageSimple")
    {
        auto parent = node->GetParent();
        ASSERT(parent->GetClassName() == "wxWizard");
        m_wizard->SetSelection(parent->GetChildPosition(node));
        return;
    }

    else if (node->GetClassName() == "BookPage")
    {
        auto parent = node->GetParent();
        auto book = wxDynamicCast(Get_wxObject(parent), wxBookCtrl);
        book->SetSelection(parent->GetChildPosition(node));
        m_mockupParent->ClearIgnoreSelection();

        return;
    }

    else if (node->GetClassName() == "wxRibbonPage")
    {
        auto parent = node->GetParent();
        ASSERT(parent->GetClassName() == "wxRibbonBar");

        auto bar = wxStaticCast(Get_wxObject(parent), wxRibbonBar);
        auto page = wxStaticCast(Get_wxObject(node), wxRibbonPage);
        bar->SetActivePage(page);

        return;
    }
    else if (node->GetClassName() == "wxRibbonPanel")
    {
        auto parent = node->GetParent();
        ASSERT(parent->GetClassName() == "wxRibbonPage");

        auto bar = wxStaticCast(Get_wxObject(parent->GetParent()), wxRibbonBar);
        auto page = wxStaticCast(Get_wxObject(parent), wxRibbonPage);
        bar->SetActivePage(page);

        return;
    }
    else if (node->GetClassName() == "wxRibbonButtonBar" || node->GetClassName() == "wxRibbonToolBar")
    {
        auto parent = node->GetParent()->GetParent();
        ASSERT(parent->GetClassName() == "wxRibbonPage");

        auto bar = wxStaticCast(Get_wxObject(parent->GetParent()), wxRibbonBar);
        auto page = wxStaticCast(Get_wxObject(parent), wxRibbonPage);
        bar->SetActivePage(page);

        return;
    }
    else if (node->GetClassName() == "ribbonButton" || node->GetClassName() == "ribbonTool")
    {
        auto parent = node->GetParent()->GetParent()->GetParent();
        ASSERT(parent->GetClassName() == "wxRibbonPage");

        auto bar = wxStaticCast(Get_wxObject(parent->GetParent()), wxRibbonBar);
        auto page = wxStaticCast(Get_wxObject(parent), wxRibbonPage);
        bar->SetActivePage(page);

        return;
    }

    // If the project is selected, then there will be no component.
    auto component = node->GetGenerator();
    if (component && component->OnSelected(node))
        Refresh();
}

void MockupContent::SelectNode(wxObject* wxobject)
{
    if (auto result = m_obj_node_pair.find(wxobject); result != m_obj_node_pair.end())
    {
        wxGetFrame().SelectNode(result->second);
    }
}
