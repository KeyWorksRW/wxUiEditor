# Contents

The following tables indicate whether or not code is being generated in a specific language.

- `---` indicates it has not been verified or not implemented yet.
- `no` indcates that the language doesn't support it.
- `partial` means that not all properties for the control are supported.

## Categories

### Bars

| Class                | C++ | Perl    | Python | Ruby | XRC | file                  |
| -------------------- | --- | ------- | ------ | ---- | --- | --------------------- |
| wxAuiToolBar         | yes | ---     | yes    | yes  | yes | gen_aui_toolbar.cpp   |
| AuiTool              | yes | ---     | yes    | yes  | yes | gen_aui_toolbar.cpp   |
| AuiToolLabel         | yes | ---     | yes    | yes  | yes | gen_aui_toolbar.cpp   |
| AuiToolSpacer        | yes | ---     | yes    | yes  | yes | gen_aui_toolbar.cpp   |
| AuiToolStretchSpacer | yes | ---     | yes    | yes  | yes | gen_aui_toolbar.cpp   |
| wxInfoBar            | yes | ---     | ---    | ---  | yes | gen_infobar.cpp       |
| wxMenu               | yes | yes     | yes    | yes  | --- | gen_menu.cpp          |
| wxMenuBar            | yes | yes     | yes    | yes  | --- | menu_widgets.cpp      |
| wxMenuItem           | yes | yes     | yes    | yes  | --- | gen_menuitem.cpp      |
| SubMenu              | yes | ---     | ---    | ---  | --- | gen_submenu.cpp       |
| wxRibbonBar          | yes | ---     | yes    | yes  | --- | gen_ribbon_bar.cpp    |
| wxRibbonButtonBar    | yes | ---     | yes    | yes  | --- | gen_ribbon_button.cpp |
| wxRibbonGallery      | yes | ---     | yes    | yes  | --- | ribbon_widgets.cpp    |
| wxRibbonToolBar      | yes | ---     | yes    | yes  | --- | ribbon_widgets.cpp    |
| RibbonButton         | yes | ---     | yes    | yes  | --- | ribbon_widgets.cpp    |
| RibbonGalleryItem    | yes | ---     | yes    | yes  | --- | ribbon_widgets.cpp    |
| RibbonTool           | yes | ---     | yes    | yes  | --- | ribbon_widgets.cpp    |
| wxRibbonPage         | yes | ---     | yes    | yes  | --- | gen_ribbon_page.cpp   |
| wxRibbonPanel        | yes | ---     | yes    | yes  | --- | gen_ribbon_page.cpp   |
| wxScrollBar          | yes | ---     | yes    | yes  | yes | gen_scrollbar.cpp     |
| wxStatusBar          | yes | partial | yes    | yes  | yes | gen_status_bar.cpp    |
| wxToolBar            | yes | ---     | yes    | yes  | yes | gen_toolbar.cpp       |
| ToolDropDown         | yes | ---     | ---    | ---  | --- | gen_toolbar.cpp       |
| ToolGenerator        | yes | ---     | ---    | ---  | --- | gen_toolbar.cpp       |
| ToolSeparator        | yes | ---     | ---    | ---  | --- | gen_toolbar.cpp       |
| ToolStretchable      | yes | ---     | ---    | ---  | --- | gen_toolbar.cpp       |
| Separator            | yes | yes     | ---    | ---  | --- | menu_widgets.cpp      |

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

### Forms

| Class                  | C++ | Perl | Python | Ruby | XRC | file                    |
| ---------------------- | --- | ---- | ------ | ---- | --- | ----------------------- |
| wxDialog               | yes | ---  | yes    | yes  | yes | gen_dialog.cpp          |
| wxFrame                | yes | yes  | yes    | yes  | yes | gen_frame.cpp           |
| wxMenu                 | yes | ---  | yes    | ---  | --- | menu_widgets.cpp        |
| wxMenuBar              | yes | ---  | yes    | ---  | --- | menu_widgets.cpp        |
| wxPanel                | yes | yes  | yes    | ---  | --- | gen_panel_form.cpp      |
| wxPopupTransientWindow | yes | ---  | yes    | yes  | no  | gen_popup_trans_win.cpp |
| wxRibbonBar            | yes | ---  | yes    | ---  | --- | gen_ribbon_bar.cpp      |
| wxToolBar              | yes | ---  | yes    | ---  | --- | gen_toolbar.cpp         |
| wxWizard               | yes | ---  | yes    | yes  | yes | gen_wizard.cpp          |

### Pickers

| Class              | C++ | Perl | Python | Ruby | XRC | file                |
| ------------------ | --- | ---- | ------ | ---- | --- | ------------------- |
| wxColourPickerCtrl | yes | ---  | yes    | yes  | yes | gen_clr_picker.cpp  |
| wxDatePickerCtrl   | yes | ---  | yes    | yes  | yes | gen_date_picker.cpp |
| wxDirPickerCtrl    | yes | ---  | yes    | yes  | yes | gen_dir_picker.cpp  |
| wxFilePickerCtrl   | yes | ---  | yes    | yes  | yes | gen_file_picker.cpp |
| wxFontPickerCtrl   | yes | ---  | yes    | yes  | yes | gen_font_picker.cpp |
| wxTimePickerCtrl   | yes | ---  | yes    | yes  | yes | gen_time_picker.cpp |

### Sizers

| Class                    | C++ | Perl | Python | Ruby | XRC     | file                       |
| ------------------------ | --- | ---- | ------ | ---- | ------- | -------------------------- |
| wxBoxSizer               | yes | yes  | yes    | yes  | yes     | gen_box_sizer.cpp          |
| wxCheckBoxSizer          | yes | ---  | no     | yes  | ---     | gen_statchkbox_sizer.cpp   |
| wxFlexGridSizer          | yes | yes  | yes    | yes  | yes     | gen_flexgrid_sizer.cpp     |
| wxGridBagSizer           | yes | yes  | yes    | yes  | yes     | gen_gridbag_sizer.cpp      |
| wxGridSizer              | yes | yes  | yes    | yes  | yes     | gen_grid_sizer.cpp         |
| wxStaticRadioBtnBoxSizer | yes | ---  | no     | yes  | ---     | gen_statradiobox_sizer.cpp |
| wxStaticBoxSizer         | yes | ---  | yes    | yes  | yes     | gen_staticbox_sizer.cpp    |
| wxStdDialogButtonSizer   | yes | ---  | yes    | yes  | partial | gen_std_dlgbtn_sizer.cpp   |
| wxWrapSizer              | yes | ---  | yes    | yes  | yes     | gen_wrap_sizer.cpp         |

### Other

| Class                  | C++ | Perl    | Python  | Ruby | XRC     | file                  |
| ---------------------- | --- | ------- | ------- | ---- | ------- | --------------------- |
| wxActivityIndicator    | yes | ---     | yes     | yes  | yes     | gen_activity.cpp      |
| wxAnimationCtrl        | yes | ---     | ---     | yes  | ---     | gen_animation.cpp     |
| wxBannerWindow         | yes | ---     | yes     | yes  | yes     | gen_banner_window.cpp |
| wxBitmapComboBox       | --- | ---     | ---     | ---  | ---     | gen_bitmap_combo.cpp  |
| wxButton               | yes | ---     | yes     | yes  | partial | gen_button.cpp        |
| wxCalendarCtrl         | yes | ---     | yes     | yes  | yes     | gen_calendar_ctrl.cpp |
| wxCheckBox             | yes | ---     | yes     | yes  | yes     | gen_checkbox.cpp      |
| wxCheckListBox         | yes | ---     | yes     | yes  | yes     | gen_check_listbox.cpp |
| wxChoice               | yes | ---     | yes     | yes  | yes     | gen_choice.cpp        |
| wxCollapsiblePane      | yes | ---     | yes     | yes  | yes     | gen_collapsible.cpp   |
| wxComboBox             | yes | ---     | yes     | yes  | yes     | gen_combobox.cpp      |
| wxCommandLinkButton    | yes | ---     | yes     | yes  | yes     | gen_cmd_link_btn.cpp  |
| wxDataViewListCtrl     | yes | ---     | yes     | no   | no      | dataview_widgets.cpp  |
| wxDataViewCtrl         | yes | ---     | yes     | no   | no      | dataview_widgets.cpp  |
| wxEditableListBox      | yes | ---     | yes     | yes  | yes     | gen_edit_listbox.cpp  |
| wxFileCtrl             | yes | ---     | yes     | yes  | yes     | gen_file_ctrl.cpp     |
| wxGauge                | yes | ---     | yes     | yes  | ---     | gen_gauge.cpp         |
| wxGenericDirCtrl       | yes | ---     | partial | yes  | partial | gen_dir_ctrl.cpp      |
| wxGrid                 | yes | ---     | yes     | yes  | no      | gen_grid.cpp          |
| wxHtmlWindow           | yes | ---     | yes     | yes  | yes     | gen_html_window.cpp   |
| wxHyperlinkCtrl        | yes | ---     | yes     | yes  | yes     | gen_hyperlink.cpp     |
| wxListBox              | yes | ---     | yes     | yes  | yes     | gen_listbox.cpp       |
| wxListView             | yes | ---     | yes     | yes  | partial | gen_listview.cpp      |
| wxPanel                | yes | yes     | yes     | yes  | yes     | gen_panel.cpp         |
| wxPropertyGrid         | yes | ---     | yes     | yes  | no      | gen_prop_grid.cpp     |
| wxPropertyGridManager  | yes | ---     | yes     | yes  | no      | grid_widgets.cpp      |
| wxPropertySheetDialog  | no  | ---     | no      | no   | no      | gen_propsheet_dlg.cpp |
| wxRadioBox             | yes | ---     | yes     | yes  | yes     | gen_prop_grid.cpp     |
| wxRadioButton          | yes | ---     | yes     | yes  | yes     | gen_radio_btn.cpp     |
| wxRearrangeCtrl        | yes | ---     | yes     | yes  | no      | gen_rearrange.cpp     |
| wxRichTextCtrl         | yes | ---     | yes     | yes  | yes     | gen_rich_text.cpp     |
| wxScrolled\<wxPanel\>  | yes | ---     | yes     | yes  | ---     | window_widgets.cpp    |
| wxScrolled\<wxWindow\> | yes | ---     | yes     | yes  | ---     | window_widgets.cpp    |
| wxSearchCtrl           | yes | ---     | yes     | yes  | yes     | gen_search_ctrl.cpp   |
| wxSimpleHtmlListBox    | yes | ---     | yes     | yes  | yes     | gen_html_listbox.cpp  |
| wxSlider               | yes | ---     | yes     | yes  | yes     | gen_slider.cpp        |
| wxSpinButton           | yes | ---     | yes     | yes  | yes     | gen_spin_btn.cpp      |
| wxSpinCtrl             | yes | ---     | yes     | yes  | yes     | gen_spin_ctrl.cpp     |
| wxSpinCtrlDouble       | yes | ---     | yes     | yes  | yes     | gen_spin_ctrl.cpp     |
| wxSplitterWindow       | yes | ---     | yes     | yes  | yes     | gen_split_win.cpp     |
| wxStaticBitmap         | yes | ---     | yes     | yes  | yes     | gen_static_bmp.cpp    |
| wxStaticBox            | yes | ---     | ---     | ---  | ---     | gen_static_box.cpp    |
| wxStaticLine           | yes | ---     | yes     | yes  | yes     | gen_static_line.cpp   |
| wxStaticText           | yes | partial | yes     | yes  | yes     | gen_static_text.cpp   |
| wxStyledTextCtrl       | yes | ---     | partial | yes  | partial | styled_text.cpp       |
| wxTextCtrl             | yes | yes     | yes     | yes  | yes     | gen_text_ctrl.cpp     |
| wxToggleButton         | yes | ---     | yes     | yes  | partial | gen_toggle_btn.cpp    |
| wxTreeCtrl             | yes | ---     | yes     | yes  | yes     | gen_tree_ctrl.cpp     |
| wxTreeListCtrl         | yes | ---     | no      | no   | no      | gen_tree_list.cpp     |
| wxWebView              | yes | ---     | yes     | no   | no      | gen_web_view.cpp      |
| wxWizardPageSimple     | yes | ---     | yes     | yes  | yes     | gen_wizard.cpp        |

# Objects

| Class              | C++ | Perl | Python | Ruby | XRC | file                 |
| ------------------ | --- | ---- | ------ | ---- | --- | -------------------- |
| BookPage           | yes | yes  | yes    | yes  | yes | gen_book_page.cpp    |
| Check3State        | yes | ---  | ---    | ---  | --- | gen_checkbox.cpp     |
| CustomControl      | yes | ---  | no     | ---  | --- | gen_custom_ctrl.cpp  |
| DataViewColumn     | yes | ---  | yes    | no   | no  | dataview_widgets.cpp |
| DataViewListColumn | yes | ---  | ---    | ---  | --- | dataview_widgets.cpp |
| NewCloseButton     | yes | ---  | yes    | yes  | yes | gen_close_btn.cpp    |
| PageCtrl           | yes | ---  | ---    | ---  | no  | gen_page_ctrl.cpp    |
| PropertyGridItem   | yes | ---  | yes    | ---  | no  | gen_prop_item.cpp    |
| PropertyGridPage   | yes | ---  | yes    | ---  | no  | grid_widgets.cpp     |
| Spacer             | yes | ---  | ---    | ---  | --- | gen_spacer_sizer.cpp |
| TreeListCtrlColumn | yes | ---  | ---    | ---  | no  | gen_tree_list.cpp    |
| wxContextMenuEvent | yes | ---  | no     | ---  | no  | gen_ctx_menu.cpp     |
| wxTextProofOptions | yes | ---  | no     | ---  | --- | ---                  |
| wxSizerFlags       | yes | no   | yes    | yes  | no  | ---                  |

# Functions

| Function                   | C++ | Perl | Python | Ruby | XRC | file                     |
| -------------------------- | --- | ---- | ------ | ---- | --- | ------------------------ |
| CreateSeparatedSizer       | yes | no   | yes    | yes  | ---  | gen_std_dlgbtn_sizer.cpp |
| CreateStdDialogButtonSizer | yes | yes  | yes    | yes  | --- | gen_std_dlgbtn_sizer.cpp |
| FromDIP                    | yes | no   | yes    | yes  | yes | ---                      |
| GetControlSizer            | yes | ---  | no     | ???  | --- | gen_choicebook.cpp       |
| GetAffirmativeButton       | yes | ???  | no     | ???  | --- | gen_std_dlgbtn_sizer.cpp |


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
- wxTreeListCtrl
- wxTextProofOptions (for wxTextCtrl)

# Perl Unsupported

- wxStatusBar doesn't support wxSB_SUNKEN style, and it doesn't appear to allow more than one field.
- wxSizerFlags
- wxStaticText.SetLabelMarkup()
- wxTextCtrl.SetHint()
- wxTextProofOptions (for wxTextCtrl)
- wxWindow.FromDIP()
- wxSimpleBook

- wxREMOVE_LEADING_SPACES not a valid constant for wxWrapSizer
- wxWRAPSIZER_DEFAULT_FLAGS not a valid constant for wxWrapSizer
- wxDefaultCoord not a valid constant
- Dialog::CreateSeparatedSizer()

# Ruby Unsupported

- wxBannerWindow
- wxInfoBar
- wxRearrangeCtrl
- wxSpinCtrlDouble
- wxWebView
- wxCommandLinkButton

- wxDataViewListCtrl
- wxDataViewCtrl
- DataViewColumn
- DataViewListColumn
- wxTreeListCtrl
