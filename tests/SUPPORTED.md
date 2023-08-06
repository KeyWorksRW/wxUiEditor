# Contents

The following tables indicate whether or not code is being generated in a specific language. A "???" or "---" indicates it has not been verified. "no" indcates that the language doesn't support it. `partial` means that not all properties for the control are supported.

This does _not_ mean that the class is fully supported in every language -- this is just a guide to indicate whether or not it is at least partially implemented in the various languages.

# Categories

### Books

| Class | C++ | Python | Ruby | XRC | file |
-----------|-----|--------|------|------|------|
| wxAuiNotebook | yes | yes | crashes | --- | ../src/generate/gen_aui_notebook.cpp |
| wxChoicebook | yes | yes | yes | --- | ../src/generate/gen_choicebook.cpp |
| wxListbook | yes | yes | yes | --- | ../src/generate/gen_listbook.cpp |
| wxNotebook | yes | yes | yes | --- | ../src/generate/gen_notebook.cpp |
| wxSimplebook | yes | ??? | yes | --- | ../src/generate/gen_simplebook.cpp |
| wxToolbook | yes | yes | --- | --- | ../src/generate/gen_toolbook.cpp |
| wxTreebook | yes | yes | --- | --- | ../src/generate/gen_treebook.cpp |

### Forms

| Class | C++ | Python | Ruby | XRC | file |
-----------|-----|--------|------|------|------|
| wxDialog | yes | yes | yes | yes | ../src/generate/gen_dialog.cpp |
| wxFrame | yes | yes | yes | yes | ../src/generate/gen_frame.cpp |
| wxMenu | yes | yes | --- | --- | ../src/generate/menu_widgets.cpp |
| wxMenuBar | yes | yes | --- | --- | ../src/generate/menu_widgets.cpp |
| wxPanel | yes | yes | yes | --- | ../src/generate/gen_panel_form.cpp |
| wxPopupTransientWindow | yes | yes | --- | no | ../src/generate/gen_popup_trans_win.cpp |
| wxRibbonBar | yes | yes | --- | --- | ../src/generate/gen_ribbon_bar.cpp |
| wxToolBar | yes | yes | --- | --- | ../src/generate/gen_toolbar.cpp |
| wxWizard | yes | yes | --- | --- | ../src/generate/gen_wizard.cpp |

### Pickers

| Class | C++ | Python | Ruby | XRC | file |
-----------|-----|--------|------|------|------|
| wxColourPickerCtrl | yes | yes | --- | --- | ../src/generate/gen_clr_picker.cpp |
| wxDatePickerCtrl | yes | yes | --- | --- | ../src/generate/gen_date_picker.cpp |
| wxDirPickerCtrl | yes | yes | --- | --- | ../src/generate/gen_dir_picker.cpp |
| wxFilePickerCtrl | yes | yes | --- | --- | ../src/generate/gen_file_picker.cpp |
| wxFontPickerCtrl | yes | yes | --- | --- | ../src/generate/gen_font_picker.cpp |
| wxTimePickerCtrl | yes | yes | --- | --- | ../src/generate/gen_time_picker.cpp |

### Sizers

| Class | C++ | Python | Ruby | XRC | file |
-----------|-----|--------|------|------|------|
| wxBoxSizer | yes | yes | yes | yes | ../src/generate/gen_box_sizer.cpp |
| wxCheckBoxSizer | yes | no | --- | --- | ../src/generate/gen_statchkbox_sizer.cpp |
| wxFlexGridSizer | yes | yes | --- | --- | ../src/generate/gen_flexgrid_sizer.cpp |
| wxGridBagSizer | yes | yes | --- | --- | ../src/generate/gen_gridbag_sizer.cpp |
| wxGridSizer | yes | ??? | --- | --- | ../src/generate/gen_grid_sizer.cpp |
| wxRadioBoxSizer | yes | no | --- | --- | ../src/generate/gen_statradiobox_sizer.cpp |
| wxRadioButtonSizer | yes | no | --- | --- | ../src/generate/gen_statradiobox_sizer.cpp |
| wxStaticBoxSizer | yes | yes | --- | --- | ../src/generate/gen_staticbox_sizer.cpp |
| wxStdDialogButtonSizer | yes | yes | yes | partial | ../src/generate/gen_std_dlgbtn_sizer.cpp |
| wxWrapSizer | yes | yes | --- | --- | ../src/generate/gen_wrap_sizer.cpp |

# Classes

| Class | C++ | Python | Ruby | XRC | file |
-----------|-----|--------|------|------|------|
| wxActivityIndicator | ??? | ??? | --- | --- | ../src/generate/gen_activity.cpp |
| wxAnimationCtrl | yes | ??? | --- | --- | ../src/generate/gen_animation.cpp |
| wxAuiToolBar | yes | ??? | --- | --- | ../src/generate/gen_aui_toolbar.cpp |
| wxBannerWindow | yes | yes | --- | --- | ../src/generate/gen_banner_window.cpp |
| wxBitmapComboBox | yes | ??? | --- | --- | ../src/generate/gen_bitmap_combo.cpp |
| wxButton | yes | yes | yes | partial | ../src/generate/gen_button.cpp |
| wxCalendarCtrl | yes | yes | --- | --- | ../src/generate/gen_calendar_ctrl.cpp |
| wxCheckBox | yes | yes | yes | yes | ../src/generate/gen_checkbox.cpp |
| wxCheckListBox | yes | yes | --- | --- | ../src/generate/gen_check_listbox.cpp |
| wxChoice | yes | yes | --- | --- | ../src/generate/gen_choice.cpp |
| wxCollapsiblePane | yes | ??? | --- | --- | ../src/generate/gen_collapsible.cpp |
| wxComboBox | yes | yes | --- | --- | ../src/generate/gen_combobox.cpp |
| wxCommandLinkButton | yes | yes | --- | --- | ../src/generate/gen_cmd_link_btn.cpp |
| wxDataViewListCtrl | yes | ??? | --- | --- | ../src/generate/dataview_widgets.cpp |
| wxDataViewCtrl | yes | ??? | --- | --- | ../src/generate/dataview_widgets.cpp |
| wxEditableListBox | yes | yes | --- | --- | ../src/generate/gen_edit_listbox.cpp |
| wxFileCtrl | yes | ??? | --- | --- | ../src/generate/gen_file_ctrl.cpp |
| wxGauge | yes | ??? | --- | --- | ../src/generate/gen_gauge.cpp |
| wxGenericDirCtrl | yes | ??? | --- | --- | ../src/generate/gen_dir_ctrl.cpp |
| wxGrid | yes | yes | --- | --- | ../src/generate/gen_grid.cpp |
| wxHtmlWindow | yes | yes | --- | --- | ../src/generate/gen_html_window.cpp |
| wxHyperlinkCtrl | yes | yes | --- | --- | ../src/generate/gen_hyperlink.cpp |
| wxInfoBar | yes | ??? | --- | yes | ../src/generate/gen_infobar.cpp |
| wxListBox | yes | yes | --- | --- | ../src/generate/gen_listbox.cpp |
| wxListView | yes | yes | --- | --- | ../src/generate/gen_listview.cpp |
| wxMenu | yes | yes | yes | --- | ../src/generate/gen_menu.cpp |
| wxMenuBar | yes | yes | yes | --- | ../src/generate/menu_widgets.cpp |
| wxMenuItem | yes | yes | --- | --- | ../src/generate/gen_menuitem.cpp |
| wxPanel | yes | ??? | yes | --- | ../src/generate/gen_panel.cpp |
| wxPropertyGrid | yes | ??? | --- | no | ../src/generate/gen_prop_grid.cpp |
| wxPropertyGridManager | yes | yes | --- | no | ../src/generate/grid_widgets.cpp |
| wxRadioBox | yes | yes | --- | --- | ../src/generate/gen_prop_grid.cpp |
| wxRadioButton | yes | yes | --- | --- | ../src/generate/gen_radio_btn.cpp |
| wxRearrangeCtrl | yes | yes | --- | no | ../src/generate/gen_rearrange.cpp |
| wxRibbonBar | yes | yes | --- | --- | ../src/generate/gen_ribbon_bar.cpp |
| wxRibbonButtonBar | yes | yes | --- | --- | ../src/generate/gen_ribbon_button.cpp |
| wxRibbonGallery | yes | yes | --- | --- | ../src/generate/ribbon_widgets.cpp |
| wxRibbonPage | yes | yes | --- | --- | ../src/generate/gen_ribbon_page.cpp |
| wxRibbonPanel | yes | yes | --- | --- | ../src/generate/gen_ribbon_page.cpp |
| wxRibbonToolBar | yes | yes | --- | --- | ../src/generate/ribbon_widgets.cpp |
| wxRichTextCtrl | yes | yes | --- | --- | ../src/generate/gen_rich_text.cpp |
| wxScrollBar | yes | yes | --- | --- | ../src/generate/gen_scrollbar.cpp |
| wxScrolled\<wxPanel\> | yes | ??? | --- | --- | ../src/generate/styled_text.cpp |
| wxScrolled\<wxWindow\> | yes | ??? | --- | --- | ../src/generate/styled_text.cpp |
| wxSearchCtrl | yes | yes | --- | --- | ../src/generate/gen_search_ctrl.cpp |
| wxSimpleHtmlListBox | yes | ??? | --- | --- | ../src/generate/gen_html_listbox.cpp |
| wxSlider | yes | ??? | --- | --- | ../src/generate/gen_slider.cpp |
| wxSpinButton | yes | yes | --- | --- | ../src/generate/gen_spin_btn.cpp |
| wxSpinCtrl | yes | yes | --- | --- | ../src/generate/gen_spin_ctrl.cpp |
| wxSpinCtrlDouble | yes | yes | --- | --- | ../src/generate/gen_spin_ctrl.cpp |
| wxSplitterWindow | yes | ??? | --- | --- | ../src/generate/gen_split_win.cpp |
| wxStaticBitmap | yes | ??? | --- | --- | ../src/generate/gen_static_bmp.cpp |
| wxStaticBox | yes | ??? | --- | --- | ../src/generate/gen_static_box.cpp |
| wxStaticLine | yes | ??? | --- | --- | ../src/generate/gen_static_line.cpp |
| wxStaticText | yes | yes | yes | yes | ../src/generate/gen_static_text.cpp |
| wxStatusBar | yes | yes | --- | --- | ../src/generate/gen_status_bar.cpp |
| wxStyledTextCtrl | yes | yes | --- | --- | ../src/generate/styled_text.cpp |
| wxTextCtrl | yes | yes | yes | yes | ../src/generate/gen_text_ctrl.cpp |
| wxToggleButton | yes | yes | yes | partial | ../src/generate/gen_toggle_btn.cpp |
| wxToolBar | yes | yes | yes | --- | ../src/generate/gen_toolbar.cpp |
| wxTreeCtrl | yes | ??? | --- | --- | ../src/generate/gen_tree_ctrl.cpp |
| wxTreeListCtrl | yes | ??? | --- | no | ../src/generate/gen_tree_list.cpp |
| wxWebView | yes | ??? | --- | no | ../src/generate/gen_web_view.cpp |
| wxWizardPageSimple | yes | yes | --- | --- | ../src/generate/gen_wizard.cpp |

# Objects

| Class | C++ | Python | Ruby | XRC | file |
-----------|-----|--------|------|------|------|
| AuiTool | yes | ??? | --- | --- | ../src/generate/gen_aui_toolbar.cpp |
| AuiToolLabel | yes | ??? | --- | --- | ../src/generate/gen_aui_toolbar.cpp |
| AuiToolSpacer | yes | ??? | --- | --- | ../src/generate/gen_aui_toolbar.cpp |
| AuiToolStretchSpacer | yes | ??? | --- | --- | ../src/generate/gen_aui_toolbar.cpp |
| BookPage | yes | yes | --- | --- | ../src/generate/gen_book_page.cpp |
| Check3State | yes | ??? | --- | --- | ../src/generate/gen_checkbox.cpp |
| CustomControl | yes | no | --- | --- | ../src/generate/gen_custom_ctrl.cpp |
| DataViewColumn | yes | ??? | --- | --- | ../src/generate/dataview_widgets.cpp |
| DataViewListColumn | yes | ??? | --- | --- | ../src/generate/dataview_widgets.cpp |
| NewCloseButton | yes | ??? | --- | --- | ../src/generate/gen_close_btn.cpp |
| PageCtrl | yes | ??? | --- | no | ../src/generate/gen_page_ctrl.cpp |
| PropertyGridItem | yes | yes | --- | no | ../src/generate/gen_prop_item.cpp |
| PropertyGridPage | yes | yes | --- | no | ../src/generate/grid_widgets.cpp |
| RibbonButton | yes | yes | --- | --- | ../src/generate/ribbon_widgets.cpp |
| RibbonGalleryItem | yes | yes | --- | --- | ../src/generate/ribbon_widgets.cpp |
| RibbonTool | yes | yes | --- | --- | ../src/generate/ribbon_widgets.cpp |
| Separator | yes | ??? | --- | --- | ../src/generate/menu_widgets.cpp |
| Spacer | yes | ??? | --- | --- | ../src/generate/gen_spacer_sizer.cpp |
| SubMenu | yes | ??? | --- | --- | ../src/generate/gen_submenu.cpp |
| ToolDropDown | yes | ??? | --- | --- | ../src/generate/gen_toolbar.cpp |
| ToolGenerator | yes | ??? | --- | --- | ../src/generate/gen_toolbar.cpp |
| ToolSeparator | yes | ??? | --- | --- | ../src/generate/gen_toolbar.cpp |
| ToolStretchable | yes | ??? | --- | --- | ../src/generate/gen_toolbar.cpp |
| TreeListCtrlColumn | yes | ??? | --- | no | ../src/generate/gen_tree_list.cpp |
| wxContextMenuEvent | yes | no | --- | no | gen_ctx_menu.cpp |

# Python unsupported

- CheckBoxSizer
- RadioBoxSizer

# Ruby unsupported

- wxInfoBar
