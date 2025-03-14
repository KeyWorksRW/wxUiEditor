/////////////////////////////////////////////////////////////////////////////
// Purpose:   Enumerations for generators
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// These maps can be used to convert the enumeration value into a string for writing out to a project file. When wxUiEditor
// is first initialized, the regular maps will be read and used to initialize the rmaps.

#include "gen_enums.h"

using namespace GenEnum;

std::map<std::string_view, PropType, std::less<>> GenEnum::umap_PropTypes = {

    { "animation", type_animation },
    { "bitlist", type_bitlist },
    { "bitmap", type_bitmap },
    { "bmp_combo_item", type_bmp_combo_item },
    { "bool", type_bool },
    { "checklist_item", type_checklist_item },
    { "code_edit", type_code_edit },
    { "custom_mockup", type_custom_mockup },
    { "editoption", type_editoption },
    { "file", type_file },
    { "float", type_float },
    { "html_edit", type_html_edit },
    { "id", type_id },
    { "image", type_image },
    { "include_files", type_include_files },
    { "int", type_int },
    { "option", type_option },
    { "path", type_path },
    { "radiobox_item", type_radiobox_item },
    { "statbar_fields", type_statbar_fields },
    { "string", type_string },
    { "string_code_grow_columns", type_string_code_grow_columns },
    { "string_code_grow_rows", type_string_code_grow_rows },
    { "string_code_cstm_param", type_string_code_cstm_param },
    { "string_code_single", type_string_code_single },
    { "string_edit", type_string_edit },
    { "string_edit_escapes", type_string_edit_escapes },
    { "string_edit_single", type_string_edit_single },
    { "string_escapes", type_string_escapes },
    { "stringlist", type_stringlist },
    { "stringlist_semi", type_stringlist_semi },
    { "stringlist_escapes", type_stringlist_escapes },
    { "uint", type_uint },
    { "uintpairlist", type_uintpairlist },
    { "wxColour", type_wxColour },
    { "wxFont", type_wxFont },
    { "wxPoint", type_wxPoint },
    { "wxSize", type_wxSize },

};

std::map<GenEnum::PropName, const char*> GenEnum::map_PropNames = {

    { prop_template_description, "template_description" },
    { prop_template_directory, "template_directory" },
    { prop_template_doc_name, "template_doc_name" },
    { prop_template_extension, "template_extension" },
    { prop_template_filter, "template_filter" },
    { prop_template_view_name, "template_view_name" },
    { prop_template_visible, "template_visible" },
    { prop_mdi_class_name, "mdi_class_name" },
    { prop_mdi_doc_name, "mdi_doc_name" },

    { prop_Apply, "Apply" },
    { prop_BottomDockable, "BottomDockable" },
    { prop_Cancel, "Cancel" },
    { prop_Close, "Close" },
    { prop_ContextHelp, "ContextHelp" },
    { prop_Help, "Help" },
    { prop_LeftDockable, "LeftDockable" },
    { prop_No, "No" },
    { prop_OK, "OK" },
    { prop_RightDockable, "RightDockable" },
    { prop_Save, "Save" },
    { prop_TopDockable, "TopDockable" },
    { prop_Yes, "Yes" },
    { prop_add_default_border, "add_default_border" },
    { prop_add_externs, "add_externs" },
    { prop_additional_carets_blink, "additional_carets_blink" },
    { prop_additional_carets_visible, "additional_carets_visible" },
    { prop_additional_inheritance, "additional_inheritance" },
    { prop_align, "align" },
    { prop_alignment, "alignment" },
    { prop_allow_mouse_rectangle, "allow_mouse_rectangle" },
    { prop_animation, "animation" },
    { prop_art_directory, "art_directory" },
    { prop_art_provider, "art_provider" },
    { prop_aui_layer, "aui_layer" },
    { prop_aui_name, "aui_name" },
    { prop_aui_position, "aui_position" },
    { prop_aui_row, "aui_row" },
    { prop_auth_needed, "auth_needed" },
    { prop_auto_add, "auto_add" },
    { prop_auto_complete, "auto_complete" },
    { prop_auto_start, "auto_start" },
    { prop_auto_update, "auto_update" },
    { prop_automatic_folding, "automatic_folding" },
    { prop_autosize_cols, "autosize_cols" },
    { prop_autosize_rows, "autosize_rows" },
    { prop_background_colour, "background_colour" },
    { prop_backspace_unindents, "backspace_unindents" },
    { prop_base_directory, "base_directory" },
    { prop_base_file, "base_file" },
    { prop_base_hdr_includes, "base_hdr_includes" },
    { prop_base_src_includes, "base_src_includes" },
    { prop_best_size, "best_size" },
    { prop_bitmap, "bitmap" },
    { prop_bitmapsize, "bitmapsize" },
    { prop_bmp_background_colour, "bmp_background_colour" },
    { prop_bmp_min_width, "bmp_min_width" },
    { prop_bmp_placement, "bmp_placement" },
    { prop_book_type, "book_type" },
    { prop_border, "border" },
    { prop_border_size, "border_size" },
    { prop_borders, "borders" },
    { prop_buttons, "buttons" },
    { prop_cancel_button, "cancel_button" },
    { prop_caption, "caption" },
    { prop_caption_visible, "caption_visible" },
    { prop_cell_bg, "cell_bg" },
    { prop_cell_fit, "cell_fit" },
    { prop_cell_font, "cell_font" },
    { prop_cell_horiz_alignment, "cell_horiz_alignment" },
    { prop_cell_text, "cell_text" },
    { prop_cell_vert_alignment, "cell_vert_alignment" },
    { prop_center, "center" },
    { prop_center_pane, "center_pane" },
    { prop_checkbox_var_name, "checkbox_var_name" },
    { prop_checked, "checked" },
    { prop_class_access, "class_access" },
    { prop_class_decoration, "class_decoration" },
    { prop_class_members, "class_members" },
    { prop_class_methods, "class_methods" },
    { prop_class_name, "class_name" },
    { prop_close_button, "close_button" },
    { prop_cmake_file, "cmake_file" },
    { prop_cmake_varname, "cmake_varname" },
    { prop_code_preference, "code_preference" },
    { prop_col_label_horiz_alignment, "col_label_horiz_alignment" },
    { prop_col_label_size, "col_label_size" },
    { prop_col_label_values, "col_label_values" },
    { prop_col_label_vert_alignment, "col_label_vert_alignment" },
    { prop_collapsed, "collapsed" },
    { prop_colour, "colour" },
    { prop_cols, "cols" },
    { prop_colspan, "colspan" },
    { prop_column, "column" },
    { prop_column_labels, "column_labels" },
    { prop_column_sizes, "column_sizes" },
    { prop_combine_all_forms, "combine_all_forms" },
    { prop_combined_xrc_file, "combined_xrc_file" },
    { prop_compiler_standard, "compiler_standard" },
    { prop_const_values, "generate_const_values" },
    { prop_construction, "construction" },
    { prop_contents, "contents" },
    { prop_context_help, "context_help" },
    { prop_context_menu, "context_menu" },
    { prop_cpp_conditional, "cpp_conditional" },
    { prop_cpp_line_length, "cpp_line_length" },
    { prop_current, "current" },
    { prop_custom_colour, "custom_colour" },
    { prop_custom_margin, "custom_margin" },
    { prop_custom_mask_folders, "custom_mask_folders" },
    { prop_custom_mockup, "custom_mockup" },
    { prop_custom_mouse_sensitive, "custom_mouse_sensitive" },
    { prop_custom_type, "custom_type" },
    { prop_custom_width, "custom_width" },
    { prop_data_file, "data_file" },
    { prop_default, "default" },
    { prop_default_button, "default_button" },
    { prop_default_col_size, "default_col_size" },
    { prop_default_pane, "default_pane" },
    { prop_default_row_size, "default_row_size" },
    { prop_defaultfilter, "defaultfilter" },
    { prop_defaultfolder, "defaultfolder" },
    { prop_derived_class, "derived_class" },
    { prop_derived_class_name, "derived_class_name" },
    { prop_derived_directory, "derived_directory" },
    { prop_derived_file, "derived_file" },
    { prop_derived_header, "derived_header" },
    { prop_derived_params, "derived_params" },
    { prop_dialog_units, "dialog_units" },
    { prop_digits, "digits" },
    { prop_direction, "direction" },
    { prop_disable_rubo_cop, "disable_rubo_cop" },
    { prop_disabled, "disabled" },
    { prop_disabled_bmp, "disabled_bmp" },
    { prop_display_images, "display_images" },
    { prop_dock, "dock" },
    { prop_dock_fixed, "dock_fixed" },
    { prop_docking, "docking" },
    { prop_drag_col_move, "drag_col_move" },
    { prop_drag_col_size, "drag_col_size" },
    { prop_drag_grid_size, "drag_grid_size" },
    { prop_drag_row_size, "drag_row_size" },
    { prop_duration, "duration" },
    { prop_editing, "editing" },
    { prop_ellipsize, "ellipsize" },
    { prop_empty_cell_size, "empty_cell_size" },
    { prop_enable_user_code, "enable_user_code" },
    { prop_end_colour, "end_colour" },
    { prop_eol_mode, "eol_mode" },
    { prop_extra_accels, "extra_accels" },
    { prop_extra_style, "extra_style" },
    { prop_factor, "factor" },
    { prop_fields, "fields" },
    { prop_filter, "filter" },
    { prop_filter_index, "filter_index" },
    { prop_flag, "flag" },
    { prop_flags, "flags" },
    { prop_flexible_direction, "flexible_direction" },
    { prop_floatable, "floatable" },
    { prop_focus, "focus" },
    { prop_focus_bmp, "focus_bmp" },
    { prop_fold_flags, "fold_flags" },
    { prop_fold_margin, "fold_margin" },
    { prop_fold_marker_colour, "fold_marker_colour" },
    { prop_fold_marker_style, "fold_marker_style" },
    { prop_fold_width, "fold_width" },
    { prop_font, "font" },
    { prop_foreground_colour, "foreground_colour" },
    { prop_generate_cmake, "generate_cmake" },
    { prop_generate_ids, "generate_ids" },
    { prop_generate_languages, "generate_languages" },
    { prop_generate_translation_unit, "generate_translation_unit" },
    { prop_get_function, "get_function" },
    { prop_grid_line_color, "grid_line_color" },
    { prop_grid_lines, "grid_lines" },
    { prop_gripper, "gripper" },
    { prop_growablecols, "growablecols" },
    { prop_growablerows, "growablerows" },
    { prop_handler_name, "handler_name" },
    { prop_header, "header" },
    { prop_header_ext, "header_ext" },
    { prop_header_preamble, "header_preamble" },
    { prop_height, "height" },
    { prop_help, "help" },
    { prop_help_provider, "help_provider" },
    { prop_hexadecimal, "hexadecimal" },
    { prop_hgap, "hgap" },
    { prop_hidden, "hidden" },
    { prop_hide_children, "hide_children" },
    { prop_hide_effect, "hide_effect" },
    { prop_hint, "hint" },
    { prop_hover_color, "hover_color" },
    { prop_html_borders, "html_borders" },
    { prop_html_content, "html_content" },
    { prop_html_file, "html_file" },
    { prop_html_url, "html_url" },
    { prop_icon, "icon" },
    { prop_id, "id" },
    { prop_id_initial_value, "id_initial_value" },
    { prop_id_prefixes, "id_prefixes" },
    { prop_id_suffixes, "id_suffixes" },
    { prop_image_size, "image_size" },                  // currently unused
    { prop_import_all_dialogs, "import_all_dialogs" },  // currently unused
    { prop_inactive_bitmap, "inactive_bitmap" },
    { prop_inc, "inc" },
    { prop_include_advanced, "include_advanced" },
    { prop_indentation_guides, "indentation_guides" },
    { prop_initial, "initial" },
    { prop_initial_enum_string, "initial_enum_string" },
    { prop_initial_filename, "initial_filename" },
    { prop_initial_folder, "initial_folder" },
    { prop_initial_font, "initial_font" },
    { prop_initial_path, "initial_path" },
    { prop_initial_state, "initial_state" },
    { prop_inner_border, "inner_border" },
    { prop_inserted_hdr_code, "inserted_hdr_code" },
    { prop_internationalize, "internationalize" },
    { prop_interval, "interval" },
    { prop_kind, "kind" },
    { prop_label, "label" },
    { prop_label_bg, "label_bg" },
    { prop_label_font, "label_font" },
    { prop_label_text, "label_text" },
    { prop_line_digits, "line_digits" },
    { prop_line_margin, "line_margin" },
    { prop_line_numbers, "line_numbers" },  // previously used in wxStyledTextCtrl, but now uses margins
    { prop_line_size, "line_size" },
    { prop_local_hdr_includes, "local_hdr_includes" },
    { prop_local_pch_file, "local_pch_file" },
    { prop_local_src_includes, "local_src_includes" },
    { prop_main_label, "main_label" },
    { prop_majorDimension, "majorDimension" },
    { prop_margin_height, "margin_height" },
    { prop_margin_width, "margin_width" },
    { prop_margins, "margins" },
    { prop_markup, "markup" },
    { prop_max, "max" },
    { prop_maxValue, "maxValue" },
    { prop_max_point_size, "max_point_size" },
    { prop_max_rows, "max_rows" },
    { prop_max_size, "max_size" },
    { prop_maximize_button, "maximize_button" },
    { prop_maximum_size, "maximum_size" },
    { prop_maxlength, "maxlength" },
    { prop_message, "message" },
    { prop_min, "min" },
    { prop_minValue, "minValue" },
    { prop_min_pane_size, "min_pane_size" },
    { prop_min_point_size, "min_point_size" },
    { prop_min_rows, "min_rows" },
    { prop_min_size, "min_size" },
    { prop_minimize_button, "minimize_button" },
    { prop_minimum_size, "minimum_size" },
    { prop_mockup_size, "mockup_size" },
    { prop_mode, "mode" },
    { prop_model_column, "model_column" },
    { prop_moveable, "moveable" },
    { prop_multiple_selection_typing, "multiple_selection_typing" },
    { prop_multiple_selections, "multiple_selections" },
    { prop_name_space, "name_space" },
    { prop_namespace, "namespace" },
    { prop_native_col_header, "native_col_header" },
    { prop_native_col_labels, "native_col_labels" },
    { prop_no_closing_brace, "no_closing_brace" },
    { prop_no_compression, "no_compression" },
    { prop_non_flexible_grow_mode, "non_flexible_grow_mode" },
    { prop_non_selected_tab_font, "non_selected_tab_font" },
    { prop_normal_color, "normal_color" },
    { prop_note, "note" },
    { prop_null_text, "null_text" },
    { prop_one_shot, "one_shot" },
    { prop_orientation, "orientation" },
    { prop_outer_border, "outer_border" },
    { prop_output_file, "output_file" },
    { prop_packing, "packing" },
    { prop_page_size, "page_size" },
    { prop_pagesize, "pagesize" },
    { prop_pane_border, "pane_border" },
    { prop_pane_position, "pane_position" },
    { prop_pane_size, "pane_size" },
    { prop_parameters, "parameters" },
    { prop_paste_multiple, "paste_multiple" },
    { prop_persist, "persist" },
    { prop_persist_name, "persist_name" },
    { prop_pin_button, "pin_button" },
    { prop_platforms, "platforms" },
    { prop_play, "play" },
    { prop_pos, "pos" },
    { prop_position, "position" },
    { prop_precision, "precision" },
    { prop_pressed, "pressed" },
    { prop_pressed_bmp, "pressed_bmp" },
    { prop_private_members, "private_members" },
    { prop_project_src_includes, "project_src_includes" },
    { prop_proportion, "proportion" },
    { prop_protected_class_methods, "protected_class_methods" },
    { prop_python_import_list, "python_import_list" },
    { prop_radiobtn_var_name, "radiobtn_var_name" },
    { prop_range, "range" },
    { prop_read_only, "read_only" },
    { prop_relative_require_list, "relative_require_list" },
    { prop_resize, "resize" },
    { prop_row, "row" },
    { prop_row_label_horiz_alignment, "row_label_horiz_alignment" },
    { prop_row_label_size, "row_label_size" },
    { prop_row_label_values, "row_label_values" },
    { prop_row_label_vert_alignment, "row_label_vert_alignment" },
    { prop_row_sizes, "row_sizes" },
    { prop_rows, "rows" },
    { prop_rowspan, "rowspan" },
    { prop_ruby_require_all_dialogs, "ruby_require_all_dialogs" },  // currently unused
    { prop_sashgravity, "sashgravity" },
    { prop_sashpos, "sashpos" },
    { prop_sashsize, "sashsize" },
    { prop_scale_border_size, "scale_border_size" },
    { prop_scale_mode, "scale_mode" },
    { prop_scroll_rate_x, "scroll_rate_x" },
    { prop_scroll_rate_y, "scroll_rate_y" },
    { prop_search_button, "search_button" },
    { prop_sel_end, "sel_end" },
    { prop_sel_start, "sel_start" },
    { prop_select, "select" },
    { prop_selected_tab_font, "selected_tab_font" },
    { prop_selection, "selection" },
    { prop_selection_int, "selection_int" },
    { prop_selection_mode, "selection_mode" },
    { prop_selection_string, "selection_string" },
    { prop_separation, "separation" },
    { prop_separator_margin, "separator_margin" },
    { prop_separator_width, "separator_width" },
    { prop_set_function, "set_function" },
    { prop_settings_code, "settings_code" },
    { prop_shortcut, "shortcut" },
    { prop_show, "show" },
    { prop_show_effect, "show_effect" },
    { prop_show_header, "show_header" },
    { prop_show_hidden, "show_hidden" },
    { prop_shrink_to_fit, "shrink_to_fit" },
    { prop_size, "size" },
    { prop_source_ext, "source_ext" },
    { prop_source_preamble, "source_preamble" },
    { prop_spellcheck, "spellcheck" },
    { prop_splitmode, "splitmode" },
    { prop_src_preamble, "src_preamble" },
    { prop_start_colour, "start_colour" },
    { prop_static_line, "static_line" },
    { prop_statusbar, "statusbar" },
    { prop_stc_indentation_size, "indentation_size" },
    { prop_stc_left_margin_width, "left_margin_width" },
    { prop_stc_lexer, "lexer" },
    { prop_stc_right_margin_width, "right_margin_width" },
    { prop_stc_select_wrapped_line, "select_wrapped_line" },
    { prop_stc_wrap_indent_mode, "wrap_indent_mode" },
    { prop_stc_wrap_mode, "wrap_mode" },
    { prop_stc_wrap_start_indent, "wrap_start_indent" },
    { prop_stc_wrap_visual_flag, "wrap_visual_flag" },
    { prop_stc_wrap_visual_location, "wrap_visual_location" },
    { prop_stock_id, "stock_id" },
    { prop_style, "style" },
    { prop_subclass, "subclass" },
    { prop_subclass_header, "subclass_header" },
    { prop_subclass_params, "subclass_params" },
    { prop_symbol_margin, "symbol_margin" },
    { prop_symbol_mouse_sensitive, "symbol_mouse_sensitive" },
    { prop_sync_hover_colour, "sync_hover_colour" },
    { prop_system_hdr_includes, "system_hdr_includes" },
    { prop_system_src_includes, "system_src_includes" },
    { prop_tab_behaviour, "tab_behaviour" },
    { prop_tab_height, "tab_height" },
    { prop_tab_indents, "tab_indents" },
    { prop_tab_position, "tab_position" },
    { prop_tab_width, "tab_width" },
    { prop_text, "text" },
    { prop_theme, "theme" },
    { prop_thumb_length, "thumb_length" },
    { prop_thumbsize, "thumbsize" },
    { prop_tick_frequency, "tick_frequency" },
    { prop_title, "title" },
    { prop_toolbar_pane, "toolbar_pane" },
    { prop_tooltip, "tooltip" },
    { prop_type, "type" },
    { prop_unchecked_bitmap, "unchecked_bitmap" },
    { prop_underlined, "underlined" },
    { prop_url, "url" },
    { prop_use_derived_class, "use_derived_class" },
    { prop_use_generic, "use_generic" },
    { prop_use_tabs, "use_tabs" },
    { prop_user_cpp_code, "user_cpp_code" },
    { prop_validator_data_type, "validator_data_type" },
    { prop_validator_style, "validator_style" },
    { prop_validator_type, "validator_type" },
    { prop_validator_variable, "validator_variable" },
    { prop_value, "value" },
    { prop_var_comment, "var_comment" },
    { prop_var_name, "var_name" },
    { prop_variant, "variant" },
    { prop_vgap, "vgap" },
    { prop_view_eol, "view_eol" },
    { prop_view_tab_strikeout, "view_tab_strikeout" },
    { prop_view_whitespace, "view_whitespace" },
    { prop_visited_color, "visited_color" },
    { prop_width, "width" },
    { prop_wildcard, "wildcard" },
    { prop_window_extra_style, "window_extra_style" },
    { prop_window_name, "window_name" },
    { prop_window_style, "window_style" },
    { prop_wrap, "wrap" },
    { prop_wrap_flags, "wrap_flags" },
    { prop_wxWidgets_version, "wxWidgets_version" },
    { prop_xml_condensed_format, "xml_condensed_format" },
    { prop_xrc_art_directory, "xrc_art_directory" },
    { prop_xrc_directory, "xrc_directory" },
    { prop_xrc_file, "xrc_file" },
    { prop_xrc_no_whitespace, "xrc_no_whitespace" },

    { prop_folder_base_directory, "folder_base_directory" },
    { prop_folder_derived_directory, "folder_derived_directory" },
    { prop_folder_namespace, "folder_namespace" },
    { prop_folder_cmake_file, "folder_cmake_file" },
    { prop_folder_cmake_varname, "folder_cmake_varname" },

    { prop_folder_xrc_directory, "folder_xrc_directory" },
    { prop_folder_combined_xrc_file, "folder_combined_xrc_file" },

    { prop_folder_python_combined_file, "folder_python_combined_file" },
    { prop_folder_python_output_folder, "folder_python_output_folder" },
    { prop_python_combine_forms, "python_combine_forms" },
    { prop_python_combined_file, "python_combined_file" },
    { prop_python_conditional, "python_conditional" },
    { prop_python_file, "python_file" },
    { prop_python_inherit_name, "python_inherit_name" },
    { prop_python_insert, "insert_python_code" },
    { prop_python_line_length, "python_line_length" },
    { prop_python_output_folder, "python_output_folder" },
    { prop_python_project_preamble, "python_project_preamble" },
    { prop_python_use_xrc, "python_use_xrc" },
    { prop_python_variable_args, "python_variable_args" },
    { prop_python_xrc_file, "python_xrc_file" },
    { prop_wxPython_version, "wxPython_version" },

    { prop_folder_ruby_combined_file, "folder_ruby_combined_file" },
    { prop_folder_ruby_output_folder, "folder_ruby_output_folder" },
    { prop_ruby_combine_forms, "ruby_combine_forms" },
    { prop_ruby_combined_file, "ruby_combined_file" },
    { prop_ruby_file, "ruby_file" },
    { prop_ruby_inherit_name, "ruby_inherit_name" },
    { prop_ruby_insert, "insert_ruby_code" },
    { prop_ruby_line_length, "ruby_line_length" },
    { prop_ruby_output_folder, "ruby_output_folder" },
    { prop_ruby_project_preamble, "ruby_project_preamble" },
    { prop_wxRuby_version, "wxRuby_version" },

    { prop_folder_fortran_output_folder, "folder_fortran_output_folder" },
    { prop_fortran_file, "fortran_file" },
    { prop_fortran_inherit_name, "fortran_inherit_name" },
    { prop_fortran_insert, "insert_fortran_code" },
    { prop_fortran_line_length, "fortran_line_length" },
    { prop_fortran_output_folder, "fortran_output_folder" },
    { prop_fortran_project_preamble, "fortran_project_preamble" },
    { prop_wxFortran_version, "wxFortran_version" },

    { prop_folder_haskell_output_folder, "folder_haskell_output_folder" },
    { prop_haskell_file, "haskell_file" },
    { prop_haskell_inherit_name, "haskell_inherit_name" },
    { prop_haskell_insert, "insert_haskell_code" },
    { prop_haskell_line_length, "haskell_line_length" },
    { prop_haskell_output_folder, "haskell_output_folder" },
    { prop_haskell_project_preamble, "haskell_project_preamble" },
    { prop_wxHaskell_version, "wxHaskell_version" },

    { prop_folder_lua_output_folder, "folder_lua_output_folder" },
    { prop_lua_file, "lua_file" },
    { prop_lua_inherit_name, "lua_inherit_name" },
    { prop_lua_insert, "insert_lua_code" },
    { prop_lua_line_length, "lua_line_length" },
    { prop_lua_output_folder, "lua_output_folder" },
    { prop_lua_project_preamble, "lua_project_preamble" },
    { prop_wxLua_version, "wxLua_version" },

    { prop_folder_perl_combined_file, "folder_perl_combined_file" },
    { prop_folder_perl_output_folder, "folder_perl_output_folder" },
    { prop_perl_file, "perl_file" },
    { prop_perl_inherit_name, "perl_inherit_name" },
    { prop_perl_insert, "insert_perl_code" },
    { prop_perl_line_length, "perl_line_length" },
    { prop_perl_output_folder, "perl_output_folder" },
    { prop_perl_project_preamble, "perl_project_preamble" },
    { prop_wxPerl_version, "wxPerl_version" },

    { prop_folder_rust_output_folder, "folder_rust_output_folder" },
    { prop_rust_file, "rust_file" },
    { prop_rust_inherit_name, "rust_inherit_name" },
    { prop_rust_insert, "insert_rust_code" },
    { prop_rust_line_length, "rust_line_length" },
    { prop_rust_output_folder, "rust_output_folder" },
    { prop_rust_project_preamble, "rust_project_preamble" },
    { prop_wxRust_version, "wxRust_version" },

};
std::map<std::string_view, GenEnum::PropName, std::less<>> GenEnum::rmap_PropNames;

std::map<GenType, std::string_view> GenEnum::map_GenTypes = {

    { type_aui_tool, "aui_tool" },
    { type_aui_toolbar, "aui_toolbar" },
    { type_aui_toolbar_form, "aui_toolbar_form" },
    { type_auinotebook, "auinotebook" },
    { type_bookpage, "bookpage" },
    { type_choicebook, "choicebook" },
    { type_container, "container" },
    { type_ctx_menu, "ctx_menu" },
    { type_data_folder, "data_folder" },
    { type_data_list, "data_list" },
    { type_data_string, "data_string" },
    { type_dataviewcolumn, "dataviewcolumn" },
    { type_dataviewctrl, "dataviewctrl" },
    { type_dataviewlistcolumn, "dataviewlistcolumn" },
    { type_dataviewlistctrl, "dataviewlistctrl" },
    { type_dataviewtreectrl, "dataviewtreectrl" },
    { type_embed_image, "embed_image" },
    { type_folder, "folder" },
    { type_form, "form" },
    { type_frame_form, "frame_form" },
    { type_gbsizer, "gbsizer" },
    { type_gbsizeritem, "gbsizeritem" },
    { type_images, "images" },
    { type_interface, "interface" },
    { type_listbook, "listbook" },
    { type_menu, "menu" },
    { type_menubar, "menubar" },
    { type_menubar_form, "menubar_form" },
    { type_menuitem, "menuitem" },
    { type_notebook, "notebook" },
    { type_oldbookpage, "oldbookpage" },
    { type_page, "page" },
    { type_panel, "panel" },
    { type_panel_form, "panel_form" },
    { type_popup_menu, "popup_menu" },
    { type_project, "project" },
    { type_propgrid, "propgrid" },
    { type_propgrid_category, "propgrid_category" },
    { type_propgriditem, "propgriditem" },
    { type_propgridman, "propgridman" },
    { type_propgridpage, "propgridpage" },
    { type_propsheetform, "propsheetform" },
    { type_ribbonbar, "ribbonbar" },
    { type_ribbonbar_form, "ribbonbar_form" },
    { type_ribbonbutton, "ribbonbutton" },
    { type_ribbonbuttonbar, "ribbonbuttonbar" },
    { type_ribbongallery, "ribbongallery" },
    { type_ribbongalleryitem, "ribbongalleryitem" },
    { type_ribbonpage, "ribbonpage" },
    { type_ribbonpanel, "ribbonpanel" },
    { type_ribbontool, "ribbontool" },
    { type_ribbontoolbar, "ribbontoolbar" },
    { type_simplebook, "simplebook" },
    { type_sizer, "sizer" },
    { type_sizeritem, "sizeritem" },
    { type_splitter, "splitter" },
    { type_splitteritem, "splitteritem" },
    { type_staticbox, "staticbox" },
    { type_statusbar, "statusbar" },
    { type_sub_folder, "sub_folder" },
    { type_submenu, "submenu" },
    { type_tool, "tool" },
    { type_timer, "timer" },
    { type_tool_dropdown, "tool_dropdown" },
    { type_tool_separator, "tool_separator" },
    { type_toolbar, "toolbar" },
    { type_toolbar_form, "toolbar_form" },
    { type_treelistctrl, "treelistctrl" },
    { type_treelistctrlcolumn, "treelistctrlcolumn" },
    { type_widget, "widget" },
    { type_wizard, "wizard" },
    { type_wizardpagesimple, "wizardpagesimple" },

    { type_DocViewApp, "DocViewApp" },
    { type_wx_document, "wx_document" },
    { type_mdi_menubar, "mdi_menubar" },
    { type_doc_menubar, "mdi_doc_menubar" },
    { type_wx_view, "wx_view" },

};

std::map<GenEnum::GenName, const char*> GenEnum::map_GenNames = {

    { gen_AUI, "AUI" },  // This is always the first one, set to a value of 0

    // If you add a category to the following list, be sure to also add it to
    // unused_gen_dlg.cpp

    // The following are categories (type="interface") -- these don't have an actual generator
    // that implements them, but they do have a NodeDeclaration with the category name.

    { gen_Bitmaps, "Bitmaps" },
    { gen_Boolean_Validator, "Boolean Validator" },
    { gen_Choice_Validator, "Choice Validator" },
    { gen_Code_Generation, "Code Generation" },
    { gen_Command_Bitmaps, "Command Bitmaps" },
    { gen_DlgWindowSettings, "Dialog Window Settings" },
    { gen_Integer_Validator, "Integer Validator" },
    { gen_List_Validator, "List Validator" },
    { gen_String_Validator, "String Validator" },
    { gen_Text_Validator, "Text Validator" },
    { gen_Window_Events, "Window Events" },
    { gen_XrcSettings, "XRC Settings" },
    { gen_flexgridsizerbase, "flexgridsizerbase" },
    { gen_folder_Code, "Folder C++ Overrides" },
    { gen_folder_XRC, "Folder XRC Overrides" },
    { gen_folder_wxPerl, "Folder wxPerl Overrides" },
    { gen_folder_wxPython, "Folder wxPython Overrides" },
    { gen_folder_wxRuby, "Folder wxRuby Overrides" },
    { gen_sizer_child, "sizer_child" },
    { gen_sizeritem_settings, "sizeritem_settings" },
    { gen_wxMdiWindow, "wxMdiWindow" },
    { gen_wxTopLevelWindow, "wxTopLevelWindow" },
    { gen_wxTreeCtrlBase, "wxTreeCtrlBase" },
    { gen_wxWindow, "wxWindow" },

    // Language categories

    { gen_Code, "C++" },
    { gen_wxPython, "wxPython" },
    { gen_wxRuby, "wxRuby" },
    { gen_XRC, "XRC" },
    { gen_wxFortran, "wxFortran" },
    { gen_wxHaskell, "wxHaskell" },
    { gen_wxLua, "wxLua" },
    { gen_wxPerl, "wxPerl" },
    { gen_wxRust, "wxRust" },

    { gen_LanguageSettings, "Language Settings" },

    { gen_CPlusSettings, "C++ Settings" },
    { gen_DerivedCPlusSettings, "C++ Derived Class Settings" },
    { gen_CPlusHeaderSettings, "C++ Header Settings" },
    { gen_FortranSettings, "wxFortran Settings" },
    { gen_HaskellSettings, "wxHaskell Settings" },
    { gen_LuaSettings, "wxLua Settings" },
    { gen_PerlSettings, "wxPerl Settings" },
    { gen_RustSettings, "wxRust Settings" },
    { gen_PythonSettings, "wxPython Settings" },
    { gen_RubySettings, "wxRuby Settings" },

    // These are special purpose generators. gen_Images is used for code, but gen_folder is
    // just for organtizing forms in the Navigation panel.

    { gen_Data, "Data" },
    { gen_Images, "Images" },
    { gen_data_string, "data_string" },
    { gen_data_folder, "data_folder" },
    { gen_data_xml, "data_xml" },
    { gen_embedded_image, "embedded_image" },
    { gen_folder, "folder" },
    { gen_sub_folder, "sub_folder" },

    // These are for wxDocManager

    { gen_DocViewApp, "DocViewApp" },
    { gen_MdiFrameMenuBar, "MdiFrameMenuBar" },
    { gen_MdiDocMenuBar, "MdiDocMenuBar" },
    { gen_DocumentTextCtrl, "DocumentTextCtrl" },
    { gen_ViewTextCtrl, "ViewTextCtrl" },

    // The following are the regular generators

    { gen_AuiToolBar, "AuiToolBar" },
    { gen_BookPage, "BookPage" },
    { gen_Check3State, "Check3State" },
    { gen_CloseButton, "CloseButton" },
    { gen_CustomControl, "CustomControl" },
    { gen_MenuBar, "MenuBar" },
    { gen_PageCtrl, "PageCtrl" },
    { gen_PanelForm, "PanelForm" },
    { gen_PopupMenu, "PopupMenu" },
    { gen_Project, "Project" },
    { gen_RibbonBar, "RibbonBar" },
    { gen_StaticCheckboxBoxSizer, "StaticCheckboxBoxSizer" },
    { gen_StaticRadioBtnBoxSizer, "StaticRadioBtnBoxSizer" },
    { gen_TextSizer, "TextSizer" },
    { gen_ToolBar, "ToolBar" },
    { gen_TreeListCtrlColumn, "TreeListCtrlColumn" },
    { gen_UserCode, "UserCode" },
    { gen_VerticalBoxSizer, "VerticalBoxSizer" },
    { gen_auitool, "auitool" },
    { gen_auitool_label, "auitool_label" },
    { gen_auitool_spacer, "auitool_spacer" },
    { gen_auitool_stretchable, "auitool_stretchable" },
    { gen_dataViewColumn, "dataViewColumn" },
    { gen_dataViewListColumn, "dataViewListColumn" },
    { gen_gbsizeritem, "gbsizeritem" },
    { gen_oldbookpage, "oldbookpage" },
    { gen_propGridCategory, "propGridCategory" },
    { gen_propGridItem, "propGridItem" },
    { gen_propGridPage, "propGridPage" },
    { gen_ribbonButton, "ribbonButton" },
    { gen_ribbonGalleryItem, "ribbonGalleryItem" },
    { gen_ribbonSeparator, "ribbonSeparator" },
    { gen_ribbonTool, "ribbonTool" },
    { gen_separator, "separator" },
    { gen_sizer_dimension, "sizer_dimension" },
    { gen_sizeritem, "sizeritem" },
    { gen_spacer, "spacer" },
    { gen_splitteritem, "splitteritem" },
    { gen_submenu, "submenu" },
    { gen_tool, "tool" },
    { gen_tool_dropdown, "tool_dropdown" },
    { gen_toolSeparator, "toolSeparator" },
    { gen_toolStretchable, "toolStretchable" },
    { gen_wxActivityIndicator, "wxActivityIndicator" },
    { gen_wxAnimationCtrl, "wxAnimationCtrl" },
    { gen_wxAuiMDIChildFrame, "wxAuiMDIChildFrame" },
    { gen_wxAuiNotebook, "wxAuiNotebook" },
    { gen_wxAuiToolBar, "wxAuiToolBar" },
    { gen_wxBannerWindow, "wxBannerWindow" },
    { gen_wxBitmapComboBox, "wxBitmapComboBox" },
    { gen_wxBitmapToggleButton, "wxBitmapToggleButton" },
    { gen_wxBoxSizer, "wxBoxSizer" },
    { gen_wxButton, "wxButton" },
    { gen_wxCalendarCtrl, "wxCalendarCtrl" },
    { gen_wxCheckBox, "wxCheckBox" },
    { gen_wxCheckListBox, "wxCheckListBox" },
    { gen_wxChoice, "wxChoice" },
    { gen_wxChoicebook, "wxChoicebook" },
    { gen_wxCollapsiblePane, "wxCollapsiblePane" },
    { gen_wxColourPickerCtrl, "wxColourPickerCtrl" },
    { gen_wxComboBox, "wxComboBox" },
    { gen_wxCommandLinkButton, "wxCommandLinkButton" },
    { gen_wxContextHelpButton, "wxContextHelpButton" },
    { gen_wxContextMenuEvent, "wxContextMenuEvent" },
    { gen_wxDataViewCtrl, "wxDataViewCtrl" },
    { gen_wxDataViewListCtrl, "wxDataViewListCtrl" },
    { gen_wxDataViewTreeCtrl, "wxDataViewTreeCtrl" },
    { gen_wxDatePickerCtrl, "wxDatePickerCtrl" },
    { gen_wxDialog, "wxDialog" },
    { gen_wxDirPickerCtrl, "wxDirPickerCtrl" },
    { gen_wxEditableListBox, "wxEditableListBox" },
    { gen_wxFileCtrl, "wxFileCtrl" },
    { gen_wxFilePickerCtrl, "wxFilePickerCtrl" },
    { gen_wxFlexGridSizer, "wxFlexGridSizer" },
    { gen_wxFontPickerCtrl, "wxFontPickerCtrl" },
    { gen_wxFrame, "wxFrame" },
    { gen_wxGauge, "wxGauge" },
    { gen_wxGenericDirCtrl, "wxGenericDirCtrl" },
    { gen_wxGrid, "wxGrid" },
    { gen_wxGridBagSizer, "wxGridBagSizer" },
    { gen_wxGridSizer, "wxGridSizer" },
    { gen_wxHtmlWindow, "wxHtmlWindow" },
    { gen_wxHyperlinkCtrl, "wxHyperlinkCtrl" },
    { gen_wxInfoBar, "wxInfoBar" },
    { gen_wxListBox, "wxListBox" },
    { gen_wxListView, "wxListView" },
    { gen_wxListbook, "wxListbook" },
    { gen_wxMenu, "wxMenu" },
    { gen_wxMenuBar, "wxMenuBar" },
    { gen_wxMenuItem, "wxMenuItem" },
    { gen_wxNotebook, "wxNotebook" },
    { gen_wxPanel, "wxPanel" },
    { gen_wxPopupTransientWindow, "wxPopupTransientWindow" },
    { gen_wxPopupWindow, "wxPopupWindow" },
    { gen_wxPropertyGrid, "wxPropertyGrid" },
    { gen_wxPropertyGridManager, "wxPropertyGridManager" },
    { gen_wxPropertySheetDialog, "wxPropertySheetDialog" },
    { gen_wxRadioBox, "wxRadioBox" },
    { gen_wxRadioButton, "wxRadioButton" },
    { gen_wxRearrangeCtrl, "wxRearrangeCtrl" },
    { gen_wxRibbonBar, "wxRibbonBar" },
    { gen_wxRibbonButtonBar, "wxRibbonButtonBar" },
    { gen_wxRibbonGallery, "wxRibbonGallery" },
    { gen_wxRibbonPage, "wxRibbonPage" },
    { gen_wxRibbonPanel, "wxRibbonPanel" },
    { gen_wxRibbonToolBar, "wxRibbonToolBar" },
    { gen_wxRichTextCtrl, "wxRichTextCtrl" },
    { gen_wxScrollBar, "wxScrollBar" },
    { gen_wxScrolledCanvas, "wxScrolledCanvas" },
    { gen_wxScrolledWindow, "wxScrolledWindow" },
    { gen_wxSearchCtrl, "wxSearchCtrl" },
    { gen_wxSimpleHtmlListBox, "wxSimpleHtmlListBox" },
    { gen_wxSimplebook, "wxSimplebook" },
    { gen_wxSlider, "wxSlider" },
    { gen_wxSpinButton, "wxSpinButton" },
    { gen_wxSpinCtrl, "wxSpinCtrl" },
    { gen_wxSpinCtrlDouble, "wxSpinCtrlDouble" },
    { gen_wxSplitterWindow, "wxSplitterWindow" },
    { gen_wxStaticBitmap, "wxStaticBitmap" },
    { gen_wxStaticBox, "wxStaticBox" },
    { gen_wxStaticBoxSizer, "wxStaticBoxSizer" },
    { gen_wxStaticLine, "wxStaticLine" },
    { gen_wxStaticText, "wxStaticText" },
    { gen_wxStatusBar, "wxStatusBar" },
    { gen_wxStdDialogButtonSizer, "wxStdDialogButtonSizer" },
    { gen_wxStyledTextCtrl, "wxStyledTextCtrl" },
    { gen_wxTextCtrl, "wxTextCtrl" },
    { gen_wxTimePickerCtrl, "wxTimePickerCtrl" },
    { gen_wxTimer, "wxTimer" },
    { gen_wxToggleButton, "wxToggleButton" },
    { gen_wxToolBar, "wxToolBar" },
    { gen_wxToolbook, "wxToolbook" },
    { gen_wxTreeCtrl, "wxTreeCtrl" },
    { gen_wxTreeListCtrl, "wxTreeListCtrl" },
    { gen_wxTreebook, "wxTreebook" },
    { gen_wxWebView, "wxWebView" },
    { gen_wxWizard, "wxWizard" },
    { gen_wxWizardPageSimple, "wxWizardPageSimple" },
    { gen_wxWrapSizer, "wxWrapSizer" },

};
std::map<std::string_view, GenEnum::GenName, std::less<>> rmap_GenNames;

std::map<GenEnum::PropName, const char*> map_PropMacros = {

    { prop_id, "${id}" },
    { prop_pos, "${pos}" },
    { prop_size, "${size}" },
    { prop_window_extra_style, "${window_extra_style}" },
    { prop_window_name, "${window_name}" },
    { prop_window_style, "${window_style}" },

    // Same as above, but supports those who prefer macros to be upper case
    { prop_id, "${ID}" },
    { prop_pos, "${POS}" },
    { prop_size, "${SIZE}" },
    { prop_window_extra_style, "${WINDOW_EXTRA_STYLE}" },
    { prop_window_name, "${WINDOW_NAME}" },
    { prop_window_style, "${WINDOW_STYLE}" },

};

// This is the opposite of map_PropMacros, and is initialized in NodeCreator::Initialize()
std::map<std::string_view, GenEnum::PropName, std::less<>> map_MacroProps;

// In order to sort the left column, everything needs to be on one line. Since these can be
// very long lines, we can't let clang-format wrap them.

// clang-format off

// These can be used for any type of property. The generator can override this using
// GetPropertyDescription(). If neither this map or the generator supply a description, it
// will be retrieved from the XML interface file.
std::map<GenEnum::PropName, const char*> GenEnum::map_PropHelp = {

    // Use \\n to add a line break

    { prop_persist, "Use wxPersistentRegisterAndRestore to save/restore the size and position of the form.\\n\\nOnly available for C++ code." },
    { prop_window_name, "The name of the window. This parameter is used to associate a name with the item, allowing the application user to set Motif resource values for individual windows." },
    { prop_platforms, "Specifies the platforms to create this item on. If a platform is unchecked, the code to create the item will be placed in a conditional block." },

};

// clang-format on
