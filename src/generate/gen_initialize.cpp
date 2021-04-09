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
    SET_GENERATOR(Class::BookPage, BookPageGenerator)

    SET_GENERATOR(Class::wxWizardPageSimple, WizardPageGenerator)
    SET_GENERATOR(Class::wxBannerWindow, BannerWindowGenerator)
    SET_GENERATOR(Class::wxBitmapComboBox, BitmapComboBoxGenerator)
    SET_GENERATOR(Class::wxActivityIndicator, ActivityIndicatorGenerator)
    SET_GENERATOR(Class::wxButton, ButtonGenerator)
    SET_GENERATOR(Class::wxCheckBox, CheckBoxGenerator)
    SET_GENERATOR(Class::Check3State, Check3StateGenerator)
    SET_GENERATOR(Class::wxCheckListBox, CheckListBoxGenerator)
    SET_GENERATOR(Class::wxChoice, ChoiceGenerator)
    SET_GENERATOR(Class::wxChoicebook, ChoicebookGenerator)
    SET_GENERATOR(Class::wxCollapsiblePane, CollapsiblePaneGenerator)
    SET_GENERATOR(Class::wxComboBox, ComboBoxGenerator)
    SET_GENERATOR(Class::wxCommandLinkButton, CommandLinkBtnGenerator)
    SET_GENERATOR(Class::wxGauge, GaugeGenerator)
    SET_GENERATOR(Class::wxHtmlWindow, HtmlWindowGenerator)
    SET_GENERATOR(Class::wxHyperlinkCtrl, HyperlinkGenerator)
    SET_GENERATOR(Class::wxInfoBar, InfoBarGenerator)
    SET_GENERATOR(Class::wxListBox, ListBoxGenerator)
    SET_GENERATOR(Class::wxSimpleHtmlListBox, HtmlListBoxGenerator)
    SET_GENERATOR(Class::wxListbook, ListbookGenerator)
    SET_GENERATOR(Class::wxNotebook, NotebookGenerator)
    SET_GENERATOR(Class::wxToolbook, ToolbookGenerator)
    SET_GENERATOR(Class::wxTreebook, TreebookGenerator)
    SET_GENERATOR(Class::wxPanel, PanelGenerator)
    SET_GENERATOR(Class::wxRadioBox, RadioBoxGenerator)
    SET_GENERATOR(Class::wxRadioButton, RadioButtonGenerator)
    SET_GENERATOR(Class::wxRichTextCtrl, RichTextCtrlGenerator)
    SET_GENERATOR(Class::wxScrollBar, ScrollBarGenerator)
    SET_GENERATOR(Class::wxScrolledWindow, ScrolledWindowGenerator)
    SET_GENERATOR(Class::wxSimplebook, SimplebookGenerator)
    SET_GENERATOR(Class::wxSlider, SliderGenerator)
    SET_GENERATOR(Class::wxSplitterWindow, SplitterWindowGenerator)
    SET_GENERATOR(Class::wxStaticBitmap, StaticBitmapGenerator)
    SET_GENERATOR(Class::wxStaticLine, StaticLineGenerator)
    SET_GENERATOR(Class::wxStaticText, StaticTextGenerator)
    SET_GENERATOR(Class::wxStatusBar, StatusBarGenerator)
    SET_GENERATOR(Class::wxStyledTextCtrl, StyledTextGenerator)
    SET_GENERATOR(Class::wxTextCtrl, TextCtrlGenerator)
    SET_GENERATOR(Class::wxToggleButton, ToggleButtonGenerator)
    SET_GENERATOR(Class::wxCalendarCtrl, CalendarCtrlGenerator)
    SET_GENERATOR(Class::wxFileCtrl, FileCtrlGenerator)
    SET_GENERATOR(Class::wxGenericDirCtrl, GenericDirCtrlGenerator)
    SET_GENERATOR(Class::wxSearchCtrl, SearchCtrlGenerator)
    SET_GENERATOR(Class::wxListView, ListViewGenerator)
    SET_GENERATOR(Class::wxGrid, GridGenerator)

    SET_GENERATOR(Class::wxColourPickerCtrl, ColourPickerGenerator)
    SET_GENERATOR(Class::wxDatePickerCtrl, DatePickerCtrlGenerator)
    SET_GENERATOR(Class::wxDirPickerCtrl, DirPickerGenerator)
    SET_GENERATOR(Class::wxFilePickerCtrl, FilePickerGenerator)
    SET_GENERATOR(Class::wxFontPickerCtrl, FontPickerGenerator)
    SET_GENERATOR(Class::wxTimePickerCtrl, TimePickerCtrlGenerator)

    SET_GENERATOR(Class::wxMenuBar, MenuBarGenerator)
    SET_GENERATOR(Class::MenuBar, MenuBarFormGenerator)
    SET_GENERATOR(Class::wxMenu, MenuGenerator)
    SET_GENERATOR(Class::submenu, SubMenuGenerator)
    SET_GENERATOR(Class::wxMenuItem, MenuItemGenerator)
    SET_GENERATOR(Class::separator, SeparatorGenerator)

    SET_GENERATOR(Class::wxDataViewCtrl, DataViewCtrl)
    SET_GENERATOR(Class::wxDataViewListCtrl, DataViewListCtrl)
    SET_GENERATOR(Class::wxDataViewTreeCtrl, DataViewTreeCtrl)
    SET_GENERATOR(Class::dataViewColumn, DataViewColumn)
    SET_GENERATOR(Class::dataViewListColumn, DataViewListColumn)

    SET_GENERATOR(Class::wxPropertyGrid, PropertyGridGenerator)
    SET_GENERATOR(Class::wxPropertyGridManager, PropertyGridManagerGenerator)
    SET_GENERATOR(Class::propGridItem, PropertyGridItemGenerator)
    SET_GENERATOR(Class::propGridPage, PropertyGridPageGenerator)

    SET_GENERATOR(Class::wxSpinButton, SpinButtonGenerator)
    SET_GENERATOR(Class::wxSpinCtrl, SpinCtrlGenerator)
    SET_GENERATOR(Class::wxSpinCtrlDouble, SpinCtrlDoubleGenerator)

    SET_GENERATOR(Class::spacer, SpacerGenerator)

    SET_GENERATOR(Class::ToolBar, ToolBarFormGenerator)
    SET_GENERATOR(Class::wxToolBar, ToolBarGenerator)
    SET_GENERATOR(Class::tool, ToolGenerator)
    SET_GENERATOR(Class::toolSeparator, ToolSeparatorGenerator)
    SET_GENERATOR(Class::wxRibbonBar, RibbonBarGenerator)
    SET_GENERATOR(Class::wxRibbonPage, RibbonPageGenerator)
    SET_GENERATOR(Class::wxRibbonPanel, RibbonPanelGenerator)
    SET_GENERATOR(Class::wxRibbonButtonBar, RibbonButtonBarGenerator)
    SET_GENERATOR(Class::wxRibbonToolBar, RibbonToolBarGenerator)
    SET_GENERATOR(Class::wxRibbonGallery, RibbonGalleryGenerator)

    SET_GENERATOR(Class::ribbonButton, RibbonButtonGenerator)
    SET_GENERATOR(Class::ribbonTool, RibbonToolGenerator)
    SET_GENERATOR(Class::ribbonGalleryItem, RibbonGalleryItemGenerator)

    SET_GENERATOR(Class::wxTreeCtrl, TreeCtrlGenerator)
    SET_GENERATOR(Class::wxTreeListCtrl, TreeListViewGenerator)
    SET_GENERATOR(Class::TreeListCtrlColumn, TreeListCtrlColumnGenerator)

    SET_GENERATOR(Class::wxDialog, DialogFormGenerator)
    SET_GENERATOR(Class::wxFrame, FrameFormGenerator)
    SET_GENERATOR(Class::wxPopupTransientWindow, PopupWinGenerator)
    SET_GENERATOR(Class::PanelForm, PanelFormGenerator)
    SET_GENERATOR(Class::wxWizard, WizardFormGenerator)

    SET_GENERATOR(Class::VerticalBoxSizer, BoxSizerGenerator)
    SET_GENERATOR(Class::wxBoxSizer, BoxSizerGenerator)
    SET_GENERATOR(Class::wxFlexGridSizer, FlexGridSizerGenerator)
    SET_GENERATOR(Class::wxGridBagSizer, GridBagSizerGenerator)
    SET_GENERATOR(Class::wxGridSizer, GridSizerGenerator)
    SET_GENERATOR(Class::wxStaticBoxSizer, StaticBoxSizerGenerator)
    SET_GENERATOR(Class::StaticCheckboxBoxSizer, StaticCheckboxBoxSizerGenerator)
    SET_GENERATOR(Class::StaticRadioBtnBoxSizer, StaticRadioBtnBoxSizerGenerator)
    SET_GENERATOR(Class::wxStdDialogButtonSizer, StdDialogButtonSizerGenerator)
    SET_GENERATOR(Class::wxWrapSizer, WrapSizerGenerator)
    SET_GENERATOR(Class::TextSizer, TextSizerGenerator)

    AddAllConstants();
}

// We put the list of constants in a separate header file because it's a huge list. It needs to be #included in this
// source module because this is the module that will #include all the wxWidgets headers that contain the
// definitions we need.

#include "add_constants.h"
