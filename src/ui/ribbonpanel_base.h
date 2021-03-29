////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor/
//
// DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/bitmap.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/panel.h>
#include <wx/ribbon/art.h>
#include <wx/ribbon/bar.h>
#include <wx/ribbon/control.h>
#include <wx/ribbon/toolbar.h>
#include <wx/sizer.h>

class RibbonPanelBase : public wxPanel
{
public:
    RibbonPanelBase(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(900, 300),
        long style = wxTAB_TRAVERSAL);

    enum
    {
        NewActivityIndicator = wxID_HIGHEST + 1,
        NewBannerWindow,
        NewBitmap,
        NewBookPage,
        NewButton,
        NewCalendar,
        NewCheckbox,
        NewChoiceBook,
        NewCollapsible,
        NewColorPicker,
        NewCombobox,
        NewDataColumn,
        NewDataCtrl,
        NewDataListColumn,
        NewDatePicker,
        NewDialog,
        NewDirPicker,
        NewFileCtrl,
        NewFilePicker,
        NewFlexGrid,
        NewFontPicker,
        NewFrame,
        NewGalleryItem,
        NewGauge,
        NewGenericDirCtrl,
        NewGrid,
        NewGridBag,
        NewGridPage,
        NewGridSizer,
        NewHorzSizer,
        NewHtml,
        NewHyperLink,
        NewInfoBar,
        NewLine,
        NewListBook,
        NewListbox,
        NewMenu,
        NewMenuBar,
        NewMenuItem,
        NewMenuSeparator,
        NewNotebook,
        NewPanel,
        NewPropGrid,
        NewPropItem,
        NewPropManager,
        NewRadioBox,
        NewRadiobtn,
        NewRibbonBar,
        NewRibbonPage,
        NewRibbonPanel,
        NewRibbonTool,
        NewRibbonType,
        NewRichText,
        NewScintilla,
        NewScrollWin,
        NewSearch,
        NewSimpleBook,
        NewSizeSpacer,
        NewSlider,
        NewSpin,
        NewSplitter,
        NewStaticCtrl,
        NewStaticSizer,
        NewStatusBar,
        NewStdBtn,
        NewSubMenu,
        NewTextCtrl,
        NewTextSizer,
        NewTimePickerCtrl,
        NewTool,
        NewToolBar,
        NewToolBook,
        NewToolSeparator,
        NewTreeBook,
        NewTreeColumn,
        NewTreeCtrl,
        NewTreeList,
        NewVertSizer,
        NewWizPage,
        NewWizard,
        NewWrapSizer
    };

protected:

    // Class member variables

    wxBoxSizer* parent_sizer;
    wxRibbonBar* m_rbnBar;

    // Virtual event handlers -- override them in your derived class

    virtual void OnDropDown(wxRibbonToolBarEvent& event) { event.Skip(); }
    virtual void OnToolClick(wxRibbonToolBarEvent& event) { event.Skip(); }
};
