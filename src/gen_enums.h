/////////////////////////////////////////////////////////////////////////////
// Purpose:   Enumerations for generators
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <unordered_map>

namespace GenEnum
{
    enum PropType : size_t
    {

        type_bitlist,
        type_bitmap,
        type_bool,
        type_editoption,  // same as option, except that property grid uses wxEditEnumProperty instead of wxEnumProperty
        type_file,
        type_float,
        type_id,
        type_image,
        type_int,
        type_option,
        type_parent,
        type_path,
        type_string,
        type_string_edit,          // this includes a button that triggers a small text editor dialog
        type_string_edit_escapes,  // includes editor dialog and also escapes characters
        type_string_edit_single,   // includes single-line text editor, does not process escapes
        type_string_escapes,       // this doubles the backslash in escaped characters: \n, \t, \r, and "\""
        type_stringlist,
        type_uint,
        type_uintlist,
        type_uintpairlist,
        type_wxColour,
        type_wxFont,
        type_wxparent,
        type_wxparent_cp,
        type_wxparent_sb,
        type_wxPoint,
        type_wxSize,

    };
    extern std::map<PropType, const char*> map_PropTypes;
    extern std::unordered_map<std::string, PropType> rmap_PropTypes;

    enum PropName : size_t
    {
        prop_Apply,
        prop_BottomDockable,
        prop_Cancel,
        prop_Close,
        prop_ContextHelp,
        prop_Help,
        prop_LeftDockable,
        prop_No,
        prop_OK,
        prop_RightDockable,
        prop_Save,
        prop_TopDockable,
        prop_Yes,
        prop_add_default_border,
        prop_align,
        prop_alignment,
        prop_aui_layer,
        prop_aui_name,
        prop_aui_position,
        prop_aui_row,
        prop_auth_needed,
        prop_autosize_cols,
        prop_autosize_rows,
        prop_background_colour,
        prop_backspace_unindents,
        prop_base_class_name,
        prop_base_file,
        prop_base_hdr_includes,
        prop_base_src_includes,
        prop_best_size,
        prop_bitmap,
        prop_bitmapsize,
        prop_border,
        prop_border_size,
        prop_borders,
        prop_cancel_button,
        prop_caption,
        prop_caption_visible,
        prop_cell_bg,
        prop_cell_font,
        prop_cell_horiz_alignment,
        prop_cell_text,
        prop_cell_vert_alignment,
        prop_center,
        prop_center_pane,
        prop_checkbox_var_name,
        prop_checked,
        prop_choices,
        prop_class_access,
        prop_class_name,
        prop_close_button,
        prop_col_label_horiz_alignment,
        prop_col_label_size,
        prop_col_label_values,
        prop_col_label_vert_alignment,
        prop_collapsed,
        prop_colour,
        prop_cols,
        prop_colspan,
        prop_column,
        prop_column_sizes,
        prop_context_help,
        prop_context_menu,
        prop_converted_art,
        prop_current,
        prop_default,
        prop_default_button,
        prop_default_pane,
        prop_defaultfilter,
        prop_defaultfolder,
        prop_derived_class_name,
        prop_derived_file,
        prop_digits,
        prop_direction,
        prop_disabled,
        prop_disabled_bmp,
        prop_display_images,
        prop_dock,
        prop_dock_fixed,
        prop_docking,
        prop_drag_col_move,
        prop_drag_col_size,
        prop_drag_grid_size,
        prop_drag_row_size,
        prop_duration,
        prop_editing,
        prop_ellipsize,
        prop_empty_cell_size,
        prop_enable_user_code,
        prop_end_colour,
        prop_extra_style,
        prop_fields,
        prop_filter,
        prop_filter_index,
        prop_flag,
        prop_flags,
        prop_flexible_direction,
        prop_floatable,
        prop_focus,
        prop_folding,
        prop_font,
        prop_foreground_colour,
        prop_get_function,
        prop_grid_line_color,
        prop_grid_lines,
        prop_gripper,
        prop_growablecols,
        prop_growablerows,
        prop_header_ext,
        prop_height,
        prop_help,
        prop_help_provider,
        prop_hgap,
        prop_hidden,
        prop_hide_effect,
        prop_hover_color,
        prop_icon,
        prop_id,
        prop_image_size,
        prop_inc,
        prop_include_advanced,
        prop_indentation_guides,
        prop_initial,
        prop_initial_filename,
        prop_initial_folder,
        prop_initial_font,
        prop_initial_path,
        prop_initial_state,
        prop_internationalize,
        prop_kind,
        prop_label,
        prop_label_bg,
        prop_label_font,
        prop_label_text,
        prop_line_numbers,
        prop_line_size,
        prop_local_pch_file,
        prop_main_label,
        prop_majorDimension,
        prop_margin_height,
        prop_margin_width,
        prop_margins,
        prop_markup,
        prop_max,
        prop_maxValue,
        prop_max_point_size,
        prop_max_rows,
        prop_max_size,
        prop_maximize_button,
        prop_maximum_size,
        prop_maxlength,
        prop_message,
        prop_min,
        prop_minValue,
        prop_min_pane_size,
        prop_min_point_size,
        prop_min_rows,
        prop_min_size,
        prop_minimize_button,
        prop_minimum_size,
        prop_mode,
        prop_model_column,
        prop_moveable,
        prop_name_space,
        prop_non_flexible_grow_mode,
        prop_normal_color,
        prop_note,
        prop_orientation,
        prop_original_art,
        prop_packing,
        prop_page_size,
        prop_pagesize,
        prop_pane_border,
        prop_pane_position,
        prop_pane_size,
        prop_persist,
        prop_persist_name,
        prop_pin_button,
        prop_pos,
        prop_position,
        prop_pressed,
        prop_pressed_bmp,
        prop_proportion,
        prop_radiobtn_var_name,
        prop_range,
        prop_read_only,
        prop_resize,
        prop_row,
        prop_row_label_horiz_alignment,
        prop_row_label_size,
        prop_row_label_values,
        prop_row_label_vert_alignment,
        prop_row_sizes,
        prop_rows,
        prop_rowspan,
        prop_sashgravity,
        prop_sashpos,
        prop_sashsize,
        prop_scale_mode,
        prop_scroll_rate_x,
        prop_scroll_rate_y,
        prop_search_button,
        prop_select,
        prop_selection,
        prop_selection_int,
        prop_selection_string,
        prop_separation,
        prop_set_function,
        prop_shortcut,
        prop_show,
        prop_show_effect,
        prop_show_hidden,
        prop_size,
        prop_source_ext,
        prop_splitmode,
        prop_src_preamble,
        prop_start_colour,
        prop_static_line,
        prop_statusbar,
        prop_style,
        prop_tab_indents,
        prop_tab_width,
        prop_text,
        prop_theme,
        prop_thumb_length,
        prop_thumbsize,
        prop_tick_frequency,
        prop_title,
        prop_toolbar_pane,
        prop_tooltip,
        prop_type,
        prop_unchecked_bitmap,
        prop_url,
        prop_use_tabs,
        prop_user_cpp_code,
        prop_validator_data_type,
        prop_validator_style,
        prop_validator_type,
        prop_validator_variable,
        prop_value,
        prop_var_name,
        prop_variant,
        prop_vgap,
        prop_view_eol,
        prop_view_whitespace,
        prop_visited_color,
        prop_width,
        prop_wildcard,
        prop_window_extra_style,
        prop_window_name,
        prop_window_style,
        prop_wrap,
        prop_wrap_flags,

        // This must always be the last item as it is used to calculate the array size needed to store all items
        prop_name_array_size,
    };
    extern std::map<GenEnum::PropName, const char*> map_PropNames;
    extern std::unordered_map<std::string, GenEnum::PropName> rmap_PropNames;

    enum GenType : size_t
    {

        type_auinotebook,

        type_bookpage,
        type_choicebook,
        type_container,
        type_dataviewcolumn,
        type_dataviewctrl,
        type_dataviewlistcolumn,
        type_dataviewlistctrl,
        type_dataviewtreectrl,
        type_expanded_widget,
        type_form,
        type_gbsizer,
        type_gbsizeritem,
        type_interface,
        type_listbook,
        type_menu,
        type_menubar,
        type_menubar_form,
        type_menuitem,
        type_notebook,
        type_oldbookpage,
        type_project,
        type_propgrid,
        type_propgriditem,
        type_propgridman,
        type_propgridpage,
        type_ribbonbar,
        type_ribbonbutton,
        type_ribbonbuttonbar,
        type_ribbongallery,
        type_ribbongalleryitem,
        type_ribbonpage,
        type_ribbonpanel,
        type_ribbontool,
        type_ribbontoolbar,
        type_simplebook,
        type_sizer,
        type_sizeritem,
        type_splitter,
        type_splitteritem,
        type_statusbar,
        type_submenu,
        type_tool,
        type_toolbar,
        type_toolbar_form,
        type_treelistctrl,
        type_treelistctrlcolumn,
        type_widget,
        type_wizard,
        type_wizardpagesimple,

        // This must always be the last item as it is used to calculate the array size needed to store all items
        gen_type_array_size,
    };
    extern std::map<GenType, const char*> map_GenTypes;
    extern std::unordered_map<std::string, GenType> rmap_GenTypes;

    enum GenName : size_t
    {

        // Note that this list not only includes all the top level generators, but also the interface generators that a top
        // level generator can include as part of it's declaration.

        gen_AUI = 0,

        gen_Bitmaps,
        gen_BookPage,
        gen_Boolean_Validator,
        gen_Check3State,
        gen_Choice_Validator,
        gen_Code_Generation,
        gen_Integer_Validator,
        gen_List_Validator,
        gen_MenuBar,
        gen_PanelForm,
        gen_Project,
        gen_StaticCheckboxBoxSizer,
        gen_StaticRadioBtnBoxSizer,
        gen_String_Validator,
        gen_TextSizer,
        gen_Text_Validator,
        gen_ToolBar,
        gen_TreeListCtrlColumn,
        gen_UserCode,
        gen_VerticalBoxSizer,
        gen_Window_Events,
        gen_dataViewColumn,
        gen_dataViewListColumn,
        gen_flexgridsizerbase,
        gen_gbsizeritem,
        gen_oldbookpage,
        gen_propGridItem,
        gen_propGridPage,
        gen_ribbonButton,
        gen_ribbonGalleryItem,
        gen_ribbonTool,
        gen_separator,
        gen_sizer_child,
        gen_sizer_dimension,
        gen_sizeritem,
        gen_sizeritem_settings,
        gen_spacer,
        gen_splitteritem,
        gen_submenu,
        gen_tool,
        gen_toolSeparator,
        gen_wxActivityIndicator,
        gen_wxAuiMDIChildFrame,
        gen_wxBannerWindow,
        gen_wxBitmapComboBox,
        gen_wxBoxSizer,
        gen_wxButton,
        gen_wxCalendarCtrl,
        gen_wxCheckBox,
        gen_wxCheckListBox,
        gen_wxChoice,
        gen_wxChoicebook,
        gen_wxCollapsiblePane,
        gen_wxColourPickerCtrl,
        gen_wxComboBox,
        gen_wxCommandLinkButton,
        gen_wxDataViewCtrl,
        gen_wxDataViewListCtrl,
        gen_wxDataViewTreeCtrl,
        gen_wxDatePickerCtrl,
        gen_wxDialog,
        gen_wxDirPickerCtrl,
        gen_wxFileCtrl,
        gen_wxFilePickerCtrl,
        gen_wxFlexGridSizer,
        gen_wxFontPickerCtrl,
        gen_wxFrame,
        gen_wxGauge,
        gen_wxGenericDirCtrl,
        gen_wxGrid,
        gen_wxGridBagSizer,
        gen_wxGridSizer,
        gen_wxHtmlWindow,
        gen_wxHyperlinkCtrl,
        gen_wxInfoBar,
        gen_wxListBox,
        gen_wxListView,
        gen_wxListbook,
        gen_wxMenu,
        gen_wxMenuBar,
        gen_wxMenuItem,
        gen_wxNotebook,
        gen_wxPanel,
        gen_wxPopupTransientWindow,
        gen_wxPropertyGrid,
        gen_wxPropertyGridManager,
        gen_wxRadioBox,
        gen_wxRadioButton,
        gen_wxRearrangeCtrl,
        gen_wxRibbonBar,
        gen_wxRibbonButtonBar,
        gen_wxRibbonGallery,
        gen_wxRibbonPage,
        gen_wxRibbonPanel,
        gen_wxRibbonToolBar,
        gen_wxRichTextCtrl,
        gen_wxScrollBar,
        gen_wxScrolledWindow,
        gen_wxSearchCtrl,
        gen_wxSimpleHtmlListBox,
        gen_wxSimplebook,
        gen_wxSlider,
        gen_wxSpinButton,
        gen_wxSpinCtrl,
        gen_wxSpinCtrlDouble,
        gen_wxSplitterWindow,
        gen_wxStaticBitmap,
        gen_wxStaticBoxSizer,
        gen_wxStaticLine,
        gen_wxStaticText,
        gen_wxStatusBar,
        gen_wxStdDialogButtonSizer,
        gen_wxStyledTextCtrl,
        gen_wxTextCtrl,
        gen_wxTimePickerCtrl,
        gen_wxToggleButton,
        gen_wxToolBar,
        gen_wxToolbook,
        gen_wxTopLevelWindow,
        gen_wxTreeCtrl,
        gen_wxTreeCtrlBase,
        gen_wxTreeListCtrl,
        gen_wxTreebook,
        gen_wxWindow,
        gen_wxWizard,
        gen_wxWizardPageSimple,
        gen_wxWrapSizer,

        // This must always be the last item as it is used to calculate the array size needed to store all items
        gen_name_array_size,

    };
    extern std::map<GenEnum::GenName, const char*> map_GenNames;
    extern std::unordered_map<std::string, GenEnum::GenName> rmap_GenNames;

};  // namespace GenEnum
