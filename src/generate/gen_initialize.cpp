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

// Note that classes are created via new but never deleted. Technically, that's a memory leak, but it doesn't really
// matter because they are only created when the application is started, and need to exist until the program exits.
// There's no real advantage to deleting each class during exit since the OS will free that memory automatically.

#define CREATE_GENERATOR(node_class_name, class)                               \
    {                                                                          \
        if (auto class_info = GetNodeDeclaration(node_class_name); class_info) \
        {                                                                      \
            auto generator = new class();                                      \
            class_info->SetGenerator(generator);                               \
        }                                                                      \
    }

void NodeCreator::InitGenerators()
{
    CREATE_GENERATOR("BookPage", BookPageGenerator)
    CREATE_GENERATOR("wxWizardPageSimple", WizardPageGenerator)
    CREATE_GENERATOR("wxBannerWindow", BannerWindowGenerator)
    CREATE_GENERATOR("wxBitmapComboBox", BitmapComboBoxGenerator)
    CREATE_GENERATOR("wxActivityIndicator", ActivityIndicatorGenerator)
    CREATE_GENERATOR("wxButton", ButtonGenerator)
    CREATE_GENERATOR("wxCheckBox", CheckBoxGenerator)
    CREATE_GENERATOR("Check3State", Check3StateGenerator)
    CREATE_GENERATOR("wxCheckListBox", CheckListBoxGenerator)
    CREATE_GENERATOR("wxChoice", ChoiceGenerator)
    CREATE_GENERATOR("wxChoicebook", ChoicebookGenerator)
    CREATE_GENERATOR("wxCollapsiblePane", CollapsiblePaneGenerator)
    CREATE_GENERATOR("wxComboBox", ComboBoxGenerator)
    CREATE_GENERATOR("wxCommandLinkButton", CommandLinkBtnGenerator)
    CREATE_GENERATOR("wxGauge", GaugeGenerator)
    CREATE_GENERATOR("wxHtmlWindow", HtmlWindowGenerator)
    CREATE_GENERATOR("wxHyperlinkCtrl", HyperlinkGenerator)
    CREATE_GENERATOR("wxInfoBar", InfoBarGenerator)
    CREATE_GENERATOR("wxListBox", ListBoxGenerator)
    CREATE_GENERATOR("wxSimpleHtmlListBox", HtmlListBoxGenerator)
    CREATE_GENERATOR("wxListbook", ListbookGenerator)
    CREATE_GENERATOR("wxNotebook", NotebookGenerator)
    CREATE_GENERATOR("wxToolbook", ToolbookGenerator)
    CREATE_GENERATOR("wxTreebook", TreebookGenerator)
    CREATE_GENERATOR("wxPanel", PanelGenerator)
    CREATE_GENERATOR("wxRadioBox", RadioBoxGenerator)
    CREATE_GENERATOR("wxRadioButton", RadioButtonGenerator)
    CREATE_GENERATOR("wxRichTextCtrl", RichTextCtrlGenerator)
    CREATE_GENERATOR("wxScrollBar", ScrollBarGenerator)
    CREATE_GENERATOR("wxScrolledWindow", ScrolledWindowGenerator)
    CREATE_GENERATOR("wxSimplebook", SimplebookGenerator)
    CREATE_GENERATOR("wxSlider", SliderGenerator)
    CREATE_GENERATOR("wxSplitterWindow", SplitterWindowGenerator)
    CREATE_GENERATOR("wxStaticBitmap", StaticBitmapGenerator)
    CREATE_GENERATOR("wxStaticLine", StaticLineGenerator)
    CREATE_GENERATOR("wxStaticText", StaticTextGenerator)
    CREATE_GENERATOR("wxStatusBar", StatusBarGenerator)
    CREATE_GENERATOR("wxStyledTextCtrl", StyledTextGenerator)
    CREATE_GENERATOR("wxTextCtrl", TextCtrlGenerator)
    CREATE_GENERATOR("wxToggleButton", ToggleButtonGenerator)
    CREATE_GENERATOR("wxCalendarCtrl", CalendarCtrlGenerator)
    CREATE_GENERATOR("wxFileCtrl", FileCtrlGenerator)
    CREATE_GENERATOR("wxGenericDirCtrl", GenericDirCtrlGenerator)
    CREATE_GENERATOR("wxSearchCtrl", SearchCtrlGenerator)
    CREATE_GENERATOR("wxListView", ListViewGenerator)
    CREATE_GENERATOR("wxGrid", GridGenerator)

    CREATE_GENERATOR("wxColourPickerCtrl", ColourPickerGenerator)
    CREATE_GENERATOR("wxDatePickerCtrl", DatePickerCtrlGenerator)
    CREATE_GENERATOR("wxDirPickerCtrl", DirPickerGenerator)
    CREATE_GENERATOR("wxFilePickerCtrl", FilePickerGenerator)
    CREATE_GENERATOR("wxFontPickerCtrl", FontPickerGenerator)
    CREATE_GENERATOR("wxTimePickerCtrl", TimePickerCtrlGenerator)

    CREATE_GENERATOR("wxMenuBar", MenuBarGenerator)
    CREATE_GENERATOR("MenuBar", MenuBarFormGenerator)
    CREATE_GENERATOR("wxMenu", MenuGenerator)
    CREATE_GENERATOR("submenu", SubMenuGenerator)
    CREATE_GENERATOR("wxMenuItem", MenuItemGenerator)
    CREATE_GENERATOR("separator", SeparatorGenerator)

    CREATE_GENERATOR("wxDataViewCtrl", DataViewCtrl)
    CREATE_GENERATOR("wxDataViewListCtrl", DataViewListCtrl)
    CREATE_GENERATOR("wxDataViewTreeCtrl", DataViewTreeCtrl)
    CREATE_GENERATOR("dataViewColumn", DataViewColumn)
    CREATE_GENERATOR("dataViewListColumn", DataViewListColumn)

    CREATE_GENERATOR("wxPropertyGrid", PropertyGridGenerator)
    CREATE_GENERATOR("wxPropertyGridManager", PropertyGridManagerGenerator)
    CREATE_GENERATOR("propGridItem", PropertyGridItemGenerator)
    CREATE_GENERATOR("propGridPage", PropertyGridPageGenerator)

    CREATE_GENERATOR("wxSpinButton", SpinButtonGenerator)
    CREATE_GENERATOR("wxSpinCtrl", SpinCtrlGenerator)
    CREATE_GENERATOR("wxSpinCtrlDouble", SpinCtrlDoubleGenerator)

    CREATE_GENERATOR("spacer", SpacerGenerator)

    CREATE_GENERATOR("ToolBar", ToolBarFormGenerator)
    CREATE_GENERATOR("wxToolBar", ToolBarGenerator)
    CREATE_GENERATOR("tool", ToolGenerator)
    CREATE_GENERATOR("toolSeparator", ToolSeparatorGenerator)
    CREATE_GENERATOR("wxRibbonBar", RibbonBarGenerator)
    CREATE_GENERATOR("wxRibbonPage", RibbonPageGenerator)
    CREATE_GENERATOR("wxRibbonPanel", RibbonPanelGenerator)
    CREATE_GENERATOR("wxRibbonButtonBar", RibbonButtonBarGenerator)
    CREATE_GENERATOR("wxRibbonToolBar", RibbonToolBarGenerator)
    CREATE_GENERATOR("wxRibbonGallery", RibbonGalleryGenerator)

    CREATE_GENERATOR("ribbonButton", RibbonButtonGenerator)
    CREATE_GENERATOR("ribbonTool", RibbonToolGenerator)
    CREATE_GENERATOR("ribbonGalleryItem", RibbonGalleryItemGenerator)

    CREATE_GENERATOR("wxTreeCtrl", TreeCtrlGenerator)
    CREATE_GENERATOR("wxTreeListCtrl", TreeListViewGenerator)
    CREATE_GENERATOR("TreeListCtrlColumn", TreeListCtrlColumnGenerator)

    CREATE_GENERATOR("wxDialog", DialogFormGenerator)
    CREATE_GENERATOR("wxFrame", FrameFormGenerator)
    CREATE_GENERATOR("PanelForm", PanelFormGenerator)
    CREATE_GENERATOR("wxWizard", WizardFormGenerator)

    CREATE_GENERATOR("VerticalBoxSizer", BoxSizerGenerator)
    CREATE_GENERATOR("wxBoxSizer", BoxSizerGenerator)
    CREATE_GENERATOR("wxFlexGridSizer", FlexGridSizerGenerator)
    CREATE_GENERATOR("wxGridBagSizer", GridBagSizerGenerator)
    CREATE_GENERATOR("wxGridSizer", GridSizerGenerator)
    CREATE_GENERATOR("wxStaticBoxSizer", StaticBoxSizerGenerator)
    CREATE_GENERATOR("StaticCheckboxBoxSizer", StaticCheckboxBoxSizerGenerator)
    CREATE_GENERATOR("StaticRadioBtnBoxSizer", StaticRadioBtnBoxSizerGenerator)
    CREATE_GENERATOR("wxStdDialogButtonSizer", StdDialogButtonSizerGenerator)
    CREATE_GENERATOR("wxWrapSizer", WrapSizerGenerator)
    CREATE_GENERATOR("TextSizer", TextSizerGenerator)

    AddAllConstants();
}

// We put the list of constants in a separate header file because it's a huge list. It needs to be #included in this
// source module because this is the module that will #include all the wxWidgets headers that contain the
// definitions we need.

#include "add_constants.h"
