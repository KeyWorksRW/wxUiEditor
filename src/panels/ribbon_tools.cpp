/////////////////////////////////////////////////////////////////////////////
// Purpose:   Displays node creation tools in a wxRibbonBar
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/menu.h>          // wxMenu and wxMenuBar classes
#include <wx/ribbon/page.h>   // Container for ribbon-bar-style interface panels
#include <wx/ribbon/panel.h>  // Ribbon-style container for a group of related tools / controls
#include <wx/sizer.h>         // provide wxSizer class for layout

#include "ribbon_tools.h"  // auto-generated: ../ui/ribbonpanel_base.h and ../ui/ribbonpanel_base.cpp

#include "gen_enums.h"     // Enumerations for generators
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

void RibbonPanel::OnDropDown(wxRibbonToolBarEvent& event)
{
    wxMenu menu;

    switch (event.GetId())
    {
        case NewCheckbox:
            menu.Append(gen_wxCheckBox, "Insert wxCheckBox");
            menu.Append(gen_Check3State, "Insert 3-state wxCheckBox");
            break;

        case NewCombobox:
            menu.Append(gen_wxComboBox, "Insert wxComboBox");
            menu.Append(gen_wxChoice, "Insert wxChoice");
            menu.Append(gen_wxBitmapComboBox, "Insert wxBitmapComboBox");
            break;

        case NewListbox:
            menu.Append(gen_wxListBox, "Insert wxListBox");
            menu.Append(gen_wxCheckListBox, "Insert wxCheckListBox");
            menu.Append(gen_wxListView, "Insert wxListView");
            menu.Append(gen_wxRearrangeCtrl, "Insert wxRearrangeCtrl");
            menu.Append(gen_wxSimpleHtmlListBox, "Insert wxSimpleHtmlListBox");
            break;

        case NewButton:
            menu.Append(gen_wxButton, "Insert wxButton");
            menu.Append(gen_wxToggleButton, "Insert wxToggleButton");
            menu.Append(gen_wxStdDialogButtonSizer, "Insert wxStdDialogButtonSizer");
            menu.Append(gen_wxCommandLinkButton, "Insert wxCommandLinkButton");
            break;

        case NewSpin:
            menu.Append(gen_wxSpinCtrl, "Insert wxSpinCtrl");
            menu.Append(gen_wxSpinCtrlDouble, "Insert wxSpinCtrlDouble");
            menu.Append(gen_wxSpinButton, "Insert wxSpinButton");
            break;

        case NewDataCtrl:
            menu.Append(gen_wxDataViewCtrl, "Insert wxDataViewCtrl");
            menu.Append(gen_wxDataViewTreeCtrl, "Insert wxDataViewTreeCtrl");
            menu.Append(gen_wxDataViewListCtrl, "Insert wxDataViewListCtrl");
            break;

        case NewRibbonType:
            menu.Append(gen_wxRibbonButtonBar, "Insert wxRibbonButtonBar");
            menu.Append(gen_wxRibbonToolBar, "Insert wxRibbonToolBar");
            menu.Append(gen_wxRibbonGallery, "Insert wxRibbonGallery");
            break;

        case NewStaticSizer:
            menu.Append(gen_wxStaticBoxSizer, "Insert wxStaticBoxSizer");
            menu.Append(gen_StaticCheckboxBoxSizer, "Insert CheckBox wxStaticBoxSizer");
            menu.Append(gen_StaticRadioBtnBoxSizer, "Insert RadioButton wxStaticBoxSizer");
            break;

        default:
            return;
    }

    menu.Bind(wxEVT_MENU, &RibbonPanel::OnMenuEvent, this, wxID_ANY);

    event.PopupMenu(&menu);
}

void RibbonPanel::OnMenuEvent(wxCommandEvent& event)
{
    ASSERT_MSG(static_cast<GenName>(event.GetId()) < gen_name_array_size, "Invalide gen_ id!")

    wxGetFrame().CreateToolNode(static_cast<GenName>(event.GetId()));
}
