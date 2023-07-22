/////////////////////////////////////////////////////////////////////////////
// Purpose:   Mockup of a form's contents
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// Note that for most forms, this is the top level wxPanel and we create the contents of the form as if we were the form. The
// notable exception is a MockupWizard -- in this case we create a MockupWizard child which itself is a wxPanel that
// substitutes for the wxWizard form.

#include <wx/aui/auibook.h>       // wxaui: wx advanced user interface - notebook
#include <wx/bookctrl.h>          // wxBookCtrlBase: common base class for wxList/Tree/Notebook
#include <wx/choicebk.h>          // wxChoicebook: wxChoice and wxNotebook combination
#include <wx/collpane.h>          // wxCollapsiblePane
#include <wx/gbsizer.h>           // wxGridBagSizer:  A sizer that can lay out items in a grid,
#include <wx/propgrid/manager.h>  // wxPropertyGridManager
#include <wx/ribbon/bar.h>        // Top-level component of the ribbon-bar-style interface
#include <wx/sizer.h>             // provide wxSizer class for layout
#include <wx/statbox.h>           // wxStaticBox base header
#include <wx/statline.h>          // wxStaticLine class interface
#include <wx/wupdlock.h>          // wxWindowUpdateLocker prevents window redrawing

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
    m_obj_node_pair.clear();
    m_node_obj_pair.clear();

    DestroyChildren();
    m_wizard = nullptr;
    SetSizer(nullptr);

    m_parent_sizer = nullptr;
}

// This is called by MockupParent in order to create all child components
void MockupContent::CreateAllGenerators()
{
    wxWindowUpdateLocker(this);

    m_parent_sizer = new wxBoxSizer(wxVERTICAL);

    ASSERT(m_mockupParent->GetSelectedForm());
    auto form = m_mockupParent->GetSelectedForm();

    if (form->isGen(gen_wxWizard))
    {
        m_wizard = new MockupWizard(this, form);
        for (const auto& child: form->getChildNodePtrs())
        {
            CreateChildren(child.get(), m_wizard, m_wizard);
        }

        m_wizard->AllChildrenAdded();
        m_wizard->SetSelection(0);

        m_parent_sizer->Add(m_wizard, wxSizerFlags(1).Expand());
    }
    else if (form->isGen(gen_Images))
    {
        ASSERT_MSG(form->getGenerator(), tt_string() << "Missing component for " << form->declName());
        auto generator = form->getGenerator();
        if (!generator)
            return;

        auto sizer = generator->CreateMockup(form, this);
        m_parent_sizer->Add(wxStaticCast(sizer, wxBoxSizer), wxSizerFlags(1).Expand());
    }

    else
    {
        if (form->isGen(gen_MenuBar) || form->isGen(gen_RibbonBar) || form->isGen(gen_ToolBar) ||
            form->isGen(gen_AuiToolBar) || form->isGen(gen_PopupMenu) || form->isGen(gen_Images))
        {
            // In this case, the form itself is created as a child
            CreateChildren(form, this, this, m_parent_sizer);
        }
        else if (form->isGen(gen_wxFrame))
        {
            // In a frame window, a menu, toolbar and statusbar can appear anywhere, but they need to be created in our
            // Mockup window in a specific order to match what the real window will look like.

            size_t pos_menu { tt::npos };
            size_t pos_toolbar { tt::npos };
            size_t pos_statusbar { tt::npos };
            for (size_t i = 0; i < form->getChildCount(); i++)
            {
                if (form->getChild(i)->isGen(gen_wxMenuBar))
                    pos_menu = i;
                else if (form->getChild(i)->isGen(gen_wxToolBar))
                    pos_toolbar = i;
                else if (form->getChild(i)->isGen(gen_wxStatusBar))
                    pos_statusbar = i;
            }

            // First create the menu and toolbar if they exist

            if (tt::is_found(pos_menu))
                CreateChildren(form->getChild(pos_menu), this, this, m_parent_sizer);
            if (tt::is_found(pos_toolbar))
                CreateChildren(form->getChild(pos_toolbar), this, this, m_parent_sizer);

            for (size_t i = 0; i < form->getChildCount(); i++)
            {
                if (i != pos_menu && i != pos_toolbar && i != pos_statusbar)
                    CreateChildren(form->getChild(i), this, this, m_parent_sizer);
            }

            if (tt::is_found(pos_statusbar))
                CreateChildren(form->getChild(pos_statusbar), this, this, m_parent_sizer);
        }

        else
        {
            for (const auto& child: form->getChildNodePtrs())
            {
                CreateChildren(child.get(), this, this, m_parent_sizer);
            }
        }
    }
    SetSizerAndFit(m_parent_sizer);
    if (m_mockupParent->IsMagnified())
    {
        auto cur_size = GetSize();
        cur_size.IncBy(200);
        SetSize(cur_size);
    }
}

void MockupContent::CreateChildren(Node* node, wxWindow* parent, wxObject* parent_object, wxBoxSizer* parent_sizer)
{
    ASSERT_MSG(node->getGenerator(), tt_string() << "Missing component for " << node->declName());
    auto generator = node->getGenerator();
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
                          node->as_int(prop_border_size));
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

    wxWindow* created_window { nullptr };
    wxSizer* created_sizer { nullptr };

    if (node->isGen(gen_wxMenuBar) || node->isGen(gen_MenuBar))
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
        if (!node->isType(type_images))
        {
            SetWindowProperties(node, created_window, m_mockupParent);
        }
    }

    // Store the wxObject/Node pair both ways.
    m_obj_node_pair[created_object] = node;
    m_node_obj_pair[node] = created_object;

    if (node->isType(type_images))
    {
        if (parent_sizer)
        {
            parent_sizer->Add(created_window, wxSizerFlags().Expand());
        }
        return;
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
                CreateChildren(child.get(), parent, parent_object);
            }
        }
    }
    else
    {
        for (const auto& child: node->getChildNodePtrs())
        {
            CreateChildren(child.get(), new_wxparent, created_object);
        }
    }

    if (node->getParent()->isType(type_wizard))
    {
        m_wizard->AddPage(wxStaticCast(created_window, MockupWizardPage));
        return;
    }

    if (parent && (created_window || created_sizer))
    {
        auto obj_parent = GetNode(parent_object);
        if (obj_parent && obj_parent->isGen(gen_wxChoicebook) && node->isType(type_widget))
        {
            wxStaticCast(parent_object, wxChoicebook)
                ->GetControlSizer()
                ->Add(created_window, wxSizerFlags().Expand().Border(wxALL));
        }
        else if (obj_parent && obj_parent->isSizer())
        {
            auto child_obj = GetNode(created_object);
            auto sizer_flags = child_obj->getSizerFlags();
            if (obj_parent->isGen(gen_wxGridBagSizer))
            {
                auto sizer = wxStaticCast(parent_object, wxGridBagSizer);
                wxGBPosition position(child_obj->as_int(prop_row), child_obj->as_int(prop_column));
                wxGBSpan span(child_obj->as_int(prop_rowspan), child_obj->as_int(prop_colspan));

                if (created_window)
                    sizer->Add(created_window, position, span, sizer_flags.GetFlags(), sizer_flags.GetBorderInPixels());
                else
                    sizer->Add(created_sizer, position, span, sizer_flags.GetFlags(), sizer_flags.GetBorderInPixels());
            }
            else
            {
                auto sizer = wxStaticCast(parent_object, wxSizer);
                if (created_window && !child_obj->isStaticBoxSizer())
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
    generator->AfterCreation(created_object, parent, node, false);

    if (parent_sizer)
    {
        if (created_window && !node->isStaticBoxSizer())
            parent_sizer->Add(created_window, wxSizerFlags().Expand());
        else if (created_sizer)
            parent_sizer->Add(created_sizer, wxSizerFlags(1).Expand());
    }

    else if ((created_sizer && wxDynamicCast(parent_object, wxWindow)) || (!parent_object && created_sizer))
    {
        parent->SetSizer(created_sizer);
        parent->Fit();
    }
}

// Note that this is a static function also called by CreateMockupChildren in mockup_preview.cpp
void MockupContent::SetWindowProperties(Node* node, wxWindow* window, wxWindow* convert_win)
{
    if (auto minsize = node->as_wxSize(prop_minimum_size); minsize != wxDefaultSize)
    {
        if (node->as_string(prop_minimum_size).contains("d", tt::CASE::either))
            window->SetMinSize(convert_win->ConvertDialogToPixels(minsize));
        else
            window->SetMinSize(minsize);
    }

    if (auto maxsize = node->as_wxSize(prop_maximum_size); maxsize != wxDefaultSize)
    {
        if (node->as_string(prop_maximum_size).contains("d", tt::CASE::either))
            window->SetMaxSize(convert_win->ConvertDialogToPixels(maxsize));
        else
            window->SetMaxSize(maxsize);
    }

    if (auto& variant = node->as_string(prop_variant); variant.size() && variant != "normal")
    {
        if (variant == "small")
            window->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
        else if (variant == "mini")
            window->SetWindowVariant(wxWINDOW_VARIANT_MINI);
        else
            window->SetWindowVariant(wxWINDOW_VARIANT_LARGE);
    }

    if (node->hasValue(prop_font))
    {
        window->SetFont(node->as_wxFont(prop_font));
    }

    if (auto& fg_colour = node->as_string(prop_foreground_colour); fg_colour.size())
    {
        window->SetForegroundColour(ConvertToColour(fg_colour));
    }

    if (auto& bg_colour = node->as_string(prop_background_colour); bg_colour.size())
    {
        window->SetBackgroundColour(ConvertToColour(bg_colour));
    }

    if (auto extra_style = node->as_int(prop_window_extra_style); extra_style > 0)
    {
        window->SetExtraStyle(extra_style);
    }

    if (node->isPropValue(prop_disabled, true))
    {
        window->Disable();
    }

    if (node->isPropValue(prop_hidden, true) && !wxGetFrame().GetMockup()->IsShowingHidden())
    {
        window->Show(false);
    }

    if (auto& tooltip = node->as_string(prop_tooltip); tooltip.size())
    {
        window->SetToolTip(tooltip.make_wxString());
    }
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

// clang-format off

// List of classes that will have multiple pages -- we want to select the correct page based
// on it's children.
static const GenEnum::GenName lst_select_nodes[] = {

    gen_BookPage,
    gen_PageCtrl,
    gen_propGridPage,
    gen_ribbonButton,
    gen_ribbonTool,
    gen_wxPropertyGridManager,
    gen_wxRibbonButtonBar,
    gen_wxRibbonPage,
    gen_wxRibbonPanel,
    gen_wxRibbonToolBar,
    gen_wxWizardPageSimple,

};
// clang-format on

void MockupContent::OnNodeSelected(Node* node)
{
    if (node->isForm())
        return;

    if (node->isType(type_embed_image))
    {
        m_mockupParent->CreateContent();
        return;
    }

    bool HavePageNode = false;
    for (;;)
    {
        for (auto& iter: lst_select_nodes)
        {
            if (node->isGen(iter))
            {
                HavePageNode = true;
                break;
            }
        }

        if (HavePageNode)
            break;

        node = node->getParent();
        if (!node || node->isForm())
            return;
    }

    if (m_wizard && node->isGen(gen_wxWizardPageSimple))
    {
        ASSERT(node->getParent());
        if (auto parent = node->getParent(); parent)
        {
            ASSERT(parent->isGen(gen_wxWizard));
            m_wizard->SetSelection(parent->getChildPosition(node));
        }
        return;
    }

    else if (node->isGen(gen_BookPage) || node->isGen(gen_PageCtrl))
    {
        ASSERT(node->getParent());
        auto parent = node->getParent();
        if (!parent)
            return;

        size_t sel_pos = 0;

        if (parent->isGen(gen_BookPage))
        {
            parent = parent->getParent();
            if (parent->isGen(gen_wxTreebook))
            {
                auto tree_book = wxDynamicCast(Get_wxObject(parent), wxBookCtrlBase);
                if (tree_book)
                {
                    auto this_book = m_node_obj_pair[node];
                    if (this_book)
                    {
                        if (auto index = tree_book->FindPage(wxStaticCast(this_book, wxWindow)); index >= 0)
                        {
                            tree_book->SetSelection(index);
                            m_mockupParent->ClearIgnoreSelection();
                            return;
                        }
                    }
                }
            }
        }

        for (size_t idx_child = 0; idx_child < parent->getChildCount(); ++idx_child)
        {
            auto child = parent->getChildNodePtrs()[idx_child].get();
            if (child == node)
            {
                if (child->getGenType() == type_page && !child->getChildCount())
                {
                    // When a PageCtrl is first created, it won't have any children and cannot be selected
                    m_mockupParent->ClearIgnoreSelection();
                    return;
                }
                break;
            }
            else if (parent->isGen(gen_wxTreebook))
            {
                if (child->isGen(gen_BookPage))
                {
                    bool is_node_found { false };
                    for (const auto& grand_child: child->getChildNodePtrs())
                    {
                        if (grand_child.get() == node)
                        {
                            is_node_found = true;
                            break;
                        }
                        if (grand_child->isGen(gen_BookPage))
                            ++sel_pos;
                    }
                    if (is_node_found)
                        break;
                }
            }

            else if (child->getGenType() == type_widget)
                continue;
            else if (child->getGenType() == type_page && !child->getChildCount())
            {
                // PageCtrl is an abstract class -- until it has a child, the parent book cannot select it as a
                // page. If this is the last page, then we must back up the selection index and break out of the
                // loop.
                if (idx_child + 1 >= parent->getChildCount())
                {
                    if (sel_pos > 0)
                        --sel_pos;
                    break;
                }
                continue;
            }
            ++sel_pos;
        }

        if (parent->isGen(gen_wxAuiNotebook))
        {
            wxStaticCast(Get_wxObject(parent), wxAuiNotebook)->SetSelection(sel_pos);
        }
        else
        {
            auto book = wxDynamicCast(Get_wxObject(parent), wxBookCtrlBase);
            if (book)
                book->SetSelection(sel_pos);
        }
        m_mockupParent->ClearIgnoreSelection();

        return;
    }
    else if (node->isGen(gen_propGridPage))
    {
        auto parent = node->getParent();
        if (!parent)
            return;

        if (parent->isGen(gen_wxPropertyGridManager))
        {
            for (size_t idx_child = 0; idx_child < parent->getChildCount(); ++idx_child)
            {
                auto* child = parent->getChildNodePtrs()[idx_child].get();
                if (child == node)
                {
                    if (auto pgm = wxStaticCast(Get_wxObject(parent), wxPropertyGridManager); pgm)
                    {
                        pgm->SelectPage((to_int) idx_child);
                    }
                }
            }
        }
    }
    else if (node->isGen(gen_wxRibbonPage))
    {
        ASSERT(node->getParent());
        if (auto parent = node->getParent(); parent)
        {
            ASSERT(parent->isGen(gen_wxRibbonBar) || parent->isGen(gen_RibbonBar));

            auto bar = wxStaticCast(Get_wxObject(parent), wxRibbonBar);
            auto page = wxStaticCast(Get_wxObject(node), wxRibbonPage);
            bar->SetActivePage(page);
        }
        return;
    }
    else if (node->isGen(gen_wxRibbonPanel))
    {
        ASSERT(node->getParent());
        if (auto parent = node->getParent(); parent)
        {
            ASSERT(parent->isGen(gen_wxRibbonPage));

            auto bar = wxStaticCast(Get_wxObject(parent->getParent()), wxRibbonBar);
            auto page = wxStaticCast(Get_wxObject(parent), wxRibbonPage);
            bar->SetActivePage(page);
        }
        return;
    }
    else if (node->isGen(gen_wxRibbonButtonBar) || node->isGen(gen_wxRibbonToolBar))
    {
        ASSERT(node->getParent());
        ASSERT(node->getParent()->getParent());
        if (auto parent = node->getParent()->getParent(); parent)
        {
            ASSERT(parent->isGen(gen_wxRibbonPage));
            auto bar = wxStaticCast(Get_wxObject(parent->getParent()), wxRibbonBar);
            auto page = wxStaticCast(Get_wxObject(parent), wxRibbonPage);
            bar->SetActivePage(page);
        }
        return;
    }
    else if (node->isGen(gen_ribbonButton) || node->isGen(gen_ribbonTool))
    {
        ASSERT(node->getParent());
        ASSERT(node->getParent()->getParent());
        ASSERT(node->getParent()->getParent()->getParent());
        if (auto parent = node->getParent()->getParent()->getParent(); parent)
        {
            ASSERT(parent->isGen(gen_wxRibbonPage));

            auto bar = wxStaticCast(Get_wxObject(parent->getParent()), wxRibbonBar);
            auto page = wxStaticCast(Get_wxObject(parent), wxRibbonPage);
            bar->SetActivePage(page);
        }
        return;
    }
}

void MockupContent::SelectNode(wxObject* wxobject)
{
    if (auto result = m_obj_node_pair.find(wxobject); result != m_obj_node_pair.end())
    {
        wxGetFrame().SelectNode(result->second);
    }
}
