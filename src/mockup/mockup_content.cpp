/////////////////////////////////////////////////////////////////////////////
// Purpose:   Mockup of a form's contents
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// Note that for most forms, this is the top level wxPanel and we create the contents of the form as if we were the form. The
// notable exception is a MockupWizard -- in this case we create a MockupWizard child which itself is a wxPanel that
// substitutes for the wxWizard form.

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/bookctrl.h>     // wxBookCtrlBase: common base class for wxList/Tree/Notebook
#include <wx/choicebk.h>     // wxChoicebook: wxChoice and wxNotebook combination
#include <wx/collpane.h>     // wxCollapsiblePane
#include <wx/gbsizer.h>      // wxGridBagSizer:  A sizer that can lay out items in a grid,
#include <wx/ribbon/bar.h>   // Top-level component of the ribbon-bar-style interface
#include <wx/sizer.h>        // provide wxSizer class for layout
#include <wx/statbox.h>      // wxStaticBox base header
#include <wx/statline.h>     // wxStaticLine class interface
#include <wx/wupdlock.h>     // wxWindowUpdateLocker prevents window redrawing

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
        for (const auto& child: form->GetChildNodePtrs())
        {
            CreateChildren(child.get(), m_wizard, m_wizard);
        }

        m_wizard->AllChildrenAdded();
        m_wizard->SetSelection(0);

        m_parent_sizer->Add(m_wizard, wxSizerFlags(1).Expand());
    }
    else if (form->isGen(gen_Images))
    {
        ASSERT_MSG(form->GetGenerator(), ttlib::cstr() << "Missing component for " << form->DeclName());
        auto comp = form->GetGenerator();
        if (!comp)
            return;

        auto sizer = comp->CreateMockup(form, this);
        m_parent_sizer->Add(wxStaticCast(sizer, wxBoxSizer), wxSizerFlags(1).Expand());
    }

    else
    {
        if (form->isGen(gen_MenuBar) || form->isGen(gen_RibbonBar) || form->isGen(gen_ToolBar) ||
            form->isGen(gen_PopupMenu) || form->isGen(gen_Images))
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
            for (size_t i = 0; i < form->GetChildCount(); i++)
            {
                if (form->GetChild(i)->isGen(gen_wxMenuBar))
                    pos_menu = i;
                else if (form->GetChild(i)->isGen(gen_wxToolBar))
                    pos_toolbar = i;
                else if (form->GetChild(i)->isGen(gen_wxStatusBar))
                    pos_statusbar = i;
            }

            // First create the menu and toolbar if they exist

            if (ttlib::is_found(pos_menu))
                CreateChildren(form->GetChild(pos_menu), this, this, m_parent_sizer);
            if (ttlib::is_found(pos_toolbar))
                CreateChildren(form->GetChild(pos_toolbar), this, this, m_parent_sizer);

            for (size_t i = 0; i < form->GetChildCount(); i++)
            {
                if (i != pos_menu && i != pos_toolbar && i != pos_statusbar)
                    CreateChildren(form->GetChild(i), this, this, m_parent_sizer);
            }

            if (ttlib::is_found(pos_statusbar))
                CreateChildren(form->GetChild(pos_statusbar), this, this, m_parent_sizer);
        }

        else
        {
            for (const auto& child: form->GetChildNodePtrs())
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

void MockupContent::CreateChildren(Node* node, wxWindow* parent, wxObject* parentNode, wxBoxSizer* parent_sizer)
{
    ASSERT_MSG(node->GetGenerator(), ttlib::cstr() << "Missing component for " << node->DeclName());
    auto comp = node->GetGenerator();
    if (!comp)
        return;

    auto created_object = comp->CreateMockup(node, parent);
    if (!created_object)
    {
        if (node->IsSpacer() && parentNode)
        {
            if (node->GetParent()->isGen(gen_wxGridBagSizer))
            {
                auto flags = node->GetSizerFlags();
                wxStaticCast(parentNode, wxGridBagSizer)
                    ->Add(node->prop_as_int(prop_width), node->prop_as_int(prop_height),
                          wxGBPosition(node->prop_as_int(prop_row), node->prop_as_int(prop_column)),
                          wxGBSpan(node->prop_as_int(prop_rowspan), node->prop_as_int(prop_colspan)), flags.GetFlags(),
                          node->prop_as_int(prop_border_size));
            }
            else
            {
                if (node->prop_as_int(prop_proportion) != 0)
                {
                    wxStaticCast(parentNode, wxSizer)->AddStretchSpacer(node->prop_as_int(prop_proportion));
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
                    wxStaticCast(parentNode, wxSizer)->Add(width, height);
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
            SetWindowProperties(node, created_window, m_mockupParent);
        }
    }

    // Store the wxObject/Node pair both ways.
    m_obj_node_pair[created_object] = node;
    m_node_obj_pair[node] = created_object;

    if (node->isType(type_images))
    {
        parent_sizer->Add(created_window, wxSizerFlags().Expand());
        return;
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
                CreateChildren(child.get(), parent, parentNode);
            }
        }
    }
    else
    {
        for (const auto& child: node->GetChildNodePtrs())
        {
            CreateChildren(child.get(), new_wxparent, created_object);
        }
    }

    if (node->GetParent()->isType(type_wizard))
    {
        m_wizard->AddPage(wxStaticCast(created_window, MockupWizardPage));
        return;
    }

    if (parent && (created_window || created_sizer))
    {
        auto obj_parent = GetNode(parentNode);
        if (obj_parent && obj_parent->isGen(gen_wxChoicebook) && node->isType(type_widget))
        {
            wxStaticCast(parentNode, wxChoicebook)
                ->GetControlSizer()
                ->Add(created_window, wxSizerFlags().Expand().Border(wxALL));
        }
        else if (obj_parent && obj_parent->IsSizer())
        {
            auto child_obj = GetNode(created_object);
            auto sizer_flags = child_obj->GetSizerFlags();
            if (obj_parent->isGen(gen_wxGridBagSizer))
            {
                auto sizer = wxStaticCast(parentNode, wxGridBagSizer);
                wxGBPosition position(child_obj->prop_as_int(prop_row), child_obj->prop_as_int(prop_column));
                wxGBSpan span(child_obj->prop_as_int(prop_rowspan), child_obj->prop_as_int(prop_colspan));

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
        if (created_window && !node->IsStaticBoxSizer())
            parent_sizer->Add(created_window, wxSizerFlags().Expand());
        else if (created_sizer)
            parent_sizer->Add(created_sizer, wxSizerFlags(1).Expand());
    }

    else if ((created_sizer && wxDynamicCast(parentNode, wxWindow)) || (!parentNode && created_sizer))
    {
        parent->SetSizer(created_sizer);
        parent->Fit();
    }
}

// Note that this is a static function also called by CreateMockupChildren in mockup_preview.cpp
void MockupContent::SetWindowProperties(Node* node, wxWindow* window, wxWindow* convert_win)
{
    bool is_smart_size { false };  // true means prop_size and prop_minimum_size will be ignored

    if (auto size = node->prop_as_wxSize(prop_smart_size); size != wxDefaultSize)
    {
        is_smart_size = true;
        if (size.x > 0)
            size.x = (size.x > window->GetBestSize().x ? size.x : -1);
        if (size.y > 0)
            size.y = (size.y > window->GetBestSize().y ? size.y : -1);

        if (node->prop_as_string(prop_smart_size).contains("d", tt::CASE::either))
            window->SetInitialSize(convert_win->ConvertDialogToPixels(size));
        else
            window->SetInitialSize(size);
    }

    if (!is_smart_size)
    {
        if (auto minsize = node->prop_as_wxSize(prop_minimum_size); minsize != wxDefaultSize)
        {
            if (node->prop_as_string(prop_minimum_size).contains("d", tt::CASE::either))
                window->SetMinSize(convert_win->ConvertDialogToPixels(minsize));
            else
                window->SetMinSize(minsize);
        }
    }

    if (auto maxsize = node->prop_as_wxSize(prop_maximum_size); maxsize != wxDefaultSize)
    {
        if (node->prop_as_string(prop_maximum_size).contains("d", tt::CASE::either))
            window->SetMaxSize(convert_win->ConvertDialogToPixels(maxsize));
        else
            window->SetMaxSize(maxsize);
    }

    if (!node->isPropValue(prop_variant, "normal"))
    {
        if (node->isPropValue(prop_variant, "small"))
            window->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
        else if (node->isPropValue(prop_variant, "mini"))
            window->SetWindowVariant(wxWINDOW_VARIANT_MINI);
        else
            window->SetWindowVariant(wxWINDOW_VARIANT_LARGE);
    }

    if (node->HasValue(prop_font))
    {
        window->SetFont(node->prop_as_font(prop_font));
    }

    if (auto fg_colour = node->get_prop_ptr(prop_foreground_colour); fg_colour && fg_colour->HasValue())
    {
        window->SetForegroundColour(ConvertToColour(fg_colour->as_string()));
    }

    if (auto bg_colour = node->get_prop_ptr(prop_background_colour); bg_colour && bg_colour->HasValue())
    {
        window->SetBackgroundColour(ConvertToColour(bg_colour->as_string()));
    }

    if (auto extra_style = node->get_prop_ptr(prop_window_extra_style); extra_style && extra_style->as_int() != 0)
    {
        window->SetExtraStyle(extra_style->as_int());
    }

    if (node->isPropValue(prop_disabled, true))
    {
        window->Disable();
    }

    if (node->isPropValue(prop_hidden, true) && !wxGetFrame().GetMockup()->IsShowingHidden())
    {
        window->Show(false);
    }

    if (auto tooltip = node->get_prop_ptr(prop_tooltip); tooltip && tooltip->as_string().size())
    {
        window->SetToolTip(tooltip->as_wxString());
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

    gen_wxWizardPageSimple,
    gen_BookPage,
    gen_PageCtrl,
    gen_wxRibbonPage,
    gen_wxRibbonPanel,
    gen_wxRibbonButtonBar,
    gen_wxRibbonToolBar,
    gen_ribbonButton,
    gen_ribbonTool,

};
// clang-format on

void MockupContent::OnNodeSelected(Node* node)
{
    if (node->IsForm())
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

        node = node->GetParent();
        if (!node || node->IsForm())
            return;
    }

    if (m_wizard && node->isGen(gen_wxWizardPageSimple))
    {
        ASSERT(node->GetParent());
        if (auto parent = node->GetParent(); parent)
        {
            ASSERT(parent->isGen(gen_wxWizard));
            m_wizard->SetSelection(parent->GetChildPosition(node));
        }
        return;
    }

    else if (node->isGen(gen_BookPage) || node->isGen(gen_PageCtrl))
    {
        ASSERT(node->GetParent());
        auto parent = node->GetParent();
        if (!parent)
            return;

        size_t sel_pos = 0;

        if (parent->isGen(gen_BookPage))
        {
            parent = parent->GetParent();
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

        for (size_t idx_child = 0; idx_child < parent->GetChildCount(); ++idx_child)
        {
            auto child = parent->GetChildNodePtrs()[idx_child].get();
            if (child == node)
            {
                if (child->gen_type() == type_page && !child->GetChildCount())
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
                    for (const auto& grand_child: child->GetChildNodePtrs())
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

            else if (child->gen_type() == type_widget)
                continue;
            else if (child->gen_type() == type_page && !child->GetChildCount())
            {
                // PageCtrl is an abstract class -- until it has a child, the parent book cannot select it as a
                // page. If this is the last page, then we must back up the selection index and break out of the
                // loop.
                if (idx_child + 1 >= parent->GetChildCount())
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

    else if (node->isGen(gen_wxRibbonPage))
    {
        ASSERT(node->GetParent());
        if (auto parent = node->GetParent(); parent)
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
        ASSERT(node->GetParent());
        if (auto parent = node->GetParent(); parent)
        {
            ASSERT(parent->isGen(gen_wxRibbonPage));

            auto bar = wxStaticCast(Get_wxObject(parent->GetParent()), wxRibbonBar);
            auto page = wxStaticCast(Get_wxObject(parent), wxRibbonPage);
            bar->SetActivePage(page);
        }
        return;
    }
    else if (node->isGen(gen_wxRibbonButtonBar) || node->isGen(gen_wxRibbonToolBar))
    {
        ASSERT(node->GetParent());
        ASSERT(node->GetParent()->GetParent());
        if (auto parent = node->GetParent()->GetParent(); parent)
        {
            ASSERT(parent->isGen(gen_wxRibbonPage));
            auto bar = wxStaticCast(Get_wxObject(parent->GetParent()), wxRibbonBar);
            auto page = wxStaticCast(Get_wxObject(parent), wxRibbonPage);
            bar->SetActivePage(page);
        }
        return;
    }
    else if (node->isGen(gen_ribbonButton) || node->isGen(gen_ribbonTool))
    {
        ASSERT(node->GetParent());
        ASSERT(node->GetParent()->GetParent());
        ASSERT(node->GetParent()->GetParent()->GetParent());
        if (auto parent = node->GetParent()->GetParent()->GetParent(); parent)
        {
            ASSERT(parent->isGen(gen_wxRibbonPage));

            auto bar = wxStaticCast(Get_wxObject(parent->GetParent()), wxRibbonBar);
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
