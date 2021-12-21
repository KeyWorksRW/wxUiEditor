set (file_list

    ${CMAKE_CURRENT_LIST_DIR}/wxUiEditor.rc         # Only processed when compiling for Windows, ignored otherwise

    ${CMAKE_CURRENT_LIST_DIR}/mainapp.cpp           # Main application class
    ${CMAKE_CURRENT_LIST_DIR}/mainframe.cpp         # Main window frame

    ${CMAKE_CURRENT_LIST_DIR}/appoptions.cpp        # Application-wide options
    ${CMAKE_CURRENT_LIST_DIR}/bitmaps.cpp           # Map of bitmaps accessed by name
    ${CMAKE_CURRENT_LIST_DIR}/clipboard.cpp         # Handles reading and writing OS clipboard data
    ${CMAKE_CURRENT_LIST_DIR}/cstm_event.cpp        # Custom Event handling
    ${CMAKE_CURRENT_LIST_DIR}/frame_status_bar.cpp  # MainFrame status bar functions
    ${CMAKE_CURRENT_LIST_DIR}/gen_enums.cpp         # Enumerations for generators
    ${CMAKE_CURRENT_LIST_DIR}/lambdas.cpp           # Functions for formatting and storage of lamda events
    ${CMAKE_CURRENT_LIST_DIR}/pjtsettings.cpp       # Hold data for currently loaded project
    ${CMAKE_CURRENT_LIST_DIR}/startup.cpp           # Dialog to display is wxUE is launched with no arguments
    ${CMAKE_CURRENT_LIST_DIR}/undo_cmds.cpp         # Undoable command classes derived from UndoStackCmd
    ${CMAKE_CURRENT_LIST_DIR}/undo_stack.cpp        # Maintain an undo and redo stack
    ${CMAKE_CURRENT_LIST_DIR}/wakatime.cpp          # Updates WakaTime metrics
    ${CMAKE_CURRENT_LIST_DIR}/xpm.cpp               # All xpm files

    ${CMAKE_CURRENT_LIST_DIR}/customprops/code_string_prop.cpp   # Derived wxStringProperty class for code
    ${CMAKE_CURRENT_LIST_DIR}/customprops/custom_param_prop.cpp  # Derived wxStringProperty class for custom control parameters
    ${CMAKE_CURRENT_LIST_DIR}/customprops/evt_string_prop.cpp    # Derived wxStringProperty class for event function
    ${CMAKE_CURRENT_LIST_DIR}/customprops/img_string_prop.cpp    # Derived wxStringProperty class for handling wxImage files or art
    ${CMAKE_CURRENT_LIST_DIR}/customprops/txt_string_prop.cpp    # Derived wxStringProperty class for single-line text
    ${CMAKE_CURRENT_LIST_DIR}/customprops/custom_colour_prop.cpp # Property editor for colour

    ${CMAKE_CURRENT_LIST_DIR}/customprops/art_prop_dlg.cpp   # Art Property Dialog for image property
    ${CMAKE_CURRENT_LIST_DIR}/customprops/img_props.cpp      # Handles property grid image properties

    ${CMAKE_CURRENT_LIST_DIR}/customprops/pg_image.cpp       # Custom property grid class for images
    ${CMAKE_CURRENT_LIST_DIR}/customprops/pg_animation.cpp   # Custom property grid class for animations
    ${CMAKE_CURRENT_LIST_DIR}/customprops/pg_point.cpp       # Custom wxPGProperty for wxPoint

    ${CMAKE_CURRENT_LIST_DIR}/custom_ctrls/colour_rect_ctrl.cpp  # Control that displays a solid color

    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_base.cpp          # Generate Src and Hdr files for Base and Derived Class
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_cmake.cpp         # Auto-generate a .cmake file
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_codefiles.cpp     # Generate code files
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_derived.cpp       # Generate Derived class code
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_inherit.cpp       # Generate Inherited class code
    ${CMAKE_CURRENT_LIST_DIR}/generate/write_code.cpp        # Write code to Scintilla or file

    ${CMAKE_CURRENT_LIST_DIR}/generate/base_generator.cpp    # Base widget generator class
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_initialize.cpp    # Initialize all widget generate classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/gen_common.cpp        # Common widget generation functions

    # The following files in generate/ are used to display the widget in the Mockup panel and to generate
    # the code to insert into the source and header files.

    ${CMAKE_CURRENT_LIST_DIR}/generate/book_widgets.cpp      # Book generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/btn_widgets.cpp       # Button generation class
    ${CMAKE_CURRENT_LIST_DIR}/generate/checkbox_widgets.cpp  # wxCheckBox generation class
    ${CMAKE_CURRENT_LIST_DIR}/generate/combo_widgets.cpp     # Combo and choice generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/ctrl_widgets.cpp      # Calendar, GenericDir, Search Ctrl generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/dataview_widgets.cpp  # wxDataView generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/form_widgets.cpp      # Form generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/grid_widgets.cpp      # Grid generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/images_form.cpp       # Embedded images generator
    ${CMAKE_CURRENT_LIST_DIR}/generate/listbox_widgets.cpp   # ListBox generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/listctrl_widgets.cpp  # ListCtrl generation class
    ${CMAKE_CURRENT_LIST_DIR}/generate/menu_widgets.cpp      # Menu generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/misc_widgets.cpp      # Miscellaneous generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/panel_widgets.cpp     # Panel generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/picker_widgets.cpp    # Picker generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/radio_widgets.cpp     # Radio button and Radio box generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/ribbon_widgets.cpp    # Ribbon generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/sizer_widgets.cpp     # Sizer generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/spin_widgets.cpp      # Spin and ScrollBar generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/text_widgets.cpp      # Text generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/toolbar_widgets.cpp   # Toolbar generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/tree_widgets.cpp      # wxTreeCtrl generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/window_widgets.cpp    # Splitter and Scroll generation classes
    ${CMAKE_CURRENT_LIST_DIR}/generate/wizard_form.cpp       # Wizard form class

    ${CMAKE_CURRENT_LIST_DIR}/import/import_crafter_maps.cpp # wxCrafter mappings
    ${CMAKE_CURRENT_LIST_DIR}/import/import_formblder.cpp    # Import a wxFormBuider project
    ${CMAKE_CURRENT_LIST_DIR}/import/import_wxcrafter.cpp    # Import a wxCrafter project
    ${CMAKE_CURRENT_LIST_DIR}/import/import_wxglade.cpp      # Import a Import a wxGlade file
    ${CMAKE_CURRENT_LIST_DIR}/import/import_wxsmith.cpp      # Process XRC files
    ${CMAKE_CURRENT_LIST_DIR}/import/import_xml.cpp          # Base class for XML importing

    ${CMAKE_CURRENT_LIST_DIR}/mockup/mockup_parent.cpp    # Top-level MockUp Parent window
    ${CMAKE_CURRENT_LIST_DIR}/mockup/mockup_content.cpp   # Mockup of a form's contents
    ${CMAKE_CURRENT_LIST_DIR}/mockup/mockup_wizard.cpp    # Emulate a wxWizard

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

    ${CMAKE_CURRENT_LIST_DIR}/winres/ctrl_utils.cpp       # resCtrl class utility functions
    ${CMAKE_CURRENT_LIST_DIR}/winres/form_utils.cpp       # resForm utility functions code
    ${CMAKE_CURRENT_LIST_DIR}/winres/import_winres.cpp    # Import a Windows resource file
    ${CMAKE_CURRENT_LIST_DIR}/winres/winres_ctrl.cpp      # Process Windows Resource control data
    ${CMAKE_CURRENT_LIST_DIR}/winres/winres_dlg.cpp       # Process a Windows Resource DIALOG or DIALOGEX
    ${CMAKE_CURRENT_LIST_DIR}/winres/winres_menu.cpp      # Process a Windows Resource MENU
    ${CMAKE_CURRENT_LIST_DIR}/winres/winres_images.cpp    # resCtrl class image/icon functions
    ${CMAKE_CURRENT_LIST_DIR}/winres/winres_layout.cpp    # resForm layout code
    ${CMAKE_CURRENT_LIST_DIR}/winres/winres_styles.cpp    # resCtrl class style processing functions

    pugixml/pugixml.cpp             # XML parser

    ${CMAKE_CURRENT_LIST_DIR}/ui/embedimg.cpp             # Convert image to Header (.h) or XPM (.xpm) file
    ${CMAKE_CURRENT_LIST_DIR}/ui/eventhandlerdlg.cpp      # Dialog for editing event handlers
    ${CMAKE_CURRENT_LIST_DIR}/ui/gridbag_item.cpp         # Dialog for inserting an item into a wxGridBagSizer node
    ${CMAKE_CURRENT_LIST_DIR}/ui/import_dlg.cpp           # Dialog to import one or more projects
    ${CMAKE_CURRENT_LIST_DIR}/ui/importwinresdlg.cpp      # Dialog for Importing a Windows resource file
    ${CMAKE_CURRENT_LIST_DIR}/ui/insertwidget.cpp         # Dialog to lookup and insert a widget
    ${CMAKE_CURRENT_LIST_DIR}/ui/newdialog.cpp            # Dialog for creating a new project dialog
    ${CMAKE_CURRENT_LIST_DIR}/ui/newframe.cpp             # Dialog for creating a new project wxFrame
    ${CMAKE_CURRENT_LIST_DIR}/ui/newribbon.cpp            # Dialog for creating a new wxRibbonBar
    ${CMAKE_CURRENT_LIST_DIR}/ui/optionsdlg.cpp           # Dialog containing special Debugging commands

    # ttLib submodule files

    ${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/ttcstr.cpp             # Class for handling zero-terminated char strings.
    ${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/ttcview.cpp            # string_view functionality on a zero-terminated char string.
    ${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/ttsview.cpp            # std::string_view with additional methods
    ${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/ttmultistr.cpp         # Breaks a single string into multiple strings
    ${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/ttparser.cpp           # Command line parser
    ${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/ttstr.cpp              # Enhanced version of wxString
    ${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/ttlibspace.cpp         # ttlib namespace functions
    ${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/tttextfile.cpp         # Classes for reading and writing text files.

    # Debug-only files

    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/debugging/debugsettings.cpp>  # Settings while running the Debug version of wxUiEditor
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/debugging/dbg_code_diff.cpp>  # Compare code generationg
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/debugging/msg_logging.cpp>    # Message logging class
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/debugging/msgframe.cpp>       # Stores messages
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/debugging/nodeinfo.cpp>       # Node memory usage dialog

    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/debugging/msgframe_base.cpp>  # wxUiEditor generated file
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/debugging/nodeinfo_base.cpp>  # wxUiEditor generated file
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/debugging/dbg_code_diff_base.cpp>
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/debugging/debugsettingsBase.cpp>  # wxUiEditor generated file

    $<$<CONFIG:Debug>:${CMAKE_CURRENT_LIST_DIR}/../ttLib/src/winsrc/ttdebug_min.cpp>  # ttAssertionMsg

)
