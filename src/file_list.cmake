set (file_list

    ${CMAKE_CURRENT_LIST_DIR}/wxUiEditor.rc         # Only processed when compiling for Windows, ignored otherwise

    ${CMAKE_CURRENT_LIST_DIR}/mainapp.cpp           # Main application class
    ${CMAKE_CURRENT_LIST_DIR}/mainframe.cpp         # Main window frame
    ${CMAKE_CURRENT_LIST_DIR}/assertion_dlg.cpp     # Assertion Dialog

    ${CMAKE_CURRENT_LIST_DIR}/appoptions.cpp        # Application-wide options
    ${CMAKE_CURRENT_LIST_DIR}/bitmaps.cpp           # Map of bitmaps accessed by name
    ${CMAKE_CURRENT_LIST_DIR}/clipboard.cpp         # Handles reading and writing OS clipboard data
    ${CMAKE_CURRENT_LIST_DIR}/cstm_event.cpp        # Custom Event handling
    ${CMAKE_CURRENT_LIST_DIR}/frame_status_bar.cpp  # MainFrame status bar functions
    ${CMAKE_CURRENT_LIST_DIR}/gen_enums.cpp         # Enumerations for generators
    ${CMAKE_CURRENT_LIST_DIR}/image_bundle.cpp      # Functions for working with wxBitmapBundle
    ${CMAKE_CURRENT_LIST_DIR}/lambdas.cpp           # Functions for formatting and storage of lamda events
    ${CMAKE_CURRENT_LIST_DIR}/paths.cpp             # Handles *_directory properties
    ${CMAKE_CURRENT_LIST_DIR}/project_class.cpp     # Project class
    ${CMAKE_CURRENT_LIST_DIR}/startup_dlg.cpp       # Startup dialog
    ${CMAKE_CURRENT_LIST_DIR}/undo_cmds.cpp         # Undoable command classes derived from UndoStackCmd
    ${CMAKE_CURRENT_LIST_DIR}/undo_stack.cpp        # Maintain an undo and redo stack
    ${CMAKE_CURRENT_LIST_DIR}/wakatime.cpp          # Updates WakaTime metrics

    # Custom property handling for Property Grid panel

    ${CMAKE_CURRENT_LIST_DIR}/customprops/code_string_prop.cpp    # Derived wxStringProperty class for code
    ${CMAKE_CURRENT_LIST_DIR}/customprops/custom_colour_prop.cpp  # Property editor for colour
    ${CMAKE_CURRENT_LIST_DIR}/customprops/custom_param_prop.cpp   # Derived wxStringProperty class for custom control parameters
    ${CMAKE_CURRENT_LIST_DIR}/customprops/directory_prop.cpp      # Derived wxStringProperty class for choosing a directory
    ${CMAKE_CURRENT_LIST_DIR}/customprops/evt_string_prop.cpp     # Derived wxStringProperty class for event function
    ${CMAKE_CURRENT_LIST_DIR}/customprops/font_string_prop.cpp    # Derived wxStringProperty class for font property
    ${CMAKE_CURRENT_LIST_DIR}/customprops/html_string_prop.cpp    # Derived wxStringProperty class for HTML
    ${CMAKE_CURRENT_LIST_DIR}/customprops/img_string_prop.cpp     # Derived wxStringProperty class for handling wxImage files or art
    ${CMAKE_CURRENT_LIST_DIR}/customprops/txt_string_prop.cpp     # Derived wxStringProperty class for single-line text

    ${CMAKE_CURRENT_LIST_DIR}/customprops/art_prop_dlg.cpp        # Art Property Dialog for image property
    ${CMAKE_CURRENT_LIST_DIR}/customprops/eventhandler_dlg.cpp    # Dialog for editing event handlers
    ${CMAKE_CURRENT_LIST_DIR}/customprops/font_prop_dlg.cpp       # Dialog for editing Font Property
    ${CMAKE_CURRENT_LIST_DIR}/customprops/img_props.cpp           # Handles property grid image properties

    ${CMAKE_CURRENT_LIST_DIR}/customprops/pg_image.cpp            # Custom property grid class for images
    ${CMAKE_CURRENT_LIST_DIR}/customprops/pg_animation.cpp        # Custom property grid class for animations
    ${CMAKE_CURRENT_LIST_DIR}/customprops/pg_point.cpp            # Custom wxPGProperty for wxPoint

    ${CMAKE_CURRENT_LIST_DIR}/custom_ctrls/colour_rect_ctrl.cpp   # Control that displays a solid color

    # Code generation modules

    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_base.cpp          # Generate Src and Hdr files for Base and Derived Class
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_cmake.cpp         # Auto-generate a .cmake file
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_codefiles.cpp     # Generate code files
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_derived.cpp       # Generate Derived class code
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_xrc.cpp           # Generate XRC file
    ${CMAKE_CURRENT_LIST_DIR}/generate/write_code.cpp        # Write code to Scintilla or file

    ${CMAKE_CURRENT_LIST_DIR}/generate/base_generator.cpp    # Base widget generator class
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_initialize.cpp    # Initialize all widget generate classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_common.cpp        # Common widget generation functions
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_xrc_utils.cpp     # Common XRC generating functions
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_book_utils.cpp    # Common Book utilities

    # Generators are responsible for displaying the widget in the Mockup window,
    # and generating both C++ and XRC code.

    ${CMAKE_CURRENT_LIST_DIR}/generate/dataview_widgets.cpp  # wxDataView generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/grid_widgets.cpp      # Grid generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/images_form.cpp       # Embedded images generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/menu_widgets.cpp      # Menu generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/project.cpp           # Project generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/ribbon_widgets.cpp    # Ribbon generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/styled_text.cpp       # wxStyledText (scintilla) generate
    ${CMAKE_CURRENT_LIST_DIR}/generate/window_widgets.cpp    # Splitter and Scroll generation classes

    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_activity.cpp       # wxActivityIndicator generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_animation.cpp      # wxAnimationCtrl generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_aui_toolbar.cpp    # wxAuiToolBar generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_banner_window.cpp  # wxBannerWindow generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_bitmap_combo.cpp   # wxBitmapComboBox generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_button.cpp         # wxButton generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_calendar_ctrl.cpp  # wxCalendarCtrl generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_check_listbox.cpp  # wxCheckListBox generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_checkbox.cpp       # wxCheckBox generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_choice.cpp         # wxChoice generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_close_btn.cpp      # wxBitmapButton::CreateCloseButton generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_cmd_link_btn.cpp   # wxCommandLinkButton generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_collapsible.cpp    # wxCollapsiblePane generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_combobox.cpp       # wxComboBox generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_custom_ctrl.cpp    # Custom Control generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_dir_ctrl.cpp       # wxGenericDirCtrl generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_edit_listbox.cpp   # wxEditableListBox generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_file_ctrl.cpp      # wxFileCtrl generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_gauge.cpp          # wxGauge generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_grid.cpp           # wxGrid generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_html_listbox.cpp   # wxSimpleHtmlListBox generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_html_window.cpp    # wxHtmlWindow generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_hyperlink.cpp      # wxHyperlinkCtrl generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_infobar.cpp        # wxInfoBar generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_listbox.cpp        # wxListBox generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_listview.cpp       # wxListView generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_panel.cpp          # wxPanel generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_prop_grid.cpp      # wxPropertyGrid generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_prop_item.cpp      # PropertyGrid/Manager Item generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_radio_box.cpp      # wxRadioBox generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_radio_btn.cpp      # wxRadioButton generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_rearrange.cpp      # wxRearrangeCtrl generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_rich_text.cpp      # wxRichTextCtrl generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_scrollbar.cpp      # wxScrollBar generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_search_ctrl.cpp    # wxSearchCtrl generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_slider.cpp         # wxSlider generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_spin_btn.cpp       # wxSpinButton generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_spin_ctrl.cpp      # wxSpinButton generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_split_win.cpp      # wxSplitterWindow generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_static_bmp.cpp     # wxStaticBitmap generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_static_box.cpp     # wxStaticBox generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_static_line.cpp    # wxStaticLine generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_static_text.cpp    # wxStaticText generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_status_bar.cpp     # wxStatusBar generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_text_ctrl.cpp      # wxTextCtrl generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_toggle_btn.cpp     # wxToggleButton generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_toolbar.cpp        # wxToolBar generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_tree_ctrl.cpp      # wxTreeCtrl generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_tree_list.cpp      # wxTreeListCtrl generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_web_view.cpp       # wxWebView generator

    # Books

    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_aui_notebook.cpp       # wxAuiNotebook generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_book_page.cpp          # Book page generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_choicebook.cpp         # wxChoicebook generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_listbook.cpp           # wxListbook generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_notebook.cpp           # wxNotebook generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_page_ctrl.cpp          # Page control generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_simplebook.cpp         # wxSimplebook generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_toolbook.cpp           # wxToolbook generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_treebook.cpp           # wxTreebook generator

    # Forms

    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_dialog.cpp             # wxDialog generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_frame.cpp              # wxFrame generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_panel_form.cpp         # wxPanel Form generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_popup_trans_win.cpp    # wxPopupTransientWindow generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_wizard.cpp             # wxWizard generator

    # Pickers

    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_clr_picker.cpp         # wxColourPickerCtrl generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_date_picker.cpp        # wxDatePickerCtrl generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_dir_picker.cpp         # wxDirPickerCtrl generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_file_picker.cpp        # wxFilePickerCtrl generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_font_picker.cpp        # wxFontPickerCtrl generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_time_picker.cpp        # wxTimePickerCtrl generator

    # Sizers

    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_box_sizer.cpp          # wxBoxSizer generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_flexgrid_sizer.cpp     # wxFlexGridSizer generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_grid_sizer.cpp         # wxGridSizer generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_gridbag_sizer.cpp      # wxGridBagSizer generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_spacer_sizer.cpp       # Add space to sizer generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_statchkbox_sizer.cpp   # wxStaticBoxSizer with wxCheckBox generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_staticbox_sizer.cpp    # wxStaticBoxSizer gemeratpr
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_statradiobox_sizer.cpp # wxStaticBoxSizer with wxRadioButton generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_std_dlgbtn_sizer.cpp   # wxStdDialogButtonSizer generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_text_sizer.cpp         # wxTextSizerWrapper generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_wrap_sizer.cpp         # wxWrapSizer generator

    # Importers (also see Windows Resource importer below)

    ${CMAKE_CURRENT_LIST_DIR}/import/import_crafter_maps.cpp  # wxCrafter mappings
    ${CMAKE_CURRENT_LIST_DIR}/import/import_formblder.cpp     # Import a wxFormBuider project
    ${CMAKE_CURRENT_LIST_DIR}/import/import_wxcrafter.cpp     # Import a wxCrafter project
    ${CMAKE_CURRENT_LIST_DIR}/import/import_wxglade.cpp       # Import a Import a wxGlade file
    ${CMAKE_CURRENT_LIST_DIR}/import/import_wxsmith.cpp       # Process XRC files
    ${CMAKE_CURRENT_LIST_DIR}/import/import_xml.cpp           # Base class for XML importing

    # Mockup panel

    ${CMAKE_CURRENT_LIST_DIR}/mockup/mockup_parent.cpp    # Top-level MockUp Parent window
    ${CMAKE_CURRENT_LIST_DIR}/mockup/mockup_content.cpp   # Mockup of a form's contents
    ${CMAKE_CURRENT_LIST_DIR}/mockup/mockup_wizard.cpp    # Emulate a wxWizard

    # Dialogs for creating new forms

    ${CMAKE_CURRENT_LIST_DIR}/newdialogs/new_common.cpp   # Contains code common between all new_ dialogs
    ${CMAKE_CURRENT_LIST_DIR}/newdialogs/new_dialog.cpp   # Dialog for creating a new project dialog
    ${CMAKE_CURRENT_LIST_DIR}/newdialogs/new_frame.cpp    # Dialog for creating a new project wxFrame
    ${CMAKE_CURRENT_LIST_DIR}/newdialogs/new_panel.cpp    # Dialog for creating a new form panel
    ${CMAKE_CURRENT_LIST_DIR}/newdialogs/new_ribbon.cpp   # Dialog for creating a new wxRibbonBar
    ${CMAKE_CURRENT_LIST_DIR}/newdialogs/new_wizard.cpp   # Dialog for creating a new wxWizard

    # Nodes

    ${CMAKE_CURRENT_LIST_DIR}/nodes/node.cpp              # Contains user-modifiable node
    ${CMAKE_CURRENT_LIST_DIR}/nodes/node_constants.cpp    # Maps wxWidgets constants to their numerical value
    ${CMAKE_CURRENT_LIST_DIR}/nodes/node_creator.cpp      # Class used to create nodes
    ${CMAKE_CURRENT_LIST_DIR}/nodes/node_decl.cpp         # Contains the declarations for a node (properties, events, etc.)
    ${CMAKE_CURRENT_LIST_DIR}/nodes/node_gridbag.cpp      # Create and modify a node containing a wxGridBagSizer
    ${CMAKE_CURRENT_LIST_DIR}/nodes/node_init.cpp         # Initialize NodeCreator class
    ${CMAKE_CURRENT_LIST_DIR}/nodes/node_prop.cpp         # NodeProperty class
    ${CMAKE_CURRENT_LIST_DIR}/nodes/tool_creator.cpp      # Functions for creating new nodes from Ribbon Panel

    ${CMAKE_CURRENT_LIST_DIR}/panels/cstm_propgrid.cpp    # Derived wxPropertyGrid class
    ${CMAKE_CURRENT_LIST_DIR}/panels/code_display.cpp     # Display code in scintilla control
    ${CMAKE_CURRENT_LIST_DIR}/panels/base_panel.cpp       # Code generation panel
    ${CMAKE_CURRENT_LIST_DIR}/panels/nav_panel.cpp        # Navigation panel
    ${CMAKE_CURRENT_LIST_DIR}/panels/navpopupmenu.cpp     # Context-menu for Navigation panel
    ${CMAKE_CURRENT_LIST_DIR}/panels/propgrid_panel.cpp   # Property panel
    ${CMAKE_CURRENT_LIST_DIR}/panels/ribbon_tools.cpp     # Displays node creation tools in a wxRibbonBar

    ${CMAKE_CURRENT_LIST_DIR}/project/loadproject.cpp     # Load wxUiEditor project
    ${CMAKE_CURRENT_LIST_DIR}/project/saveproject.cpp     # Save a wxUiEditor project file

    ${CMAKE_CURRENT_LIST_DIR}/utils/font_prop.cpp         # FontProperty class
    ${CMAKE_CURRENT_LIST_DIR}/utils/utils.cpp             # Utility functions that work with properties

    # Windows resource importer

    ${CMAKE_CURRENT_LIST_DIR}/winres/ctrl_utils.cpp       # resCtrl class utility functions
    ${CMAKE_CURRENT_LIST_DIR}/winres/form_utils.cpp       # resForm utility functions code
    ${CMAKE_CURRENT_LIST_DIR}/winres/import_winres.cpp    # Import a Windows resource file
    ${CMAKE_CURRENT_LIST_DIR}/winres/winres_ctrl.cpp      # Process Windows Resource control data
    ${CMAKE_CURRENT_LIST_DIR}/winres/winres_dlg.cpp       # Process a Windows Resource DIALOG or DIALOGEX
    ${CMAKE_CURRENT_LIST_DIR}/winres/winres_menu.cpp      # Process a Windows Resource MENU
    ${CMAKE_CURRENT_LIST_DIR}/winres/winres_images.cpp    # resCtrl class image/icon functions
    ${CMAKE_CURRENT_LIST_DIR}/winres/winres_layout.cpp    # resForm layout code
    ${CMAKE_CURRENT_LIST_DIR}/winres/winres_styles.cpp    # resCtrl class style processing functions

    ${CMAKE_CURRENT_LIST_DIR}/../pugixml/pugixml.cpp      # XML parser

    ${CMAKE_CURRENT_LIST_DIR}/ui/gridbag_item.cpp         # Dialog for inserting an item into a wxGridBagSizer node
    ${CMAKE_CURRENT_LIST_DIR}/ui/import_dlg.cpp           # Dialog to import one or more projects
    ${CMAKE_CURRENT_LIST_DIR}/ui/importwinresdlg.cpp      # Dialog for Importing a Windows resource file
    ${CMAKE_CURRENT_LIST_DIR}/ui/insertwidget.cpp         # Dialog to lookup and insert a widget
    ${CMAKE_CURRENT_LIST_DIR}/ui/optionsdlg.cpp           # Dialog containing special Debugging commands

    # ttLib submodule files

    ${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/ttcstr.cpp      # Class for handling zero-terminated char strings.
    ${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/ttcview.cpp     # string_view functionality on a zero-terminated char string.
    ${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/ttsview.cpp     # std::string_view with additional methods
    ${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/ttmultistr.cpp  # Breaks a single string into multiple strings
    ${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/ttparser.cpp    # Command line parser
    ${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/ttstr.cpp       # Enhanced version of wxString
    ${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/ttlibspace.cpp  # ttlib namespace functions
    ${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/tttextfile.cpp  # Classes for reading and writing text files.

    # Debug-only files
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/code_compare.cpp>    # Compare code generation
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/convert_img.cpp>     # Convert image
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/nodeinfo.cpp>        # Node memory usage dialog
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/xrccompare.cpp>      # C++/XRC UI Comparison dialog
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/xrcpreview.cpp>      # Test XRC
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/mockup_preview.cpp>  # Preview Mockup
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/import_panel.cpp>    # Panel to display original imported file

    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/code_compare_base.cpp>
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/convert_img_base.cpp>
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/nodeinfo_base.cpp>
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/nodeinfo_base.cpp>
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/xrcpreview_base.cpp>

    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/debugsettings.cpp>  # Settings while running the Debug version of wxUiEditor
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/msg_logging.cpp>    # Message logging class
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/msgframe.cpp>       # Stores messages

    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/msgframe_base.cpp>       # wxUiEditor generated file
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/internal/debugsettings_base.cpp>   # wxUiEditor generated file
)
