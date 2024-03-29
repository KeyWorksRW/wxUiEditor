# Contents

The following tables list classes, objects and functions that wxUiEditor can generate code for. If `no` is listed for a column it means that language does not support the item. If '??' is listed for a column it means support has not been added or completed for the item.

# Categories

### Books

| Class | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| wxAuiNotebook | yes | yes | yes | gen_aui_notebook.cpp |
| wxChoicebook | yes | yes | yes | gen_choicebook.cpp |
| wxListbook | yes | yes | yes | gen_listbook.cpp |
| wxNotebook | yes | yes | yes | gen_notebook.cpp |
| wxSimplebook | yes | yes | yes | gen_simplebook.cpp |
| wxToolbook | yes | yes | yes | gen_toolbook.cpp |
| wxTreebook | yes | yes | yes | gen_treebook.cpp |

### Forms

| Class | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| wxDialog | yes| yes | yes | gen_dialog.cpp |
| wxFrame | yes| yes | yes | gen_frame.cpp |
| wxMenu | yes| yes | yes | menu_widgets.cpp |
| wxMenuBar | yes| yes | yes | menu_widgets.cpp |
| wxPanel | yes| yes | yes | gen_panel_form.cpp |
| wxPopupTransientWindow | yes| yes | no | gen_popup_trans_win.cpp |
| wxRibbonBar | yes| yes | yes | gen_ribbon_bar.cpp |
| wxToolBar | yes| yes | yes | gen_toolbar.cpp |
| wxWizard | yes| yes | yes | gen_wizard.cpp |

### Pickers

| Class | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| wxColourPickerCtrl | yes | yes | yes | gen_clr_picker.cpp |
| wxDatePickerCtrl | yes | yes | yes | gen_date_picker.cpp |
| wxDirPickerCtrl | yes | yes | yes | gen_dir_picker.cpp |
| wxFilePickerCtrl | yes | yes | yes | gen_file_picker.cpp |
| wxFontPickerCtrl | yes | yes | yes | gen_font_picker.cpp |
| wxTimePickerCtrl | yes | yes | yes | gen_time_picker.cpp |

### Sizers

| Class | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| wxBoxSizer | yes | yes | yes | gen_box_sizer.cpp |
| wxCheckBoxSizer | yes |  no | yes | gen_statchkbox_sizer.cpp |
| wxFlexGridSizer | yes | yes | yes | gen_flexgrid_sizer.cpp |
| wxGridBagSizer | yes | yes | yes | gen_gridbag_sizer.cpp |
| wxGridSizer | yes | yes | yes | gen_grid_sizer.cpp |
| wxRadioButtonSizer | yes |  no | yes | gen_statradiobox_sizer.cpp |
| wxStaticBoxSizer | yes | yes | yes | gen_staticbox_sizer.cpp |
| wxStdDialogButtonSizer | yes | yes | yes | gen_std_dlgbtn_sizer.cpp |
| wxWrapSizer | yes | yes | yes | gen_wrap_sizer.cpp |

# Classes

| Class | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| wxActivityIndicator | yes | yes | yes | gen_activity.cpp |
| wxAnimationCtrl | yes | yes | yes | gen_animation.cpp |
| wxAuiToolBar | yes | yes | yes | gen_aui_toolbar.cpp |
| wxBannerWindow | yes | yes | yes | gen_banner_window.cpp |
| wxBitmapComboBox | yes | yes | yes | gen_bitmap_combo.cpp |
| wxButton | yes | yes | yes | gen_button.cpp |
| wxCalendarCtrl | yes | yes | yes | gen_calendar_ctrl.cpp |
| wxCheckBox | yes | yes | yes | gen_checkbox.cpp |
| wxCheckListBox | yes | yes | yes | gen_check_listbox.cpp |
| wxChoice | yes | yes | yes | gen_choice.cpp |
| wxCollapsiblePane | yes | yes | yes | gen_collapsible.cpp |
| wxComboBox | yes | yes | yes | gen_combobox.cpp |
| wxCommandLinkButton | yes | yes | yes | gen_cmd_link_btn.cpp |
| wxDataViewListCtrl | yes | yes | yes | dataview_widgets.cpp |
| wxDataViewCtrl | yes | yes | yes | dataview_widgets.cpp |
| wxEditableListBox | yes | yes | yes | gen_edit_listbox.cpp |
| wxFileCtrl | yes | yes | yes | gen_file_ctrl.cpp |
| wxGauge | yes | yes | yes | gen_gauge.cpp |
| wxGenericDirCtrl | yes |  yes | yes | gen_dir_ctrl.cpp |
| wxGrid | yes | yes | yes | gen_grid.cpp |
| wxHtmlWindow | yes | yes | yes | gen_html_window.cpp |
| wxHyperlinkCtrl | yes | yes | yes | gen_hyperlink.cpp |
| wxInfoBar | yes | yes | yes | gen_infobar.cpp |
| wxListBox | yes | yes | yes | gen_listbox.cpp |
| wxListView | yes | yes | yes | gen_listview.cpp |
| wxMenu | yes | yes | yes | gen_menu.cpp |
| wxMenuBar | yes | yes | yes | menu_widgets.cpp |
| wxMenuItem | yes | yes | yes | gen_menuitem.cpp |
| wxPanel | yes | yes | yes | gen_panel.cpp |
| wxPropertyGrid | yes | yes | no | gen_prop_grid.cpp |
| wxPropertyGridManager | yes | yes | no | grid_widgets.cpp |
| wxRadioBox | yes | yes | yes | gen_prop_grid.cpp |
| wxRadioButton | yes | yes | yes | gen_radio_btn.cpp |
| wxRearrangeCtrl | yes | yes | no | gen_rearrange.cpp |
| wxRibbonBar | yes | yes | yes | gen_ribbon_bar.cpp |
| wxRibbonButtonBar | yes | yes | yes | gen_ribbon_button.cpp |
| wxRibbonGallery | yes | yes | yes | ribbon_widgets.cpp |
| wxRibbonPage | yes | yes | yes | gen_ribbon_page.cpp |
| wxRibbonPanel | yes | yes | yes | gen_ribbon_page.cpp |
| wxRibbonToolBar | yes | yes | yes | ribbon_widgets.cpp |
| wxRichTextCtrl | yes | yes | yes | gen_rich_text.cpp |
| wxScrollBar | yes | yes | yes | gen_scrollbar.cpp |
| wxScrolled\<wxPanel\> | yes | yes | yes | styled_text.cpp |
| wxScrolled\<wxWindow\> | yes | yes | yes | styled_text.cpp |
| wxSearchCtrl | yes | yes | yes | gen_search_ctrl.cpp |
| wxSimpleHtmlListBox | yes | yes | yes | gen_html_listbox.cpp |
| wxSlider | yes | yes | yes | gen_slider.cpp |
| wxSpinButton | yes | yes | yes | gen_spin_btn.cpp |
| wxSpinCtrl | yes | yes | yes | gen_spin_ctrl.cpp |
| wxSpinCtrlDouble | yes | yes | yes | gen_spin_ctrl.cpp |
| wxSplitterWindow | yes | yes | yes | gen_split_win.cpp |
| wxStaticBitmap | yes | yes | yes | gen_static_bmp.cpp |
| wxStaticBox | yes | yes | yes | gen_static_box.cpp |
| wxStaticLine | yes | yes | yes | gen_static_line.cpp |
| wxStaticText | yes | yes | yes | gen_static_text.cpp |
| wxStatusBar | yes | yes | yes | gen_status_bar.cpp |
| wxStyledTextCtrl | yes | yes | yes | styled_text.cpp |
| wxTextCtrl | yes | yes | yes | gen_text_ctrl.cpp |
| wxToggleButton | yes | yes | yes | gen_toggle_btn.cpp |
| wxToolBar | yes | yes | yes | gen_toolbar.cpp |
| wxTreeCtrl | yes | yes | yes | gen_tree_ctrl.cpp |
| wxTreeListCtrl | yes | yes | no | gen_tree_list.cpp |
| wxWebView | yes | yes | no | gen_web_view.cpp |
| wxWizardPageSimple | yes | yes | yes | gen_wizard.cpp |

# Objects

| Object | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| AuiTool | yes | yes | yes | gen_aui_toolbar.cpp |
| AuiToolLabel | yes | yes | yes | gen_aui_toolbar.cpp |
| AuiToolSpacer | yes | yes | yes | gen_aui_toolbar.cpp |
| AuiToolStretchSpacer | yes | yes | yes | gen_aui_toolbar.cpp |
| BookPage | yes | yes | yes | gen_book_page.cpp |
| Check3State | yes | yes | yes | gen_checkbox.cpp |
| CustomControl | yes | no | yes | gen_custom_ctrl.cpp |
| DataViewColumn | yes | yes | yes | dataview_widgets.cpp |
| DataViewListColumn | yes | yes | yes | dataview_widgets.cpp |
| NewCloseButton | yes | yes | yes | gen_close_btn.cpp |
| PageCtrl | yes | yes | no | gen_page_ctrl.cpp |
| PropertyGridItem | yes | yes | no | gen_prop_item.cpp |
| PropertyGridPage | yes | yes | no | grid_widgets.cpp |
| RibbonButton | yes | yes | yes | ribbon_widgets.cpp |
| RibbonGalleryItem | yes | yes | yes | ribbon_widgets.cpp |
| RibbonTool | yes | yes | yes | ribbon_widgets.cpp |
| Separator | yes | yes | yes | menu_widgets.cpp |
| Spacer | yes | yes | yes | gen_spacer_sizer.cpp |
| SubMenu | yes | yes | yes | gen_submenu.cpp |
| ToolDropDown | yes | yes | yes | gen_toolbar.cpp |
| ToolGenerator | yes | yes | yes | gen_toolbar.cpp |
| ToolSeparator | yes | yes | yes | gen_toolbar.cpp |
| ToolStretchable | yes | yes | yes | gen_toolbar.cpp |
| TreeListCtrlColumn | yes | yes | no | gen_tree_list.cpp |
| wxContextMenuEvent | yes | ?? | no | gen_ctx_menu.cpp |
