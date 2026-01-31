# Contents

The following tables indicate whether or not code is being generated in a specific language.

- `---` indicates it has not been verified or not implemented yet.
- `no` indicates that the language doesn't support it.
- `partial` means that not all properties for the control are supported.
- 'y' means that wxPerl supports the control, but needs wxUiEditor to generate the code for it.
- 'n' means that wxPerl does not support the control

## Categories

### Bars

| Class                | C++ | Perl    | Python | Ruby | XRC | file                  |
| -------------------- | --- | ------- | ------ | ---- | --- | --------------------- |
| wxAuiToolBar         | yes | no      | yes    | yes  | yes | gen_aui_toolbar.cpp   |
| AuiTool              | yes | no      | yes    | yes  | yes | gen_aui_toolbar.cpp   |
| AuiToolLabel         | yes | no      | yes    | yes  | yes | gen_aui_toolbar.cpp   |
| AuiToolSpacer        | yes | no      | yes    | yes  | yes | gen_aui_toolbar.cpp   |
| AuiToolStretchSpacer | yes | no      | yes    | yes  | yes | gen_aui_toolbar.cpp   |
| wxInfoBar            | yes | yes     | yes    | yes  | yes | gen_infobar.cpp       |
| wxRibbonBar          | yes | y       | yes    | yes  | --- | gen_ribbon_bar.cpp    |
| wxRibbonButtonBar    | yes | y       | yes    | yes  | --- | gen_ribbon_button.cpp |
| wxRibbonGallery      | yes | y       | yes    | yes  | --- | ribbon_widgets.cpp    |
| wxRibbonToolBar      | yes | y       | yes    | yes  | --- | ribbon_widgets.cpp    |
| RibbonButton         | yes | y       | yes    | yes  | --- | ribbon_widgets.cpp    |
| RibbonGalleryItem    | yes | y       | yes    | yes  | --- | ribbon_widgets.cpp    |
| RibbonTool           | yes | y       | yes    | yes  | --- | ribbon_widgets.cpp    |
| wxRibbonPage         | yes | y       | yes    | yes  | --- | gen_ribbon_page.cpp   |
| wxRibbonPanel        | yes | y       | yes    | yes  | --- | gen_ribbon_page.cpp   |
| wxScrollBar          | yes | y       | yes    | yes  | yes | gen_scrollbar.cpp     |
| wxStatusBar          | yes | partial | yes    | yes  | yes | gen_status_bar.cpp    |
| wxToolBar            | yes | y       | yes    | yes  | yes | gen_toolbar.cpp       |
| ToolDropDown         | yes | y       | ---    | ---  | --- | gen_toolbar.cpp       |
| ToolGenerator        | yes | y       | ---    | ---  | --- | gen_toolbar.cpp       |
| ToolSeparator        | yes | y       | ---    | ---  | --- | gen_toolbar.cpp       |
| ToolStretchable      | yes | y       | ---    | ---  | --- | gen_toolbar.cpp       |
| Separator            | yes | yes     | yes    | yes  | --- | menu_widgets.cpp      |

### Books

| Class         | C++ | Perl | Python | Ruby | XRC | file                 |
| ------------- | --- | ---- | ------ | ---- | --- | -------------------- |
| wxAuiNotebook | yes | yes  | yes    | yes  | yes | gen_aui_notebook.cpp |
| wxChoicebook  | yes | yes  | yes    | yes  | yes | gen_choicebook.cpp   |
| wxListbook    | yes | yes  | yes    | yes  | yes | gen_listbook.cpp     |
| wxNotebook    | yes | yes  | yes    | yes  | yes | gen_notebook.cpp     |
| wxSimplebook  | yes | no   | yes    | yes  | yes | gen_simplebook.cpp   |
| wxToolbook    | yes | yes  | yes    | yes  | yes | gen_toolbook.cpp     |
| wxTreebook    | yes | yes  | yes    | yes  | yes | gen_treebook.cpp     |

### Containers

| Class                  | C++ | Perl | Python | Ruby | XRC | file                |
| ---------------------- | --- | ---- | ------ | ---- | --- | ------------------- |
| wxCollapsiblePane      | yes | yes  | yes    | yes  | yes | gen_collapsible.cpp |
| wxPanel                | yes | yes  | yes    | yes  | yes | gen_panel.cpp       |
| wxSplitterWindow       | yes | yes  | yes    | yes  | yes | gen_split_win.cpp   |
| wxScrolled\<wxPanel\>  | yes | yes  | yes    | yes  | yes | window_widgets.cpp  |
| wxScrolled\<wxWindow\> | yes | y    | yes    | yes  | --- | window_widgets.cpp  |

### Forms

| Class                  | C++ | Perl | Python | Ruby | XRC | file                     |
| ---------------------- | --- | ---- | ------ | ---- | --- | ------------------------ |
| MdiFrameMenuBar        | --- | ---  | ---    | ---  | --- | gen_mdi_menu.cpp         |
| MdiDocumentMenuBar     | --- | ---  | ---    | ---  | --- | gen_mdi_menu.cpp         |
| wxDialog               | yes | yes  | yes    | yes  | yes | gen_dialog.cpp           |
| wxFrame                | yes | yes  | yes    | yes  | yes | gen_frame.cpp            |
| wxDocParentFrame       | yes | y    | ---    | no   | --- | gen_doc_parent_frame.cpp |
| wxDocChildFrame        | yes | y    | ---    | no   | --- | gen_doc_parent_frame.cpp |
| wxDocMDIParentFrame    | yes | y    | ---    | no   | --- | gen_doc_mdi_frame.cpp    |
| wxDocMDIChildFrame     | yes | y    | ---    | no   | --- | gen_doc_mdi_frame.cpp    |
| wxAuiMDIParentFrame    | yes | n    | ---    | no   | --- | gen_aui_mdi_frame.cpp    |
| wxAuiMDIChildFrame     | yes | n    | ---    | no   | --- | gen_aui_mdi_frame.cpp    |
| wxMenu                 | yes | y    | yes    | ---  | --- | menu_widgets.cpp         |
| wxMenuBar              | yes | y    | yes    | ---  | --- | menu_widgets.cpp         |
| wxPanel                | yes | yes  | yes    | yes  | --- | gen_panel_form.cpp       |
| wxPopupTransientWindow | yes | y    | yes    | yes  | no  | gen_popup_trans_win.cpp  |
| wxPropertySheetDialog  | yes | y    | ---    | ---  | --- | gen_propsheet_dlg.cpp    |
| wxRibbonBar            | yes | y    | yes    | ---  | --- | gen_ribbon_bar.cpp       |
| wxToolBar              | yes | y    | yes    | ---  | --- | gen_toolbar.cpp          |
| wxWizard               | yes | y    | yes    | yes  | yes | gen_wizard.cpp           |

### Menus

| Class            | C++ | Perl | Python | Ruby | XRC | file             |
| ---------------- | --- | ---- | ------ | ---- | --- | ---------------- |
| CtxMenuGenerator | yes | no   | no     | no   | no  | gen_ctx_menu.cpp |
| wxMenu           | yes | yes  | yes    | yes  | yes | gen_menu.cpp     |
| wxMenuBar        | yes | yes  | yes    | yes  | yes | menu_widgets.cpp |
| wxMenuItem       | yes | yes  | yes    | yes  | yes | gen_menuitem.cpp |
| SubMenu          | yes | yes  | yes    | yes  | yes | gen_submenu.cpp  |

### Pickers

| Class              | C++ | Perl | Python | Ruby | XRC | file                |
| ------------------ | --- | ---- | ------ | ---- | --- | ------------------- |
| wxColourPickerCtrl | yes | yes  | yes    | yes  | yes | gen_clr_picker.cpp  |
| wxDatePickerCtrl   | yes | yes  | yes    | yes  | yes | gen_date_picker.cpp |
| wxDirPickerCtrl    | yes | yes  | yes    | yes  | yes | gen_dir_picker.cpp  |
| wxFilePickerCtrl   | yes | yes  | yes    | yes  | yes | gen_file_picker.cpp |
| wxFontPickerCtrl   | yes | yes  | yes    | yes  | yes | gen_font_picker.cpp |
| wxTimePickerCtrl   | yes | yes  | yes    | yes  | yes | gen_time_picker.cpp |

### PropertyGrid

| Class                 | C++ | Perl | Python | Ruby | XRC | file                  |
| --------------------- | --- | ---- | ------ | ---- | --- | --------------------- |
| PropertyGridCategory  | yes | y    | ---    | ---  | no  | gen_prop_category.cpp |
| PropertyGridItem      | yes | y    | yes    | yes  | no  | gen_prop_item.cpp     |
| wxPropertyGrid        | yes | y    | yes    | yes  | no  | gen_prop_grid.cpp     |
| wxPropertyGridManager | yes | y    | yes    | yes  | no  | gen_prop_grid_mgr.cpp |

### Sizers

| Class                    | C++ | Perl | Python | Ruby | XRC     | file                       |
| ------------------------ | --- | ---- | ------ | ---- | ------- | -------------------------- |
| wxBoxSizer               | yes | yes  | yes    | yes  | yes     | gen_box_sizer.cpp          |
| wxCheckBoxSizer          | yes | y    | no     | yes  | ---     | gen_statchkbox_sizer.cpp   |
| wxFlexGridSizer          | yes | yes  | yes    | yes  | yes     | gen_flexgrid_sizer.cpp     |
| wxGridBagSizer           | yes | yes  | yes    | yes  | yes     | gen_gridbag_sizer.cpp      |
| wxGridSizer              | yes | yes  | yes    | yes  | yes     | gen_grid_sizer.cpp         |
| wxStaticRadioBtnBoxSizer | yes | y    | no     | yes  | ---     | gen_statradiobox_sizer.cpp |
| wxStaticBoxSizer         | yes | yes  | yes    | yes  | yes     | gen_staticbox_sizer.cpp    |
| wxStdDialogButtonSizer   | yes | yes  | yes    | yes  | partial | gen_std_dlgbtn_sizer.cpp   |
| wxWrapSizer              | yes | yes  | yes    | yes  | yes     | gen_wrap_sizer.cpp         |

### Other

| Class                  | C++ | Perl    | Python  | Ruby | XRC     | file                  |
| ---------------------- | --- | ------- | ------- | ---- | ------- | --------------------- |
| wxActivityIndicator    | yes | no     | yes     | yes  | yes     | gen_activity.cpp      |
| wxAnimationCtrl        | yes | y       | ---     | yes  | ---     | gen_animation.cpp     |
| wxBannerWindow         | yes | y       | yes     | yes  | yes     | gen_banner_window.cpp |
| wxBitmapComboBox       | --- | y       | ---     | ---  | ---     | gen_bitmap_combo.cpp  |
| wxButton               | yes | yes     | yes     | yes  | partial | gen_button.cpp        |
| wxCalendarCtrl         | yes | yes     | yes     | yes  | yes     | gen_calendar_ctrl.cpp |
| wxCheckBox             | yes | yes     | yes     | yes  | yes     | gen_checkbox.cpp      |
| wxCheckListBox         | yes | y       | yes     | yes  | yes     | gen_check_listbox.cpp |
| wxChoice               | yes | yes     | yes     | yes  | yes     | gen_choice.cpp        |
| wxComboBox             | yes | yes     | yes     | yes  | yes     | gen_combobox.cpp      |
| wxCommandLinkButton    | yes | yes     | yes     | yes  | yes     | gen_cmd_link_btn.cpp  |
| wxDataViewListCtrl     | yes | y       | yes     | no   | no      | dataview_widgets.cpp  |
| wxDataViewCtrl         | yes | y       | yes     | no   | no      | dataview_widgets.cpp  |
| wxEditableListBox      | yes | y       | yes     | yes  | yes     | gen_edit_listbox.cpp  |
| wxFileCtrl             | yes | yes     | yes     | yes  | yes     | gen_file_ctrl.cpp     |
| wxGauge                | yes | yes     | yes     | yes  | yes     | gen_gauge.cpp         |
| wxGenericDirCtrl       | yes | y       | partial | yes  | partial | gen_dir_ctrl.cpp      |
| wxGrid                 | yes | y       | yes     | yes  | no      | gen_grid.cpp          |
| wxHtmlWindow           | yes | yes     | yes     | yes  | yes     | gen_html_window.cpp   |
| wxHyperlinkCtrl        | yes | yes     | yes     | yes  | yes     | gen_hyperlink.cpp     |
| wxListBox              | yes | y       | yes     | yes  | yes     | gen_listbox.cpp       |
| wxListView             | yes | y       | yes     | yes  | partial | gen_listview.cpp      |
| wxRadioBox             | yes | y       | yes     | yes  | yes     | gen_prop_grid.cpp     |
| wxRadioButton          | yes | yes     | yes     | yes  | yes     | gen_radio_btn.cpp     |
| wxRearrangeCtrl        | yes | y       | yes     | yes  | no      | gen_rearrange.cpp     |
| wxRichTextCtrl         | yes | y       | yes     | yes  | yes     | gen_rich_text.cpp     |
| wxSearchCtrl           | yes | y       | yes     | yes  | yes     | gen_search_ctrl.cpp   |
| wxSimpleHtmlListBox    | yes | y       | yes     | yes  | yes     | gen_html_listbox.cpp  |
| wxSlider               | yes | yes     | yes     | yes  | partial | gen_slider.cpp        |
| wxSpinButton           | yes | y       | yes     | yes  | yes     | gen_spin_btn.cpp      |
| wxSpinCtrl             | yes | y       | yes     | yes  | yes     | gen_spin_ctrl.cpp     |
| wxSpinCtrlDouble       | yes | y       | yes     | yes  | yes     | gen_spin_ctrl.cpp     |
| wxStaticBitmap         | yes | y       | yes     | yes  | yes     | gen_static_bmp.cpp    |
| wxStaticBox            | yes | y       | ---     | ---  | ---     | gen_static_box.cpp    |
| wxStaticLine           | yes | yes     | yes     | yes  | yes     | gen_static_line.cpp   |
| wxStaticText           | yes | partial | yes     | yes  | yes     | gen_static_text.cpp   |
| wxStyledTextCtrl       | yes | y       | partial | yes  | partial | styled_text.cpp       |
| wxTextCtrl             | yes | yes     | yes     | yes  | yes     | gen_text_ctrl.cpp     |
| wxToggleButton         | yes | y       | yes     | yes  | partial | gen_toggle_btn.cpp    |
| wxTreeCtrl             | yes | y       | yes     | yes  | yes     | gen_tree_ctrl.cpp     |
| wxTreeListCtrl         | yes | y       | yes     | no   | no      | gen_tree_list.cpp     |
| wxWebView              | yes | y       | yes     | no   | no      | gen_web_view.cpp      |
| wxWizardPageSimple     | yes | y       | yes     | yes  | yes     | gen_wizard.cpp        |

# Objects

| Class              | C++ | Perl | Python | Ruby | XRC | file                 |
| ------------------ | --- | ---- | ------ | ---- | --- | -------------------- |
| BookPage           | yes | yes  | yes    | yes  | yes | gen_book_page.cpp    |
| Check3State        | yes | yes  | yes    | yes  | yes | gen_checkbox.cpp     |
| CustomControl      | yes | n    | no     | ---  | --- | gen_custom_ctrl.cpp  |
| DataViewColumn     | yes | y    | yes    | no   | no  | dataview_widgets.cpp |
| DataViewListColumn | yes | y    | ---    | ---  | --- | dataview_widgets.cpp |
| NewCloseButton     | yes | yes  | yes    | yes  | yes | gen_close_btn.cpp    |
| PageCtrl           | yes | n    | ---    | ---  | no  | gen_page_ctrl.cpp    |
| PropertyGridItem   | yes | y    | yes    | ---  | no  | gen_prop_item.cpp    |
| PropertyGridPage   | yes | y    | yes    | ---  | no  | grid_widgets.cpp     |
| Spacer             | yes | yes  | yes    | yes  | yes | gen_spacer_sizer.cpp |
| TreeListCtrlColumn | yes | y    | ---    | ---  | no  | gen_tree_list.cpp    |
| wxContextMenuEvent | yes | n    | no     | ---  | no  | gen_ctx_menu.cpp     |
| wxTextProofOptions | yes | no   | no     | ---  | --- | ---                  |
| wxSizerFlags       | yes | no   | yes    | yes  | no  | ---                  |

# Functions

| Function                   | C++ | Perl | Python | Ruby | XRC | file                     |
| -------------------------- | --- | ---- | ------ | ---- | --- | ------------------------ |
| CreateSeparatedSizer       | yes | no   | yes    | yes  | --- | gen_std_dlgbtn_sizer.cpp |
| CreateStdDialogButtonSizer | yes | yes  | yes    | yes  | --- | gen_std_dlgbtn_sizer.cpp |
| FromDIP                    | yes | no   | yes    | yes  | yes | ---                      |
| GetControlSizer            | yes | n    | no     | ???  | --- | gen_choicebook.cpp       |
| GetAffirmativeButton       | yes | n    | no     | ???  | --- | gen_std_dlgbtn_sizer.cpp |


# Constants

| Constant       | C++ | Perl | Python | Ruby | XRC |
| -------------- | --- | ---- | ------ | ---- | --- |
| wxDefaultCoord | yes | no   | yes    | yes  | yes |

# Special Notes:

Neither wxPython nor XRC support setting the initial folder in wxGenericDirCtrl, though it works fine in wxRuby3

The Unsupported lists below are for controls that the language port doesn't support.

# Python Unsupported

- CheckBoxSizer
- RadioBoxSizer
- wxTextProofOptions (for wxTextCtrl)

# Perl Unsupported

## Classes
- wxAuiToolbar
- wxSimpleBook

- wxSizerFlags
- wxTextProofOptions (for wxTextCtrl)

## Constants
- wxSHOW_EFFECT_* constants
- wxStatusBar doesn't support wxSB_SUNKEN style, and it doesn't appear to allow more than one field.
- wxREMOVE_LEADING_SPACES not a valid constant for wxWrapSizer
- wxWRAPSIZER_DEFAULT_FLAGS not a valid constant for wxWrapSizer
- wxBU_NOTEXT not supported as a button style
- wxDefaultCoord not a valid constant

## Functions
- wxButton::SetLabelMarkup()
- wxDialog::CreateSeparatedSizer()
- wxStaticText.SetLabelMarkup()
- wxTextCtrl::SetHint()
- wxWindow.FromDIP()

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
