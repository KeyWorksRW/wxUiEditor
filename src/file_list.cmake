set( file_list_dir ${CMAKE_CURRENT_LIST_DIR} )

# wxui/wxui_code.cmake
# ../in.cmake
set( file_list

    wxUiEditor.rc # Only processed when compiling for Windows, ignored otherwise

    mainapp.cpp # Main application class
    mainframe.cpp # Main window frame
    mainframe_events.cpp # Main window frame event handlers
    mainframe_updates.cpp # Main window frame event handlers
    assertion_dlg.cpp # Assertion Dialog

    bitmaps.cpp # Map of bitmaps accessed by name
    clipboard.cpp # Handles reading and writing OS clipboard data
    cstm_event.cpp # Custom Event handling
    frame_status_bar.cpp # MainFrame status bar functions
    gen_enums.cpp # Enumerations for generators
    id_lists.cpp # wxID_ strings
    paths.cpp # Handles *_directory properties
    preferences.cpp # Prefs/UserPrefs -- Set/Get wxUiEditor preferences
    previews.cpp # Top level Preview functions
    undo_cmds.cpp # Undoable command classes derived from UndoStackCmd
    undo_stack.cpp # Maintain an undo and redo stack
    wakatime.cpp # Updates WakaTime metrics

    ui/startup_dlg.cpp # Dialog to display if wxUiEditor is launched with no arguments

    ttwx/ttwx.cpp # ttwx namespace functions and declarations
    ttwx/ttwx_view_vector.cpp # ttwx::ViewVector class
    ttwx/ttwx_string_vector.cpp # ttwx::StringVector class

    tt/tt.cpp # tt namespace functions
    tt/tt_string.cpp # std::string with additional methods
    tt/tt_string_view.cpp # std::string_view with additional methods
    tt/tt_string_vector.cpp # Read/Write line-oriented strings/files
    tt/tt_view_vector.cpp # Read/Write line-oriented strings/files

    # Custom property handling for Property Grid panel
    customprops/code_single_prop.cpp # EditCodeSingleProperty -- Derived wxStringProperty class for single line code
    customprops/code_string_prop.cpp # EditCodeProperty --Derived wxStringProperty class for code
    customprops/custom_colour_prop.cpp # EditColourProperty -- Property editor for colour
    customprops/custom_param_prop.cpp # EditParamProperty -- Derived wxStringProperty class for custom control parameters
    customprops/directory_prop.cpp # DirectoryProperty -- Derived wxStringProperty class for choosing a directory
    customprops/edit_custom_mockup.cpp # Custom Property editor for pop_custom_mockup
    customprops/evt_string_prop.cpp # EventStringProperty -- Derived wxStringProperty class for event function
    customprops/font_string_prop.cpp # FontStringProperty -- Derived wxStringProperty class for font property
    customprops/html_string_prop.cpp # EditHtmlProperty - Derived wxStringProperty class for HTML
    customprops/id_prop.cpp # ID_Property -- Uses IDEditorDlg to edit a custom ID
    customprops/img_string_prop.cpp # ImageDialogAdapter -- used by ImageStringProperty (img_string_prop.h)
    customprops/include_files_prop.cpp # IncludeFilesProperty -- Derived wxStringProperty class for Include Files
    customprops/rearrange_prop.cpp # RearrangeProperty -- wxRearangeList contents editor
    customprops/sb_fields_prop.cpp # SBarFieldsProperty -- Property editor for status bar fields
    customprops/sizer_grow_columns.cpp # GrowColumnsProperty -- Property editor for Growable Sizer Columns
    customprops/sizer_grow_rows.cpp # GrowRowsProperty -- Property editor for Growable Sizer Rows
    customprops/txt_string_prop.cpp # EditStringProperty -- Derived wxStringProperty class for single-line text

    customprops/eventhandler_dlg.cpp # EventHandlerDlg -- Dialog for editing event handlers
    customprops/font_prop_dlg.cpp # FontPropDlg -- Dialog for editing Font Property
    customprops/img_props.cpp # ImageProperties -- Handles property grid image properties

    customprops/tt_file_property.cpp # ttFileProperty -- Version of wxFileProperty specific to wxUiEditor

    customprops/pg_image.cpp # PropertyGrid_Image -- Custom property grid class for images
    customprops/pg_animation.cpp # PropertyGrid_Animation -- Custom property grid class for animations
    customprops/pg_point.cpp # CustomPointProperty -- Custom wxPGProperty for wxPoint

    # (generated) customprops/art_prop_dlg.cpp        # Art Property Dialog for image property
    # (generated) customprops/include_files_dlg.cpp
    # (generated) customprops/id_editor_dlg.cpp
    custom_ctrls/colour_rect_ctrl.cpp # ColourRectCtrl -- Control that displays a solid color
    custom_ctrls/kw_color_picker.cpp # kwColourPickerCtrl -- Modified version of wxColourPickerCtrl

    # Code generation modules
    generate/code.cpp # Code -- Helper class for generating code
    generate/code_add.cpp # Code::Add...() functions
    generate/code_bundle.cpp # Code::Bundle() functions
    generate/code_gen.cpp # Code::Gen...() functions
    generate/code_pos_style.cpp # Code::Pose...()/Style() functions

    generate/file_codewriter.cpp # FileCodeWriter -- Class to write code to disk
    generate/gen_base.cpp # BaseCodeGenerator -- Generate Src and Hdr files for Base and Derived Class
    generate/gen_codefiles.cpp # Generate code functions
    generate/gen_construction.cpp # Top level Object construction code
    generate/gen_cmake.cpp # Auto-generate a .cmake file
    generate/gen_events.cpp # Generate events for all languages
    generate/write_code.cpp # Write code to Scintilla or file
    generate/image_gen.cpp # Functions for generating embedded images

    generate/base_generator.cpp # BaseGenerator -- Base widget generator class
    generate/gen_initialize.cpp # Initialize all widget generate classes
    generate/gen_common.cpp # Common widget generation functions

    generate/gen_xrc_utils.cpp # Common XRC generating functions
    generate/gen_book_utils.cpp # Common Book utilities
    generate/utils_prop_grid.cpp # PropertyGrid utilities

    # ############################ begin generators ##############################

    # Code generation output languages
    generate/gen_cpp.cpp # Generate C++ code
    generate/gen_cpp_header.cpp # Generate C++ code header content
    generate/gen_cpp_variables.cpp # C++ code generation variable and validator functions
    generate/gen_derived.cpp # Generate C++ Derived code
    generate/gen_perl.cpp # Generate wxPerl code
    generate/gen_python.cpp # Generate wxPython code
    generate/gen_ruby.cpp # Generate wxRuby3 code
    generate/gen_rust.cpp # Generate wxRust code
    generate/gen_xrc.cpp # Generate XRC

    # Generators are responsible for displaying the widget in the Mockup window,
    # and generating both C++, Python, Ruby and XRC code. If the generated object
    # can be used in a Dialog or Frame, then the generator must also be able to
    # create a preview of the object for the Preview display.
    generate/gen_images_list.cpp # Images List Embedded images generator
    generate/gen_data_list.cpp # Data List generator
    generate/gen_project.cpp # Project generator

    # Misc widgets
    generate/dataview_widgets.cpp # wxDataView generation classes
    generate/gen_activity.cpp # wxActivityIndicator generator
    generate/gen_animation.cpp # wxAnimationCtrl generator
    generate/gen_banner_window.cpp # wxBannerWindow generator
    generate/gen_bitmap_combo.cpp # wxBitmapComboBox generator
    generate/gen_button.cpp # wxButton generator
    generate/gen_calendar_ctrl.cpp # wxCalendarCtrl generator
    generate/gen_check_listbox.cpp # wxCheckListBox generator
    generate/gen_checkbox.cpp # wxCheckBox generator
    generate/gen_choice.cpp # wxChoice generator
    generate/gen_close_btn.cpp # wxBitmapButton::CreateCloseButton generator
    generate/gen_cmd_link_btn.cpp # wxCommandLinkButton generator
    generate/gen_combobox.cpp # wxComboBox generator
    generate/gen_ctx_help_btn.cpp # wxContextHelpButton generator
    generate/gen_custom_ctrl.cpp # Custom Control generator
    generate/gen_edit_listbox.cpp # wxEditableListBox generator
    generate/gen_file_ctrl.cpp # wxFileCtrl generator
    generate/gen_gauge.cpp # wxGauge generator
    generate/gen_grid.cpp # wxGrid generator
    generate/gen_html_listbox.cpp # wxSimpleHtmlListBox generator
    generate/gen_html_window.cpp # wxHtmlWindow generator
    generate/gen_hyperlink.cpp # wxHyperlinkCtrl generator
    generate/gen_infobar.cpp # wxInfoBar generator
    generate/gen_listbox.cpp # wxListBox generator
    generate/gen_listview.cpp # wxListView generator
    generate/gen_radio_box.cpp # wxRadioBox generator
    generate/gen_radio_btn.cpp # wxRadioButton generator
    generate/gen_rearrange.cpp # wxRearrangeCtrl generator
    generate/gen_rich_text.cpp # wxRichTextCtrl generator
    generate/gen_scrollbar.cpp # wxScrollBar generator
    generate/gen_search_ctrl.cpp # wxSearchCtrl generator
    generate/gen_slider.cpp # wxSlider generator
    generate/gen_spin_btn.cpp # wxSpinButton generator
    generate/gen_spin_ctrl.cpp # wxSpinCtrl generator
    generate/gen_static_bmp.cpp # wxStaticBitmap generator
    generate/gen_static_box.cpp # wxStaticBox generator
    generate/gen_static_line.cpp # wxStaticLine generator
    generate/gen_static_text.cpp # wxStaticText generator
    generate/gen_status_bar.cpp # wxStatusBar generator
    generate/gen_styled_text.cpp # wxStyledText (scintilla) generate
    generate/gen_text_ctrl.cpp # wxTextCtrl generator
    generate/gen_timer.cpp # wxTimer generator
    generate/gen_toggle_btn.cpp # wxToggleButton generator
    generate/gen_tree_ctrl.cpp # wxTreeCtrl generator
    generate/gen_tree_list.cpp # wxTreeListCtrl generator
    generate/gen_web_view.cpp # wxWebView generator

    # Non-python widgets
    generate/gen_dir_ctrl.cpp # wxGenericDirCtrl generator

    # Books
    generate/gen_aui_notebook.cpp # wxAuiNotebook generator
    generate/gen_book_page.cpp # Book page generator
    generate/gen_choicebook.cpp # wxChoicebook generator
    generate/gen_listbook.cpp # wxListbook generator
    generate/gen_notebook.cpp # wxNotebook generator
    generate/gen_page_ctrl.cpp # Page control generator
    generate/gen_simplebook.cpp # wxSimplebook generator
    generate/gen_toolbook.cpp # wxToolbook generator
    generate/gen_treebook.cpp # wxTreebook generator

    # Containers
    generate/gen_collapsible.cpp # wxCollapsiblePane generator
    generate/gen_panel.cpp # wxPanel generator
    generate/gen_split_win.cpp # wxSplitterWindow generator
    generate/window_widgets.cpp # ScrolledCanvasGenerator and ScrolledWindowGenerator

    # Document Management (MDI)
    newdialogs/new_mdi.cpp # Dialog for creating a new MDI application

    generate/mdi/gen_doc_view_app.cpp # Generates base class for wxDocument/wxView applications

    generate/mdi/gen_doc_image.cpp # wxImage document class
    generate/mdi/gen_doc_richtext.cpp # wxRichTextCtrl document class
    generate/mdi/gen_doc_splitter.cpp # wxSplitterWindow document class
    generate/mdi/gen_doc_scintilla.cpp # Scintilla document class
    generate/mdi/gen_doc_textctrl.cpp # wxTextCtrl document class

    generate/mdi/gen_view_image.cpp # wxImage view class
    generate/mdi/gen_view_richtext.cpp # wxRichTextCtrl view class
    generate/mdi/gen_view_splitter.cpp # wxSplitterWindow view class
    generate/mdi/gen_view_scintilla.cpp # wxStyledTextCtrl view class
    generate/mdi/gen_view_textctrl.cpp # wxTextCtrl view class

    generate/mdi/gen_doc_parent_frame.cpp # wxDocParentFrame generator
    generate/mdi/gen_doc_mdi_frame.cpp # wxDocMdiParentFrame generator
    generate/mdi/gen_aui_mdi_frame.cpp # wxAuiMDIParentFrame generator

    generate/mdi/gen_mdi_menu.cpp # Menu bar classes for an MDI frame

    # Forms
    generate/gen_dialog.cpp # wxDialog generator
    generate/gen_frame.cpp # wxFrame generator
    generate/gen_panel_form.cpp # wxPanel Form generator
    generate/gen_popup_win.cpp # wxPopupTransientWindow generator
    generate/gen_propsheet_dlg.cpp # wxPropertySheetDialog generator
    generate/gen_wizard.cpp # wxWizard generator

    generate/gen_frame_common.cpp # wxDocParent common generator functions

    # Menus
    generate/menu_widgets.cpp # MenuBar, MenuBarForm, PopupMenu, Separator
    generate/gen_ctx_menu.cpp # CtxMenuGenerator -- generates function and includes
    generate/gen_menu.cpp # Menu Generator
    generate/gen_menuitem.cpp # Menu Item Generator
    generate/gen_submenu.cpp # SubMenu Generator

    # Pickers
    generate/gen_clr_picker.cpp # wxColourPickerCtrl generator
    generate/gen_date_picker.cpp # wxDatePickerCtrl generator
    generate/gen_dir_picker.cpp # wxDirPickerCtrl generator
    generate/gen_file_picker.cpp # wxFilePickerCtrl generator
    generate/gen_font_picker.cpp # wxFontPickerCtrl generator
    generate/gen_time_picker.cpp # wxTimePickerCtrl generator

    # PropertyGrid
    generate/gen_prop_category.cpp # PropertyGridCategory (propGridItem) generator
    generate/gen_prop_grid.cpp # wxPropertyGrid generator
    generate/gen_prop_grid_mgr.cpp # wxPropertyGridManager and wxPropertyGridPage generators
    generate/gen_prop_item.cpp # PropertyGridItem (propGridItem) generator

    # Sizers
    generate/gen_box_sizer.cpp # wxBoxSizer generator
    generate/gen_flexgrid_sizer.cpp # wxFlexGridSizer generator
    generate/gen_grid_sizer.cpp # wxGridSizer generator
    generate/gen_gridbag_sizer.cpp # wxGridBagSizer generator
    generate/gen_spacer_sizer.cpp # Add space to sizer generator
    generate/gen_statchkbox_sizer.cpp # wxStaticBoxSizer with wxCheckBox generator
    generate/gen_staticbox_sizer.cpp # wxStaticBoxSizer generator
    generate/gen_statradiobox_sizer.cpp # wxStaticBoxSizer with wxRadioButton generator
    generate/gen_std_dlgbtn_sizer.cpp # wxStdDialogButtonSizer generator
    generate/gen_text_sizer.cpp # wxTextSizerWrapper generator
    generate/gen_wrap_sizer.cpp # wxWrapSizer generator

    # Toolbars
    generate/gen_aui_toolbar.cpp # wxAuiToolBar generator
    generate/gen_ribbon_bar.cpp # wxRibbonButtonBar generator
    generate/gen_ribbon_button.cpp # wxRibbonBar -- form and regular
    generate/gen_ribbon_gallery.cpp # wxRibbonGallery generator
    generate/gen_ribbon_page.cpp # wxRibbonPage and wxRibbonPanel generators
    generate/gen_ribbon_tool.cpp # wxRibbonToolBar generator
    generate/gen_toolbar.cpp # wxToolBar generator
    generate/gen_tools.cpp # Generate tools for wxToolBar

    # ############################ Importers ##############################

    # Importers (also see Windows Resource importer below)
    import/import_dialogblocks.cpp # Import a DialogBlocks project
    import/import_formblder.cpp # Import a wxFormBuider project
    import/import_wxcrafter.cpp # Import a wxCrafter project
    import/import_wxglade.cpp # Import a Import a wxGlade file
    import/import_wxsmith.cpp # Process XRC files
    import/import_xml.cpp # Base class for XML importing

    # (generated) import/import_winres_dlg.cpp

    # ############################ Mockup Panel ##############################
    mockup/mockup_content.cpp # Mockup of a form's contents
    mockup/mockup_parent.cpp # Top-level MockUp Parent window
    mockup/mockup_preview.cpp # Preview Mockup
    mockup/mockup_wizard.cpp # Emulate a wxWizard

    # ############################ New Form Dialogs ##############################
    # Dialogs for creating new forms
    newdialogs/new_common.cpp # Contains code common between all new_ dialogs

    # (generated) newdialogs/new_dialog.cpp     # Dialog for creating a new project dialog
    # (generated) newdialogs/new_frame.cpp      # Dialog for creating a new project wxFrame
    # (generated) newdialogs/new_panel.cpp      # Dialog for creating a new form panel
    # (generated) newdialogs/new_propsheet.cpp  # Dialog for creating a new wxPropertySheetDialog
    # (generated) newdialogs/new_ribbon.cpp     # Dialog for creating a new wxRibbonBar
    # (generated) newdialogs/new_wizard.cpp     # Dialog for creating a new wxWizard
    # (generated) newdialogs/new_dialog.cpp     # Dialog for creating a new project dialog

    # ############################ Nodes ##############################
    nodes/node.cpp # Contains user-modifiable node
    nodes/node_constants.cpp # Maps wxWidgets constants to their numerical value
    nodes/node_creator.cpp # Class used to create nodes
    nodes/node_decl.cpp # Contains the declarations for a node (properties, events, etc.)
    nodes/node_gridbag.cpp # Create and modify a node containing a wxGridBagSizer
    nodes/node_init.cpp # Initialize NodeCreator class
    nodes/node_prop.cpp # NodeProperty class
    nodes/tool_creator.cpp # Functions for creating new nodes from Ribbon Panel

    # ############################ Panels ##############################
    panels/base_panel.cpp # Code generation panel
    panels/code_display.cpp # Display code in scintilla control

    # internal/import_panel.cpp  # Internal-only panel to display original Import file
    panels/nav_panel.cpp # Navigation panel
    panels/navpopupmenu.cpp # Context-menu for Navigation panel
    panels/propgrid_create.cpp # PropGridPanel Create functions
    panels/propgrid_events.cpp # PropGridPanel event handlers
    panels/propgrid_modify.cpp # PropGridPanel modification functions
    panels/propgrid_panel.cpp # Property panel
    panels/cstm_propgrid.cpp # Derived wxPropertyGrid class
    panels/ribbon_tools.cpp # Displays node creation tools in a wxRibbonBar

    # (generated) panels/doc_view.cpp         # Panel for displaying docs in wxWebView
    # (generated) panels/nav_toolbar.cpp

    # Project classes
    project/data_handler.cpp # DataHandler class
    project/embed_image.cpp # class to manage images stored in the generated code
    project/image_handler.cpp # ProjectImage class
    project/loadproject.cpp # Load wxUiEditor project
    project/project_handler.cpp # ProjectHandler class
    project/saveproject.cpp # Save a wxUiEditor project file

    utils/dlg_msgs.cpp # wxMessageDialog dialogs
    utils/font_prop.cpp # FontProperty class
    utils/set_stc_colors.cpp # Contains function for initializing wxStyledTextCtrl colors
    utils/utils.cpp # Utility functions that work with properties

    # Testing
    internal/node_info.cpp # Node memory usage dialog
    internal/msgframe.cpp # Stores messages
    internal/msg_logging.cpp # Message logging class
    internal/import_panel.cpp # Panel to display original imported file
    internal/import_panel.cpp # Panel to display original imported file
    tests/test_xrc_import.cpp # XRC Import tests

    internal/xrcpreview.cpp # Generated XrcPreview Dialog code
    internal/xrcpreview_handlers.cpp # Handlers for XrcPreview Dialog class
    internal/xrc_list_dlg.cpp

    # Tools

    # tools/generate_dlg.cpp        # Dialog for choosing and generating specific language file(s)
    # tools/preview_settings.cpp    # Dialog for setting preview options
    # (generated) tools/global_ids_dlg.cpp  # Dialog to Globally edit Custom IDs

    # UI

    # (generated) ui/code_preference_dlg.cpp
    # (generated) ui/startup_dlg.cpp
    # (generated) ui/generate_xrc_dlg.cpp  # Dialog for generating XRC file(s)

    # Windows resource importer
    winres/ctrl_utils.cpp # resCtrl class utility functions
    winres/form_utils.cpp # resForm utility functions code
    winres/import_winres.cpp # Import a Windows resource file
    winres/winres_ctrl.cpp # Process Windows Resource control data
    winres/winres_dlg.cpp # Process a Windows Resource DIALOG or DIALOGEX
    winres/winres_menu.cpp # Process a Windows Resource MENU
    winres/winres_images.cpp # resCtrl class image/icon functions
    winres/winres_layout.cpp # resForm layout code
    winres/winres_styles.cpp # resCtrl class style processing functions

    pugixml/pugixml.cpp # XML parser

    ui/generate_xrc_dlg.cpp # Dialog for generating XRC file(s)
    ui/gridbag_item.cpp # Dialog for inserting an item into a wxGridBagSizer node
    ui/import_dlg.cpp # Dialog to import one or more projects
    ui/xrccompare.cpp # C++/XRC UI Comparison dialog

    # (generated) ui/preferences_dlg.cpp  # Preferences dialog
    $<$<CONFIG:Debug>:internal/convert_img.cpp> # Convert image
    $<$<CONFIG:Debug>:tests/test_xrc_import.cpp> # XRC Import tests
)

set( debug_files

    # These are just here to make it easier to open with a VSCode extension
    internal/code_compare.cpp
    internal/convert_img.cpp

    internal/code_compare.cpp
    internal/convert_img_base.cpp
    internal/debugsettings.cpp
    internal/msgframe_base.cpp
    internal/node_search_dlg.cpp
    internal/undo_info.cpp
    internal/unused_gen_dlg.cpp

    tools/generate_dlg.cpp
    tools/preview_settings.cpp

    wxui/optionsdlg.cpp
    wxui/code_preference_dlg.cpp
    wxui/dlg_gen_results.cpp
)

set( doc_list

    generate/SUPPORTED.md
    ../CHANGELOG.md
    ../README.md
    ../docs/build_notes.md
    ../docs/images.md
    ../docs/release_prep.md
    ../docs/import_crafter.md
    ../docs/import_dialogblocks.md
    ../docs/import_formbuilder.md
    ../docs/import_winres.md
    ../docs/xrc.md

    ../dev_docs/dev_notes.md
    ../dev_docs/xml_interface.md
    ../dev_docs/generators.md
    ../dev_docs/windows_res.md
    ../dev_docs/node_classes.md

    ../.github/copilot-instructions.md
    ../Claude.md
)

set( ttwx_files
    ttwx/tests/QUICK_START.md
    ttwx/tests/README.md
    ttwx/tests/test_ttwx.cpp
    ttwx/tests/CMakeLists.txt
    ttwx/tests/test_string_vector.cpp
    ttwx/tests/test_view_vector.cpp

    ttwx/ttwx.cpp
    ttwx/ttwx.h
    ttwx/ttwx_view_vector.cpp
    ttwx/ttwx_view_vector.h
    ttwx/ttwx_string_vector.cpp
    ttwx/ttwx_string_vector.h
)

set( xml_list

    # interface declarations
    xml/interfaces.xml

    # generator declarations
    xml/bars.xml
    xml/boxes.xml
    xml/buttons.xml
    xml/containers.xml
    xml/data_ctrls.xml
    xml/forms.xml
    xml/mdi.xml
    xml/pickers.xml
    xml/project.xml
    xml/sizers.xml
    xml/text_ctrls.xml
    xml/widgets.xml
)

set( workflow_list
    ../.github/workflows/build-ubuntu-setup.yml
    ../.github/workflows/build-win-setup.yml
    ../.github/workflows/check_build.yml
    ../.github/workflows/cleanup-workflows.yml
    ../.github/workflows/codeql-analysis.yml
    ../.github/workflows/daily_build.yml
    ../.github/workflows/test_clang_format.yml
)
