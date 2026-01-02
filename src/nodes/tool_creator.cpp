/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for creating new nodes from Ribbon Panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <format>

#include "gen_enums.h"
#include "node.h"

#include "../panels/nav_panel.h"     // NavigationPanel -- Navigation Panel
#include "../panels/ribbon_tools.h"  // RibbonPanel -- Displays component tools in a wxRibbonBar
#include "data_handler.h"            // DataHandler class
#include "mainframe.h"               // MainFrame -- Main window frame
#include "node_creator.h"            // NodeCreator class
#include "node_prop.h"               // NodeProperty -- NodeProperty class
#include "preferences.h"             // Preferences -- Stores user preferences
#include "project_handler.h"         // ProjectHandler class
#include "undo_cmds.h"               //  Undoable command classes derived from UndoAction

using namespace GenEnum;

namespace
{
    auto PostProcessBook(Node* book_node) -> void
    {
        auto page_node = book_node->CreateChildNode(gen_BookPage).first;
        if (page_node->FixDuplicateName())
        {
            wxGetFrame().FirePropChangeEvent(page_node->get_PropPtr(prop_var_name));
        }

        if (auto sizer = page_node->CreateChildNode(gen_VerticalBoxSizer).first; sizer)
        {
            sizer->set_value(prop_var_name, "page_sizer");
            sizer->FixDuplicateName();
            wxGetFrame().FirePropChangeEvent(sizer->get_PropPtr(prop_var_name));
        }
    }

    auto PostProcessPage(Node* page_node) -> void
    {
        if (page_node->FixDuplicateName())
        {
            wxGetFrame().FirePropChangeEvent(page_node->get_PropPtr(prop_var_name));
        }

        if (auto sizer = page_node->CreateChildNode(gen_VerticalBoxSizer).first; sizer)
        {
            sizer->set_value(prop_var_name, "page_sizer");
            sizer->FixDuplicateName();
            wxGetFrame().FirePropChangeEvent(sizer->get_PropPtr(prop_var_name));
        }
    }

    auto PostProcessPanel(Node* panel_node) -> void
    {
        if (panel_node->FixDuplicateName())
        {
            wxGetFrame().FirePropChangeEvent(panel_node->get_PropPtr(prop_var_name));
        }

        if (auto sizer = panel_node->CreateChildNode(gen_VerticalBoxSizer).first; sizer)
        {
            sizer->set_value(prop_var_name, "panel_sizer");
            sizer->FixDuplicateName();
            wxGetFrame().FirePropChangeEvent(sizer->get_PropPtr(prop_var_name));
        }
    }
}  // anonymous namespace

auto SetUniqueRibbonToolID(Node* node) -> void
{
    auto* bar_parent = node->get_Parent();
    while (bar_parent &&
           (!bar_parent->is_Gen(gen_wxRibbonBar) && !bar_parent->is_Gen(GenEnum::gen_RibbonBar)))
    {
        bar_parent = bar_parent->get_Parent();
    }
    ASSERT(bar_parent);
    if (!bar_parent)
    {
        return;  // should never happen, but don't crash if it does
    }

    std::unordered_set<std::string> name_set;

    auto rlambda = [&](Node* child, auto&& rlambda) -> void
    {
        if (child->is_Gen(gen_ribbonTool) || child->is_Gen(gen_ribbonButton))
        {
            if (child->HasValue(prop_id) && !child->as_string(prop_id).starts_with("wx"))
            {
                name_set.insert(child->as_string(prop_id));
            }
            return;
        }
        for (const auto& iter: child->get_ChildNodePtrs())
        {
            rlambda(iter.get(), rlambda);
        }
    };

    for (const auto& iter: bar_parent->get_ChildNodePtrs())
    {
        rlambda(iter.get(), rlambda);
    }

    std::string new_name("tool1");

    if (auto iter = name_set.find(new_name); iter != name_set.end())
    {
        // Keep adding higher and higher numbers until we get a unique one.
        for (int i = 2; iter != name_set.end(); iter = name_set.find(new_name), ++i)
        {
            new_name = std::format("tool{}", i);
        }
    }

    node->set_value(prop_id, new_name);
}

auto Node::CreateToolNode(GenName name, int pos) -> bool
{
    if (is_Type(type_frame_form))
    {
        if (name == gen_MenuBar)
        {
            const auto has_menubar = std::ranges::any_of(get_ChildNodePtrs(),
                                                         [](const auto& iter)
                                                         {
                                                             return iter->is_Gen(gen_MenuBar);
                                                         });
            if (!has_menubar)
            {
                name = gen_wxMenuBar;
            }
        }
        else if (name == gen_ToolBar)
        {
            const auto has_toolbar = std::ranges::any_of(get_ChildNodePtrs(),
                                                         [](const auto& iter)
                                                         {
                                                             return iter->is_Gen(gen_ToolBar);
                                                         });
            if (!has_toolbar)
            {
                name = gen_wxToolBar;
            }
        }
    }

    if (is_Gen(gen_Project))
    {
        // If needed, change the names to the Form version version the normal child version
        if (name == gen_wxMenuBar)
        {
            name = gen_MenuBar;
        }
        else if (name == gen_wxToolBar)
        {
            name = gen_ToolBar;
        }
    }
    else if (name == gen_folder)
    {
        if (!is_FormParent() && !is_Form())
        {
            wxMessageBox("A folder can only be created when a form, another folder or the project "
                         "is selected.",
                         "Cannot create folder", wxOK | wxICON_ERROR);
            return true;  // indicate that we have full processed creation even though it's just an
                          // error message
        }
        auto* parent = is_Form() ? get_Parent() : this;
        if (parent->is_Gen(gen_folder) || parent->is_Gen(gen_sub_folder))
        {
            name = gen_sub_folder;
        }

        if (auto new_node = NodeCreation.CreateNode(name, parent).first; new_node)
        {
            if (new_node->is_Gen(gen_folder))
            {
                new_node->set_value(prop_code_preference, Project.as_string(prop_code_preference));
            }
            wxGetFrame().Freeze();
            auto childPos = is_Form() ? parent->get_ChildPosition(this) : 0;
            wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(
                new_node.get(), parent, "Insert new folder", childPos));

            // InsertNodeAction does not fire the creation event since that's usually handled by the
            // caller as needed. We don't want to fire an event because we don't want the Mockup or
            // Code panels to update until we have changed the parent. However we *do* need to let
            // the navigation panel know that a new node has been added.

            wxGetFrame().getNavigationPanel()->InsertNode(new_node.get());

            if (is_Form())
            {
                wxGetFrame().PushUndoAction(
                    std::make_shared<ChangeParentAction>(this, new_node.get()));
            }
            wxGetFrame().SelectNode(new_node, evt_flags::fire_event | evt_flags::force_selection);
            wxGetFrame().Thaw();
            return true;
        }
    }

    if (name == gen_Images)
    {
        if (std::ranges::any_of(Project.get_ChildNodePtrs(),
                                [](const auto& iter)
                                {
                                    return iter->is_Gen(gen_Images);
                                }))
        {
            wxMessageBox("Only one Images List is allowed per project.",
                         "Cannot create Images List", wxOK | wxICON_ERROR);
            return true;  // indicate that we have fully processed creation even though it's
                          // just an error message
        }

        auto new_node = NodeCreation.CreateNode(name, Project.get_ProjectNode()).first;
        if (!new_node)
        {
            return false;
        }
        // Note that this will insert itself in front of any Data List
        auto insert_node = std::make_shared<InsertNodeAction>(
            new_node.get(), Project.get_ProjectNode(), "insert Images list", 0);
        insert_node->SetFireCreatedEvent(true);
        wxGetFrame().PushUndoAction(insert_node);
        wxGetFrame().SelectNode(new_node, evt_flags::fire_event | evt_flags::force_selection);
        return true;
    }
    if (name == gen_Data)
    {
        size_t insert_pos = 0;
        for (const auto& iter: Project.get_ChildNodePtrs())
        {
            if (iter->is_Gen(gen_Data))
            {
                wxMessageBox("Only one Data List is allowed per project.",
                             "Cannot create Data List", wxOK | wxICON_ERROR);
                return true;  // indicate that we have fully processed creation even though it's
                              // just an error message
            }
            if (iter->is_Gen(gen_Images))
            {
                // Always insert *after* any Images List
                insert_pos = 1;
            }
        }

        auto new_node = NodeCreation.CreateNode(name, Project.get_ProjectNode()).first;
        if (!new_node)
        {
            return false;
        }
        auto insert_node = std::make_shared<InsertNodeAction>(
            new_node.get(), Project.get_ProjectNode(), "insert Data list", insert_pos);
        insert_node->SetFireCreatedEvent(true);
        wxGetFrame().PushUndoAction(insert_node);
        wxGetFrame().SelectNode(new_node, evt_flags::fire_event | evt_flags::force_selection);
        return true;
    }
    if (name == gen_embedded_image)
    {
        auto* image_node = Project.get_ImagesForm();
        if (!image_node)
        {
            wxMessageBox("An Images List must be created before you can add an embedded image.",
                         "Cannot create embedded image", wxOK | wxICON_ERROR);
            return true;  // indicate that we have fully processed creation even though it's just an
                          // error message
        }
    }
    if (name == gen_data_string)
    {
        auto* data_node = data_list::FindDataList();
        if (!data_node)
        {
            wxMessageBox("A Data List must be created before you can add a data string.",
                         "Cannot create data string", wxOK | wxICON_ERROR);
            return true;  // indicate that we have fully processed creation even though it's just an
                          // error message
        }
        data_node->CreateChildNode(name);
        return true;
    }
    if (name == gen_ribbonButton &&
        (is_Gen(gen_wxRibbonToolBar) || get_Parent()->is_Gen(gen_wxRibbonToolBar)))
    {
        name = gen_ribbonTool;
    }

    if (auto* valid_parent = NodeCreation.is_ValidCreateParent(name, this);
        valid_parent && valid_parent != this)
    {
        if (valid_parent && valid_parent == get_Parent() &&
            !valid_parent->is_Gen(gen_wxGridBagSizer))
        {
            auto new_pos = valid_parent->get_ChildPosition(this) + 1;
            return valid_parent->CreateToolNode(name, static_cast<int>(new_pos));
        }
        return valid_parent->CreateToolNode(name, pos);
    }

    auto result = CreateChildNode(name, true, pos);
    if (result.second == Node::Validity::unsupported_language)
    {
        return true;
    }
    auto new_node = result.first;
    if (!new_node)
    {
        switch (name)
        {
            case gen_wxRibbonPanel:
                if (is_Sizer())
                {
                    // Note that neither the wxRibbonBar or wxRibbonPage are added to the undo
                    // stack
                    if (auto parent = CreateChildNode(gen_wxRibbonBar).first; parent)
                    {
                        auto page = parent->CreateChildNode(gen_wxRibbonPage).first;
                        new_node = page->CreateChildNode(name).first;
                        return new_node != nullptr;
                    }
                }
                return false;

            case gen_wxRibbonPage:
                if (is_Sizer())
                {
                    // Note that neither the wxRibbonBar is not added to the undo stack
                    if (auto parent = CreateChildNode(gen_wxRibbonBar).first; parent)
                    {
                        new_node = parent->CreateChildNode(name).first;
                        return new_node != nullptr;
                    }
                }
                return false;

            case gen_ribbonTool:
                if (get_Parent()->is_Gen(gen_wxRibbonToolBar))
                {
                    new_node = get_Parent()->CreateChildNode(name).first;
                    return new_node != nullptr;
                }
                return false;

            case gen_ribbonButton:
                if (get_Parent()->is_Gen(gen_wxRibbonButtonBar))
                {
                    new_node = get_Parent()->CreateChildNode(name).first;
                    return new_node != nullptr;
                }
                return false;

            default:
                return false;
        }
    }

    // The following switch statement does post-processing of the newly created node.

    switch (name)
    {
        case gen_wxDialog:
        case gen_PanelForm:
        case gen_wxPopupTransientWindow:
            if (auto sizer = new_node->CreateChildNode(gen_VerticalBoxSizer).first; sizer)
            {
                sizer->set_value(prop_var_name, "parent_sizer");
                sizer->FixDuplicateName();
                wxGetFrame().FirePropChangeEvent(sizer->get_PropPtr(prop_var_name));
            }
            break;

        case gen_wxNotebook:
        case gen_wxSimplebook:
        case gen_wxChoicebook:
        case gen_wxListbook:
        case gen_wxAuiNotebook:
            PostProcessBook(new_node.get());
            break;

        case gen_BookPage:
        case gen_wxWizardPageSimple:
            PostProcessPage(new_node.get());
            break;

        case gen_wxPanel:
        case gen_wxScrolledWindow:
            PostProcessPanel(new_node.get());
            break;

        case gen_wxWizard:
            new_node = new_node->CreateChildNode(gen_wxWizardPageSimple).first;
            PostProcessPage(new_node.get());
            break;

        case gen_wxMenuBar:
        case gen_MenuBar:
            if (auto node_menu = new_node->CreateChildNode(gen_wxMenu).first; node_menu)
            {
                node_menu->CreateChildNode(gen_wxMenuItem);
            }
            if (name == gen_MenuBar)
            {
                wxGetFrame().getRibbonPanel()->ActivateBarPage();
            }
            break;

        case gen_PopupMenu:
            new_node->CreateChildNode(gen_wxMenuItem);
            wxGetFrame().getRibbonPanel()->ActivateBarPage();
            break;

        case gen_wxToolBar:
        case gen_ToolBar:
            new_node->CreateChildNode(gen_tool);
            break;

        case gen_wxBoxSizer:
        case gen_VerticalBoxSizer:
        case gen_wxWrapSizer:
        case gen_wxGridSizer:
        case gen_wxFlexGridSizer:
        case gen_wxGridBagSizer:
        case gen_wxStaticBoxSizer:
        case gen_StaticCheckboxBoxSizer:
        case gen_StaticRadioBtnBoxSizer:
            if (auto* node = new_node->get_Parent(); node)
            {
                if (auto* prop = node->get_PropPtr(prop_borders); prop)
                {
                    if (UserPrefs.is_SizersAllBorders())
                    {
                        prop->set_value("wxALL");
                    }
                }

                if (auto* prop = node->get_PropPtr(prop_flags); prop)
                {
                    if (UserPrefs.is_SizersExpand())
                    {
                        prop->set_value("wxEXPAND");
                    }
                }
            }
            break;

        case gen_wxStaticLine:
            if (auto* sizer = new_node->get_Parent(); sizer->is_Sizer())
            {
                // Set a default width that is large enough to see
                new_node->set_value(prop_size, "20,-1d");
                wxGetFrame().FirePropChangeEvent(new_node->get_PropPtr(prop_size));
            }
            if (auto* prop = new_node->get_PropPtr(prop_flags); prop)
            {
                prop->set_value("wxEXPAND");
                wxGetFrame().FirePropChangeEvent(prop);
            }
            break;

        case gen_wxStdDialogButtonSizer:
            if (auto* prop = new_node->get_PropPtr(prop_flags); prop)
            {
                prop->set_value("wxEXPAND");
                wxGetFrame().FirePropChangeEvent(prop);
            }
            break;

        case gen_wxContextMenuEvent:
            if (auto* event = new_node->get_Parent()->get_Event("wxEVT_CONTEXT_MENU"); event)
            {
                event->set_value(new_node->as_string(prop_handler_name));
            }

            // Create an initial menu item
            new_node->CreateChildNode(gen_wxMenuItem);
            break;

        case gen_wxHtmlWindow:
        case gen_wxStyledTextCtrl:
        case gen_wxRichTextCtrl:
        case gen_wxGenericDirCtrl:
            new_node->set_value(prop_flags, "wxEXPAND");
            new_node->set_value(prop_proportion, 1);
            break;

        case gen_ribbonTool:
            SetUniqueRibbonToolID(new_node.get());
            wxGetFrame().FirePropChangeEvent(new_node->get_PropPtr(prop_id));
            break;

        case gen_wxSplitterWindow:
            if (auto* sizer = new_node->get_Parent(); sizer->is_Sizer())
            {
                // Set a default width that is large enough to see
                new_node->set_value(prop_size, "200,-1d");
                wxGetFrame().FirePropChangeEvent(new_node->get_PropPtr(prop_size));
            }
            break;

        case gen_wxContextHelpButton:
            new_node->set_value(prop_id, "wxID_CONTEXT_HELP");
            break;

        default:
            break;
    }

    return true;
}

auto MainFrame::CreateToolNode(GenName name) -> void
{
    if (!m_selected_node)
    {
        wxMessageBox("You need to select something first in order to properly place this widget.");
        return;
    }

    if (name == gen_tool &&
        (m_selected_node->is_Type(type_aui_toolbar) || m_selected_node->is_Type(type_aui_tool)))
    {
        name = gen_auitool;
    }

    if (!m_selected_node->CreateToolNode(name))
    {
        if (m_selected_node->is_Gen(gen_wxSplitterWindow))
        {
            return;  // The user has already been notified of the problem
        }

        switch (name)
        {
            case gen_wxRibbonToolBar:
                wxMessageBox("A wxRibbonToolBar can only be created as a child of a wxRibbonPanel.",
                             "Cannot create wxRibbonToolBar", wxOK | wxICON_ERROR);
                break;

            case gen_wxRibbonGallery:
                wxMessageBox("A wxRibbonGallery can only be created as a child of a wxRibbonPanel.",
                             "Cannot create wxRibbonGallery", wxOK | wxICON_ERROR);
                break;

            case gen_wxRibbonButtonBar:
                wxMessageBox(
                    "A wxRibbonButtonBar can only be created as a child of a wxRibbonPanel.",
                    "Cannot create wxRibbonButtonBar", wxOK | wxICON_ERROR);
                break;

            case gen_wxRibbonPanel:
                wxMessageBox("A wxRibbonPanel can only be created as a child of a wxRibbonPage.",
                             "Cannot create wxRibbonPanel", wxOK | wxICON_ERROR);
                break;

            default:
                wxMessageBox(std::format("Unable to create {} as a child of {}",
                                         map_GenNames.at(name), m_selected_node->get_DeclName()));
        }
    }
}
