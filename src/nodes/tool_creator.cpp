/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for creating new nodes from Ribbon Panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "node.h"

#include "../panels/ribbon_tools.h"  // RibbonPanel -- Displays component tools in a wxRibbonBar
#include "appoptions.h"              // AppOptions -- Application-wide options
#include "mainframe.h"               // MainFrame -- Main window frame
#include "node_creator.h"            // NodeCreator class
#include "node_decl.h"               // NodeDeclaration class
#include "node_prop.h"               // NodeProperty -- NodeProperty class

using namespace GenEnum;

bool Node::CreateToolNode(GenName name)
{
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
            if (GetAppOptions().get_SizersAllBorders())
                prop->set_value("wxALL");
        }

        if (auto prop = node->get_prop_ptr(prop_flags); prop)
        {
            if (GetAppOptions().get_SizersExpand())
                prop->set_value("wxEXPAND");
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
    return true;
}

void Node::PostProcessBook(Node* book_node)
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

void Node::PostProcessPage(Node* page_node)
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

void Node::PostProcessPanel(Node* panel_node)
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
