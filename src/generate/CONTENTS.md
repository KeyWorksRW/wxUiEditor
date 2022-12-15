# Contents

The following tables list classes, objects and functions that wxUiEditor can generate code for. If `no` is listed for a column it means that language does not support the item. If '??' is listed for a column it means support has not been added or completed for the item.

# Categories

### Books

| Class | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| wxAuiNotebook | yes | ?? | yes | gen_aui_notebook.cpp |
| wxChoicebook | yes | ?? |  yes | gen_choicebook.cpp |
| wxListbook | yes | ?? |  yes | gen_listbook.cpp |
| wxNotebook | yes | ?? |  yes | gen_notebook.cpp |
| wxSimplebook | yes | ?? |  yes | gen_simplebook.cpp |
| wxToolbook | yes | ?? |  yes | gen_toolbook.cpp |
| wxTreebook | yes | ?? |  yes | gen_treebook.cpp |

### Forms

| Class | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| wxDialog | yes| ?? | yes | gen_dialog.cpp |
| wxFrame | yes| ?? | yes | gen_frame.cpp |
| wxMenuBar | yes| ?? | yes | menu_widgets.cpp |
| wxPanel | yes| ?? | yes | gen_panel_form.cpp |
| wxPopupTransientWindow | yes| ?? | no | gen_popup_trans_win.cpp |
| wxRibbonBar | yes| ?? | yes | ribbon_widgets.cpp |
| wxToolBar | yes| ?? | yes | gen_toolbar.cpp |
| wxWizard | yes| ?? | yes | gen_wizard.cpp |

### Pickers

| Class | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| wxColourPickerCtrl | yes |  ?? | yes | gen_clr_picker.cpp |
| wxDatePickerCtrl | yes |  ?? | yes | gen_date_picker.cpp |
| wxDirPickerCtrl | yes |  ?? | yes | gen_dir_picker.cpp |
| wxFilePickerCtrl | yes |  ?? | yes | gen_file_picker.cpp |
| wxFontPickerCtrl | yes |  ?? | yes | gen_font_picker.cpp |
| wxTimePickerCtrl | yes |  ?? | yes | gen_time_picker.cpp |

### Sizers

| Class | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| wxBoxSizer | yes | yes | yes | gen_box_sizer.cpp |
| wxCheckBoxSizer | yes |  ?? | yes | gen_statchkbox_sizer.cpp |
| wxFlexGridSizer | yes |  ?? | yes | gen_flexgrid_sizer.cpp |
| wxGridBagSizer | yes |  ?? | yes | gen_gridbag_sizer.cpp |
| wxGridSizer | yes |  ?? | yes | gen_grid_sizer.cpp |
| wxRadioButtonSizer | yes |  ?? | yes | gen_statradiobox_sizer.cpp |
| wxStaticBoxSizer | yes |  ?? | yes | gen_staticbox_sizer.cpp |
| wxStdDialogButtonSizer | yes |  yes | yes | gen_std_dlgbtn_sizer.cpp |
| wxTextSizerWrapper | yes |  ?? | no | gen_text_sizer.cpp |
| wxWrapSizer | yes |  ?? | yes | gen_wrap_sizer.cpp |

# Classes

| Class | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| wxActivityIndicator | yes |  yes | yes | gen_activity.cpp |
| wxAnimationCtrl | yes |  yes | yes | gen_animation.cpp |
| wxAuiToolBar | yes |  ?? | yes | gen_aui_toolbar.cpp |
| wxBannerWindow | yes |  yes | yes | gen_banner_window.cpp |
| wxBitmapComboBox | yes |  ?? | yes | gen_bitmap_combo.cpp |
| wxButton | yes |  ?? | yes | gen_button.cpp |
| wxCalendarCtrl | yes |  ?? | yes | gen_calendar_ctrl.cpp |
| wxCheckBox | yes |  yes | yes | gen_checkbox.cpp |
| wxCheckListBox | yes |  ?? | yes | gen_check_listbox.cpp |
| wxChoice | yes |  yes | yes | gen_choice.cpp |
| wxCollapsiblePane | yes |  ?? | yes | gen_collapsible.cpp |
| wxComboBox | yes |  yes | yes | gen_combobox.cpp |
| wxCommandLinkButton | yes |  ?? | yes | gen_cmd_link_btn.cpp |
| wxDataViewCtrl | yes |  ?? | yes | dataview_widgets.cpp |
| wxDataViewListCtrl | yes |  ?? | yes | dataview_widgets.cpp |
| wxEditableListBox | yes |  ?? | yes | gen_edit_listbox.cpp |
| wxFileCtrl | yes |  ?? | yes | gen_file_ctrl.cpp |
| wxGauge | yes |  yes | yes | gen_gauge.cpp |
| wxGenericDirCtrl | yes |  ?? | yes | gen_dir_ctrl.cpp |
| wxGrid | yes |  ?? | yes | gen_grid.cpp |
| wxHtmlWindow | yes |  ?? | yes | gen_html_window.cpp |
| wxHyperlinkCtrl | yes |  ?? | yes | gen_hyperlink.cpp |
| wxInfoBar | yes |  ?? | yes | gen_infobar.cpp |
| wxListBox | yes |  ?? | yes | gen_listbox.cpp |
| wxListView | yes |  ?? | yes | gen_listview.cpp |
| wxMenu | yes |  ?? | yes | menu_widgets.cpp |
| wxMenuBar | yes |  ?? | yes | menu_widgets.cpp |
| wxMenuItem | yes |  ?? | yes | menu_widgets.cpp |
| wxPanel | yes |  ?? | yes | gen_panel.cpp |
| wxPropertyGrid | yes |  ?? | no | gen_prop_grid.cpp |
| wxPropertyGridManager | yes |  ?? | no | grid_widgets.cpp |
| wxRadioBox | yes |  ?? | yes | gen_prop_grid.cpp |
| wxRadioButton | yes |  yes | yes | gen_radio_btn.cpp |
| wxRearrangeCtrl | yes |  ?? | no | gen_rearrange.cpp |
| wxRibbonBar | yes |  ?? | yes | ribbon_widgets.cpp |
| wxRibbonButtonBar | yes |  ?? | yes | ribbon_widgets.cpp |
| wxRibbonGallery | yes |  ?? | yes | ribbon_widgets.cpp |
| wxRibbonPage | yes |  ?? | yes | ribbon_widgets.cpp |
| wxRibbonPanel | yes |  ?? | yes | ribbon_widgets.cpp |
| wxRibbonToolBar | yes |  ?? | yes | ribbon_widgets.cpp |
| wxRichTextCtrl | yes |  ?? | yes | gen_rich_text.cpp |
| wxScrollBar | yes |  ?? | yes | gen_scrollbar.cpp |
| wxScrolled\<wxPanel\> | yes |  ?? | yes | styled_text.cpp |
| wxScrolled\<wxWindow\> | yes |  ?? | yes | styled_text.cpp |
| wxSearchCtrl | yes |  ?? | yes | gen_search_ctrl.cpp |
| wxSimpleHtmlListBox | yes |  ?? | yes | gen_html_listbox.cpp |
| wxSlider | yes |  yes | yes | gen_slider.cpp |
| wxSpinButton | yes |  ?? | yes | gen_spin_btn.cpp |
| wxSpinCtrl | yes |  ?? | yes | gen_spin_ctrl.cpp |
| wxSpinCtrlDouble | yes |  ?? | yes | gen_spin_ctrl.cpp |
| wxSplitterWindow | yes |  ?? | yes | gen_split_win.cpp |
| wxStaticBitmap | yes |  yes | yes | gen_static_bmp.cpp |
| wxStaticBox | yes |  ?? | yes | gen_static_box.cpp |
| wxStaticLine | yes |  yes | yes | gen_static_line.cpp |
| wxStaticText | yes |  yes | yes | gen_static_text.cpp |
| wxStatusBar | yes |  ?? | yes | gen_status_bar.cpp |
| wxStyledTextCtrl | yes |  ?? | yes | styled_text.cpp |
| wxTextCtrl | yes |  ?? | yes | gen_text_ctrl.cpp |
| wxToggleButton | yes |  ?? | yes | gen_toggle_btn.cpp |
| wxToolBar | yes |  ?? | yes | gen_toolbar.cpp |
| wxTreeCtrl | yes |  ?? | yes | gen_toolbar.cpp |
| wxTreeListCtrl | yes |  ?? | no | gen_tree_list.cpp |
| wxWebView | yes |  ?? | no | gen_web_view.cpp |
| wxWizardPageSimple | yes |  ?? | yes | gen_wizard.cpp |

# Objects

| Object | C++ | Python | XRC |file |
-----------|-----|--------|------|------|
| AuiTool | yes |  ?? | yes | gen_aui_toolbar.cpp |
| AuiToolLabel | yes |  ?? | yes | gen_aui_toolbar.cpp |
| AuiToolSpacer | yes |  ?? | yes | gen_aui_toolbar.cpp |
| AuiToolStretchSpacer | yes |  ?? | yes | gen_aui_toolbar.cpp |
| BookPage | yes |  ?? | yes | gen_book_page.cpp |
| Check3State | yes |  ?? | yes | gen_checkbox.cpp |
| CustomControl | yes |  ?? | yes | gen_custom_ctrl.cpp |
| DataViewColumn | yes |  ?? | yes | dataview_widgets.cpp |
| DataViewListColumn | yes |  ?? | yes | dataview_widgets.cpp |
| NewCloseButton | yes |  yes | yes | gen_close_btn.cpp |
| PageCtrl | yes |  ?? | no | gen_page_ctrl.cpp |
| PropertyGridItem | yes |  ?? | no | gen_prop_item.cpp |
| PropertyGridPage | yes |  ?? | no | grid_widgets.cpp |
| RibbonButton | yes |  ?? | yes | ribbon_widgets.cpp |
| RibbonGalleryItem | yes |  ?? | yes | ribbon_widgets.cpp |
| RibbonTool | yes |  ?? | yes | ribbon_widgets.cpp |
| Separator | yes |  ?? | yes | menu_widgets.cpp |
| Spacer | yes |  ?? | yes | gen_spacer_sizer.cpp |
| SubMenu | yes |  ?? | yes | menu_widgets.cpp |
| SubMenu | yes |  ?? | yes | menu_widgets.cpp |
| ToolDropDown | yes |  ?? | yes | gen_toolbar.cpp |
| ToolGenerator | yes |  ?? | yes | gen_toolbar.cpp |
| ToolSeparator | yes |  ?? | yes | gen_toolbar.cpp |
| ToolStretchable | yes |  ?? | yes | gen_toolbar.cpp |
| TreeListCtrlColumn | yes |  ?? | no | gen_tree_list.cpp |
