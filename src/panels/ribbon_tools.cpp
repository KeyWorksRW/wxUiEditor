/////////////////////////////////////////////////////////////////////////////
// Purpose:   Displays node creation tools in a wxRibbonBar
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/menu.h>          // wxMenu and wxMenuBar classes
#include <wx/ribbon/page.h>   // Container for ribbon-bar-style interface panels
#include <wx/ribbon/panel.h>  // Ribbon-style container for a group of related tools / controls
#include <wx/sizer.h>         // provide wxSizer class for layout

#include "ribbon_tools.h"  // auto-generated: ../ui/ribbonpanel_base.h and ../ui/ribbonpanel_base.cpp

#include "mainframe.h"     // MainFrame -- Main window frame
#include "node_creator.h"  // NodeCreator class

// The base class specifies a larger size for the panel to make it easier to work with in the Mockup window. We switch
// that to a default size here.
RibbonPanel::RibbonPanel(wxWindow* parent) : RibbonPanelBase(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize) {}

void RibbonPanel::OnToolClick(wxRibbonToolBarEvent& event)
{
    auto name = event.GetBar()->GetToolHelpString(event.GetId());
    if (name.size())
        wxGetFrame().CreateToolNode(ttlib::cstr() << name.wx_str());
}

// clang-format off
enum
{

    MenuComboNormal,
    MenuComboChoice,
    MenuComboBitmap,

    MenuListBox,
    MenuCheckListBox,
    MenuListView,
    MenuHtmlListBox,

    MenuButton,
    MenuToggleButton,
    MenuStdDialogButton,
    MenuCmdLinkButton,

    MenuSpin,
    MenuSpinDbl,
    MenuSpinBtn,

    MenuStaticBox,
    MenuStaticCheckboxBox,
    MenuStaticRadioBtnBox,

    MenuDataCtrl,
    MenuDataTreeCtrl,
    MenuDataListCtrl,

    MenuRibbonBtn,
    MenuRibbonTool,
    MenuRibbonGallery,
    MenuRibbonSizer,

    MenuCheckBoxNormal,
    MenuCheck3State,

};
// clang-format on

void RibbonPanel::OnDropDown(wxRibbonToolBarEvent& event)
{
    wxMenu menu;

    switch (event.GetId())
    {
        case NewCheckbox:
            menu.Append(MenuCheckBoxNormal, "Insert wxCheckBox");
            menu.Append(MenuCheck3State, "Insert 3-state wxCheckBox");
            break;

        case NewCombobox:
            menu.Append(MenuComboNormal, "Insert wxComboBox");
            menu.Append(MenuComboChoice, "Insert wxChoice");
            menu.Append(MenuComboBitmap, "Insert wxBitmapComboBox");
            break;

        case NewListbox:
            menu.Append(MenuListBox, "Insert wxListBox");
            menu.Append(MenuCheckListBox, "Insert wxCheckListBox");
            menu.Append(MenuListView, "Insert wxListView");
            menu.Append(MenuHtmlListBox, "Insert wxSimpleHtmlListBox");
            break;

        case NewButton:
            menu.Append(MenuButton, "Insert wxButton");
            menu.Append(MenuToggleButton, "Insert wxToggleButton");
            menu.Append(MenuStdDialogButton, "Insert wxStdDialogButtonSizer");
            menu.Append(MenuCmdLinkButton, "Insert wxCommandLinkButton");
            break;

        case NewSpin:
            menu.Append(MenuSpin, "Insert wxSpinCtrl");
            menu.Append(MenuSpinDbl, "Insert wxSpinCtrlDouble");
            menu.Append(MenuSpinBtn, "Insert wxSpinButton");
            break;

        case NewDataCtrl:
            menu.Append(MenuDataCtrl, "Insert wxDataViewCtrl");
            menu.Append(MenuDataTreeCtrl, "Insert wxDataViewTreeCtrl");
            menu.Append(MenuDataListCtrl, "Insert wxDataViewListCtrl");
            break;

        case NewRibbonType:
            menu.Append(MenuRibbonBtn, "Insert wxRibbonButtonBar");
            menu.Append(MenuRibbonTool, "Insert wxRibbonToolBar");
            menu.Append(MenuRibbonGallery, "Insert wxRibbonGallery");
            menu.Append(MenuRibbonSizer, "Insert wxBoxSizer");
            break;

        case NewStaticSizer:
            menu.Append(MenuStaticBox, "Insert wxStaticBoxSizer");
            menu.Append(MenuStaticCheckboxBox, "Insert CheckBox wxStaticBoxSizer");
            menu.Append(MenuStaticRadioBtnBox, "Insert RadioButton wxStaticBoxSizer");
            break;

        default:
            return;
    }

    menu.Bind(wxEVT_MENU, &RibbonPanel::OnMenuEvent, this, wxID_ANY);

    event.PopupMenu(&menu);
}

void RibbonPanel::OnMenuEvent(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case MenuCheckBoxNormal:
            wxGetFrame().CreateToolNode("wxCheckBox");
            break;

        case MenuCheck3State:
            wxGetFrame().CreateToolNode("Check3State");
            break;

        case MenuRibbonBtn:
            wxGetFrame().CreateToolNode("wxRibbonButtonBar");
            break;

        case MenuRibbonTool:
            wxGetFrame().CreateToolNode("wxRibbonToolBar");
            break;

        case MenuRibbonGallery:
            wxGetFrame().CreateToolNode("wxRibbonGallery");
            break;

        case MenuRibbonSizer:
            wxGetFrame().CreateToolNode("wxBoxSizer");
            break;

        case MenuDataCtrl:
            wxGetFrame().CreateToolNode("wxDataViewCtrl");
            break;

        case MenuDataTreeCtrl:
            wxGetFrame().CreateToolNode("wxDataViewTreeCtrl");
            break;

        case MenuDataListCtrl:
            wxGetFrame().CreateToolNode("wxDataViewListCtrl");
            break;

        case MenuComboNormal:
            wxGetFrame().CreateToolNode("wxComboBox");
            break;

        case MenuComboChoice:
            wxGetFrame().CreateToolNode("wxChoice");
            break;

        case MenuComboBitmap:
            wxGetFrame().CreateToolNode("wxBitmapComboBox");
            break;

        case MenuListBox:
            wxGetFrame().CreateToolNode("wxListBox");
            break;

        case MenuHtmlListBox:
            wxGetFrame().CreateToolNode("wxSimpleHtmlListBox");
            break;

        case MenuCheckListBox:
            wxGetFrame().CreateToolNode("wxCheckListBox");
            break;

        case MenuListView:
            wxGetFrame().CreateToolNode("wxListView");
            break;

        case MenuButton:
            wxGetFrame().CreateToolNode("wxButton");
            break;

        case MenuCmdLinkButton:
            wxGetFrame().CreateToolNode("wxCommandLinkButton");
            break;

        case MenuToggleButton:
            wxGetFrame().CreateToolNode("wxToggleButton");
            break;

        case MenuStdDialogButton:
            wxGetFrame().CreateToolNode("wxStdDialogButtonSizer");
            break;

        case MenuSpin:
            wxGetFrame().CreateToolNode("wxSpinCtrl");
            break;

        case MenuSpinDbl:
            wxGetFrame().CreateToolNode("wxSpinCtrlDouble");
            break;

        case MenuSpinBtn:
            wxGetFrame().CreateToolNode("wxSpinButton");
            break;

        case MenuStaticBox:
            wxGetFrame().CreateToolNode("wxStaticBoxSizer");
            break;

        case MenuStaticCheckboxBox:
            wxGetFrame().CreateToolNode("StaticCheckboxBoxSizer");
            break;

        case MenuStaticRadioBtnBox:
            wxGetFrame().CreateToolNode("StaticRadioBtnBoxSizer");
            break;

        default:
            break;
    }
}
