/////////////////////////////////////////////////////////////////////////////
// Purpose:   Displays node creation tools in a wxRibbonBar
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/menu.h>          // wxMenu and wxMenuBar classes
#include <wx/ribbon/page.h>   // Container for ribbon-bar-style interface panels
#include <wx/ribbon/panel.h>  // Ribbon-style container for a group of related tools / controls
#include <wx/sizer.h>         // provide wxSizer class for layout

#include "ribbon_tools.h"  // auto-generated: wxui/ribbonpanel_base.h and wxui/ribbonpanel_base.cpp

#include "gen_enums.h"     // Enumerations for generators
#include "mainframe.h"     // MainFrame -- Main window frame
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator class

#include "../newdialogs/new_dialog.h"     // NewDialog -- Dialog for creating a new project dialog
#include "../newdialogs/new_frame.h"      // NewFrame -- Dialog for creating a new project wxFrame
#include "../newdialogs/new_mdi.h"        // NewMdiForm -- Dialog for creating a new MDI application
#include "../newdialogs/new_panel.h"      // NewPanel -- Dialog for creating a new form panel
#include "../newdialogs/new_propsheet.h"  // NewPropSheet -- Dialog for creating a new wxPropertySheetDialog
#include "../newdialogs/new_ribbon.h"     // NewRibbon -- Dialog for creating a new wxRibbonBar
#include "../newdialogs/new_wizard.h"     // NewWizard -- Dialog for creating a new wizard

#include "ribbon_ids.h"

#include "menu_auibar.h"
#include "menu_bartools.h"
#include "menubutton.h"
#include "menucheckbox.h"
#include "menucombobox.h"
#include "menudatactrl.h"
#include "menulistbox.h"
#include "menuribbontype.h"
#include "menuspin.h"
#include "menustaticsizer.h"

bool CreateViaNewDlg(size_t id)
{
    switch (id)
    {
        case CreateNewDialog:
            {
                NewDialog dlg(wxGetFrame().getWindow());
                if (dlg.ShowModal() == wxID_OK)
                {
                    dlg.createNode();
                }
                return true;
            }
            break;

        case CreateNewFrame:
            {
                NewFrame dlg(wxGetFrame().getWindow());
                if (dlg.ShowModal() == wxID_OK)
                {
                    dlg.createNode();
                }
                return true;
            }
            break;

        case CreateMdiFrame:
            {
                NewMdiForm dlg(wxGetFrame().getWindow());
                if (dlg.ShowModal() == wxID_OK)
                {
                    dlg.createNode();
                }
                return true;
            }
            break;

        case CreateNewPanel:
            {
                NewPanel dlg(wxGetFrame().getWindow());
                dlg.WantFormVersion();
                if (dlg.ShowModal() == wxID_OK)
                {
                    dlg.createNode();
                }
                return true;
            }
            break;

        case CreateNewPropertySheet:
            {
                NewPropSheet dlg(wxGetFrame().getWindow());
                if (dlg.ShowModal() == wxID_OK)
                {
                    dlg.createNode();
                }
                return true;
            }
            break;

        case CreateNewRibbon:
            {
                NewRibbon dlg(wxGetFrame().getWindow());
                if (dlg.IsCreatable())
                {
                    if (dlg.ShowModal() == wxID_OK)
                    {
                        dlg.createNode();
                    }
                }
                return true;
            }
            break;

        case CreateNewFormRibbon:
            {
                NewRibbon dlg(wxGetFrame().getWindow());
                dlg.WantFormVersion();
                if (dlg.ShowModal() == wxID_OK)
                {
                    dlg.createNode();
                }
                return true;
            }
            break;

        case CreateNewWizard:
            {
                NewWizard dlg(wxGetFrame().getWindow());
                if (dlg.ShowModal() == wxID_OK)
                {
                    dlg.createNode();
                }
                return true;
            }
            break;
    }

    return false;
}

// The base class specifies a larger size for the panel to make it easier to work with in the Mockup window. We switch
// that to a default size here.
RibbonPanel::RibbonPanel(wxWindow* parent) : RibbonPanelBase(parent) {}

void RibbonPanel::OnToolClick(wxRibbonToolBarEvent& event)
{
    size_t id = event.GetId();

    if (id == CreateNewRibbon && (!wxGetFrame().getSelectedNode() || wxGetFrame().getSelectedNode()->isGen(gen_Project)))
    {
        id = CreateNewFormRibbon;
    }

    if (id < gen_name_array_size)
    {
        wxGetFrame().createToolNode(static_cast<GenName>(event.GetId()));
        return;
    }
    else
    {
        if (CreateViaNewDlg(id))
            return;
    }

    FAIL_MSG("This will only happen if the tool is a) not a dropdown, or b) doesn't have a valid id.");

    // For release build, we'll at least attempt to create it in case the help string specifies a widget.
    auto name = event.GetBar()->GetToolHelpString(event.GetId());
    if (auto result = rmap_GenNames.find(name.utf8_string()); result != rmap_GenNames.end())
    {
        wxGetFrame().createToolNode(result->second);
    }
}

void RibbonPanel::OnDropDown(wxRibbonToolBarEvent& event)
{
    switch (event.GetId())
    {
        case NewCheckbox:
            {
                MenuCheckbox popup_menu;
                popup_menu.Bind(wxEVT_MENU, &RibbonPanel::OnMenuEvent, this, wxID_ANY);
                event.PopupMenu(&popup_menu);
                return;
            }

        case BarTools:
            {
                const auto* cur_sel = wxGetFrame().getSelectedNode();
                if (!cur_sel || cur_sel->isGen(gen_Project))
                    return;
                if (cur_sel && (cur_sel->isGen(gen_wxAuiToolBar) || cur_sel->getParent()->isGen(gen_wxAuiToolBar)))
                {
                    MenuAuiBar popup_menu;
                    popup_menu.Bind(wxEVT_MENU, &RibbonPanel::OnMenuEvent, this, wxID_ANY);
                    event.PopupMenu(&popup_menu);
                }
                else
                {
                    MenuBarTools popup_menu;
                    popup_menu.Bind(wxEVT_MENU, &RibbonPanel::OnMenuEvent, this, wxID_ANY);
                    event.PopupMenu(&popup_menu);
                }
                return;
            }

        case AuiBarTools:
            {
                const auto* cur_sel = wxGetFrame().getSelectedNode();
                if (!cur_sel || cur_sel->isGen(gen_Project))
                    return;
                if (cur_sel && (cur_sel->isGen(gen_wxToolBar) || cur_sel->isGen(gen_ToolBar) ||
                                cur_sel->getParent()->isGen(gen_wxToolBar) || cur_sel->getParent()->isGen(gen_ToolBar)))
                {
                    MenuBarTools popup_menu;
                    popup_menu.Bind(wxEVT_MENU, &RibbonPanel::OnMenuEvent, this, wxID_ANY);
                    event.PopupMenu(&popup_menu);
                }
                else
                {
                    MenuAuiBar popup_menu;
                    popup_menu.Bind(wxEVT_MENU, &RibbonPanel::OnMenuEvent, this, wxID_ANY);
                    event.PopupMenu(&popup_menu);
                }
                return;
            }

        case NewCombobox:
            {
                MenuCombobox popup_menu;
                popup_menu.Bind(wxEVT_MENU, &RibbonPanel::OnMenuEvent, this, wxID_ANY);
                event.PopupMenu(&popup_menu);
                return;
            }

        case NewListbox:
            {
                MenuListbox popup_menu;
                popup_menu.Bind(wxEVT_MENU, &RibbonPanel::OnMenuEvent, this, wxID_ANY);
                event.PopupMenu(&popup_menu);
                return;
            }

        case NewButton:
            {
                MenuButton popup_menu;
                popup_menu.Bind(wxEVT_MENU, &RibbonPanel::OnMenuEvent, this, wxID_ANY);
                event.PopupMenu(&popup_menu);
                return;
            }

        case NewSpin:
            {
                MenuSpin popup_menu;
                popup_menu.Bind(wxEVT_MENU, &RibbonPanel::OnMenuEvent, this, wxID_ANY);
                event.PopupMenu(&popup_menu);
                return;
            }

        case NewDataCtrl:
            {
                MenuDataCtrl popup_menu;
                popup_menu.Bind(wxEVT_MENU, &RibbonPanel::OnMenuEvent, this, wxID_ANY);
                event.PopupMenu(&popup_menu);
                return;
            }

        case NewRibbonType:
            {
                MenuRibbonType popup_menu;
                popup_menu.Bind(wxEVT_MENU, &RibbonPanel::OnMenuEvent, this, wxID_ANY);
                event.PopupMenu(&popup_menu);
                return;
            }

        case NewStaticSizer:
            {
                MenuStaticSizer popup_menu;
                popup_menu.Bind(wxEVT_MENU, &RibbonPanel::OnMenuEvent, this, wxID_ANY);
                event.PopupMenu(&popup_menu);
                return;
            }

        case NewScrolled:
            {
                wxMenu menu;
                menu.Append(gen_wxScrolledWindow, "Insert wxScrolledWindow");
                menu.Append(gen_wxScrolledCanvas, "Insert wxScrolledCanvas");

                menu.Bind(wxEVT_MENU, &RibbonPanel::OnMenuEvent, this, wxID_ANY);
                event.PopupMenu(&menu);
                return;
            }

        default:
            return;
    }
}

void RibbonPanel::OnMenuEvent(wxCommandEvent& event)
{
    ASSERT_MSG(static_cast<GenName>(event.GetId()) < gen_name_array_size, "Invalid gen_ id!")

    wxGetFrame().createToolNode(static_cast<GenName>(event.GetId()));
}
