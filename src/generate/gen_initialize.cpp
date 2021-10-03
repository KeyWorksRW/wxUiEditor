/////////////////////////////////////////////////////////////////////////////
// Purpose:   Initialize all generators
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

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
#include "images_form.h"       // ImagesGenerator -- Embedded images generator
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

#define SET_GENERATOR(name, generator)                        \
    {                                                         \
        ASSERT(get_declaration(name));                        \
        get_declaration(name)->SetGenerator(new generator()); \
    }

void NodeCreator::InitGenerators()
{
    SET_GENERATOR(gen_BookPage, BookPageGenerator)
    SET_GENERATOR(gen_PageCtrl, PageCtrlGenerator)

    SET_GENERATOR(gen_wxWizardPageSimple, WizardPageGenerator)
    SET_GENERATOR(gen_wxBannerWindow, BannerWindowGenerator)
    SET_GENERATOR(gen_wxBitmapComboBox, BitmapComboBoxGenerator)
    SET_GENERATOR(gen_wxActivityIndicator, ActivityIndicatorGenerator)
    SET_GENERATOR(gen_wxAnimationCtrl, AnimationGenerator)
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
    SET_GENERATOR(gen_wxWebView, WebViewGenerator)
    SET_GENERATOR(gen_wxInfoBar, InfoBarGenerator)
    SET_GENERATOR(gen_wxListBox, ListBoxGenerator)
    SET_GENERATOR(gen_wxSimpleHtmlListBox, HtmlListBoxGenerator)
    SET_GENERATOR(gen_wxAuiNotebook, AuiNotebookGenerator)
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
    SET_GENERATOR(gen_PopupMenu, PopupMenuGenerator)
    SET_GENERATOR(gen_wxMenu, MenuGenerator)
    SET_GENERATOR(gen_submenu, SubMenuGenerator)
    SET_GENERATOR(gen_wxMenuItem, MenuItemGenerator)
    SET_GENERATOR(gen_separator, SeparatorGenerator)
    SET_GENERATOR(gen_wxContextMenuEvent, CtxMenuGenerator)

    SET_GENERATOR(gen_Images, ImagesGenerator)
    SET_GENERATOR(gen_embedded_image, EmbeddedImageGenerator)

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

    SET_GENERATOR(gen_wxAuiToolBar, AuiToolBarGenerator)
    SET_GENERATOR(gen_auitool, AuiToolGenerator)
    SET_GENERATOR(gen_ToolBar, ToolBarFormGenerator)
    SET_GENERATOR(gen_wxToolBar, ToolBarGenerator)
    SET_GENERATOR(gen_tool, ToolGenerator)
    SET_GENERATOR(gen_toolSeparator, ToolSeparatorGenerator)

    SET_GENERATOR(gen_RibbonBar, RibbonBarFormGenerator)
    SET_GENERATOR(gen_wxRibbonBar, RibbonBarGenerator)
    SET_GENERATOR(gen_wxRibbonPage, RibbonPageGenerator)
    SET_GENERATOR(gen_wxRibbonPanel, RibbonPanelGenerator)
    SET_GENERATOR(gen_wxRibbonButtonBar, RibbonButtonBarGenerator)
    SET_GENERATOR(gen_wxRibbonToolBar, RibbonToolBarGenerator)
    SET_GENERATOR(gen_ribbonSeparator, ToolSeparatorGenerator)
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

    SET_GENERATOR(gen_CustomControl, CustomControl)

    AddAllConstants();
}
