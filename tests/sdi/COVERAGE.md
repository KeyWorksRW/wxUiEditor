# Contents

The following tables indicate whether or not code is being generated in the `pytest` or `cpptest` apps. A "???" indicates it is not currently used. "no" indcates that the language doesn't support it. "---" in the XRC column indicates that it could be generated, but there is no current testing for it.


XRC is listed, but there is currently no testing code for it. "---" in the XRC column means it could be generated. "no" indicates that XRC does not support the class.

# Categories

### Books

| Class | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| wxAuiNotebook | yes | yes | --- | ../src/generate/gen_aui_notebook.cpp |
| wxChoicebook | yes | yes | --- | ../src/generate/gen_choicebook.cpp |
| wxListbook | yes | yes | --- | ../src/generate/gen_listbook.cpp |
| wxNotebook | yes | yes | --- | ../src/generate/gen_notebook.cpp |
| wxSimplebook | ??? | ??? | --- | ../src/generate/gen_simplebook.cpp |
| wxToolbook | yes | yes | --- | ../src/generate/gen_toolbook.cpp |
| wxTreebook | yes | yes | --- | ../src/generate/gen_treebook.cpp |

### Forms

| Class | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| wxDialog | yes | yes | yes | ../src/generate/gen_dialog.cpp |
| wxFrame | yes | yes | yes | ../src/generate/gen_frame.cpp |
| wxMenu | yes | yes | --- | ../src/generate/menu_widgets.cpp |
| wxMenuBar | yes | yes | --- | ../src/generate/menu_widgets.cpp |
| wxPanel | ??? | ??? | --- | ../src/generate/gen_panel_form.cpp |
| wxPopupTransientWindow | yes | yes | no | ../src/generate/gen_popup_trans_win.cpp |
| wxRibbonBar | yes | yes | --- | ../src/generate/gen_ribbon_bar.cpp |
| wxToolBar | yes | yes | --- | ../src/generate/gen_toolbar.cpp |
| wxWizard | yes | yes | --- | ../src/generate/gen_wizard.cpp |

### Pickers

| Class | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| wxColourPickerCtrl | yes | yes | --- | ../src/generate/gen_clr_picker.cpp |
| wxDatePickerCtrl | yes | yes | --- | ../src/generate/gen_date_picker.cpp |
| wxDirPickerCtrl | yes | yes | --- | ../src/generate/gen_dir_picker.cpp |
| wxFilePickerCtrl | yes | yes | --- | ../src/generate/gen_file_picker.cpp |
| wxFontPickerCtrl | yes | yes | --- | ../src/generate/gen_font_picker.cpp |
| wxTimePickerCtrl | yes | yes | --- | ../src/generate/gen_time_picker.cpp |

### Sizers

| Class | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| wxBoxSizer | yes | yes | --- | ../src/generate/gen_box_sizer.cpp |
| wxCheckBoxSizer | ??? | no | --- | ../src/generate/gen_statchkbox_sizer.cpp |
| wxFlexGridSizer | yes | yes | --- | ../src/generate/gen_flexgrid_sizer.cpp |
| wxGridBagSizer | yes | yes | --- | ../src/generate/gen_gridbag_sizer.cpp |
| wxGridSizer | ??? | ??? | --- | ../src/generate/gen_grid_sizer.cpp |
| wxRadioButtonSizer | ??? | no | --- | ../src/generate/gen_statradiobox_sizer.cpp |
| wxStaticBoxSizer | yes | yes | --- | ../src/generate/gen_staticbox_sizer.cpp |
| wxStdDialogButtonSizer | yes | yes | --- | ../src/generate/gen_std_dlgbtn_sizer.cpp |
| wxWrapSizer | yes | yes | --- | ../src/generate/gen_wrap_sizer.cpp |

# Classes

| Class | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| wxActivityIndicator | ??? | ??? | --- | ../src/generate/gen_activity.cpp |
| wxAnimationCtrl | ??? | ??? | --- | ../src/generate/gen_animation.cpp |
| wxAuiToolBar | ??? | ??? | --- | ../src/generate/gen_aui_toolbar.cpp |
| wxBannerWindow | yes | yes | --- | ../src/generate/gen_banner_window.cpp |
| wxBitmapComboBox | ??? | ??? | --- | ../src/generate/gen_bitmap_combo.cpp |
| wxButton | ??? | ??? | --- | ../src/generate/gen_button.cpp |
| wxCalendarCtrl | yes | yes | --- | ../src/generate/gen_calendar_ctrl.cpp |
| wxCheckBox | yes | yes | --- | ../src/generate/gen_checkbox.cpp |
| wxCheckListBox | yes | yes | --- | ../src/generate/gen_check_listbox.cpp |
| wxChoice | yes | yes | --- | ../src/generate/gen_choice.cpp |
| wxCollapsiblePane | ??? | ??? | --- | ../src/generate/gen_collapsible.cpp |
| wxComboBox | yes | yes | --- | ../src/generate/gen_combobox.cpp |
| wxCommandLinkButton | yes | yes | --- | ../src/generate/gen_cmd_link_btn.cpp |
| wxDataViewListCtrl | ??? | ??? | --- | ../src/generate/dataview_widgets.cpp |
| wxDataViewCtrl | ??? | ??? | --- | ../src/generate/dataview_widgets.cpp |
| wxEditableListBox | yes | yes | --- | ../src/generate/gen_edit_listbox.cpp |
| wxFileCtrl | ??? | ??? | --- | ../src/generate/gen_file_ctrl.cpp |
| wxGauge | ??? | ??? | --- | ../src/generate/gen_gauge.cpp |
| wxGenericDirCtrl | ??? | ??? | --- | ../src/generate/gen_dir_ctrl.cpp |
| wxGrid | yes | yes | --- | ../src/generate/gen_grid.cpp |
| wxHtmlWindow | yes | yes | --- | ../src/generate/gen_html_window.cpp |
| wxHyperlinkCtrl | yes | yes | --- | ../src/generate/gen_hyperlink.cpp |
| wxInfoBar | ??? | ??? | --- | ../src/generate/gen_infobar.cpp |
| wxListBox | yes | yes | --- | ../src/generate/gen_listbox.cpp |
| wxListView | yes | yes | --- | ../src/generate/gen_listview.cpp |
| wxMenu | yes | yes | --- | ../src/generate/gen_menu.cpp |
| wxMenuBar | yes | yes | --- | ../src/generate/menu_widgets.cpp |
| wxMenuItem | yes | yes | --- | ../src/generate/gen_menuitem.cpp |
| wxPanel | ??? | ??? | --- | ../src/generate/gen_panel.cpp |
| wxPropertyGrid | ??? | ??? | no | ../src/generate/gen_prop_grid.cpp |
| wxPropertyGridManager | yes | yes | no | ../src/generate/grid_widgets.cpp |
| wxRadioBox | yes | yes | --- | ../src/generate/gen_prop_grid.cpp |
| wxRadioButton | yes | yes | --- | ../src/generate/gen_radio_btn.cpp |
| wxRearrangeCtrl | yes | yes | no | ../src/generate/gen_rearrange.cpp |
| wxRibbonBar | yes | yes | --- | ../src/generate/gen_ribbon_bar.cpp |
| wxRibbonButtonBar | yes | yes | --- | ../src/generate/gen_ribbon_button.cpp |
| wxRibbonGallery | yes | yes | --- | ../src/generate/ribbon_widgets.cpp |
| wxRibbonPage | yes | yes | --- | ../src/generate/gen_ribbon_page.cpp |
| wxRibbonPanel | yes | yes | --- | ../src/generate/gen_ribbon_page.cpp |
| wxRibbonToolBar | yes | yes | --- | ../src/generate/ribbon_widgets.cpp |
| wxRichTextCtrl | yes | yes | --- | ../src/generate/gen_rich_text.cpp |
| wxScrollBar | yes | yes | --- | ../src/generate/gen_scrollbar.cpp |
| wxScrolled\<wxPanel\> | ??? | ??? | --- | ../src/generate/styled_text.cpp |
| wxScrolled\<wxWindow\> | ??? | ??? | --- | ../src/generate/styled_text.cpp |
| wxSearchCtrl | yes | yes | --- | ../src/generate/gen_search_ctrl.cpp |
| wxSimpleHtmlListBox | ??? | ??? | --- | ../src/generate/gen_html_listbox.cpp |
| wxSlider | ??? | ??? | --- | ../src/generate/gen_slider.cpp |
| wxSpinButton | yes | yes | --- | ../src/generate/gen_spin_btn.cpp |
| wxSpinCtrl | yes | yes | --- | ../src/generate/gen_spin_ctrl.cpp |
| wxSpinCtrlDouble | yes | yes | --- | ../src/generate/gen_spin_ctrl.cpp |
| wxSplitterWindow | ??? | ??? | --- | ../src/generate/gen_split_win.cpp |
| wxStaticBitmap | ??? | ??? | --- | ../src/generate/gen_static_bmp.cpp |
| wxStaticBox | ??? | ??? | --- | ../src/generate/gen_static_box.cpp |
| wxStaticLine | ??? | ??? | --- | ../src/generate/gen_static_line.cpp |
| wxStaticText | yes | yes | --- | ../src/generate/gen_static_text.cpp |
| wxStatusBar | yes | yes | --- | ../src/generate/gen_status_bar.cpp |
| wxStyledTextCtrl | yes | yes | --- | ../src/generate/styled_text.cpp |
| wxTextCtrl | yes | yes | --- | ../src/generate/gen_text_ctrl.cpp |
| wxToggleButton | yes | yes | --- | ../src/generate/gen_toggle_btn.cpp |
| wxToolBar | yes | yes | --- | ../src/generate/gen_toolbar.cpp |
| wxTreeCtrl | ??? | ??? | --- | ../src/generate/gen_tree_ctrl.cpp |
| wxTreeListCtrl | ??? | ??? | no | ../src/generate/gen_tree_list.cpp |
| wxWebView | ??? | ??? | no | ../src/generate/gen_web_view.cpp |
| wxWizardPageSimple | yes | yes | --- | ../src/generate/gen_wizard.cpp |

# Objects

| Object | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| AuiTool | ??? | ??? | --- | ../src/generate/gen_aui_toolbar.cpp |
| AuiToolLabel | ??? | ??? | --- | ../src/generate/gen_aui_toolbar.cpp |
| AuiToolSpacer | ??? | ??? | --- | ../src/generate/gen_aui_toolbar.cpp |
| AuiToolStretchSpacer | ??? | ??? | --- | ../src/generate/gen_aui_toolbar.cpp |
| BookPage | yes | yes | --- | ../src/generate/gen_book_page.cpp |
| Check3State | ??? | ??? | --- | ../src/generate/gen_checkbox.cpp |
| CustomControl | ??? | no | --- | ../src/generate/gen_custom_ctrl.cpp |
| DataViewColumn | ??? | ??? | --- | ../src/generate/dataview_widgets.cpp |
| DataViewListColumn | ??? | ??? | --- | ../src/generate/dataview_widgets.cpp |
| NewCloseButton | ??? | ??? | --- | ../src/generate/gen_close_btn.cpp |
| PageCtrl | ??? | ??? | no | ../src/generate/gen_page_ctrl.cpp |
| PropertyGridItem | yes | yes | no | ../src/generate/gen_prop_item.cpp |
| PropertyGridPage | yes | yes | no | ../src/generate/grid_widgets.cpp |
| RibbonButton | yes | yes | --- | ../src/generate/ribbon_widgets.cpp |
| RibbonGalleryItem | yes | yes | --- | ../src/generate/ribbon_widgets.cpp |
| RibbonTool | yes | yes | --- | ../src/generate/ribbon_widgets.cpp |
| Separator | ??? | ??? | --- | ../src/generate/menu_widgets.cpp |
| Spacer | ??? | ??? | --- | ../src/generate/gen_spacer_sizer.cpp |
| SubMenu | ??? | ??? | --- | ../src/generate/gen_submenu.cpp |
| ToolDropDown | ??? | ??? | --- | ../src/generate/gen_toolbar.cpp |
| ToolGenerator | ??? | ??? | --- | ../src/generate/gen_toolbar.cpp |
| ToolSeparator | ??? | ??? | --- | ../src/generate/gen_toolbar.cpp |
| ToolStretchable | ??? | ??? | --- | ../src/generate/gen_toolbar.cpp |
| TreeListCtrlColumn | ??? | ??? | no | ../src/generate/gen_tree_list.cpp |
| wxContextMenuEvent | ??? | no | no | gen_ctx_menu.cpp |
