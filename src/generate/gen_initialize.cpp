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

#define ADD_GENERATOR(name, class, type)                            \
    {                                                               \
        auto generator = new class();                               \
        generator->SetNodeType(type);                               \
        if (auto class_info = GetNodeDeclaration(name); class_info) \
        {                                                           \
            class_info->SetGenerator(generator);                    \
        }                                                           \
    }

void NodeCreator::InitGenerators()
{
    ADD_GENERATOR("BookPage", BookPageGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxWizardPageSimple", WizardPageGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxBannerWindow", BannerWindowGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxBitmapComboBox", BitmapComboBoxGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxActivityIndicator", ActivityIndicatorGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxButton", ButtonGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxCheckBox", CheckBoxGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("Check3State", Check3StateGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxCheckListBox", CheckListBoxGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxChoice", ChoiceGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxChoicebook", ChoicebookGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxCollapsiblePane", CollapsiblePaneGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxComboBox", ComboBoxGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxCommandLinkButton", CommandLinkBtnGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxGauge", GaugeGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxHtmlWindow", HtmlWindowGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxHyperlinkCtrl", HyperlinkGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxInfoBar", InfoBarGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxListBox", ListBoxGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxSimpleHtmlListBox", HtmlListBoxGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxListbook", ListbookGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxNotebook", NotebookGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxPanel", PanelGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxRadioBox", RadioBoxGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxRadioButton", RadioButtonGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxRichTextCtrl", RichTextCtrlGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxScrollBar", ScrollBarGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxScrolledWindow", ScrolledWindowGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxSimplebook", SimplebookGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxSlider", SliderGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxSplitterWindow", SplitterWindowGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxStaticBitmap", StaticBitmapGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxStaticLine", StaticLineGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxStaticText", StaticTextGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxStatusBar", StatusBarGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxStyledTextCtrl", StyledTextGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxTextCtrl", TextCtrlGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxToggleButton", ToggleButtonGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxCalendarCtrl", CalendarCtrlGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxFileCtrl", FileCtrlGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxGenericDirCtrl", GenericDirCtrlGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxSearchCtrl", SearchCtrlGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxListView", ListViewGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxGrid", GridGenerator, GENERATOR_TYPE_WINDOW)

    ADD_GENERATOR("wxColourPickerCtrl", ColourPickerGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxDatePickerCtrl", DatePickerCtrlGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxDirPickerCtrl", DirPickerGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxFilePickerCtrl", FilePickerGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxFontPickerCtrl", FontPickerGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxTimePickerCtrl", TimePickerCtrlGenerator, GENERATOR_TYPE_WINDOW)

    ADD_GENERATOR("wxMenuBar", MenuBarGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("MenuBar", MenuBarFormGenerator, GENERATOR_TYPE_ABSTRACT)
    ADD_GENERATOR("wxMenu", MenuGenerator, GENERATOR_TYPE_ABSTRACT)
    ADD_GENERATOR("submenu", SubMenuGenerator, GENERATOR_TYPE_ABSTRACT)
    ADD_GENERATOR("wxMenuItem", MenuItemGenerator, GENERATOR_TYPE_ABSTRACT)
    ADD_GENERATOR("separator", SeparatorGenerator, GENERATOR_TYPE_ABSTRACT)

    ADD_GENERATOR("wxDataViewCtrl", DataViewCtrl, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxDataViewListCtrl", DataViewListCtrl, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxDataViewTreeCtrl", DataViewTreeCtrl, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("dataViewColumn", DataViewColumn, GENERATOR_TYPE_ABSTRACT)
    ADD_GENERATOR("dataViewListColumn", DataViewListColumn, GENERATOR_TYPE_ABSTRACT)

    ADD_GENERATOR("wxPropertyGrid", PropertyGridGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxPropertyGridManager", PropertyGridManagerGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("propGridItem", PropertyGridItemGenerator, GENERATOR_TYPE_ABSTRACT)
    ADD_GENERATOR("propGridPage", PropertyGridPageGenerator, GENERATOR_TYPE_ABSTRACT)

    ADD_GENERATOR("wxSpinButton", SpinButtonGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxSpinCtrl", SpinCtrlGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxSpinCtrlDouble", SpinCtrlDoubleGenerator, GENERATOR_TYPE_WINDOW)

    ADD_GENERATOR("spacer", SpacerGenerator, GENERATOR_TYPE_ABSTRACT)

    ADD_GENERATOR("ToolBar", ToolBarFormGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxToolBar", ToolBarGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("tool", ToolGenerator, GENERATOR_TYPE_ABSTRACT)
    ADD_GENERATOR("toolSeparator", ToolSeparatorGenerator, GENERATOR_TYPE_ABSTRACT)
    ADD_GENERATOR("wxRibbonBar", RibbonBarGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxRibbonPage", RibbonPageGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxRibbonPanel", RibbonPanelGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxRibbonButtonBar", RibbonButtonBarGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxRibbonToolBar", RibbonToolBarGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxRibbonGallery", RibbonGalleryGenerator, GENERATOR_TYPE_WINDOW)

    ADD_GENERATOR("ribbonButton", RibbonButtonGenerator, GENERATOR_TYPE_ABSTRACT)
    ADD_GENERATOR("ribbonTool", RibbonToolGenerator, GENERATOR_TYPE_ABSTRACT)
    ADD_GENERATOR("ribbonGalleryItem", RibbonGalleryItemGenerator, GENERATOR_TYPE_ABSTRACT)

    ADD_GENERATOR("wxTreeCtrl", TreeCtrlGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("wxTreeListCtrl", TreeListViewGenerator, GENERATOR_TYPE_WINDOW)
    ADD_GENERATOR("TreeListCtrlColumn", TreeListCtrlColumnGenerator, GENERATOR_TYPE_ABSTRACT)

    ADD_GENERATOR("wxDialog", DialogFormGenerator, GENERATOR_TYPE_ABSTRACT)
    ADD_GENERATOR("wxFrame", FrameFormGenerator, GENERATOR_TYPE_ABSTRACT)
    ADD_GENERATOR("PanelForm", PanelFormGenerator, GENERATOR_TYPE_ABSTRACT)
    ADD_GENERATOR("wxWizard", WizardFormGenerator, GENERATOR_TYPE_ABSTRACT)

    ADD_GENERATOR("VerticalBoxSizer", BoxSizerGenerator, GENERATOR_TYPE_SIZER)
    ADD_GENERATOR("wxBoxSizer", BoxSizerGenerator, GENERATOR_TYPE_SIZER)
    ADD_GENERATOR("wxFlexGridSizer", FlexGridSizerGenerator, GENERATOR_TYPE_SIZER)
    ADD_GENERATOR("wxGridBagSizer", GridBagSizerGenerator, GENERATOR_TYPE_SIZER)
    ADD_GENERATOR("wxGridSizer", GridSizerGenerator, GENERATOR_TYPE_SIZER)
    ADD_GENERATOR("wxStaticBoxSizer", StaticBoxSizerGenerator, GENERATOR_TYPE_SIZER)
    ADD_GENERATOR("wxStdDialogButtonSizer", StdDialogButtonSizerGenerator, GENERATOR_TYPE_SIZER)
    ADD_GENERATOR("wxWrapSizer", WrapSizerGenerator, GENERATOR_TYPE_SIZER)
    ADD_GENERATOR("TextSizer", TextSizerGenerator, GENERATOR_TYPE_SIZER)

    AddAllConstants();
}

// We put the list of constants in a separate header file because it's a huge list. It needs to be #included in this
// source module because this is the module that will #include all the wxWidgets headers that contain the
// definitions we need.

#include "add_constants.h"
