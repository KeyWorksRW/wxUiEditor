# Contents

The following tables indicate whether or not code is being generated in a specific language.

- `---` indicates it has not been verified or not implemented yet.
- `no` indicates that the language doesn't support it.
- `partial` means that not all properties for the control are supported.

## Categories

### Bars

| Class                | C++ | Python | Ruby | Fortran | GO  | Julia | Lua | Rust | TypeScript | XRC | file                  |
| -------------------- | --- | ------ | ---- | ------- | --- | ----- | --- | ---- | ---------- | --- | --------------------- |
| AuiTool              | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_aui_toolbar.cpp   |
| AuiToolLabel         | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_aui_toolbar.cpp   |
| AuiToolSpacer        | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_aui_toolbar.cpp   |
| AuiToolStretchSpacer | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_aui_toolbar.cpp   |
| infobar_btn          | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_infobar_btn.cpp   |
| RibbonButton         | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | --- | ribbon_widgets.cpp    |
| RibbonGalleryItem    | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | --- | ribbon_widgets.cpp    |
| RibbonSeparator      | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_tools.cpp         |
| RibbonTool           | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | --- | ribbon_widgets.cpp    |
| Separator            | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | --- | menu_widgets.cpp      |
| ToolDropDown         | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_toolbar.cpp       |
| ToolGenerator        | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_toolbar.cpp       |
| ToolSeparator        | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_toolbar.cpp       |
| ToolStretchable      | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_toolbar.cpp       |
| wxAuiToolBar         | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_aui_toolbar.cpp   |
| wxInfoBar            | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_infobar.cpp       |
| wxRibbonBar          | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_ribbon_bar.cpp    |
| wxRibbonButtonBar    | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_ribbon_button.cpp |
| wxRibbonGallery      | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | --- | ribbon_widgets.cpp    |
| wxRibbonPage         | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_ribbon_page.cpp   |
| wxRibbonPanel        | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_ribbon_page.cpp   |
| wxRibbonToolBar      | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | --- | ribbon_widgets.cpp    |
| wxScrollBar          | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_scrollbar.cpp     |
| wxStatusBar          | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_status_bar.cpp    |
| wxToolBar            | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_toolbar.cpp       |

### Books

| Class         | C++ | Python | Ruby | Fortran | GO  | Julia | Lua | Rust | TypeScript | XRC | file                 |
| ------------- | --- | ------ | ---- | ------- | --- | ----- | --- | ---- | ---------- | --- | -------------------- |
| wxAuiNotebook | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_aui_notebook.cpp |
| wxChoicebook  | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_choicebook.cpp   |
| wxListbook    | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_listbook.cpp     |
| wxNotebook    | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_notebook.cpp     |
| wxSimplebook  | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_simplebook.cpp   |
| wxToolbook    | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_toolbook.cpp     |
| wxTreebook    | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_treebook.cpp     |

### Containers

| Class                  | C++ | Python | Ruby | Fortran | GO  | Julia | Lua | Rust | TypeScript | XRC | file                |
| ---------------------- | --- | ------ | ---- | ------- | --- | ----- | --- | ---- | ---------- | --- | ------------------- |
| wxCollapsiblePane      | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_collapsible.cpp |
| wxPanel                | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_panel.cpp       |
| wxScrolled\<wxPanel\>  | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | window_widgets.cpp  |
| wxScrolled\<wxWindow\> | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | --- | window_widgets.cpp  |
| wxSplitterWindow       | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_split_win.cpp   |

### Forms

| Class                  | C++ | Python | Ruby | Fortran | GO  | Julia | Lua | Rust | TypeScript | XRC | file                     |
| ---------------------- | --- | ------ | ---- | ------- | --- | ----- | --- | ---- | ---------- | --- | ------------------------ |
| MdiDocumentMenuBar     | --- | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_mdi_menu.cpp         |
| MdiFrameMenuBar        | --- | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_mdi_menu.cpp         |
| PopupMenu              | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | menu_widgets.cpp         |
| wxAuiMDIChildFrame     | yes | ---    | no   | ---     | --- | ---   | --- | ---  | ---        | --- | gen_aui_mdi_frame.cpp    |
| wxAuiMDIParentFrame    | yes | ---    | no   | ---     | --- | ---   | --- | ---  | ---        | --- | gen_aui_mdi_frame.cpp    |
| wxDialog               | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_dialog.cpp           |
| wxDocChildFrame        | yes | ---    | no   | ---     | --- | ---   | --- | ---  | ---        | --- | gen_doc_parent_frame.cpp |
| wxDocMDIChildFrame     | yes | ---    | no   | ---     | --- | ---   | --- | ---  | ---        | --- | gen_doc_mdi_frame.cpp    |
| wxDocMDIParentFrame    | yes | ---    | no   | ---     | --- | ---   | --- | ---  | ---        | --- | gen_doc_mdi_frame.cpp    |
| wxDocParentFrame       | yes | ---    | no   | ---     | --- | ---   | --- | ---  | ---        | --- | gen_doc_parent_frame.cpp |
| wxFrame                | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_frame.cpp            |
| wxMenu                 | yes | yes    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | menu_widgets.cpp         |
| wxMenuBar              | yes | yes    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | menu_widgets.cpp         |
| wxPanel                | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_panel_form.cpp       |
| wxPopupTransientWindow | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | no  | gen_popup_trans_win.cpp  |
| wxPopupWindow          | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_popup_win.cpp        |
| wxPropertySheetDialog  | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_propsheet_dlg.cpp    |
| wxRibbonBar            | yes | yes    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_ribbon_bar.cpp       |
| wxToolBar              | yes | yes    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_toolbar.cpp          |
| wxWizard               | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_wizard.cpp           |

### Menus

| Class            | C++ | Python | Ruby | Fortran | GO  | Julia | Lua | Rust | TypeScript | XRC | file             |
| ---------------- | --- | ------ | ---- | ------- | --- | ----- | --- | ---- | ---------- | --- | ---------------- |
| CtxMenuGenerator | yes | no     | no   | ---     | --- | ---   | --- | ---  | ---        | no  | gen_ctx_menu.cpp |
| SubMenu          | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_submenu.cpp  |
| wxMenu           | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_menu.cpp     |
| wxMenuBar        | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | menu_widgets.cpp |
| wxMenuItem       | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_menuitem.cpp |

### Pickers

| Class              | C++ | Python | Ruby | Fortran | GO  | Julia | Lua | Rust | TypeScript | XRC | file                |
| ------------------ | --- | ------ | ---- | ------- | --- | ----- | --- | ---- | ---------- | --- | ------------------- |
| wxColourPickerCtrl | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_clr_picker.cpp  |
| wxDatePickerCtrl   | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_date_picker.cpp |
| wxDirPickerCtrl    | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_dir_picker.cpp  |
| wxFilePickerCtrl   | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_file_picker.cpp |
| wxFontPickerCtrl   | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_font_picker.cpp |
| wxTimePickerCtrl   | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_time_picker.cpp |

### PropertyGrid

| Class                 | C++ | Python | Ruby | Fortran | GO  | Julia | Lua | Rust | TypeScript | XRC | file                  |
| --------------------- | --- | ------ | ---- | ------- | --- | ----- | --- | ---- | ---------- | --- | --------------------- |
| PropertyGridCategory  | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | no  | gen_prop_category.cpp |
| PropertyGridItem      | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | no  | gen_prop_item.cpp     |
| wxPropertyGrid        | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | no  | gen_prop_grid.cpp     |
| wxPropertyGridManager | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | no  | gen_prop_grid_mgr.cpp |

### Sizers

| Class                    | C++ | Python | Ruby | Fortran | GO  | Julia | Lua | Rust | TypeScript | XRC     | file                       |
| ------------------------ | --- | ------ | ---- | ------- | --- | ----- | --- | ---- | ---------- | ------- | -------------------------- |
| TextSizer                | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | ---     | gen_text_sizer.cpp         |
| VerticalBoxSizer         | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | ---     | gen_box_sizer.cpp          |
| wxBoxSizer               | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_box_sizer.cpp          |
| wxCheckBoxSizer          | yes | no     | yes  | ---     | --- | ---   | --- | ---  | ---        | ---     | gen_statchkbox_sizer.cpp   |
| wxFlexGridSizer          | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_flexgrid_sizer.cpp     |
| wxGridBagSizer           | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_gridbag_sizer.cpp      |
| wxGridSizer              | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_grid_sizer.cpp         |
| wxStaticBoxSizer         | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_staticbox_sizer.cpp    |
| wxStaticRadioBtnBoxSizer | yes | no     | yes  | ---     | --- | ---   | --- | ---  | ---        | ---     | gen_statradiobox_sizer.cpp |
| wxStdDialogButtonSizer   | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | partial | gen_std_dlgbtn_sizer.cpp   |
| wxWrapSizer              | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_wrap_sizer.cpp         |

### Other

| Class                  | C++ | Python  | Ruby | Fortran | GO  | Julia | Lua | Rust | TypeScript | XRC     | file                  |
| ---------------------- | --- | ------- | ---- | ------- | --- | ----- | --- | ---- | ---------- | ------- | --------------------- |
| wxActivityIndicator    | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_activity.cpp      |
| wxAnimationCtrl        | yes | ---     | yes  | ---     | --- | ---   | --- | ---  | ---        | ---     | gen_animation.cpp     |
| wxBannerWindow         | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_banner_window.cpp |
| wxBitmapComboBox       | --- | ---     | ---  | ---     | --- | ---   | --- | ---  | ---        | ---     | gen_bitmap_combo.cpp  |
| wxBitmapToggleButton   | yes | ---     | ---  | ---     | --- | ---   | --- | ---  | ---        | ---     | gen_toggle_btn.cpp    |
| wxButton               | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | partial | gen_button.cpp        |
| wxCalendarCtrl         | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_calendar_ctrl.cpp |
| wxCheckBox             | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_checkbox.cpp      |
| wxCheckListBox         | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_check_listbox.cpp |
| wxChoice               | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_choice.cpp        |
| wxComboBox             | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_combobox.cpp      |
| wxCommandLinkButton    | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_cmd_link_btn.cpp  |
| wxContextHelpButton    | yes | ---     | ---  | ---     | --- | ---   | --- | ---  | ---        | ---     | gen_ctx_help_btn.cpp  |
| wxDataViewCtrl         | yes | yes     | no   | ---     | --- | ---   | --- | ---  | ---        | no      | dataview_widgets.cpp  |
| wxDataViewListCtrl     | yes | yes     | no   | ---     | --- | ---   | --- | ---  | ---        | no      | dataview_widgets.cpp  |
| wxDataViewTreeCtrl     | yes | ---     | ---  | ---     | --- | ---   | --- | ---  | ---        | ---     | dataview_widgets.cpp  |
| wxEditableListBox      | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_edit_listbox.cpp  |
| wxFileCtrl             | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_file_ctrl.cpp     |
| wxGauge                | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_gauge.cpp         |
| wxGenericDirCtrl       | yes | partial | yes  | ---     | --- | ---   | --- | ---  | ---        | partial | gen_dir_ctrl.cpp      |
| wxGrid                 | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | no      | gen_grid.cpp          |
| wxHtmlWindow           | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_html_window.cpp   |
| wxHyperlinkCtrl        | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_hyperlink.cpp     |
| wxListBox              | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_listbox.cpp       |
| wxListView             | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | partial | gen_listview.cpp      |
| wxRadioBox             | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_prop_grid.cpp     |
| wxRadioButton          | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_radio_btn.cpp     |
| wxRearrangeCtrl        | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | no      | gen_rearrange.cpp     |
| wxRichTextCtrl         | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_rich_text.cpp     |
| wxSearchCtrl           | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_search_ctrl.cpp   |
| wxSimpleHtmlListBox    | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_html_listbox.cpp  |
| wxSlider               | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | partial | gen_slider.cpp        |
| wxSpinButton           | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_spin_btn.cpp      |
| wxSpinCtrl             | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_spin_ctrl.cpp     |
| wxSpinCtrlDouble       | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_spin_ctrl.cpp     |
| wxStaticBitmap         | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_static_bmp.cpp    |
| wxStaticBox            | yes | ---     | ---  | ---     | --- | ---   | --- | ---  | ---        | ---     | gen_static_box.cpp    |
| wxStaticLine           | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_static_line.cpp   |
| wxStaticText           | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_static_text.cpp   |
| wxStyledTextCtrl       | yes | partial | yes  | ---     | --- | ---   | --- | ---  | ---        | partial | styled_text.cpp       |
| wxTextCtrl             | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_text_ctrl.cpp     |
| wxTimer                | yes | ---     | ---  | ---     | --- | ---   | --- | ---  | ---        | ---     | gen_timer.cpp         |
| wxToggleButton         | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | partial | gen_toggle_btn.cpp    |
| wxTreeCtrl             | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_tree_ctrl.cpp     |
| wxTreeListCtrl         | yes | yes     | no   | ---     | --- | ---   | --- | ---  | ---        | no      | gen_tree_list.cpp     |
| wxVListBox             | yes | ---     | ---  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_vlistbox.cpp      |
| wxWebView              | yes | yes     | no   | ---     | --- | ---   | --- | ---  | ---        | no      | gen_web_view.cpp      |
| wxWizardPageSimple     | yes | yes     | yes  | ---     | --- | ---   | --- | ---  | ---        | yes     | gen_wizard.cpp        |

### DocView

| Class                    | C++ | Python | Ruby | Fortran | GO  | Julia | Lua | Rust | TypeScript | XRC | file                        |
| ------------------------ | --- | ------ | ---- | ------- | --- | ----- | --- | ---- | ---------- | --- | --------------------------- |
| DocumentImage            | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | mdi/gen_doc_image.cpp       |
| DocumentRichTextCtrl     | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | mdi/gen_doc_richtext.cpp    |
| DocumentSplitterWindow   | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | mdi/gen_doc_splitter.cpp    |
| DocumentStyledTextCtrl   | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | mdi/gen_doc_scintilla.cpp   |
| DocumentTextCtrl         | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | mdi/gen_doc_textctrl.cpp    |
| DocViewApp               | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | mdi/gen_doc_view_app.cpp    |
| ViewImage                | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | mdi/gen_view_image.cpp      |
| ViewRichTextCtrl         | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | mdi/gen_view_richtext.cpp   |
| ViewSplitterWindow       | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | mdi/gen_view_splitter.cpp   |
| ViewStyledTextCtrl       | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | mdi/gen_view_scintilla.cpp  |
| ViewTextCtrl             | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | mdi/gen_view_textctrl.cpp   |

# Objects

| Class              | C++ | Python | Ruby | Fortran | GO  | Julia | Lua | Rust | TypeScript | XRC | file                 |
| ------------------ | --- | ------ | ---- | ------- | --- | ----- | --- | ---- | ---------- | --- | -------------------- |
| BookPage           | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_book_page.cpp    |
| Check3State        | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_checkbox.cpp     |
| CustomControl      | yes | no     | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_custom_ctrl.cpp  |
| Data               | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_data_list.cpp    |
| data_string        | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_data_list.cpp    |
| data_xml           | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_data_list.cpp    |
| DataViewColumn     | yes | yes    | no   | ---     | --- | ---   | --- | ---  | ---        | no  | dataview_widgets.cpp |
| DataViewListColumn | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | dataview_widgets.cpp |
| embedded_image     | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_images_list.cpp  |
| folder             | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_project.cpp      |
| Images             | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_images_list.cpp  |
| NewCloseButton     | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_close_btn.cpp    |
| PageCtrl           | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | no  | gen_page_ctrl.cpp    |
| Project            | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_project.cpp      |
| PropertyGridItem   | yes | yes    | ---  | ---     | --- | ---   | --- | ---  | ---        | no  | gen_prop_item.cpp    |
| PropertyGridPage   | yes | yes    | ---  | ---     | --- | ---   | --- | ---  | ---        | no  | grid_widgets.cpp     |
| Spacer             | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_spacer_sizer.cpp |
| sub_folder         | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_project.cpp      |
| TreeListCtrlColumn | yes | ---    | ---  | ---     | --- | ---   | --- | ---  | ---        | no  | gen_tree_list.cpp    |
| TreePage           | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | gen_tree_page.cpp    |
| wxContextMenuEvent | yes | no     | ---  | ---     | --- | ---   | --- | ---  | ---        | no  | gen_ctx_menu.cpp     |
| wxSizerFlags       | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | no  | ---                  |
| wxTextProofOptions | yes | no     | ---  | ---     | --- | ---   | --- | ---  | ---        | --- | ---                  |

# Functions

| Function                   | C++ | Python | Ruby | Fortran | GO  | Julia | Lua | Rust | TypeScript | XRC | file                     |
| -------------------------- | --- | ------ | ---- | ------- | --- | ----- | --- | ---- | ---------- | --- | ------------------------ |
| CreateSeparatedSizer       | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_std_dlgbtn_sizer.cpp |
| CreateStdDialogButtonSizer | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_std_dlgbtn_sizer.cpp |
| FromDIP                    | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes | ---                      |
| GetAffirmativeButton       | yes | no     | ???  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_std_dlgbtn_sizer.cpp |
| GetControlSizer            | yes | no     | ???  | ---     | --- | ---   | --- | ---  | ---        | --- | gen_choicebook.cpp       |


# Constants

| Constant       | C++ | Python | Ruby | Fortran | GO  | Julia | Lua | Rust | TypeScript | XRC |
| -------------- | --- | ------ | ---- | ------- | --- | ----- | --- | ---- | ---------- | --- |
| wxDefaultCoord | yes | yes    | yes  | ---     | --- | ---   | --- | ---  | ---        | yes |

# Special Notes:

Neither wxPython nor XRC support setting the initial folder in wxGenericDirCtrl, though it works fine in wxRuby3

The Unsupported lists below are for controls that the language port doesn't support.

# Python Unsupported

- CheckBoxSizer
- RadioBoxSizer
- wxTextProofOptions (for wxTextCtrl)

# Ruby Unsupported

- wxBannerWindow
- wxRearrangeCtrl
- wxSpinCtrlDouble
- wxWebView
- wxCommandLinkButton
- wxDocParentFrame
- wxDocMDIParentFrame

- wxDataViewListCtrl
- wxDataViewCtrl
- DataViewColumn
- DataViewListColumn
- wxTreeListCtrl
