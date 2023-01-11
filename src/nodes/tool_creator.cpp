/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for creating new nodes from Ribbon Panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "node.h"

#include "../panels/nav_panel.h"     // NavigationPanel -- Navigation Panel
#include "../panels/ribbon_tools.h"  // RibbonPanel -- Displays component tools in a wxRibbonBar
#include "mainframe.h"               // MainFrame -- Main window frame
#include "node_creator.h"            // NodeCreator class
#include "node_decl.h"               // NodeDeclaration class
#include "node_prop.h"               // NodeProperty -- NodeProperty class
#include "preferences.h"             // Preferences -- Stores user preferences
#include "undo_cmds.h"               // InsertNodeAction -- Undoable command classes derived from UndoAction

using namespace GenEnum;

static void PostProcessBook(Node* book_node)
{
    auto page_node = book_node->CreateChildNode(gen_BookPage);
    if (page_node->FixDuplicateName())
        wxGetFrame().FirePropChangeEvent(page_node->get_prop_ptr(prop_var_name));

    if (auto sizer = page_node->CreateChildNode(gen_VerticalBoxSizer); sizer)
    {
        sizer->prop_set_value(prop_var_name, "page_sizer");
        sizer->FixDuplicateName();
        wxGetFrame().FirePropChangeEvent(sizer->get_prop_ptr(prop_var_name));
    }
}

static void PostProcessPage(Node* page_node)
{
    if (page_node->FixDuplicateName())
        wxGetFrame().FirePropChangeEvent(page_node->get_prop_ptr(prop_var_name));

    if (auto sizer = page_node->CreateChildNode(gen_VerticalBoxSizer); sizer)
    {
        sizer->prop_set_value(prop_var_name, "page_sizer");
        sizer->FixDuplicateName();
        wxGetFrame().FirePropChangeEvent(sizer->get_prop_ptr(prop_var_name));
    }
}

static void PostProcessPanel(Node* panel_node)
{
    if (panel_node->FixDuplicateName())
        wxGetFrame().FirePropChangeEvent(panel_node->get_prop_ptr(prop_var_name));

    if (auto sizer = panel_node->CreateChildNode(gen_VerticalBoxSizer); sizer)
    {
        sizer->prop_set_value(prop_var_name, "panel_sizer");
        sizer->FixDuplicateName();
        wxGetFrame().FirePropChangeEvent(sizer->get_prop_ptr(prop_var_name));
    }
}

bool Node::CreateToolNode(GenName name)
{
    if (isGen(gen_Project))
    {
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
        if (!IsFormParent() && !IsForm())
        {
            wxMessageBox("A folder can only be created when a form, another folder or the project is selected.",
                         "Cannot create folder", wxOK | wxICON_ERROR);
            return true;  // indicate that we have full processed creation even though it's just an error message
        }
        auto* parent = IsForm() ? GetParent() : this;
        if (parent->isGen(gen_folder) || parent->isGen(gen_sub_folder))
            name = gen_sub_folder;

        if (auto new_node = g_NodeCreator.CreateNode(name, parent); new_node)
        {
            wxGetFrame().Freeze();
            ttlib::cstr undo_string("Insert new folder");
            auto childPos = IsForm() ? parent->GetChildPosition(this) : 0;
            wxGetFrame().PushUndoAction(
                std::make_shared<InsertNodeAction>(new_node.get(), parent, "Insert new folder", childPos));

            // InsertNodeAction does not fire the creation event since that's usually handled by the caller as
            // needed. We don't want to fire an event because we don't want the Mockup or Code panels to update until
            // we have changed the parent. However we *do* need to let the navigation panel know that a new node has
            // been added.

            wxGetFrame().GetNavigationPanel()->InsertNode(new_node.get());

            if (IsForm())
            {
                wxGetFrame().PushUndoAction(std::make_shared<ChangeParentAction>(this, new_node.get()));
            }
            wxGetFrame().SelectNode(new_node, evt_flags::fire_event | evt_flags::force_selection);
            wxGetFrame().Thaw();
            return true;
        }
    }

    auto new_node = CreateChildNode(name);
    if (!new_node)
        return false;

    if (name == gen_wxDialog || name == gen_PanelForm || name == gen_wxPopupTransientWindow)
    {
        if (auto sizer = new_node->CreateChildNode(gen_VerticalBoxSizer); sizer)
        {
            sizer->prop_set_value(prop_var_name, "parent_sizer");
            sizer->FixDuplicateName();
            wxGetFrame().FirePropChangeEvent(sizer->get_prop_ptr(prop_var_name));
        }
    }
    else if (name == gen_wxNotebook || name == gen_wxSimplebook || name == gen_wxChoicebook || name == gen_wxListbook ||
             name == gen_wxAuiNotebook)
    {
        PostProcessBook(new_node);
    }
    else if (name == gen_BookPage)
    {
        PostProcessPage(new_node);
    }
    else if (name == gen_wxPanel)
    {
        PostProcessPanel(new_node);
    }
    else if (name == gen_wxWizard)
    {
        new_node = new_node->CreateChildNode(gen_wxWizardPageSimple);
        PostProcessPage(new_node);
    }
    else if (name == gen_wxWizardPageSimple)
    {
        PostProcessPage(new_node);
    }
    else if (name == gen_wxMenuBar || name == gen_MenuBar)
    {
        if (auto node_menu = new_node->CreateChildNode(gen_wxMenu); node_menu)
        {
            node_menu->CreateChildNode(gen_wxMenuItem);
        }
        if (name == gen_MenuBar)
        {
            wxGetFrame().GetRibbonPanel()->ActivateBarPage();
        }
    }
    else if (name == gen_PopupMenu)
    {
        new_node->CreateChildNode(gen_wxMenuItem);
        wxGetFrame().GetRibbonPanel()->ActivateBarPage();
    }
    else if (name == gen_wxToolBar || name == gen_ToolBar)
    {
        new_node->CreateChildNode(gen_tool);
    }
    else if (name == gen_wxBoxSizer || name == gen_VerticalBoxSizer || name == gen_wxWrapSizer || name == gen_wxGridSizer ||
             name == gen_wxFlexGridSizer || name == gen_wxGridBagSizer || name == gen_wxStaticBoxSizer ||
             name == gen_StaticCheckboxBoxSizer || name == gen_StaticRadioBtnBoxSizer)
    {
        auto node = new_node->GetParent();
        ASSERT(node);

        if (auto prop = node->get_prop_ptr(prop_borders); prop)
        {
            if (Preferences().is_SizersAllBorders())
                prop->set_value("wxALL");
        }

        if (auto prop = node->get_prop_ptr(prop_flags); prop)
        {
            if (Preferences().is_SizersExpand())
                prop->set_value("wxEXPAND");
        }
    }
    else if (name == gen_wxStaticLine)
    {
        if (auto sizer = new_node->GetParent(); sizer->IsSizer())
        {
            // Set a default width that is large enough to see
            new_node->prop_set_value(prop_size, "20,-1d");
            wxGetFrame().FirePropChangeEvent(new_node->get_prop_ptr(prop_size));
        }
    }
    else if (name == gen_wxStdDialogButtonSizer || name == gen_wxStaticLine)
    {
        if (auto prop = new_node->get_prop_ptr(prop_flags); prop)
        {
            prop->set_value("wxEXPAND");
            wxGetFrame().FirePropChangeEvent(prop);
        }
    }
    else if (name == gen_wxContextMenuEvent)
    {
        auto event = new_node->GetParent()->GetEvent("wxEVT_CONTEXT_MENU");
        if (event)
        {
            event->set_value(new_node->prop_as_string(prop_handler_name));
        }

        // Create an initial menu item
        new_node->CreateChildNode(gen_wxMenuItem);
    }
    else if (name == gen_wxHtmlWindow || name == gen_wxStyledTextCtrl || name == gen_wxRichTextCtrl ||
             name == gen_wxGenericDirCtrl)
    {
        new_node->prop_set_value(prop_flags, "wxEXPAND");
        new_node->prop_set_value(prop_proportion, 1);
    }

    return true;
}
