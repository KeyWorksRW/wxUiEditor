/////////////////////////////////////////////////////////////////////////////
// Purpose:   Initialize all generators
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_common.h"    // GeneratorLibrary -- Generator classes
#include "node_creator.h"  // NodeCreator

#include "dataview_widgets.h"  // DataViewCtrl -- wxDataView component classes
#include "grid_widgets.h"      // PropertyGridGenerator -- Grid component classes
#include "images_form.h"       // ImagesGenerator -- Embedded images generator
#include "menu_widgets.h"      // Menu component classes
#include "project.h"           // Project generator
#include "styled_text.h"       // StyledTextGenerator -- wxStyledText (scintilla) generate
#include "window_widgets.h"    // Splitter and Scroll component classes

#include "gen_activity.h"            // ActivityIndicatorGenerator -- wxActivityIndicator generator
#include "gen_animation.h"           // AnimationGenerator -- wxAnimationCtrl generator
#include "gen_aui_notebook.h"        // AuiNotebookGenerator -- wxAuiNotebook generator
#include "gen_aui_toolbar.h"         // AuiToolBarGenerator -- wxAuiToolBar generator
#include "gen_banner_window.h"       // BannerWindowGenerator -- wxBannerWindow generator
#include "gen_bitmap_combo.h"        // BitmapComboBoxGenerator -- wxBitmapComboBox generator
#include "gen_book_page.h"           // BookPageGenerator -- Book page generator
#include "gen_box_sizer.h"           // BoxSizerGenerator -- wxBoxSizer generator
#include "gen_button.h"              // ButtonGenerator -- wxButton generator
#include "gen_calendar_ctrl.h"       // CalendarCtrlGenerator -- wxCalendarCtrl generator
#include "gen_check_listbox.h"       // CheckListBoxGenerator -- wxCheckListBox generator
#include "gen_checkbox.h"            // CheckBoxGenerator -- wxCheckBox generator
#include "gen_choice.h"              // ChoiceGenerator -- wxChoice generator
#include "gen_choicebook.h"          // ChoicebookGenerator -- wxChoicebook generator
#include "gen_close_btn.h"           // CloseButtonGenerator -- wxBitmapButton::CreateCloseButton generator
#include "gen_clr_picker.h"          // ColourPickerGenerator -- wxColourPickerCtrl generator
#include "gen_cmd_link_btn.h"        // CommandLinkBtnGenerator -- wxCommandLinkButton generator
#include "gen_collapsible.h"         // CollapsiblePaneGenerator -- wxCollapsiblePane generator
#include "gen_combobox.h"            // ComboBoxGenerator -- wxComboBox generator
#include "gen_ctx_menu.h"            // CtxMenuGenerator -- generates function and includes
#include "gen_custom_ctrl.h"         // CustomControl -- Custom Control generator
#include "gen_date_picker.h"         // DatePickerCtrlGenerator -- wxDatePickerCtrl generator
#include "gen_dialog.h"              // DialogFormGenerator -- wxDialog generator
#include "gen_dir_ctrl.h"            // GenericDirCtrlGenerator -- wxGenericDirCtrl generator
#include "gen_dir_picker.h"          // DirPickerGenerator -- wxDirPickerCtrl generator
#include "gen_edit_listbox.h"        // EditListBoxGenerator -- wxEditableListBox generator
#include "gen_file_ctrl.h"           // FileCtrlGenerator -- wxFileCtrl generator
#include "gen_file_picker.h"         // FilePickerGenerator -- wxFilePickerCtrl generator
#include "gen_flexgrid_sizer.h"      // FlexGridSizerGenerator -- wxFlexGridSizer generator
#include "gen_font_picker.h"         // FontPickerGenerator -- wxFontPickerCtrl generator
#include "gen_frame.h"               // FrameFormGenerator -- wxFrame generator
#include "gen_gauge.h"               // GaugeGenerator -- wxGauge generator
#include "gen_grid.h"                // GridGenerator -- wxGrid generator
#include "gen_grid_sizer.h"          // GridSizerGenerator -- wxGridSizer generator
#include "gen_gridbag_sizer.h"       // GridBagSizerGenerator -- wxGridBagSizer generator
#include "gen_html_listbox.h"        // HtmlListBoxGenerator -- wxSimpleHtmlListBox generator
#include "gen_html_window.h"         // HtmlWindowGenerator -- wxHtmlWindow generator
#include "gen_hyperlink.h"           // HyperlinkGenerator -- wxHyperlinkCtrl generator
#include "gen_infobar.h"             // InfoBarGenerator -- wxInfoBar generator
#include "gen_listbook.h"            // ListbookGenerator -- wxListbook generator
#include "gen_listbox.h"             // ListBoxGenerator -- wxListBox generator
#include "gen_listview.h"            // ListViewGenerator -- wxListView generator
#include "gen_notebook.h"            // NotebookGenerator -- wxNotebook generator
#include "gen_page_ctrl.h"           // PageCtrlGenerator -- Page control generator
#include "gen_panel.h"               // PanelGenerator -- wxPanel generator
#include "gen_panel_form.h"          // PanelFormGenerator -- wxPanel Form generator
#include "gen_popup_trans_win.h"     // PopupWinGenerator -- wxPopupTransientWindow generator
#include "gen_prop_grid.h"           // PropertyGridGenerator -- wxPropertyGrid generator
#include "gen_prop_item.h"           // PropertyGridItemGenerator -- PropertyGrid/Manager Item generator
#include "gen_radio_box.h"           // RadioBoxGenerator -- wxRadioBox generator
#include "gen_radio_btn.h"           // RadioButtonGenerator -- Radio button and Radio box component classes
#include "gen_rearrange.h"           // RearrangeCtrlGenerator -- wxRearrangeCtrl generator
#include "gen_rich_text.h"           // RichTextCtrlGenerator -- wxRichTextCtrl generator
#include "gen_scrollbar.h"           // ScrollBarGenerator -- wxScrollBar generator
#include "gen_search_ctrl.h"         // SearchCtrlGenerator -- wxSearchCtrl generator
#include "gen_simplebook.h"          // SimplebookGenerator -- wxSimplebook generator
#include "gen_slider.h"              // SliderGenerator -- wxSlider generator
#include "gen_spacer_sizer.h"        // SpacerGenerator -- Add space to sizer generator
#include "gen_spin_btn.h"            // SpinButtonGenerator -- wxSpinButton generator
#include "gen_spin_ctrl.h"           // SpinCtrlGenerator -- Spin and ScrollBar component classes
#include "gen_split_win.h"           // SplitterWindowGenerator -- wxSplitterWindow generator
#include "gen_statchkbox_sizer.h"    // StaticCheckboxBoxSizerGenerator -- wxStaticBoxSizer with wxCheckBox generator
#include "gen_static_bmp.h"          // StaticBitmapGenerator -- wxStaticBitmap generator
#include "gen_static_box.h"          // StaticBoxGenerator -- wxStaticBox generator
#include "gen_static_line.h"         // StaticLineGenerator -- wxStaticLine generator
#include "gen_static_text.h"         // StaticTextGenerator -- wxStaticText generator
#include "gen_staticbox_sizer.h"     // StaticBoxSizerGenerator -- wxStaticBoxSizer generator
#include "gen_statradiobox_sizer.h"  // StaticRadioBtnBoxSizerGenerator -- wxStaticBoxSizer with wxRadioButton generator
#include "gen_status_bar.h"          // StatusBarGenerator -- wxStatusBar generator
#include "gen_std_dlgbtn_sizer.h"    // StdDialogButtonSizerGenerator -- wxStdDialogButtonSizer generator
#include "gen_text_ctrl.h"           // TextCtrlGenerator -- wxTextCtrl generator
#include "gen_text_sizer.h"          // TextSizerGenerator -- wxTextSizerWrapper generator
#include "gen_time_picker.h"         // TimePickerCtrlGenerator -- wxTimePickerCtrl generator
#include "gen_toggle_btn.h"          // ToggleButtonGenerator -- wxToggleButton generator
#include "gen_toolbar.h"             // wxToolBar generator
#include "gen_toolbook.h"            // ToolbookGenerator -- wxToolbook generator
#include "gen_tree_ctrl.h"           // TreeCtrlGenerator -- wxTreeCtrl generator
#include "gen_tree_list.h"           // TreeListCtrlGenerator -- wxTreeCtrl component classes
#include "gen_treebook.h"            // TreebookGenerator -- wxTreebook generator
#include "gen_web_view.h"            // WebViewGenerator -- wxWebView generator
#include "gen_wizard.h"              // WizardFormGenerator -- wxWizard generator
#include "gen_wrap_sizer.h"          // WrapSizerGenerator -- wxGridSizer generator

#include "gen_ribbon_bar.h"      // RibbonBarFormGenerator -- wxRibbonBar -- form and regular
#include "gen_ribbon_button.h"   // wxRibbonButtonBar generator
#include "gen_ribbon_gallery.h"  // RibbonGalleryGenerator -- wxRibbonGallery generator
#include "gen_ribbon_page.h"     // RibbonPageGenerator -- wxRibbonPage and wxRibbonPanel generators
#include "gen_ribbon_tool.h"     // RibbonToolBarGenerator -- wxRibbonButtonBar generator

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
    SET_GENERATOR(gen_CloseButton, CloseButtonGenerator)
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
    SET_GENERATOR(gen_wxScrolledCanvas, ScrolledCanvasGenerator)
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
    SET_GENERATOR(gen_wxEditableListBox, EditListBoxGenerator)
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
    SET_GENERATOR(gen_auitool_label, AuiToolLabelGenerator)
    SET_GENERATOR(gen_auitool_spacer, AuiToolSpacerGenerator)
    SET_GENERATOR(gen_auitool_stretchable, AuiToolStretchSpacerGenerator)
    SET_GENERATOR(gen_ToolBar, ToolBarFormGenerator)
    SET_GENERATOR(gen_wxToolBar, ToolBarGenerator)
    SET_GENERATOR(gen_tool, ToolGenerator)
    SET_GENERATOR(gen_tool_dropdown, ToolDropDownGenerator)
    SET_GENERATOR(gen_toolSeparator, ToolSeparatorGenerator)
    SET_GENERATOR(gen_toolStretchable, ToolStretchableGenerator)

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
    SET_GENERATOR(gen_wxTreeListCtrl, TreeListCtrlGenerator)
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
    SET_GENERATOR(gen_wxStaticBox, StaticBoxGenerator)
    SET_GENERATOR(gen_wxStaticBoxSizer, StaticBoxSizerGenerator)
    SET_GENERATOR(gen_StaticCheckboxBoxSizer, StaticCheckboxBoxSizerGenerator)
    SET_GENERATOR(gen_StaticRadioBtnBoxSizer, StaticRadioBtnBoxSizerGenerator)
    SET_GENERATOR(gen_wxStdDialogButtonSizer, StdDialogButtonSizerGenerator)
    SET_GENERATOR(gen_wxWrapSizer, WrapSizerGenerator)
    SET_GENERATOR(gen_TextSizer, TextSizerGenerator)

    SET_GENERATOR(gen_CustomControl, CustomControl)

    SET_GENERATOR(gen_Project, ProjectGenerator)
    SET_GENERATOR(gen_folder, FolderGenerator)
    SET_GENERATOR(gen_sub_folder, SubFolderGenerator)

    AddAllConstants();
}
