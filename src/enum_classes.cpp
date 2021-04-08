/////////////////////////////////////////////////////////////////////////////
// Purpose:   Enumerations for nodes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// These maps can be used to convert the enumeration value into a string for writing out to a project file. When wxUiEditor
// is first initialized, the regular maps will be read and used to initialize the rmaps.

#include "pch.h"

#include "enum_classes.h"

using namespace NodeEnums;

std::map<size_t, const char*> NodeEnums::map_PropTypes = {

    { enum_bitlist, "bitlist" },
    { enum_bitmap, "bitmap" },
    { enum_bool, "bool" },
    { enum_editoption, "editoption" },
    { enum_file, "file" },
    { enum_float, "float" },
    { enum_id, "id" },
    { enum_image, "image" },
    { enum_int, "int" },
    { enum_option, "option" },
    { enum_parent, "parent" },
    { enum_path, "path" },
    { enum_string, "string" },
    { enum_string_edit, "string_edit" },
    { enum_string_edit_escapes, "string_edit_escapes" },
    { enum_string_edit_single, "string_edit_single" },
    { enum_string_escapes, "string_escapes" },
    { enum_stringlist, "stringlist" },
    { enum_uint, "uint" },
    { enum_uintlist, "uintlist" },
    { enum_uintpairlist, "uintpairlist" },
    { enum_wxColour, "wxColour" },
    { enum_wxFont, "wxFont" },
    { enum_wxparent, "wxparent" },
    { enum_wxparent_cp, "wxparent_cp" },
    { enum_wxparent_sb, "wxparent_sb" },
    { enum_wxPoint, "wxPoint" },
    { enum_wxSize, "wxSize" },

};
std::unordered_map<std::string, size_t> NodeEnums::rmap_PropTypes;

std::map<NodeEnums::Prop, const char*> NodeEnums::map_PropNames = {

    { Prop::Apply, "Apply" },
    { Prop::BottomDockable, "BottomDockable" },
    { Prop::Cancel, "Cancel" },
    { Prop::Close, "Close" },
    { Prop::ContextHelp, "ContextHelp" },
    { Prop::Help, "Help" },
    { Prop::LeftDockable, "LeftDockable" },
    { Prop::No, "No" },
    { Prop::OK, "OK" },
    { Prop::RightDockable, "RightDockable" },
    { Prop::Save, "Save" },
    { Prop::TopDockable, "TopDockable" },
    { Prop::Yes, "Yes" },
    { Prop::add_default_border, "add_default_border" },
    { Prop::align, "align" },
    { Prop::alignment, "alignment" },
    { Prop::aui_layer, "aui_layer" },
    { Prop::aui_name, "aui_name" },
    { Prop::aui_position, "aui_position" },
    { Prop::aui_row, "aui_row" },
    { Prop::auth_needed, "auth_needed" },
    { Prop::autosize_cols, "autosize_cols" },
    { Prop::autosize_rows, "autosize_rows" },
    { Prop::background_colour, "background_colour" },
    { Prop::backspace_unindents, "backspace_unindents" },
    { Prop::base_class_name, "base_class_name" },
    { Prop::base_file, "base_file" },
    { Prop::base_hdr_includes, "base_hdr_includes" },
    { Prop::base_src_includes, "base_src_includes" },
    { Prop::best_size, "best_size" },
    { Prop::bitmap, "bitmap" },
    { Prop::bitmapsize, "bitmapsize" },
    { Prop::border, "border" },
    { Prop::border_size, "border_size" },
    { Prop::borders, "borders" },
    { Prop::cancel_button, "cancel_button" },
    { Prop::caption, "caption" },
    { Prop::caption_visible, "caption_visible" },
    { Prop::cell_bg, "cell_bg" },
    { Prop::cell_font, "cell_font" },
    { Prop::cell_horiz_alignment, "cell_horiz_alignment" },
    { Prop::cell_text, "cell_text" },
    { Prop::cell_vert_alignment, "cell_vert_alignment" },
    { Prop::center, "center" },
    { Prop::center_pane, "center_pane" },
    { Prop::checkbox_var_name, "checkbox_var_name" },
    { Prop::checked, "checked" },
    { Prop::choices, "choices" },
    { Prop::class_access, "class_access" },
    { Prop::class_name, "class_name" },
    { Prop::close_button, "close_button" },
    { Prop::col_label_horiz_alignment, "col_label_horiz_alignment" },
    { Prop::col_label_size, "col_label_size" },
    { Prop::col_label_values, "col_label_values" },
    { Prop::col_label_vert_alignment, "col_label_vert_alignment" },
    { Prop::collapsed, "collapsed" },
    { Prop::colour, "colour" },
    { Prop::cols, "cols" },
    { Prop::colspan, "colspan" },
    { Prop::column, "column" },
    { Prop::column_sizes, "column_sizes" },
    { Prop::context_help, "context_help" },
    { Prop::context_menu, "context_menu" },
    { Prop::converted_art, "converted_art" },
    { Prop::current, "current" },
    { Prop::default, "default" },
    { Prop::default_button, "default_button" },
    { Prop::default_pane, "default_pane" },
    { Prop::defaultfilter, "defaultfilter" },
    { Prop::defaultfolder, "defaultfolder" },
    { Prop::derived_class_name, "derived_class_name" },
    { Prop::derived_file, "derived_file" },
    { Prop::digits, "digits" },
    { Prop::direction, "direction" },
    { Prop::disabled, "disabled" },
    { Prop::disabled_bmp, "disabled_bmp" },
    { Prop::display_images, "display_images" },
    { Prop::dock, "dock" },
    { Prop::dock_fixed, "dock_fixed" },
    { Prop::docking, "docking" },
    { Prop::drag_col_move, "drag_col_move" },
    { Prop::drag_col_size, "drag_col_size" },
    { Prop::drag_grid_size, "drag_grid_size" },
    { Prop::drag_row_size, "drag_row_size" },
    { Prop::duration, "duration" },
    { Prop::editing, "editing" },
    { Prop::ellipsize, "ellipsize" },
    { Prop::empty_cell_size, "empty_cell_size" },
    { Prop::enable_user_code, "enable_user_code" },
    { Prop::end_colour, "end_colour" },
    { Prop::extra_style, "extra_style" },
    { Prop::fields, "fields" },
    { Prop::filter, "filter" },
    { Prop::filter_index, "filter_index" },
    { Prop::flag, "flag" },
    { Prop::flags, "flags" },
    { Prop::flexible_direction, "flexible_direction" },
    { Prop::floatable, "floatable" },
    { Prop::focus, "focus" },
    { Prop::folding, "folding" },
    { Prop::font, "font" },
    { Prop::foreground_colour, "foreground_colour" },
    { Prop::get_function, "get_function" },
    { Prop::grid_line_color, "grid_line_color" },
    { Prop::grid_lines, "grid_lines" },
    { Prop::gripper, "gripper" },
    { Prop::growablecols, "growablecols" },
    { Prop::growablerows, "growablerows" },
    { Prop::header_ext, "header_ext" },
    { Prop::height, "height" },
    { Prop::help, "help" },
    { Prop::help_provider, "help_provider" },
    { Prop::hgap, "hgap" },
    { Prop::hidden, "hidden" },
    { Prop::hide_effect, "hide_effect" },
    { Prop::hover_color, "hover_color" },
    { Prop::icon, "icon" },
    { Prop::id, "id" },
    { Prop::image_size, "image_size" },
    { Prop::inc, "inc" },
    { Prop::include_advanced, "include_advanced" },
    { Prop::indentation_guides, "indentation_guides" },
    { Prop::initial, "initial" },
    { Prop::initial_filename, "initial_filename" },
    { Prop::initial_folder, "initial_folder" },
    { Prop::initial_font, "initial_font" },
    { Prop::initial_path, "initial_path" },
    { Prop::initial_state, "initial_state" },
    { Prop::internationalize, "internationalize" },
    { Prop::kind, "kind" },
    { Prop::label, "label" },
    { Prop::label_bg, "label_bg" },
    { Prop::label_font, "label_font" },
    { Prop::label_text, "label_text" },
    { Prop::line_numbers, "line_numbers" },
    { Prop::line_size, "line_size" },
    { Prop::local_pch_file, "local_pch_file" },
    { Prop::main_label, "main_label" },
    { Prop::majorDimension, "majorDimension" },
    { Prop::margin_height, "margin_height" },
    { Prop::margin_width, "margin_width" },
    { Prop::margins, "margins" },
    { Prop::markup, "markup" },
    { Prop::max, "max" },
    { Prop::maxValue, "maxValue" },
    { Prop::max_point_size, "max_point_size" },
    { Prop::max_rows, "max_rows" },
    { Prop::max_size, "max_size" },
    { Prop::maximize_button, "maximize_button" },
    { Prop::maximum_size, "maximum_size" },
    { Prop::maxlength, "maxlength" },
    { Prop::message, "message" },
    { Prop::min, "min" },
    { Prop::minValue, "minValue" },
    { Prop::min_pane_size, "min_pane_size" },
    { Prop::min_point_size, "min_point_size" },
    { Prop::min_rows, "min_rows" },
    { Prop::min_size, "min_size" },
    { Prop::minimize_button, "minimize_button" },
    { Prop::minimum_size, "minimum_size" },
    { Prop::mode, "mode" },
    { Prop::model_column, "model_column" },
    { Prop::moveable, "moveable" },
    { Prop::name_space, "name_space" },
    { Prop::non_flexible_grow_mode, "non_flexible_grow_mode" },
    { Prop::normal_color, "normal_color" },
    { Prop::note, "note" },
    { Prop::orientation, "orientation" },
    { Prop::original_art, "original_art" },
    { Prop::packing, "packing" },
    { Prop::page_size, "page_size" },
    { Prop::pagesize, "pagesize" },
    { Prop::pane_border, "pane_border" },
    { Prop::pane_position, "pane_position" },
    { Prop::pane_size, "pane_size" },
    { Prop::persist, "persist" },
    { Prop::persist_name, "persist_name" },
    { Prop::pin_button, "pin_button" },
    { Prop::pos, "pos" },
    { Prop::position, "position" },
    { Prop::pressed, "pressed" },
    { Prop::pressed_bmp, "pressed_bmp" },
    { Prop::proportion, "proportion" },
    { Prop::radiobtn_var_name, "radiobtn_var_name" },
    { Prop::range, "range" },
    { Prop::read_only, "read_only" },
    { Prop::resize, "resize" },
    { Prop::row, "row" },
    { Prop::row_label_horiz_alignment, "row_label_horiz_alignment" },
    { Prop::row_label_size, "row_label_size" },
    { Prop::row_label_values, "row_label_values" },
    { Prop::row_label_vert_alignment, "row_label_vert_alignment" },
    { Prop::row_sizes, "row_sizes" },
    { Prop::rows, "rows" },
    { Prop::rowspan, "rowspan" },
    { Prop::sashgravity, "sashgravity" },
    { Prop::sashpos, "sashpos" },
    { Prop::sashsize, "sashsize" },
    { Prop::scale_mode, "scale_mode" },
    { Prop::scroll_rate_x, "scroll_rate_x" },
    { Prop::scroll_rate_y, "scroll_rate_y" },
    { Prop::search_button, "search_button" },
    { Prop::select, "select" },
    { Prop::selection, "selection" },
    { Prop::selection_int, "selection_int" },
    { Prop::selection_string, "selection_string" },
    { Prop::separation, "separation" },
    { Prop::set_function, "set_function" },
    { Prop::shortcut, "shortcut" },
    { Prop::show, "show" },
    { Prop::show_effect, "show_effect" },
    { Prop::show_hidden, "show_hidden" },
    { Prop::size, "size" },
    { Prop::source_ext, "source_ext" },
    { Prop::splitmode, "splitmode" },
    { Prop::src_preamble, "src_preamble" },
    { Prop::start_colour, "start_colour" },
    { Prop::static_line, "static_line" },
    { Prop::statusbar, "statusbar" },
    { Prop::style, "style" },
    { Prop::tab_indents, "tab_indents" },
    { Prop::tab_width, "tab_width" },
    { Prop::text, "text" },
    { Prop::theme, "theme" },
    { Prop::thumb_length, "thumb_length" },
    { Prop::thumbsize, "thumbsize" },
    { Prop::tick_frequency, "tick_frequency" },
    { Prop::title, "title" },
    { Prop::toolbar_pane, "toolbar_pane" },
    { Prop::tooltip, "tooltip" },
    { Prop::type, "type" },
    { Prop::unchecked_bitmap, "unchecked_bitmap" },
    { Prop::url, "url" },
    { Prop::use_tabs, "use_tabs" },
    { Prop::user_cpp_code, "user_cpp_code" },
    { Prop::validator_data_type, "validator_data_type" },
    { Prop::validator_style, "validator_style" },
    { Prop::validator_type, "validator_type" },
    { Prop::validator_variable, "validator_variable" },
    { Prop::value, "value" },
    { Prop::var_name, "var_name" },
    { Prop::vgap, "vgap" },
    { Prop::view_eol, "view_eol" },
    { Prop::view_whitespace, "view_whitespace" },
    { Prop::visited_color, "visited_color" },
    { Prop::width, "width" },
    { Prop::wildcard, "wildcard" },
    { Prop::window_extra_style, "window_extra_style" },
    { Prop::window_name, "window_name" },
    { Prop::window_style, "window_style" },
    { Prop::wrap, "wrap" },
    { Prop::wrap_flags, "wrap_flags" },

};
std::unordered_map<std::string, NodeEnums::Prop> NodeEnums::rmap_PropNames;

std::map<size_t, const char*> NodeEnums::map_ClassTypes = {

    { enum_bookpage, "bookpage" },
    { enum_choicebook, "choicebook" },
    { enum_container, "container" },
    { enum_dataviewcolumn, "dataviewcolumn" },
    { enum_dataviewctrl, "dataviewctrl" },
    { enum_dataviewlistcolumn, "dataviewlistcolumn" },
    { enum_dataviewlistctrl, "dataviewlistctrl" },
    { enum_dataviewtreectrl, "dataviewtreectrl" },
    { enum_expanded_widget, "expanded_widget" },
    { enum_form, "form" },
    { enum_gbsizer, "gbsizer" },
    { enum_gbsizeritem, "gbsizeritem" },
    { enum_interface, "interface" },
    { enum_listbook, "listbook" },
    { enum_menu, "menu" },
    { enum_menubar, "menubar" },
    { enum_menubar_form, "menubar_form" },
    { enum_menuitem, "menuitem" },
    { enum_notebook, "notebook" },
    { enum_oldbookpage, "oldbookpage" },
    { enum_project, "project" },
    { enum_propgrid, "propgrid" },
    { enum_propgriditem, "propgriditem" },
    { enum_propgridman, "propgridman" },
    { enum_propgridpage, "propgridpage" },
    { enum_ribbonbar, "ribbonbar" },
    { enum_ribbonbutton, "ribbonbutton" },
    { enum_ribbonbuttonbar, "ribbonbuttonbar" },
    { enum_ribbongallery, "ribbongallery" },
    { enum_ribbongalleryitem, "ribbongalleryitem" },
    { enum_ribbonpage, "ribbonpage" },
    { enum_ribbonpanel, "ribbonpanel" },
    { enum_ribbontool, "ribbontool" },
    { enum_ribbontoolbar, "ribbontoolbar" },
    { enum_simplebook, "simplebook" },
    { enum_sizer, "sizer" },
    { enum_sizeritem, "sizeritem" },
    { enum_splitter, "splitter" },
    { enum_splitteritem, "splitteritem" },
    { enum_statusbar, "statusbar" },
    { enum_submenu, "submenu" },
    { enum_tool, "tool" },
    { enum_toolbar, "toolbar" },
    { enum_toolbar_form, "toolbar_form" },
    { enum_treelistctrl, "treelistctrl" },
    { enum_treelistctrlcolumn, "treelistctrlcolumn" },
    { enum_widget, "widget" },
    { enum_wizard, "wizard" },
    { enum_wizardpagesimple, "wizardpagesimple" },

};
std::unordered_map<std::string, size_t> NodeEnums::rmap_ClassTypes;

std::map<NodeEnums::Class, const char*> NodeEnums::map_ClassNames = {

    { Class::AUI, "AUI" },
    { Class::Bitmaps, "Bitmaps" },
    { Class::BookPage, "BookPage" },
    { Class::Boolean_Validator, "Boolean Validator" },
    { Class::Check3State, "Check3State" },
    { Class::Choice_Validator, "Choice Validator" },
    { Class::Code_Generation, "Code Generation" },
    { Class::Integer_Validator, "Integer Validator" },
    { Class::List_Validator, "List Validator" },
    { Class::MenuBar, "MenuBar" },
    { Class::PanelForm, "PanelForm" },
    { Class::Project, "Project" },
    { Class::StaticCheckboxBoxSizer, "StaticCheckboxBoxSizer" },
    { Class::StaticRadioBtnBoxSizer, "StaticRadioBtnBoxSizer" },
    { Class::String_Validator, "String Validator" },
    { Class::TextSizer, "TextSizer" },
    { Class::Text_Validator, "Text Validator" },
    { Class::ToolBar, "ToolBar" },
    { Class::TreeListCtrlColumn, "TreeListCtrlColumn" },
    { Class::UserCode, "UserCode" },
    { Class::VerticalBoxSizer, "VerticalBoxSizer" },
    { Class::Window_Events, "Window Events" },
    { Class::dataViewColumn, "dataViewColumn" },
    { Class::dataViewListColumn, "dataViewListColumn" },
    { Class::flexgridsizerbase, "flexgridsizerbase" },
    { Class::gbsizeritem, "gbsizeritem" },
    { Class::oldbookpage, "oldbookpage" },
    { Class::propGridItem, "propGridItem" },
    { Class::propGridPage, "propGridPage" },
    { Class::ribbonButton, "ribbonButton" },
    { Class::ribbonGalleryItem, "ribbonGalleryItem" },
    { Class::ribbonTool, "ribbonTool" },
    { Class::separator, "separator" },
    { Class::sizer_child, "sizer_child" },
    { Class::sizer_dimension, "sizer_dimension" },
    { Class::sizeritem, "sizeritem" },
    { Class::sizeritem_settings, "sizeritem_settings" },
    { Class::spacer, "spacer" },
    { Class::splitteritem, "splitteritem" },
    { Class::submenu, "submenu" },
    { Class::tool, "tool" },
    { Class::toolSeparator, "toolSeparator" },
    { Class::wxActivityIndicator, "wxActivityIndicator" },
    { Class::wxBannerWindow, "wxBannerWindow" },
    { Class::wxBitmapComboBox, "wxBitmapComboBox" },
    { Class::wxBoxSizer, "wxBoxSizer" },
    { Class::wxButton, "wxButton" },
    { Class::wxCalendarCtrl, "wxCalendarCtrl" },
    { Class::wxCheckBox, "wxCheckBox" },
    { Class::wxCheckListBox, "wxCheckListBox" },
    { Class::wxChoice, "wxChoice" },
    { Class::wxChoicebook, "wxChoicebook" },
    { Class::wxCollapsiblePane, "wxCollapsiblePane" },
    { Class::wxColourPickerCtrl, "wxColourPickerCtrl" },
    { Class::wxComboBox, "wxComboBox" },
    { Class::wxCommandLinkButton, "wxCommandLinkButton" },
    { Class::wxDataViewCtrl, "wxDataViewCtrl" },
    { Class::wxDataViewListCtrl, "wxDataViewListCtrl" },
    { Class::wxDataViewTreeCtrl, "wxDataViewTreeCtrl" },
    { Class::wxDatePickerCtrl, "wxDatePickerCtrl" },
    { Class::wxDialog, "wxDialog" },
    { Class::wxDirPickerCtrl, "wxDirPickerCtrl" },
    { Class::wxFileCtrl, "wxFileCtrl" },
    { Class::wxFilePickerCtrl, "wxFilePickerCtrl" },
    { Class::wxFlexGridSizer, "wxFlexGridSizer" },
    { Class::wxFontPickerCtrl, "wxFontPickerCtrl" },
    { Class::wxFrame, "wxFrame" },
    { Class::wxGauge, "wxGauge" },
    { Class::wxGenericDirCtrl, "wxGenericDirCtrl" },
    { Class::wxGrid, "wxGrid" },
    { Class::wxGridBagSizer, "wxGridBagSizer" },
    { Class::wxGridSizer, "wxGridSizer" },
    { Class::wxHtmlWindow, "wxHtmlWindow" },
    { Class::wxHyperlinkCtrl, "wxHyperlinkCtrl" },
    { Class::wxInfoBar, "wxInfoBar" },
    { Class::wxListBox, "wxListBox" },
    { Class::wxListView, "wxListView" },
    { Class::wxListbook, "wxListbook" },
    { Class::wxMenu, "wxMenu" },
    { Class::wxMenuBar, "wxMenuBar" },
    { Class::wxMenuItem, "wxMenuItem" },
    { Class::wxNotebook, "wxNotebook" },
    { Class::wxPanel, "wxPanel" },
    { Class::wxPopupTransientWindow, "wxPopupTransientWindow" },
    { Class::wxPropertyGrid, "wxPropertyGrid" },
    { Class::wxPropertyGridManager, "wxPropertyGridManager" },
    { Class::wxRadioBox, "wxRadioBox" },
    { Class::wxRadioButton, "wxRadioButton" },
    { Class::wxRibbonBar, "wxRibbonBar" },
    { Class::wxRibbonButtonBar, "wxRibbonButtonBar" },
    { Class::wxRibbonGallery, "wxRibbonGallery" },
    { Class::wxRibbonPage, "wxRibbonPage" },
    { Class::wxRibbonPanel, "wxRibbonPanel" },
    { Class::wxRibbonToolBar, "wxRibbonToolBar" },
    { Class::wxRichTextCtrl, "wxRichTextCtrl" },
    { Class::wxScrollBar, "wxScrollBar" },
    { Class::wxScrolledWindow, "wxScrolledWindow" },
    { Class::wxSearchCtrl, "wxSearchCtrl" },
    { Class::wxSimpleHtmlListBox, "wxSimpleHtmlListBox" },
    { Class::wxSimplebook, "wxSimplebook" },
    { Class::wxSlider, "wxSlider" },
    { Class::wxSpinButton, "wxSpinButton" },
    { Class::wxSpinCtrl, "wxSpinCtrl" },
    { Class::wxSpinCtrlDouble, "wxSpinCtrlDouble" },
    { Class::wxSplitterWindow, "wxSplitterWindow" },
    { Class::wxStaticBitmap, "wxStaticBitmap" },
    { Class::wxStaticBoxSizer, "wxStaticBoxSizer" },
    { Class::wxStaticLine, "wxStaticLine" },
    { Class::wxStaticText, "wxStaticText" },
    { Class::wxStatusBar, "wxStatusBar" },
    { Class::wxStdDialogButtonSizer, "wxStdDialogButtonSizer" },
    { Class::wxStyledTextCtrl, "wxStyledTextCtrl" },
    { Class::wxTextCtrl, "wxTextCtrl" },
    { Class::wxTimePickerCtrl, "wxTimePickerCtrl" },
    { Class::wxToggleButton, "wxToggleButton" },
    { Class::wxToolBar, "wxToolBar" },
    { Class::wxToolbook, "wxToolbook" },
    { Class::wxTopLevelWindow, "wxTopLevelWindow" },
    { Class::wxTreeCtrl, "wxTreeCtrl" },
    { Class::wxTreeCtrlBase, "wxTreeCtrlBase" },
    { Class::wxTreeListCtrl, "wxTreeListCtrl" },
    { Class::wxTreebook, "wxTreebook" },
    { Class::wxWindow, "wxWindow" },
    { Class::wxWizard, "wxWizard" },
    { Class::wxWizardPageSimple, "wxWizardPageSimple" },
    { Class::wxWrapSizer, "wxWrapSizer" },

};
std::unordered_map<std::string, NodeEnums::Class> NodeEnums::rmap_ClassNames;
