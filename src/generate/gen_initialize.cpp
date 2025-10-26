/////////////////////////////////////////////////////////////////////////////
// Purpose:   Initialize all generators
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "node_creator.h"  // NodeCreator

#include "dataview_widgets.h"  // DataViewCtrl -- wxDataView component classes
#include "gen_data_list.h"     // DataGenerator -- Data List generator
#include "gen_images_list.h"   // ImagesGenerator -- Embedded images generator
#include "gen_project.h"       // Project generator
#include "gen_styled_text.h"   // StyledTextGenerator -- wxStyledText (scintilla) generate
#include "menu_widgets.h"      // Menu component classes
#include "window_widgets.h"    // Splitter and Scroll component classes

#include "gen_activity.h"        // ActivityIndicatorGenerator -- wxActivityIndicator generator
#include "gen_animation.h"       // AnimationGenerator -- wxAnimationCtrl generator
#include "gen_aui_notebook.h"    // AuiNotebookGenerator -- wxAuiNotebook generator
#include "gen_aui_toolbar.h"     // AuiToolBarGenerator -- wxAuiToolBar generator
#include "gen_banner_window.h"   // BannerWindowGenerator -- wxBannerWindow generator
#include "gen_bitmap_combo.h"    // BitmapComboBoxGenerator -- wxBitmapComboBox generator
#include "gen_book_page.h"       // BookPageGenerator -- Book page generator
#include "gen_box_sizer.h"       // BoxSizerGenerator -- wxBoxSizer generator
#include "gen_button.h"          // ButtonGenerator -- wxButton generator
#include "gen_calendar_ctrl.h"   // CalendarCtrlGenerator -- wxCalendarCtrl generator
#include "gen_check_listbox.h"   // CheckListBoxGenerator -- wxCheckListBox generator
#include "gen_checkbox.h"        // CheckBoxGenerator -- wxCheckBox generator
#include "gen_choice.h"          // ChoiceGenerator -- wxChoice generator
#include "gen_choicebook.h"      // ChoicebookGenerator -- wxChoicebook generator
#include "gen_close_btn.h"       // wxBitmapButton::CreateCloseButton generator
#include "gen_clr_picker.h"      // wxColourPickerCtrl generator
#include "gen_cmd_link_btn.h"    // CommandLinkBtnGenerator -- wxCommandLinkButton generator
#include "gen_collapsible.h"     // CollapsiblePaneGenerator -- wxCollapsiblePane generator
#include "gen_combobox.h"        // ComboBoxGenerator -- wxComboBox generator
#include "gen_ctx_help_btn.h"    // CtxHelpButtonGenerator -- wxContextHelpButton generator
#include "gen_ctx_menu.h"        // CtxMenuGenerator -- generates function and includes
#include "gen_custom_ctrl.h"     // CustomControl -- Custom Control generator
#include "gen_date_picker.h"     // DatePickerCtrlGenerator -- wxDatePickerCtrl generator
#include "gen_dialog.h"          // DialogFormGenerator -- wxDialog generator
#include "gen_dir_ctrl.h"        // GenericDirCtrlGenerator -- wxGenericDirCtrl generator
#include "gen_dir_picker.h"      // DirPickerGenerator -- wxDirPickerCtrl generator
#include "gen_edit_listbox.h"    // EditListBoxGenerator -- wxEditableListBox generator
#include "gen_file_ctrl.h"       // FileCtrlGenerator -- wxFileCtrl generator
#include "gen_file_picker.h"     // FilePickerGenerator -- wxFilePickerCtrl generator
#include "gen_flexgrid_sizer.h"  // FlexGridSizerGenerator -- wxFlexGridSizer generator
#include "gen_font_picker.h"     // FontPickerGenerator -- wxFontPickerCtrl generator
#include "gen_frame.h"           // FrameFormGenerator -- wxFrame generator
#include "gen_gauge.h"           // GaugeGenerator -- wxGauge generator
#include "gen_grid.h"            // GridGenerator -- wxGrid generator
#include "gen_grid_sizer.h"      // GridSizerGenerator -- wxGridSizer generator
#include "gen_gridbag_sizer.h"   // GridBagSizerGenerator -- wxGridBagSizer generator
#include "gen_html_listbox.h"    // HtmlListBoxGenerator -- wxSimpleHtmlListBox generator
#include "gen_html_window.h"     // HtmlWindowGenerator -- wxHtmlWindow generator
#include "gen_hyperlink.h"       // HyperlinkGenerator -- wxHyperlinkCtrl generator
#include "gen_infobar.h"         // InfoBarGenerator -- wxInfoBar generator
#include "gen_listbook.h"        // ListbookGenerator -- wxListbook generator
#include "gen_listbox.h"         // ListBoxGenerator -- wxListBox generator
#include "gen_listview.h"        // ListViewGenerator -- wxListView generator
#include "gen_notebook.h"        // NotebookGenerator -- wxNotebook generator
#include "gen_page_ctrl.h"       // PageCtrlGenerator -- Page control generator
#include "gen_panel.h"           // PanelGenerator -- wxPanel generator
#include "gen_panel_form.h"      // PanelFormGenerator -- wxPanel Form generator
#include "gen_popup_win.h"       // wxPopupWindow/wxPopupTransientWindow generator
#include "gen_prop_category.h"   // PropertyGrid/Manager Category generator
#include "gen_prop_grid.h"       // wxPropertyGrid generator
#include "gen_prop_grid_mgr.h"   // wxPropertyGridManager and wxPropertyGridPage generators
#include "gen_prop_item.h"       // PropertyGrid/Manager Item generator
#include "gen_propsheet_dlg.h"   // wxDialog generator
#include "gen_radio_box.h"       // wxRadioBox generator
#include "gen_radio_btn.h"       // Radio button and Radio box component classes
#include "gen_rearrange.h"       // wxRearrangeCtrl generator
#include "gen_rich_text.h"       // wxRichTextCtrl generator
#include "gen_scrollbar.h"       // wxScrollBar generator
#include "gen_search_ctrl.h"     // SearchCtrlGenerator -- wxSearchCtrl generator
#include "gen_simplebook.h"      // SimplebookGenerator -- wxSimplebook generator
#include "gen_slider.h"          // SliderGenerator -- wxSlider generator
#include "gen_spacer_sizer.h"    // SpacerGenerator -- Add space to sizer generator
#include "gen_spin_btn.h"        // SpinButtonGenerator -- wxSpinButton generator
#include "gen_spin_ctrl.h"       // SpinCtrlGenerator -- Spin and ScrollBar component classes
#include "gen_split_win.h"       // SplitterWindowGenerator -- wxSplitterWindow generator
#include "gen_statchkbox_sizer.h"  // StaticCheckboxBoxSizerGenerator -- wxStaticBoxSizer with wxCheckBox generator
#include "gen_static_bmp.h"       // StaticBitmapGenerator -- wxStaticBitmap generator
#include "gen_static_box.h"       // StaticBoxGenerator -- wxStaticBox generator
#include "gen_static_line.h"      // StaticLineGenerator -- wxStaticLine generator
#include "gen_static_text.h"      // StaticTextGenerator -- wxStaticText generator
#include "gen_staticbox_sizer.h"  // StaticBoxSizerGenerator -- wxStaticBoxSizer generator
#include "gen_statradiobox_sizer.h"  // StaticRadioBtnBoxSizerGenerator -- wxStaticBoxSizer with wxRadioButton generator
#include "gen_status_bar.h"        // StatusBarGenerator -- wxStatusBar generator
#include "gen_std_dlgbtn_sizer.h"  // StdDialogButtonSizerGenerator -- wxStdDialogButtonSizer generator
#include "gen_text_ctrl.h"         // TextCtrlGenerator -- wxTextCtrl generator
#include "gen_text_sizer.h"        // TextSizerGenerator -- wxTextSizerWrapper generator
#include "gen_time_picker.h"       // TimePickerCtrlGenerator -- wxTimePickerCtrl generator
#include "gen_timer.h"             // TimerGenerator -- wxTimer generator
#include "gen_toggle_btn.h"        // ToggleButtonGenerator -- wxToggleButton generator
#include "gen_toolbar.h"           // wxToolBar generator
#include "gen_toolbook.h"          // ToolbookGenerator -- wxToolbook generator
#include "gen_tree_ctrl.h"         // TreeCtrlGenerator -- wxTreeCtrl generator
#include "gen_tree_list.h"         // TreeListCtrlGenerator -- wxTreeCtrl component classes
#include "gen_treebook.h"          // TreebookGenerator -- wxTreebook generator
#include "gen_web_view.h"          // WebViewGenerator -- wxWebView generator
#include "gen_wizard.h"            // WizardFormGenerator -- wxWizard generator
#include "gen_wrap_sizer.h"        // WrapSizerGenerator -- wxGridSizer generator

#include "gen_ribbon_bar.h"      // RibbonBarFormGenerator -- wxRibbonBar -- form and regular
#include "gen_ribbon_button.h"   // wxRibbonButtonBar generator
#include "gen_ribbon_gallery.h"  // RibbonGalleryGenerator -- wxRibbonGallery generator
#include "gen_ribbon_page.h"     // RibbonPageGenerator -- wxRibbonPage and wxRibbonPanel generators
#include "gen_ribbon_tool.h"     // RibbonToolBarGenerator -- wxRibbonButtonBar generator

#include "mdi/gen_aui_mdi_frame.h"     // AuiMdiFrameGenerator -- wxFrame generator
#include "mdi/gen_doc_mdi_frame.h"     // DocParentFrameGenerator -- wxDocParentFrame generator
#include "mdi/gen_doc_parent_frame.h"  // DocParentFrameGenerator -- wxDocParentFrame generamdi/tor

#include "mdi/gen_doc_view_app.h"  // Generates base class for wxDocument/wView applications
#include "mdi/gen_mdi_menu.h"      // Menu bar classes for an MDI frame

#include "mdi/gen_doc_image.h"      // wxImage document class
#include "mdi/gen_doc_richtext.h"   // wxRichTextCtrl document class
#include "mdi/gen_doc_scintilla.h"  // wxStyledTextCtrl document class
#include "mdi/gen_doc_splitter.h"   // wxSplitterWindow document class
#include "mdi/gen_doc_textctrl.h"   // wxTextCtrl document class

#include "mdi/gen_view_image.h"      // wxImage view class
#include "mdi/gen_view_richtext.h"   // wxRichTextCtrl view class
#include "mdi/gen_view_scintilla.h"  // wxStyledTextCtrl view class
#include "mdi/gen_view_splitter.h"   // wxSplitterWindow view class
#include "mdi/gen_view_textctrl.h"   // wxTextCtrl view class

#include "gen_enums.h"  // Enumerations for generators

using namespace GenEnum;

namespace
{
    template <typename GeneratorType>
    auto SetGenerator(GenName name) -> void
    {
        auto* decl = NodeCreation.get_declaration(name);
        ASSERT(decl);
        decl->SetGenerator(new GeneratorType());
    }
}  // namespace

void NodeCreator::InitGenerators()
{
    SetGenerator<BookPageGenerator>(gen_BookPage);
    SetGenerator<PageCtrlGenerator>(gen_PageCtrl);

    SetGenerator<WizardPageGenerator>(gen_wxWizardPageSimple);
    SetGenerator<BannerWindowGenerator>(gen_wxBannerWindow);
    SetGenerator<BitmapComboBoxGenerator>(gen_wxBitmapComboBox);
    SetGenerator<ActivityIndicatorGenerator>(gen_wxActivityIndicator);
    SetGenerator<AnimationGenerator>(gen_wxAnimationCtrl);
    SetGenerator<ButtonGenerator>(gen_wxButton);
    SetGenerator<CloseButtonGenerator>(gen_CloseButton);
    SetGenerator<CheckBoxGenerator>(gen_wxCheckBox);
    SetGenerator<Check3StateGenerator>(gen_Check3State);
    SetGenerator<CheckListBoxGenerator>(gen_wxCheckListBox);
    SetGenerator<ChoiceGenerator>(gen_wxChoice);
    SetGenerator<ChoicebookGenerator>(gen_wxChoicebook);
    SetGenerator<CollapsiblePaneGenerator>(gen_wxCollapsiblePane);
    SetGenerator<ComboBoxGenerator>(gen_wxComboBox);
    SetGenerator<CommandLinkBtnGenerator>(gen_wxCommandLinkButton);
    SetGenerator<CtxHelpButtonGenerator>(gen_wxContextHelpButton);
    SetGenerator<GaugeGenerator>(gen_wxGauge);
    SetGenerator<HtmlWindowGenerator>(gen_wxHtmlWindow);
    SetGenerator<HyperlinkGenerator>(gen_wxHyperlinkCtrl);
    SetGenerator<WebViewGenerator>(gen_wxWebView);
    SetGenerator<InfoBarGenerator>(gen_wxInfoBar);
    SetGenerator<ListBoxGenerator>(gen_wxListBox);
    SetGenerator<HtmlListBoxGenerator>(gen_wxSimpleHtmlListBox);
    SetGenerator<AuiNotebookGenerator>(gen_wxAuiNotebook);
    SetGenerator<ListbookGenerator>(gen_wxListbook);
    SetGenerator<NotebookGenerator>(gen_wxNotebook);
    SetGenerator<ToolbookGenerator>(gen_wxToolbook);
    SetGenerator<TreebookGenerator>(gen_wxTreebook);
    SetGenerator<PanelGenerator>(gen_wxPanel);
    SetGenerator<RadioBoxGenerator>(gen_wxRadioBox);
    SetGenerator<RearrangeCtrlGenerator>(gen_wxRearrangeCtrl);
    SetGenerator<RadioButtonGenerator>(gen_wxRadioButton);
    SetGenerator<RichTextCtrlGenerator>(gen_wxRichTextCtrl);
    SetGenerator<ScrollBarGenerator>(gen_wxScrollBar);
    SetGenerator<ScrolledCanvasGenerator>(gen_wxScrolledCanvas);
    SetGenerator<ScrolledWindowGenerator>(gen_wxScrolledWindow);
    SetGenerator<SimplebookGenerator>(gen_wxSimplebook);
    SetGenerator<SliderGenerator>(gen_wxSlider);
    SetGenerator<SplitterWindowGenerator>(gen_wxSplitterWindow);
    SetGenerator<StaticBitmapGenerator>(gen_wxStaticBitmap);
    SetGenerator<StaticLineGenerator>(gen_wxStaticLine);
    SetGenerator<StaticTextGenerator>(gen_wxStaticText);
    SetGenerator<StatusBarGenerator>(gen_wxStatusBar);
    SetGenerator<StyledTextGenerator>(gen_wxStyledTextCtrl);
    SetGenerator<TextCtrlGenerator>(gen_wxTextCtrl);
    SetGenerator<TimerGenerator>(gen_wxTimer);
    SetGenerator<ToggleButtonGenerator>(gen_wxToggleButton);
    SetGenerator<BitmapToggleButtonGenerator>(gen_wxBitmapToggleButton);
    SetGenerator<CalendarCtrlGenerator>(gen_wxCalendarCtrl);
    SetGenerator<FileCtrlGenerator>(gen_wxFileCtrl);
    SetGenerator<GenericDirCtrlGenerator>(gen_wxGenericDirCtrl);
    SetGenerator<SearchCtrlGenerator>(gen_wxSearchCtrl);
    SetGenerator<ListViewGenerator>(gen_wxListView);
    SetGenerator<EditListBoxGenerator>(gen_wxEditableListBox);
    SetGenerator<GridGenerator>(gen_wxGrid);

    SetGenerator<ColourPickerGenerator>(gen_wxColourPickerCtrl);
    SetGenerator<DatePickerCtrlGenerator>(gen_wxDatePickerCtrl);
    SetGenerator<DirPickerGenerator>(gen_wxDirPickerCtrl);
    SetGenerator<FilePickerGenerator>(gen_wxFilePickerCtrl);
    SetGenerator<FontPickerGenerator>(gen_wxFontPickerCtrl);
    SetGenerator<TimePickerCtrlGenerator>(gen_wxTimePickerCtrl);

    SetGenerator<MenuBarGenerator>(gen_wxMenuBar);
    SetGenerator<MenuBarFormGenerator>(gen_MenuBar);
    SetGenerator<PopupMenuGenerator>(gen_PopupMenu);
    SetGenerator<MenuGenerator>(gen_wxMenu);
    SetGenerator<SubMenuGenerator>(gen_submenu);
    SetGenerator<MenuItemGenerator>(gen_wxMenuItem);
    SetGenerator<SeparatorGenerator>(gen_separator);
    SetGenerator<CtxMenuGenerator>(gen_wxContextMenuEvent);

    SetGenerator<MdiFrameMenuBar>(gen_MdiFrameMenuBar);
    SetGenerator<MdiDocumentMenuBar>(gen_MdiDocMenuBar);

    SetGenerator<DataGenerator>(gen_Data);
    SetGenerator<DataStringGenerator>(gen_data_string);
    SetGenerator<DataXmlGenerator>(gen_data_xml);
    SetGenerator<ImagesGenerator>(gen_Images);
    SetGenerator<EmbeddedImageGenerator>(gen_embedded_image);

    SetGenerator<DataViewCtrl>(gen_wxDataViewCtrl);
    SetGenerator<DataViewListCtrl>(gen_wxDataViewListCtrl);
    SetGenerator<DataViewTreeCtrl>(gen_wxDataViewTreeCtrl);
    SetGenerator<DataViewColumn>(gen_dataViewColumn);
    SetGenerator<DataViewListColumn>(gen_dataViewListColumn);

    SetGenerator<PropertyGridGenerator>(gen_wxPropertyGrid);
    SetGenerator<PropertyGridManagerGenerator>(gen_wxPropertyGridManager);
    SetGenerator<PropertyGridCategoryGenerator>(gen_propGridCategory);
    SetGenerator<PropertyGridItemGenerator>(gen_propGridItem);
    SetGenerator<PropertyGridPageGenerator>(gen_propGridPage);

    SetGenerator<SpinButtonGenerator>(gen_wxSpinButton);
    SetGenerator<SpinCtrlGenerator>(gen_wxSpinCtrl);
    SetGenerator<SpinCtrlDoubleGenerator>(gen_wxSpinCtrlDouble);

    SetGenerator<SpacerGenerator>(gen_spacer);

    SetGenerator<AuiToolBarFormGenerator>(gen_AuiToolBar);
    SetGenerator<AuiToolBarGenerator>(gen_wxAuiToolBar);
    SetGenerator<AuiToolGenerator>(gen_auitool);
    SetGenerator<AuiToolLabelGenerator>(gen_auitool_label);
    SetGenerator<AuiToolSpacerGenerator>(gen_auitool_spacer);
    SetGenerator<AuiToolStretchSpacerGenerator>(gen_auitool_stretchable);
    SetGenerator<ToolBarFormGenerator>(gen_ToolBar);
    SetGenerator<ToolBarGenerator>(gen_wxToolBar);
    SetGenerator<ToolGenerator>(gen_tool);
    SetGenerator<ToolDropDownGenerator>(gen_tool_dropdown);
    SetGenerator<ToolSeparatorGenerator>(gen_toolSeparator);
    SetGenerator<ToolStretchableGenerator>(gen_toolStretchable);

    SetGenerator<RibbonBarFormGenerator>(gen_RibbonBar);
    SetGenerator<RibbonBarGenerator>(gen_wxRibbonBar);
    SetGenerator<RibbonPageGenerator>(gen_wxRibbonPage);
    SetGenerator<RibbonPanelGenerator>(gen_wxRibbonPanel);
    SetGenerator<RibbonButtonBarGenerator>(gen_wxRibbonButtonBar);
    SetGenerator<RibbonToolBarGenerator>(gen_wxRibbonToolBar);
    SetGenerator<ToolSeparatorGenerator>(gen_ribbonSeparator);
    SetGenerator<RibbonGalleryGenerator>(gen_wxRibbonGallery);

    SetGenerator<RibbonButtonGenerator>(gen_ribbonButton);
    SetGenerator<RibbonToolGenerator>(gen_ribbonTool);
    SetGenerator<RibbonGalleryItemGenerator>(gen_ribbonGalleryItem);

    SetGenerator<TreeCtrlGenerator>(gen_wxTreeCtrl);
    SetGenerator<TreeListCtrlGenerator>(gen_wxTreeListCtrl);
    SetGenerator<TreeListCtrlColumnGenerator>(gen_TreeListCtrlColumn);

    SetGenerator<PanelFormGenerator>(gen_PanelForm);
    SetGenerator<DialogFormGenerator>(gen_wxDialog);
    SetGenerator<FrameFormGenerator>(gen_wxFrame);
    SetGenerator<PopupWinGenerator>(gen_wxPopupWindow);
    SetGenerator<PopupTransientWinGenerator>(gen_wxPopupTransientWindow);
    SetGenerator<PropSheetDlgGenerator>(gen_wxPropertySheetDialog);
    SetGenerator<WizardFormGenerator>(gen_wxWizard);

    SetGenerator<BoxSizerGenerator>(gen_VerticalBoxSizer);
    SetGenerator<BoxSizerGenerator>(gen_wxBoxSizer);
    SetGenerator<FlexGridSizerGenerator>(gen_wxFlexGridSizer);
    SetGenerator<GridBagSizerGenerator>(gen_wxGridBagSizer);
    SetGenerator<GridSizerGenerator>(gen_wxGridSizer);
    SetGenerator<StaticBoxGenerator>(gen_wxStaticBox);
    SetGenerator<StaticBoxSizerGenerator>(gen_wxStaticBoxSizer);
    SetGenerator<StaticCheckboxBoxSizerGenerator>(gen_StaticCheckboxBoxSizer);
    SetGenerator<StaticRadioBtnBoxSizerGenerator>(gen_StaticRadioBtnBoxSizer);
    SetGenerator<StdDialogButtonSizerGenerator>(gen_wxStdDialogButtonSizer);
    SetGenerator<WrapSizerGenerator>(gen_wxWrapSizer);
    SetGenerator<TextSizerGenerator>(gen_TextSizer);

    SetGenerator<CustomControl>(gen_CustomControl);

    SetGenerator<DocParentFrameGenerator>(gen_wxDocParentFrame);
    SetGenerator<DocMdiParentFrameGenerator>(gen_wxDocMDIParentFrame);
    SetGenerator<AuiMdiFrameGenerator>(gen_wxAuiMDIParentFrame);
    SetGenerator<DocChildFrame>(gen_wxDocChildFrame);
    SetGenerator<DocMDIChildFrame>(gen_wxDocMDIChildFrame);
    SetGenerator<AuiMDIChildFrame>(gen_wxAuiMDIChildFrame);

    SetGenerator<DocViewAppGenerator>(gen_DocViewApp);

    SetGenerator<ImageDocGenerator>(gen_DocumentImage);
    SetGenerator<ImageViewGenerator>(gen_ViewImage);
    SetGenerator<RichTextDocGenerator>(gen_DocumentRichTextCtrl);
    SetGenerator<RichTextViewGenerator>(gen_ViewRichTextCtrl);
    SetGenerator<SplitterDocGenerator>(gen_DocumentSplitterWindow);
    SetGenerator<SplitterViewGenerator>(gen_ViewSplitterWindow);
    SetGenerator<ScintillaDocGenerator>(gen_DocumentStyledTextCtrl);
    SetGenerator<ScintillaViewGenerator>(gen_ViewStyledTextCtrl);
    SetGenerator<TextDocGenerator>(gen_DocumentTextCtrl);
    SetGenerator<TextViewGenerator>(gen_ViewTextCtrl);

    SetGenerator<ProjectGenerator>(gen_Project);
    SetGenerator<FolderGenerator>(gen_folder);
    SetGenerator<SubFolderGenerator>(gen_sub_folder);

    AddAllConstants();
}
