/////////////////////////////////////////////////////////////////////////////
// Purpose:   Mockup of a form's contents
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// Note that for most forms, this is the top level wxPanel and we create the contents of the form as
// if we were the form. The notable exception is a MockupWizard -- in this case we create a
// MockupWizard child which itself is a wxPanel that substitutes for the wxWizard form.

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

auto MockupContent::RemoveNodes() -> void
{
    m_obj_node_pair.clear();
    m_node_obj_pair.clear();

    DestroyChildren();
    m_wizard = nullptr;
    SetSizer(nullptr);

    m_parent_sizer = nullptr;

    if (m_variant != wxWINDOW_VARIANT_NORMAL)
        ResetWindowVariant();
}

// This is called by MockupParent in order to create all child components
auto MockupContent::CreateAllGenerators() -> void
{
    wxWindowUpdateLocker lock(this);

    m_parent_sizer = new wxBoxSizer(wxVERTICAL);

    ASSERT(m_mockupParent->getSelectedForm());
    auto form = m_mockupParent->getSelectedForm();
    if (m_variant != wxWINDOW_VARIANT_NORMAL)
        ResetWindowVariant();

    if (form->is_Gen(gen_wxWizard))
    {
        m_wizard = new MockupWizard(this, form);
        for (const auto& child: form->get_ChildNodePtrs())
        {
            CreateChildren(child.get(), m_wizard, m_wizard);
        }

        m_wizard->AllChildrenAdded();
        m_wizard->SetSelection(0);

        m_parent_sizer->Add(m_wizard, wxSizerFlags(1).Expand());
    }
    else if (form->is_Gen(gen_Images))
    {
        ASSERT_MSG(form->get_Generator(),
                   wxString() << "Missing component for " << wxString(form->get_DeclName()));
        auto generator = form->get_Generator();
        if (!generator)
            return;

        auto sizer = generator->CreateMockup(form, this);
        m_parent_sizer->Add(wxStaticCast(sizer, wxBoxSizer), wxSizerFlags(1).Expand());
    }
    else if (form->is_Gen(gen_Data))
    {
        ASSERT_MSG(form->get_Generator(),
                   wxString() << "Missing component for " << wxString(form->get_DeclName()));
        auto generator = form->get_Generator();
        if (!generator)
            return;

        auto sizer = generator->CreateMockup(form, this);
        // sizer type needs to match DataGenerator::CreateMockup in ../generate/gen_data_list.cpp
        m_parent_sizer->Add(wxStaticCast(sizer, wxFlexGridSizer), wxSizerFlags(1).Expand());
    }
    else
    {
        if (form->HasProp(prop_variant) && form->as_string(prop_variant) != "normal")
        {
            if (m_variant != wxWINDOW_VARIANT_NORMAL)
                ResetWindowVariant();

            if (form->is_PropValue(prop_variant, "small"))
            {
                MockupSetWindowVariant(wxWINDOW_VARIANT_SMALL);
            }
            else if (form->is_PropValue(prop_variant, "mini"))
            {
                MockupSetWindowVariant(wxWINDOW_VARIANT_MINI);
            }
            else if (form->is_PropValue(prop_variant, "large"))
            {
                MockupSetWindowVariant(wxWINDOW_VARIANT_LARGE);
            }
        }

        if (form->is_Gen(gen_MenuBar) || form->is_Gen(gen_RibbonBar) || form->is_Gen(gen_ToolBar) ||
            form->is_Gen(gen_AuiToolBar) || form->is_Gen(gen_PopupMenu) ||
            form->is_Gen(gen_wxPropertySheetDialog))
        {
            // In this case, the form itself is created as a child
            CreateChildren(form, this, this, m_parent_sizer);
        }
        else if (form->is_Type(type_frame_form))
        {
            // In a frame window, a menu, toolbar and statusbar can appear anywhere, but they need
            // to be created in our Mockup window in a specific order to match what the real window
            // will look like.

            size_t pos_menu { wxue::npos };
            size_t pos_toolbar { wxue::npos };
            size_t pos_statusbar { wxue::npos };
            for (size_t i = 0; i < form->get_ChildCount(); i++)
            {
                if (form->get_Child(i)->is_Gen(gen_wxMenuBar))
                    pos_menu = i;
                else if (form->get_Child(i)->is_Gen(gen_wxToolBar))
                    pos_toolbar = i;
                else if (form->get_Child(i)->is_Gen(gen_wxStatusBar))
                    pos_statusbar = i;
            }

            // First create the menu and toolbar if they exist

            if (wxue::is_found(pos_menu))
                CreateChildren(form->get_Child(pos_menu), this, this, m_parent_sizer);
            if (wxue::is_found(pos_toolbar))
                CreateChildren(form->get_Child(pos_toolbar), this, this, m_parent_sizer);

            for (size_t i = 0; i < form->get_ChildCount(); i++)
            {
                if (i != pos_menu && i != pos_toolbar && i != pos_statusbar)
                    CreateChildren(form->get_Child(i), this, this, m_parent_sizer);
            }

            if (wxue::is_found(pos_statusbar))
                CreateChildren(form->get_Child(pos_statusbar), this, this, m_parent_sizer);
        }

        else
        {
            for (const auto& child: form->get_ChildNodePtrs())
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

auto MockupContent::CreateChildren(Node* node, wxWindow* parent, wxObject* parent_object,
                                   wxBoxSizer* parent_sizer) -> void
{
    ASSERT_MSG(node->get_Generator(),
               wxString() << "Missing component for " << wxString(node->get_DeclName()));
    auto generator = node->get_Generator();
    if (!generator)
        return;

    auto created_object = generator->CreateMockup(node, parent);
    if (!created_object)
    {
        if (node->is_Spacer() && parent_object)
        {
            if (node->get_Parent()->is_Gen(gen_wxGridBagSizer))
            {
                auto flags = node->getSizerFlags();
                wxStaticCast(parent_object, wxGridBagSizer)
                    ->Add(node->as_int(prop_width), node->as_int(prop_height),
                          wxGBPosition(node->as_int(prop_row), node->as_int(prop_column)),
                          wxGBSpan(node->as_int(prop_rowspan), node->as_int(prop_colspan)),
                          flags.GetFlags(),
                          parent->FromDIP(wxSize(node->as_int(prop_border_size), -1)).x);
            }
            else
            {
                if (node->as_int(prop_proportion) != 0)
                {
                    wxStaticCast(parent_object, wxSizer)
                        ->AddStretchSpacer(node->as_int(prop_proportion));
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

    if (node->is_Gen(gen_wxMenuBar) || node->is_Gen(gen_MenuBar))
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
    else if (node->is_Sizer() || node->is_Gen(gen_wxStdDialogButtonSizer) ||
             node->is_Gen(gen_TextSizer))
    {
        if (node->is_StaticBoxSizer())
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
        if (!node->is_Type(type_images) && !node->is_Type(type_data_list))
        {
            SetWindowProperties(node, created_window, m_mockupParent);
        }
    }

    // Store the wxObject/Node pair both ways.
    m_obj_node_pair[created_object] = node;
    m_node_obj_pair[node] = created_object;

    if (node->is_Type(type_images) || node->is_Type(type_data_list))
    {
        if (parent_sizer)
        {
            parent_sizer->Add(created_window, wxSizerFlags().Expand());
        }
        return;
    }

    wxWindow* new_wxparent = (created_window ? created_window : parent);

    if (node->is_Gen(gen_wxCollapsiblePane))
    {
        auto collpane = wxStaticCast(created_object, wxCollapsiblePane);
        new_wxparent = collpane->GetPane();
    }

    if (node->is_Gen(gen_PageCtrl) && node->get_ChildCount())
    {
        auto page_child = node->get_Child(0);
        if (page_child)
        {
            for (const auto& child: page_child->get_ChildNodePtrs())
            {
                CreateChildren(child.get(), parent, parent_object);
            }
        }
    }
    else
    {
        for (const auto& child: node->get_ChildNodePtrs())
        {
            CreateChildren(child.get(), new_wxparent, created_object);
        }
    }

    if (node->get_Parent()->is_Type(type_wizard))
    {
        m_wizard->AddPage(wxStaticCast(created_window, MockupWizardPage));
        return;
    }

    if (parent && (created_window || created_sizer))
    {
        auto obj_parent = getNode(parent_object);
        if (obj_parent && obj_parent->is_Gen(gen_wxChoicebook) && node->is_Type(type_widget))
        {
            wxStaticCast(parent_object, wxChoicebook)
                ->GetControlSizer()
                ->Add(created_window, wxSizerFlags().Expand().Border(wxALL));
        }
        else if (obj_parent && obj_parent->is_Sizer())
        {
            auto child_obj = getNode(created_object);
            auto sizer_flags = child_obj->getSizerFlags();
            int border_size = child_obj->as_int(prop_border_size);
            if (child_obj->as_bool(prop_scale_border_size) && border_size != 0 &&
                border_size != 5 && border_size != 10 && border_size != 15)
            {
                border_size = FromDIP(wxSize(border_size, -1)).x;
            }
            if (obj_parent->is_Gen(gen_wxGridBagSizer))
            {
                auto sizer = wxStaticCast(parent_object, wxGridBagSizer);
                wxGBPosition position(child_obj->as_int(prop_row), child_obj->as_int(prop_column));
                wxGBSpan span(child_obj->as_int(prop_rowspan), child_obj->as_int(prop_colspan));

                if (created_window)
                    sizer->Add(created_window, position, span, sizer_flags.GetFlags(),
                               sizer_flags.GetBorderInPixels());
                else
                    sizer->Add(created_sizer, position, span, sizer_flags.GetFlags(), border_size);
            }
            else
            {
                auto sizer = wxStaticCast(parent_object, wxSizer);
                if (created_window && !child_obj->is_StaticBoxSizer())
                {
                    sizer->Add(created_window, sizer_flags.GetProportion(), sizer_flags.GetFlags(),
                               border_size);
                }
                else
                {
                    sizer->Add(created_sizer, sizer_flags.GetProportion(), sizer_flags.GetFlags(),
                               border_size);
                }
            }
        }
    }
    generator->AfterCreation(created_object, parent, node, false);

    if (parent_sizer)
    {
        if (created_window && !node->is_StaticBoxSizer())
            parent_sizer->Add(created_window, wxSizerFlags().Expand());
        else if (created_sizer)
            parent_sizer->Add(created_sizer, wxSizerFlags(1).Expand());
    }

    else if ((created_sizer && wxDynamicCast(parent_object, wxWindow)) ||
             (!parent_object && created_sizer))
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
        ASSERT_MSG(!node->as_string(prop_minimum_size).contains("d", wxue::CASE::either),
                   "Minimum size should not contain 'd' for dialog units");
        if (node->as_string(prop_minimum_size).contains("d", wxue::CASE::either))
            window->SetMinSize(convert_win->ConvertDialogToPixels(minsize));
        else
            window->SetMinSize(convert_win->FromDIP(minsize));
    }

    if (auto maxsize = node->as_wxSize(prop_maximum_size); maxsize != wxDefaultSize)
    {
        ASSERT_MSG(!node->as_string(prop_maximum_size).contains("d", wxue::CASE::either),
                   "Maximum size should not contain 'd' for dialog units");
        if (node->as_string(prop_maximum_size).contains("d", wxue::CASE::either))
        {
            window->SetMaxSize(convert_win->ConvertDialogToPixels(maxsize));
        }
        else
        {
            window->SetMaxSize(convert_win->FromDIP(maxsize));
        }
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

    if (node->HasValue(prop_font))
    {
        window->SetFont(node->as_wxFont(prop_font));
    }

    if (node->HasValue(prop_foreground_colour))
    {
        window->SetForegroundColour(node->as_wxColour(prop_foreground_colour));
    }

    if (node->HasValue(prop_background_colour))
    {
        window->SetBackgroundColour(node->as_wxColour(prop_background_colour));
    }

    if (auto extra_style = node->as_int(prop_window_extra_style); extra_style > 0)
    {
        window->SetExtraStyle(extra_style);
    }

    if (node->is_PropValue(prop_disabled, true))
    {
        window->Disable();
    }

    if (node->is_PropValue(prop_hidden, true) && !wxGetFrame().getMockup()->IsShowingHidden())
    {
        window->Show(false);
    }

    if (auto& tooltip = node->as_string(prop_tooltip); tooltip.size())
    {
        window->SetToolTip(tooltip.wx());
    }
}

[[nodiscard]] auto MockupContent::getNode(wxObject* wxobject) -> Node*
{
    if (auto node = m_obj_node_pair.find(wxobject); node != m_obj_node_pair.end())
    {
        return node->second;
    }
    return nullptr;
}

[[nodiscard]] auto MockupContent::Get_wxObject(Node* node) -> wxObject*
{
    if (auto wxobject = m_node_obj_pair.find(node); wxobject != m_node_obj_pair.end())
    {
        return wxobject->second;
    }
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

auto MockupContent::OnNodeSelected(Node* node) -> void
{
    if (!node || node->is_Form())
        return;

    if (node->is_Type(type_embed_image))
    {
        m_mockupParent->CreateContent();
        return;
    }

    bool HavePageNode = false;
    for (;;)
    {
        for (auto& iter: lst_select_nodes)
        {
            if (node->is_Gen(iter))
            {
                HavePageNode = true;
                break;
            }
        }

        if (HavePageNode)
            break;

        node = node->get_Parent();
        if (!node || node->is_Form())
            return;
    }

    if (m_wizard && node->is_Gen(gen_wxWizardPageSimple))
    {
        ASSERT(node->get_Parent());
        if (auto parent = node->get_Parent(); parent)
        {
            ASSERT(parent->is_Gen(gen_wxWizard));
            m_wizard->SetSelection(parent->get_ChildPosition(node));
        }
        return;
    }

    if (node->is_Gen(gen_BookPage) || node->is_Gen(gen_PageCtrl))
    {
        ASSERT(node->get_Parent());
        auto parent = node->get_Parent();
        if (!parent)
        {
            return;
        }

        size_t sel_pos = 0;

        if (parent->is_Gen(gen_BookPage))
        {
            parent = parent->get_Parent();
            if (parent->is_Gen(gen_wxTreebook))
            {
                auto tree_book = wxDynamicCast(Get_wxObject(parent), wxBookCtrlBase);
                if (tree_book)
                {
                    auto this_book = m_node_obj_pair[node];
                    if (this_book)
                    {
                        if (auto index = tree_book->FindPage(wxStaticCast(this_book, wxWindow));
                            index >= 0)
                        {
                            tree_book->SetSelection(index);
                            m_mockupParent->ClearIgnoreSelection();
                            return;
                        }
                    }
                }
            }
        }

        for (size_t idx_child = 0; idx_child < parent->get_ChildCount(); ++idx_child)
        {
            auto child = parent->get_ChildNodePtrs()[idx_child].get();
            if (child == node)
            {
                if (child->get_GenType() == type_page && !child->get_ChildCount())
                {
                    // When a PageCtrl is first created, it won't have any children and cannot be
                    // selected
                    m_mockupParent->ClearIgnoreSelection();
                    return;
                }
                break;
            }
            else if (parent->is_Gen(gen_wxTreebook))
            {
                if (child->is_Gen(gen_BookPage))
                {
                    bool is_node_found { false };
                    for (const auto& grand_child: child->get_ChildNodePtrs())
                    {
                        if (grand_child.get() == node)
                        {
                            is_node_found = true;
                            break;
                        }
                        if (grand_child->is_Gen(gen_BookPage))
                            ++sel_pos;
                    }
                    if (is_node_found)
                        break;
                }
            }

            else if (child->get_GenType() == type_widget)
                continue;
            else if (child->get_GenType() == type_page && !child->get_ChildCount())
            {
                // PageCtrl is an abstract class -- until it has a child, the parent book cannot
                // select it as a page. If this is the last page, then we must back up the selection
                // index and break out of the loop.
                if (idx_child + 1 >= parent->get_ChildCount())
                {
                    if (sel_pos > 0)
                        --sel_pos;
                    break;
                }
                continue;
            }
            ++sel_pos;
        }

        if (parent->is_Gen(gen_wxAuiNotebook))
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
    else if (node->is_Gen(gen_propGridPage))
    {
        auto parent = node->get_Parent();
        if (!parent)
            return;

        if (parent->is_Gen(gen_wxPropertyGridManager))
        {
            for (size_t idx_child = 0; idx_child < parent->get_ChildCount(); ++idx_child)
            {
                auto* child = parent->get_ChildNodePtrs()[idx_child].get();
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
    else if (node->is_Gen(gen_wxRibbonPage))
    {
        ASSERT(node->get_Parent());
        if (auto parent = node->get_Parent(); parent)
        {
            ASSERT(parent->is_Gen(gen_wxRibbonBar) || parent->is_Gen(gen_RibbonBar));

            auto bar = wxStaticCast(Get_wxObject(parent), wxRibbonBar);
            auto page = wxStaticCast(Get_wxObject(node), wxRibbonPage);
            bar->SetActivePage(page);
        }
        return;
    }
    else if (node->is_Gen(gen_wxRibbonPanel))
    {
        ASSERT(node->get_Parent());
        if (auto parent = node->get_Parent(); parent)
        {
            ASSERT(parent->is_Gen(gen_wxRibbonPage));

            auto bar = wxStaticCast(Get_wxObject(parent->get_Parent()), wxRibbonBar);
            auto page = wxStaticCast(Get_wxObject(parent), wxRibbonPage);
            bar->SetActivePage(page);
        }
        return;
    }
    else if (node->is_Gen(gen_wxRibbonButtonBar) || node->is_Gen(gen_wxRibbonToolBar))
    {
        ASSERT(node->get_Parent());
        ASSERT(node->get_Parent()->get_Parent());
        if (auto parent = node->get_Parent()->get_Parent(); parent)
        {
            ASSERT(parent->is_Gen(gen_wxRibbonPage));
            auto bar = wxStaticCast(Get_wxObject(parent->get_Parent()), wxRibbonBar);
            auto page = wxStaticCast(Get_wxObject(parent), wxRibbonPage);
            bar->SetActivePage(page);
        }
        return;
    }
    else if (node->is_Gen(gen_ribbonButton) || node->is_Gen(gen_ribbonTool))
    {
        ASSERT(node->get_Parent());
        ASSERT(node->get_Parent()->get_Parent());
        ASSERT(node->get_Parent()->get_Parent()->get_Parent());
        if (auto parent = node->get_Parent()->get_Parent()->get_Parent(); parent)
        {
            ASSERT(parent->is_Gen(gen_wxRibbonPage));

            auto bar = wxStaticCast(Get_wxObject(parent->get_Parent()), wxRibbonBar);
            auto page = wxStaticCast(Get_wxObject(parent), wxRibbonPage);
            bar->SetActivePage(page);
        }
        return;
    }
}

auto MockupContent::SelectNode(wxObject* wxobject) -> void
{
    if (auto result = m_obj_node_pair.find(wxobject); result != m_obj_node_pair.end())
    {
        wxGetFrame().SelectNode(result->second);
    }
}

auto MockupContent::ResetWindowVariant() -> void
{
    // Essentially this is the opposite of wxWindowBase::DoSetWindowVariant found in
    // wxWidgets/src/common/wincmn.cpp -- this just multiplies rather than divides if smaller, or
    // divides rather than multiplies if larger.

    if (m_variant == wxWINDOW_VARIANT_NORMAL)
        return;

    wxFont font = GetFont();
    double size = font.GetFractionalPointSize();

    switch (m_variant)
    {
        case wxWINDOW_VARIANT_SMALL:
            size *= 1.2;
            break;

        case wxWINDOW_VARIANT_MINI:
            size *= 1.2 * 1.2;
            break;

        case wxWINDOW_VARIANT_LARGE:
            size /= 1.2;
            break;

        default:
            FAIL_MSG("unexpected window variant");
            break;
    }

    font.SetFractionalPointSize(size);
    SetFont(font);
    m_variant = wxWINDOW_VARIANT_NORMAL;
}

auto MockupContent::MockupSetWindowVariant(wxWindowVariant variant) -> void
{
    // adjust the font height to correspond to our new variant (notice that
    // we're only called if something really changed)
    wxFont font = GetFont();
    double size = font.GetFractionalPointSize();
    switch (variant)
    {
        case wxWINDOW_VARIANT_NORMAL:
            break;

        case wxWINDOW_VARIANT_SMALL:
            size /= 1.2;
            break;

        case wxWINDOW_VARIANT_MINI:
            size /= 1.2 * 1.2;
            break;

        case wxWINDOW_VARIANT_LARGE:
            size *= 1.2;
            break;

        default:
            wxFAIL_MSG(wxT("unexpected window variant"));
            break;
    }

    font.SetFractionalPointSize(size);
    SetFont(font);
    m_variant = variant;
}
