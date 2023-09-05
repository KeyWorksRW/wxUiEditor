/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for creating new nodes from Ribbon Panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "node.h"

#include "../panels/nav_panel.h"     // NavigationPanel -- Navigation Panel
#include "../panels/ribbon_tools.h"  // RibbonPanel -- Displays component tools in a wxRibbonBar
#include "images_list.h"             // ImagesGenerator -- Images List Embedded images generator
#include "mainframe.h"               // MainFrame -- Main window frame
#include "node_creator.h"            // NodeCreator class
#include "node_decl.h"               // NodeDeclaration class
#include "node_prop.h"               // NodeProperty -- NodeProperty class
#include "preferences.h"             // Preferences -- Stores user preferences
#include "project_handler.h"         // ProjectHandler class
#include "undo_cmds.h"               // InsertNodeAction -- Undoable command classes derived from UndoAction

using namespace GenEnum;

static void PostProcessBook(Node* book_node)
{
    auto page_node = book_node->createChildNode(gen_BookPage);
    if (page_node->fixDuplicateName())
        wxGetFrame().FirePropChangeEvent(page_node->getPropPtr(prop_var_name));

    if (auto sizer = page_node->createChildNode(gen_VerticalBoxSizer); sizer)
    {
        sizer->set_value(prop_var_name, "page_sizer");
        sizer->fixDuplicateName();
        wxGetFrame().FirePropChangeEvent(sizer->getPropPtr(prop_var_name));
    }
}

static void PostProcessPage(Node* page_node)
{
    if (page_node->fixDuplicateName())
        wxGetFrame().FirePropChangeEvent(page_node->getPropPtr(prop_var_name));

    if (auto sizer = page_node->createChildNode(gen_VerticalBoxSizer); sizer)
    {
        sizer->set_value(prop_var_name, "page_sizer");
        sizer->fixDuplicateName();
        wxGetFrame().FirePropChangeEvent(sizer->getPropPtr(prop_var_name));
    }
}

static void PostProcessPanel(Node* panel_node)
{
    if (panel_node->fixDuplicateName())
        wxGetFrame().FirePropChangeEvent(panel_node->getPropPtr(prop_var_name));

    if (auto sizer = panel_node->createChildNode(gen_VerticalBoxSizer); sizer)
    {
        sizer->set_value(prop_var_name, "panel_sizer");
        sizer->fixDuplicateName();
        wxGetFrame().FirePropChangeEvent(sizer->getPropPtr(prop_var_name));
    }
}

bool Node::createToolNode(GenName name)
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
        if (!isFormParent() && !isForm())
        {
            wxMessageBox("A folder can only be created when a form, another folder or the project is selected.",
                         "Cannot create folder", wxOK | wxICON_ERROR);
            return true;  // indicate that we have full processed creation even though it's just an error message
        }
        auto* parent = isForm() ? getParent() : this;
        if (parent->isGen(gen_folder) || parent->isGen(gen_sub_folder))
            name = gen_sub_folder;

        if (auto new_node = NodeCreation.createNode(name, parent); new_node)
        {
            if (new_node->isGen(gen_folder))
            {
                new_node->set_value(prop_code_preference, Project.as_string(prop_code_preference));
            }
            wxGetFrame().Freeze();
            tt_string undo_string("Insert new folder");
            auto childPos = isForm() ? parent->getChildPosition(this) : 0;
            wxGetFrame().PushUndoAction(
                std::make_shared<InsertNodeAction>(new_node.get(), parent, "Insert new folder", childPos));

            // InsertNodeAction does not fire the creation event since that's usually handled by the caller as
            // needed. We don't want to fire an event because we don't want the Mockup or Code panels to update until
            // we have changed the parent. However we *do* need to let the navigation panel know that a new node has
            // been added.

            wxGetFrame().getNavigationPanel()->InsertNode(new_node.get());

            if (isForm())
            {
                wxGetFrame().PushUndoAction(std::make_shared<ChangeParentAction>(this, new_node.get()));
            }
            wxGetFrame().SelectNode(new_node, evt_flags::fire_event | evt_flags::force_selection);
            wxGetFrame().Thaw();
            return true;
        }
    }

    if (name == gen_Images)
    {
        for (const auto& iter: Project.getChildNodePtrs())
        {
            if (iter->isGen(gen_Images))
            {
                wxMessageBox("Only one Images List is allowed per project.", "Cannot create Images List",
                             wxOK | wxICON_ERROR);
                return true;  // indicate that we have fully processed creation even though it's just an error message
            }
        }

        auto new_node = NodeCreation.createNode(name, Project.getProjectNode());
        if (!new_node)
            return false;
        auto insert_node =
            std::make_shared<InsertNodeAction>(new_node.get(), Project.getProjectNode(), "insert Images list", 0);
        insert_node->SetFireCreatedEvent(true);
        wxGetFrame().PushUndoAction(insert_node);
        wxGetFrame().SelectNode(new_node, evt_flags::fire_event | evt_flags::force_selection);
        return true;
    }
    else if (name == gen_embedded_image)
    {
        auto* image_node = img_list::FindImageList();
        if (!image_node)
        {
            wxMessageBox("An Images List must be created before you can add an embedded image.",
                         "Cannot create embedded image", wxOK | wxICON_ERROR);
            return true;  // indicate that we have fully processed creation even though it's just an error message
        }
        image_node->createChildNode(name);
        return true;
    }

    auto new_node = createChildNode(name);
    if (!new_node)
    {
        switch (name)
        {
            case gen_wxRibbonPanel:
                if (isSizer())
                {
                    // Note that neither the wxRibbonBar or wxRibbonPage are added to the undo
                    // stack
                    if (auto parent = createChildNode(gen_wxRibbonBar); parent)
                    {
                        auto page = parent->createChildNode(gen_wxRibbonPage);
                        new_node = page->createChildNode(name);
                        return (new_node != nullptr);
                    }
                }
                return false;

            case gen_wxRibbonPage:
                if (isSizer())
                {
                    // Note that neither the wxRibbonBar is not added to the undo stack
                    if (auto parent = createChildNode(gen_wxRibbonBar); parent)
                    {
                        new_node = parent->createChildNode(name);
                        return (new_node != nullptr);
                    }
                }
                return false;

            case gen_ribbonTool:
                if (getParent()->isGen(gen_wxRibbonToolBar))
                {
                    new_node = getParent()->createChildNode(name);
                    return (new_node != nullptr);
                }
                return false;

            case gen_ribbonButton:
                if (getParent()->isGen(gen_wxRibbonButtonBar))
                {
                    new_node = getParent()->createChildNode(name);
                    return (new_node != nullptr);
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
            if (auto sizer = new_node->createChildNode(gen_VerticalBoxSizer); sizer)
            {
                sizer->set_value(prop_var_name, "parent_sizer");
                sizer->fixDuplicateName();
                wxGetFrame().FirePropChangeEvent(sizer->getPropPtr(prop_var_name));
            }
            break;

        case gen_wxNotebook:
        case gen_wxSimplebook:
        case gen_wxChoicebook:
        case gen_wxListbook:
        case gen_wxAuiNotebook:
            PostProcessBook(new_node);
            break;

        case gen_BookPage:
        case gen_wxWizardPageSimple:
            PostProcessPage(new_node);
            break;

        case gen_wxPanel:
        case gen_wxScrolledWindow:
            PostProcessPanel(new_node);
            break;

        case gen_wxWizard:
            new_node = new_node->createChildNode(gen_wxWizardPageSimple);
            PostProcessPage(new_node);
            break;

        case gen_wxMenuBar:
        case gen_MenuBar:
            if (auto node_menu = new_node->createChildNode(gen_wxMenu); node_menu)
            {
                node_menu->createChildNode(gen_wxMenuItem);
            }
            if (name == gen_MenuBar)
            {
                wxGetFrame().getRibbonPanel()->ActivateBarPage();
            }
            break;

        case gen_PopupMenu:
            new_node->createChildNode(gen_wxMenuItem);
            wxGetFrame().getRibbonPanel()->ActivateBarPage();
            break;

        case gen_wxToolBar:
        case gen_ToolBar:
            new_node->createChildNode(gen_tool);
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
            if (auto node = new_node->getParent(); node)
            {
                if (auto prop = node->getPropPtr(prop_borders); prop)
                {
                    if (UserPrefs.is_SizersAllBorders())
                        prop->set_value("wxALL");
                }

                if (auto prop = node->getPropPtr(prop_flags); prop)
                {
                    if (UserPrefs.is_SizersExpand())
                        prop->set_value("wxEXPAND");
                }
            }
            break;

        case gen_wxStaticLine:
            if (auto sizer = new_node->getParent(); sizer->isSizer())
            {
                // Set a default width that is large enough to see
                new_node->set_value(prop_size, "20,-1d");
                wxGetFrame().FirePropChangeEvent(new_node->getPropPtr(prop_size));
            }
            if (auto prop = new_node->getPropPtr(prop_flags); prop)
            {
                prop->set_value("wxEXPAND");
                wxGetFrame().FirePropChangeEvent(prop);
            }
            break;

        case gen_wxStdDialogButtonSizer:
            if (auto prop = new_node->getPropPtr(prop_flags); prop)
            {
                prop->set_value("wxEXPAND");
                wxGetFrame().FirePropChangeEvent(prop);
            }
            break;

        case gen_wxContextMenuEvent:
            if (auto event = new_node->getParent()->getEvent("wxEVT_CONTEXT_MENU"); event)
            {
                event->set_value(new_node->as_string(prop_handler_name));
            }

            // Create an initial menu item
            new_node->createChildNode(gen_wxMenuItem);
            break;

        case gen_wxHtmlWindow:
        case gen_wxStyledTextCtrl:
        case gen_wxRichTextCtrl:
        case gen_wxGenericDirCtrl:
            new_node->set_value(prop_flags, "wxEXPAND");
            new_node->set_value(prop_proportion, 1);
            break;

        default:
            break;
    }

    return true;
}

void MainFrame::createToolNode(GenName name)
{
    if (!m_selected_node)
    {
        wxMessageBox("You need to select something first in order to properly place this widget.");
        return;
    }

    if (name == gen_tool && (m_selected_node->isType(type_aui_toolbar) || m_selected_node->isType(type_aui_tool)))
    {
        name = gen_auitool;
    }

    if (!m_selected_node->createToolNode(name))
    {
        if (m_selected_node->isGen(gen_wxSplitterWindow))
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
                wxMessageBox("A wxRibbonButtonBar can only be created as a child of a wxRibbonPanel.",
                             "Cannot create wxRibbonButtonBar", wxOK | wxICON_ERROR);
                break;

            case gen_wxRibbonPanel:
                wxMessageBox("A wxRibbonPanel can only be created as a child of a wxRibbonPage.",
                             "Cannot create wxRibbonPanel", wxOK | wxICON_ERROR);
                break;

            default:
                wxMessageBox(tt_string() << "Unable to create " << map_GenNames[name] << " as a child of "
                                         << m_selected_node->declName());
        }
    }
}
