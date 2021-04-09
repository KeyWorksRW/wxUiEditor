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

#include "enum_classes.h"  // Enumerations for nodes

using namespace NodeEnums;

// Note that classes are created via new but never deleted. Technically, that's a memory leak, but it doesn't really
// matter because they are only created when the application is started, and need to exist until the program exits.
// There's no real advantage to deleting each class during exit since the OS will free that memory automatically.

#define SET_GENERATOR(class_enum, component_class) get_declaration(class_enum)->SetGenerator(new component_class());

void NodeCreator::InitGenerators()
{
    // CREATE_GENERATOR("BookPage", BookPageGenerator)
    SET_GENERATOR(ClassName::BookPage, BookPageGenerator)

    SET_GENERATOR(ClassName::wxWizardPageSimple, WizardPageGenerator)
    SET_GENERATOR(ClassName::wxBannerWindow, BannerWindowGenerator)
    SET_GENERATOR(ClassName::wxBitmapComboBox, BitmapComboBoxGenerator)
    SET_GENERATOR(ClassName::wxActivityIndicator, ActivityIndicatorGenerator)
    SET_GENERATOR(ClassName::wxButton, ButtonGenerator)
    SET_GENERATOR(ClassName::wxCheckBox, CheckBoxGenerator)
    SET_GENERATOR(ClassName::Check3State, Check3StateGenerator)
    SET_GENERATOR(ClassName::wxCheckListBox, CheckListBoxGenerator)
    SET_GENERATOR(ClassName::wxChoice, ChoiceGenerator)
    SET_GENERATOR(ClassName::wxChoicebook, ChoicebookGenerator)
    SET_GENERATOR(ClassName::wxCollapsiblePane, CollapsiblePaneGenerator)
    SET_GENERATOR(ClassName::wxComboBox, ComboBoxGenerator)
    SET_GENERATOR(ClassName::wxCommandLinkButton, CommandLinkBtnGenerator)
    SET_GENERATOR(ClassName::wxGauge, GaugeGenerator)
    SET_GENERATOR(ClassName::wxHtmlWindow, HtmlWindowGenerator)
    SET_GENERATOR(ClassName::wxHyperlinkCtrl, HyperlinkGenerator)
    SET_GENERATOR(ClassName::wxInfoBar, InfoBarGenerator)
    SET_GENERATOR(ClassName::wxListBox, ListBoxGenerator)
    SET_GENERATOR(ClassName::wxSimpleHtmlListBox, HtmlListBoxGenerator)
    SET_GENERATOR(ClassName::wxListbook, ListbookGenerator)
    SET_GENERATOR(ClassName::wxNotebook, NotebookGenerator)
    SET_GENERATOR(ClassName::wxToolbook, ToolbookGenerator)
    SET_GENERATOR(ClassName::wxTreebook, TreebookGenerator)
    SET_GENERATOR(ClassName::wxPanel, PanelGenerator)
    SET_GENERATOR(ClassName::wxRadioBox, RadioBoxGenerator)
    SET_GENERATOR(ClassName::wxRadioButton, RadioButtonGenerator)
    SET_GENERATOR(ClassName::wxRichTextCtrl, RichTextCtrlGenerator)
    SET_GENERATOR(ClassName::wxScrollBar, ScrollBarGenerator)
    SET_GENERATOR(ClassName::wxScrolledWindow, ScrolledWindowGenerator)
    SET_GENERATOR(ClassName::wxSimplebook, SimplebookGenerator)
    SET_GENERATOR(ClassName::wxSlider, SliderGenerator)
    SET_GENERATOR(ClassName::wxSplitterWindow, SplitterWindowGenerator)
    SET_GENERATOR(ClassName::wxStaticBitmap, StaticBitmapGenerator)
    SET_GENERATOR(ClassName::wxStaticLine, StaticLineGenerator)
    SET_GENERATOR(ClassName::wxStaticText, StaticTextGenerator)
    SET_GENERATOR(ClassName::wxStatusBar, StatusBarGenerator)
    SET_GENERATOR(ClassName::wxStyledTextCtrl, StyledTextGenerator)
    SET_GENERATOR(ClassName::wxTextCtrl, TextCtrlGenerator)
    SET_GENERATOR(ClassName::wxToggleButton, ToggleButtonGenerator)
    SET_GENERATOR(ClassName::wxCalendarCtrl, CalendarCtrlGenerator)
    SET_GENERATOR(ClassName::wxFileCtrl, FileCtrlGenerator)
    SET_GENERATOR(ClassName::wxGenericDirCtrl, GenericDirCtrlGenerator)
    SET_GENERATOR(ClassName::wxSearchCtrl, SearchCtrlGenerator)
    SET_GENERATOR(ClassName::wxListView, ListViewGenerator)
    SET_GENERATOR(ClassName::wxGrid, GridGenerator)

    SET_GENERATOR(ClassName::wxColourPickerCtrl, ColourPickerGenerator)
    SET_GENERATOR(ClassName::wxDatePickerCtrl, DatePickerCtrlGenerator)
    SET_GENERATOR(ClassName::wxDirPickerCtrl, DirPickerGenerator)
    SET_GENERATOR(ClassName::wxFilePickerCtrl, FilePickerGenerator)
    SET_GENERATOR(ClassName::wxFontPickerCtrl, FontPickerGenerator)
    SET_GENERATOR(ClassName::wxTimePickerCtrl, TimePickerCtrlGenerator)

    SET_GENERATOR(ClassName::wxMenuBar, MenuBarGenerator)
    SET_GENERATOR(ClassName::MenuBar, MenuBarFormGenerator)
    SET_GENERATOR(ClassName::wxMenu, MenuGenerator)
    SET_GENERATOR(ClassName::submenu, SubMenuGenerator)
    SET_GENERATOR(ClassName::wxMenuItem, MenuItemGenerator)
    SET_GENERATOR(ClassName::separator, SeparatorGenerator)

    SET_GENERATOR(ClassName::wxDataViewCtrl, DataViewCtrl)
    SET_GENERATOR(ClassName::wxDataViewListCtrl, DataViewListCtrl)
    SET_GENERATOR(ClassName::wxDataViewTreeCtrl, DataViewTreeCtrl)
    SET_GENERATOR(ClassName::dataViewColumn, DataViewColumn)
    SET_GENERATOR(ClassName::dataViewListColumn, DataViewListColumn)

    SET_GENERATOR(ClassName::wxPropertyGrid, PropertyGridGenerator)
    SET_GENERATOR(ClassName::wxPropertyGridManager, PropertyGridManagerGenerator)
    SET_GENERATOR(ClassName::propGridItem, PropertyGridItemGenerator)
    SET_GENERATOR(ClassName::propGridPage, PropertyGridPageGenerator)

    SET_GENERATOR(ClassName::wxSpinButton, SpinButtonGenerator)
    SET_GENERATOR(ClassName::wxSpinCtrl, SpinCtrlGenerator)
    SET_GENERATOR(ClassName::wxSpinCtrlDouble, SpinCtrlDoubleGenerator)

    SET_GENERATOR(ClassName::spacer, SpacerGenerator)

    SET_GENERATOR(ClassName::ToolBar, ToolBarFormGenerator)
    SET_GENERATOR(ClassName::wxToolBar, ToolBarGenerator)
    SET_GENERATOR(ClassName::tool, ToolGenerator)
    SET_GENERATOR(ClassName::toolSeparator, ToolSeparatorGenerator)
    SET_GENERATOR(ClassName::wxRibbonBar, RibbonBarGenerator)
    SET_GENERATOR(ClassName::wxRibbonPage, RibbonPageGenerator)
    SET_GENERATOR(ClassName::wxRibbonPanel, RibbonPanelGenerator)
    SET_GENERATOR(ClassName::wxRibbonButtonBar, RibbonButtonBarGenerator)
    SET_GENERATOR(ClassName::wxRibbonToolBar, RibbonToolBarGenerator)
    SET_GENERATOR(ClassName::wxRibbonGallery, RibbonGalleryGenerator)

    SET_GENERATOR(ClassName::ribbonButton, RibbonButtonGenerator)
    SET_GENERATOR(ClassName::ribbonTool, RibbonToolGenerator)
    SET_GENERATOR(ClassName::ribbonGalleryItem, RibbonGalleryItemGenerator)

    SET_GENERATOR(ClassName::wxTreeCtrl, TreeCtrlGenerator)
    SET_GENERATOR(ClassName::wxTreeListCtrl, TreeListViewGenerator)
    SET_GENERATOR(ClassName::TreeListCtrlColumn, TreeListCtrlColumnGenerator)

    SET_GENERATOR(ClassName::wxDialog, DialogFormGenerator)
    SET_GENERATOR(ClassName::wxFrame, FrameFormGenerator)
    SET_GENERATOR(ClassName::wxPopupTransientWindow, PopupWinGenerator)
    SET_GENERATOR(ClassName::PanelForm, PanelFormGenerator)
    SET_GENERATOR(ClassName::wxWizard, WizardFormGenerator)

    SET_GENERATOR(ClassName::VerticalBoxSizer, BoxSizerGenerator)
    SET_GENERATOR(ClassName::wxBoxSizer, BoxSizerGenerator)
    SET_GENERATOR(ClassName::wxFlexGridSizer, FlexGridSizerGenerator)
    SET_GENERATOR(ClassName::wxGridBagSizer, GridBagSizerGenerator)
    SET_GENERATOR(ClassName::wxGridSizer, GridSizerGenerator)
    SET_GENERATOR(ClassName::wxStaticBoxSizer, StaticBoxSizerGenerator)
    SET_GENERATOR(ClassName::StaticCheckboxBoxSizer, StaticCheckboxBoxSizerGenerator)
    SET_GENERATOR(ClassName::StaticRadioBtnBoxSizer, StaticRadioBtnBoxSizerGenerator)
    SET_GENERATOR(ClassName::wxStdDialogButtonSizer, StdDialogButtonSizerGenerator)
    SET_GENERATOR(ClassName::wxWrapSizer, WrapSizerGenerator)
    SET_GENERATOR(ClassName::TextSizer, TextSizerGenerator)

    AddAllConstants();
}

// We put the list of constants in a separate header file because it's a huge list. It needs to be #included in this
// source module because this is the module that will #include all the wxWidgets headers that contain the
// definitions we need.

#include "add_constants.h"
