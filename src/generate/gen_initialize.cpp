/////////////////////////////////////////////////////////////////////////////
// Purpose:   Initialize all generators
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/activityindicator.h>      // wxActivityIndicator declaration.
#include <wx/anybutton.h>              // wxAnyButtonBase class
#include <wx/aui/auibar.h>             // wxaui: wx advanced user interface - docking window manager
#include <wx/aui/auibook.h>            // wxaui: wx advanced user interface - notebook
#include <wx/bannerwindow.h>           // wxBannerWindow class declaration
#include <wx/button.h>                 // wxButtonBase class
#include <wx/calctrl.h>                // date-picker control
#include <wx/checkbox.h>               // wxCheckBox class interface
#include <wx/choicebk.h>               // wxChoicebook: wxChoice and wxNotebook combination
#include <wx/clrpicker.h>              // wxColourPickerCtrl base header
#include <wx/collpane.h>               // wxCollapsiblePane
#include <wx/commandlinkbutton.h>      // wxCommandLinkButtonBase and wxGenericCommandLinkButton classes
#include <wx/dataview.h>               // wxDataViewCtrl base classes
#include <wx/datectrl.h>               // implements wxDatePickerCtrl
#include <wx/dirctrl.h>                // Directory control base header
#include <wx/filectrl.h>               // Header for wxFileCtrlBase and other common functions used by
#include <wx/filepicker.h>             // wxFilePickerCtrl, wxDirPickerCtrl base header
#include <wx/fontpicker.h>             // wxFontPickerCtrl base header
#include <wx/frame.h>                  // wxFrame class interface
#include <wx/gauge.h>                  // wxGauge interface
#include <wx/gbsizer.h>                // wxGridBagSizer:  A sizer that can lay out items in a grid,
#include <wx/html/htmlwin.h>           // wxHtmlWindow class for parsing & displaying HTML
#include <wx/htmllbox.h>               // wxHtmlListBox is a listbox whose items are wxHtmlCells
#include <wx/hyperlink.h>              // Hyperlink control
#include <wx/infobar.h>                // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/listbook.h>               // wxListbook: wxListView and wxNotebook combination
#include <wx/listctrl.h>               // wxListView class
#include <wx/notebook.h>               // wxNotebook interface
#include <wx/panel.h>                  // Base header for wxPanel
#include <wx/popupwin.h>               // wxPopupWindow interface declaration
#include <wx/propgrid/manager.h>       // wxPropertyGridManager
#include <wx/rearrangectrl.h>          // various controls for rearranging the items interactively
#include <wx/ribbon/buttonbar.h>       // Ribbon control similar to a tool bar
#include <wx/ribbon/gallery.h>         // Ribbon control which displays a gallery of items to choose from
#include <wx/ribbon/toolbar.h>         // Ribbon-style tool bar
#include <wx/richtext/richtextctrl.h>  // A rich edit control
#include <wx/slider.h>                 // wxSlider interface
#include <wx/splitter.h>               // Base header for wxSplitterWindow
#include <wx/statbmp.h>                // wxStaticBitmap class interface
#include <wx/stattext.h>               // wxStaticText base header
#include <wx/statusbr.h>               // wxStatusBar class interface
#include <wx/stc/stc.h>                // A wxWidgets implementation of Scintilla.
#include <wx/tglbtn.h>                 // wxToggleButtonBase
#include <wx/timectrl.h>               // Declaration of wxTimePickerCtrl class.
#include <wx/toolbar.h>                // wxToolBar interface declaration
#include <wx/toolbook.h>               // wxToolbook: wxToolBar and wxNotebook combination
#include <wx/treebook.h>               // wxTreebook: wxNotebook-like control presenting pages in a tree
#include <wx/treelist.h>               // wxTreeListCtrl class declaration.
#include <wx/wizard.h>                 // wxWizard class: a GUI control presenting the user with a
#include <wx/wrapsizer.h>              // provide wrapping sizer for layout (wxWrapSizer)

#include "gen_common.h"    // GeneratorLibrary -- Generator classes
#include "mainapp.h"       // App -- App class
#include "node_creator.h"  // NodeCreator

#include "book_widgets.h"      // ChoicebookGenerator -- Book component classes
#include "btn_widgets.h"       // ButtonGenerator -- Button component class
#include "checkbox_widgets.h"  // CheckBoxGenerator -- wxCheckBox component class
#include "combo_widgets.h"     // Combo and choice component classes
#include "ctrl_widgets.h"      // CalendarCtrlGenerator -- Calendar, GenericDir, Search Ctrl component classes
#include "dataview_widgets.h"  // DataViewCtrl -- wxDataView component classes
#include "form_widgets.h"      // FrameFormGenerator -- Form component classes
#include "grid_widgets.h"      // PropertyGridGenerator -- Grid component classes
#include "listbox_widgets.h"   // ListBox component classes
#include "listctrl_widgets.h"  // ListViewGenerator -- ListCtrl component class
#include "menu_widgets.h"      // Menu component classes
#include "misc_widgets.h"      // Miscellaneous component classes
#include "panel_widgets.h"     // Panel component classes
#include "picker_widgets.h"    // DatePickerCtrlGenerator -- Picker component classes
#include "radio_widgets.h"     // RadioButtonGenerator -- Radio button and Radio box component classes
#include "ribbon_widgets.h"    // RibbonBarGenerator -- Ribbon component classes
#include "sizer_widgets.h"     // Sizer component classes
#include "spin_widgets.h"      // Spin component classes
#include "text_widgets.h"      // Text component classes
#include "toolbar_widgets.h"   // Toolbar component classes
#include "tree_widgets.h"      // TreeCtrlGenerator -- wxTreeCtrl component classes
#include "window_widgets.h"    // Splitter and Scroll component classes
#include "wizard_form.h"       // Wizard form class

#include "gen_enums.h"  // Enumerations for generators

using namespace GenEnum;

#define SET_GENERATOR(name, generator) get_declaration(name)->SetGenerator(new generator());

void NodeCreator::InitGenerators()
{
    // CREATE_GENERATOR("BookPage", BookPageGenerator)
    SET_GENERATOR(gen_BookPage, BookPageGenerator)

    SET_GENERATOR(gen_wxWizardPageSimple, WizardPageGenerator)
    SET_GENERATOR(gen_wxBannerWindow, BannerWindowGenerator)
    SET_GENERATOR(gen_wxBitmapComboBox, BitmapComboBoxGenerator)
    SET_GENERATOR(gen_wxActivityIndicator, ActivityIndicatorGenerator)
    SET_GENERATOR(gen_wxButton, ButtonGenerator)
    SET_GENERATOR(gen_wxCheckBox, CheckBoxGenerator)
    SET_GENERATOR(gen_Check3State, Check3StateGenerator)
    SET_GENERATOR(gen_wxCheckListBox, CheckListBoxGenerator)
    SET_GENERATOR(gen_wxChoice, ChoiceGenerator)
    SET_GENERATOR(gen_wxChoicebook, ChoicebookGenerator)
    SET_GENERATOR(gen_wxCollapsiblePane, CollapsiblePaneGenerator)
    SET_GENERATOR(gen_wxComboBox, ComboBoxGenerator)
    SET_GENERATOR(gen_wxCommandLinkButton, CommandLinkBtnGenerator)
    SET_GENERATOR(gen_wxGauge, GaugeGenerator)
    SET_GENERATOR(gen_wxHtmlWindow, HtmlWindowGenerator)
    SET_GENERATOR(gen_wxHyperlinkCtrl, HyperlinkGenerator)
    SET_GENERATOR(gen_wxInfoBar, InfoBarGenerator)
    SET_GENERATOR(gen_wxListBox, ListBoxGenerator)
    SET_GENERATOR(gen_wxSimpleHtmlListBox, HtmlListBoxGenerator)
    SET_GENERATOR(gen_wxListbook, ListbookGenerator)
    SET_GENERATOR(gen_wxNotebook, NotebookGenerator)
    SET_GENERATOR(gen_wxToolbook, ToolbookGenerator)
    SET_GENERATOR(gen_wxTreebook, TreebookGenerator)
    SET_GENERATOR(gen_wxPanel, PanelGenerator)
    SET_GENERATOR(gen_wxRadioBox, RadioBoxGenerator)
    SET_GENERATOR(gen_wxRearrangeCtrl, RearrangeCtrlGenerator)
    SET_GENERATOR(gen_wxRadioButton, RadioButtonGenerator)
    SET_GENERATOR(gen_wxRichTextCtrl, RichTextCtrlGenerator)
    SET_GENERATOR(gen_wxScrollBar, ScrollBarGenerator)
    SET_GENERATOR(gen_wxScrolledWindow, ScrolledWindowGenerator)
    SET_GENERATOR(gen_wxSimplebook, SimplebookGenerator)
    SET_GENERATOR(gen_wxSlider, SliderGenerator)
    SET_GENERATOR(gen_wxSplitterWindow, SplitterWindowGenerator)
    SET_GENERATOR(gen_wxStaticBitmap, StaticBitmapGenerator)
    SET_GENERATOR(gen_wxStaticLine, StaticLineGenerator)
    SET_GENERATOR(gen_wxStaticText, StaticTextGenerator)
    SET_GENERATOR(gen_wxStatusBar, StatusBarGenerator)
    SET_GENERATOR(gen_wxStyledTextCtrl, StyledTextGenerator)
    SET_GENERATOR(gen_wxTextCtrl, TextCtrlGenerator)
    SET_GENERATOR(gen_wxToggleButton, ToggleButtonGenerator)
    SET_GENERATOR(gen_wxCalendarCtrl, CalendarCtrlGenerator)
    SET_GENERATOR(gen_wxFileCtrl, FileCtrlGenerator)
    SET_GENERATOR(gen_wxGenericDirCtrl, GenericDirCtrlGenerator)
    SET_GENERATOR(gen_wxSearchCtrl, SearchCtrlGenerator)
    SET_GENERATOR(gen_wxListView, ListViewGenerator)
    SET_GENERATOR(gen_wxGrid, GridGenerator)

    SET_GENERATOR(gen_wxColourPickerCtrl, ColourPickerGenerator)
    SET_GENERATOR(gen_wxDatePickerCtrl, DatePickerCtrlGenerator)
    SET_GENERATOR(gen_wxDirPickerCtrl, DirPickerGenerator)
    SET_GENERATOR(gen_wxFilePickerCtrl, FilePickerGenerator)
    SET_GENERATOR(gen_wxFontPickerCtrl, FontPickerGenerator)
    SET_GENERATOR(gen_wxTimePickerCtrl, TimePickerCtrlGenerator)

    SET_GENERATOR(gen_wxMenuBar, MenuBarGenerator)
    SET_GENERATOR(gen_MenuBar, MenuBarFormGenerator)
    SET_GENERATOR(gen_wxMenu, MenuGenerator)
    SET_GENERATOR(gen_submenu, SubMenuGenerator)
    SET_GENERATOR(gen_wxMenuItem, MenuItemGenerator)
    SET_GENERATOR(gen_separator, SeparatorGenerator)

    SET_GENERATOR(gen_wxDataViewCtrl, DataViewCtrl)
    SET_GENERATOR(gen_wxDataViewListCtrl, DataViewListCtrl)
    SET_GENERATOR(gen_wxDataViewTreeCtrl, DataViewTreeCtrl)
    SET_GENERATOR(gen_dataViewColumn, DataViewColumn)
    SET_GENERATOR(gen_dataViewListColumn, DataViewListColumn)

    SET_GENERATOR(gen_wxPropertyGrid, PropertyGridGenerator)
    SET_GENERATOR(gen_wxPropertyGridManager, PropertyGridManagerGenerator)
    SET_GENERATOR(gen_propGridItem, PropertyGridItemGenerator)
    SET_GENERATOR(gen_propGridPage, PropertyGridPageGenerator)

    SET_GENERATOR(gen_wxSpinButton, SpinButtonGenerator)
    SET_GENERATOR(gen_wxSpinCtrl, SpinCtrlGenerator)
    SET_GENERATOR(gen_wxSpinCtrlDouble, SpinCtrlDoubleGenerator)

    SET_GENERATOR(gen_spacer, SpacerGenerator)

    SET_GENERATOR(gen_ToolBar, ToolBarFormGenerator)
    SET_GENERATOR(gen_wxToolBar, ToolBarGenerator)
    SET_GENERATOR(gen_tool, ToolGenerator)
    SET_GENERATOR(gen_toolSeparator, ToolSeparatorGenerator)
    SET_GENERATOR(gen_wxRibbonBar, RibbonBarGenerator)
    SET_GENERATOR(gen_wxRibbonPage, RibbonPageGenerator)
    SET_GENERATOR(gen_wxRibbonPanel, RibbonPanelGenerator)
    SET_GENERATOR(gen_wxRibbonButtonBar, RibbonButtonBarGenerator)
    SET_GENERATOR(gen_wxRibbonToolBar, RibbonToolBarGenerator)
    SET_GENERATOR(gen_wxRibbonGallery, RibbonGalleryGenerator)

    SET_GENERATOR(gen_ribbonButton, RibbonButtonGenerator)
    SET_GENERATOR(gen_ribbonTool, RibbonToolGenerator)
    SET_GENERATOR(gen_ribbonGalleryItem, RibbonGalleryItemGenerator)

    SET_GENERATOR(gen_wxTreeCtrl, TreeCtrlGenerator)
    SET_GENERATOR(gen_wxTreeListCtrl, TreeListViewGenerator)
    SET_GENERATOR(gen_TreeListCtrlColumn, TreeListCtrlColumnGenerator)

    SET_GENERATOR(gen_wxDialog, DialogFormGenerator)
    SET_GENERATOR(gen_wxFrame, FrameFormGenerator)
    SET_GENERATOR(gen_wxPopupTransientWindow, PopupWinGenerator)
    SET_GENERATOR(gen_PanelForm, PanelFormGenerator)
    SET_GENERATOR(gen_wxWizard, WizardFormGenerator)

    SET_GENERATOR(gen_VerticalBoxSizer, BoxSizerGenerator)
    SET_GENERATOR(gen_wxBoxSizer, BoxSizerGenerator)
    SET_GENERATOR(gen_wxFlexGridSizer, FlexGridSizerGenerator)
    SET_GENERATOR(gen_wxGridBagSizer, GridBagSizerGenerator)
    SET_GENERATOR(gen_wxGridSizer, GridSizerGenerator)
    SET_GENERATOR(gen_wxStaticBoxSizer, StaticBoxSizerGenerator)
    SET_GENERATOR(gen_StaticCheckboxBoxSizer, StaticCheckboxBoxSizerGenerator)
    SET_GENERATOR(gen_StaticRadioBtnBoxSizer, StaticRadioBtnBoxSizerGenerator)
    SET_GENERATOR(gen_wxStdDialogButtonSizer, StdDialogButtonSizerGenerator)
    SET_GENERATOR(gen_wxWrapSizer, WrapSizerGenerator)
    SET_GENERATOR(gen_TextSizer, TextSizerGenerator)

    AddAllConstants();
}

// We put the list of constants in a separate header file because it's a huge list. It needs to be #included in this
// source module because this is the module that will #include all the wxWidgets headers that contain the
// definitions we need.

#include "add_constants.h"
