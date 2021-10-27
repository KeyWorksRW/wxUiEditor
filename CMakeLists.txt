cmake_minimum_required(VERSION 3.20)

project(wxUiEditor
    LANGUAGES CXX
    VERSION 0.5.0
    DESCRIPTION "wxWidgets UI designer"
    HOMEPAGE_URL "https://github.com/KeyWorksRW/wxUiEditor")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    # /O1 often results in faster code than /O2 due to CPU caching
    string(REPLACE "/O2" "/O1" cl_optimize ${CMAKE_CXX_FLAGS_RELEASE})
    set(CMAKE_CXX_FLAGS_RELEASE ${cl_optimize} CACHE STRING "C++ Release flags" FORCE)

    # Using /Z7 instead of /Zi to avoid blocking while parallel compilers write to the pdb file.
    # This can considerably speed up build times at the cost of larger object files.
    string(REPLACE "/Zi" "/Z7" z_seven ${CMAKE_CXX_FLAGS_DEBUG})
    set(CMAKE_CXX_FLAGS_DEBUG ${z_seven} CACHE STRING "C++ Debug flags" FORCE)
endif()

get_property(isMultiConfig GLOBAL
  PROPERTY GENERATOR_IS_MULTI_CONFIG
)

if (NOT isMultiConfig)
    message("\nBecause you are using a single target generator, you MUST specify")
    message("    a \"--config [Debug|Release]\" option with the cmake --build command\n")

    set(allowedBuildTypes Debug Release)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")

    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
    elseif (NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
        message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
    endif()
endif()

add_compile_definitions($<$<CONFIG:Release>:NDEBUG>)
add_compile_definitions($<$<CONFIG:Debug>:WXUSINGDLL>)

# Setting CMAKE_MODULE_PATH causes ninja to fail rebuilding until CMake re-generates. Specifying the full path and extension
# means ninja sees this as a normal dependency that didn't change any time one of the files it specifies changes.

include(src/ui/wxui_code.cmake)  # This will set ${wxue_generated_code} with list of generated files

# Note the requirement that --config Debug is used to get the additional debug files
add_executable(wxUiEditor WIN32
    src/wxUiEditor.rc         # Only processed when compiling for Windows, ignored otherwise

    src/mainapp.cpp           # Main application class
    src/mainframe.cpp         # Main window frame

    src/appoptions.cpp        # Application-wide options
    src/bitmaps.cpp           # Map of bitmaps accessed by name
    src/clipboard.cpp         # Handles reading and writing OS clipboard data
    src/cstm_event.cpp        # Custom Event handling
    src/frame_status_bar.cpp  # MainFrame status bar functions
    src/gen_enums.cpp         # Enumerations for generators
    src/gzip.cpp              # Convert compiled GZIP arrays into original strings/images
    src/lambdas.cpp           # Functions for formatting and storage of lamda events
    src/pjtsettings.cpp       # Hold data for currently loaded project
    src/startup.cpp           # Dialog to display is wxUE is launched with no arguments
    src/strings.cpp           # Contains translatable strings
    src/undo_cmds.cpp         # Undoable command classes derived from UndoStackCmd
    src/undo_stack.cpp        # Maintain an undo and redo stack
    src/wakatime.cpp          # Updates WakaTime metrics
    src/xpm.cpp               # All xpm files

    src/customprops/evt_string_prop.cpp   # Derived wxStringProperty class for event function
    src/customprops/img_string_prop.cpp   # Derived wxStringProperty class for handling wxImage files or art
    src/customprops/txt_string_prop.cpp   # Derived wxStringProperty class for single-line text

    src/customprops/art_prop_dlg.cpp   # Art Property Dialog for image property
    src/customprops/img_props.cpp      # Handles property grid image properties

    src/customprops/pg_image.cpp       # Custom property grid class for images
    src/customprops/pg_animation.cpp   # Custom property grid class for animations
    src/customprops/pg_point.cpp       # Custom wxPGProperty for wxPoint

    src/generate/gen_base.cpp          # Generate Src and Hdr files for Base and Derived Class
    src/generate/gen_cmake.cpp         # Auto-generate a .cmake file
    src/generate/gen_codefiles.cpp     # Generate code files
    src/generate/gen_derived.cpp       # Generate Derived class code
    src/generate/gen_inherit.cpp       # Generate Inherited class code
    src/generate/write_code.cpp        # Write code to Scintilla or file

    src/generate/base_generator.cpp    # Base widget generator class
    src/generate/gen_initialize.cpp    # Initialize all widget generate classes
    src/generate/gen_common.cpp        # Common widget generation functions

    # The following files in generate/ are used to display the widget in the Mockup panel and to generate
    # the code to insert into the source and header files.

    src/generate/book_widgets.cpp      # Book generation classes
    src/generate/btn_widgets.cpp       # Button generation class
    src/generate/checkbox_widgets.cpp  # wxCheckBox generation class
    src/generate/combo_widgets.cpp     # Combo and choice generation classes
    src/generate/ctrl_widgets.cpp      # Calendar, GenericDir, Search Ctrl generation classes
    src/generate/dataview_widgets.cpp  # wxDataView generation classes
    src/generate/form_widgets.cpp      # Form generation classes
    src/generate/grid_widgets.cpp      # Grid generation classes
    src/generate/images_form.cpp       # Embedded images generator
    src/generate/listbox_widgets.cpp   # ListBox generation classes
    src/generate/listctrl_widgets.cpp  # ListCtrl generation class
    src/generate/menu_widgets.cpp      # Menu generation classes
    src/generate/misc_widgets.cpp      # Miscellaneous generation classes
    src/generate/panel_widgets.cpp     # Panel generation classes
    src/generate/picker_widgets.cpp    # Picker generation classes
    src/generate/radio_widgets.cpp     # Radio button and Radio box generation classes
    src/generate/ribbon_widgets.cpp    # Ribbon generation classes
    src/generate/sizer_widgets.cpp     # Sizer generation classes
    src/generate/spin_widgets.cpp      # Spin and ScrollBar generation classes
    src/generate/text_widgets.cpp      # Text generation classes
    src/generate/toolbar_widgets.cpp   # Toolbar generation classes
    src/generate/tree_widgets.cpp      # wxTreeCtrl generation classes
    src/generate/window_widgets.cpp    # Splitter and Scroll generation classes
    src/generate/wizard_form.cpp       # Wizard form class

    src/import/import_formblder.cpp    # Import a wxFormBuider project
    src/import/import_wxglade.cpp      # Import a Import a wxGlade file
    src/import/import_wxsmith.cpp      # Process XRC files
    src/import/import_xml.cpp          # Base class for XML importing

    src/mockup/mockup_parent.cpp    # Top-level MockUp Parent window
    src/mockup/mockup_content.cpp   # Mockup of a form's contents
    src/mockup/mockup_wizard.cpp    # Emulate a wxWizard

    src/nodes/node.cpp              # Contains user-modifiable node
    src/nodes/node_constants.cpp    # Maps wxWidgets constants to their numerical value
    src/nodes/node_creator.cpp      # Class used to create nodes
    src/nodes/node_decl.cpp         # Contains the declarations for a node (properties, events, etc.)
    src/nodes/node_gridbag.cpp      # Create and modify a node containing a wxGridBagSizer
    src/nodes/node_init.cpp         # Initialize NodeCreator class
    src/nodes/node_prop.cpp         # NodeProperty class
    src/nodes/tool_creator.cpp      # Functions for creating new nodes from Ribbon Panel

    src/panels/cstm_propgrid.cpp    # Derived wxPropertyGrid class
    src/panels/code_display.cpp     # Display code in scintilla control
    src/panels/base_panel.cpp       # Code generation panel
    src/panels/nav_panel.cpp        # Navigation panel
    src/panels/navpopupmenu.cpp     # Context-menu for Navigation panel
    src/panels/propgrid_panel.cpp   # Property panel
    src/panels/ribbon_tools.cpp     # Displays node creation tools in a wxRibbonBar

    src/project/loadproject.cpp     # Load wxUiEditor project
    src/project/saveproject.cpp     # Save a wxUiEditor project file

    src/utils/font_prop.cpp         # FontProperty class
    src/utils/utils.cpp             # Utility functions that work with properties

    src/winres/import_winres.cpp    # Import a Windows resource file
    src/winres/winres_ctrl.cpp      # Process Windows Resource control data
    src/winres/winres_form.cpp      # Process a Windows Resource form (usually a dialog)
    src/winres/winres_images.cpp    # rcCtrl class image/icon functions
    src/winres/winres_layout.cpp    # rcForm layout code
    src/winres/winres_styles.cpp    # rcCtrl style processing functions
    src/winres/winres_utils.cpp     # rcCtrl class utility functions

    pugixml/pugixml.cpp             # XML parser

    src/ui/embedimg.cpp             # Convert image to Header (.h) or XPM (.xpm) file
    src/ui/eventhandlerdlg.cpp      # Dialog for editing event handlers
    src/ui/gridbag_item.cpp         # Dialog for inserting an item into a wxGridBagSizer node
    src/ui/import_dlg.cpp           # Dialog to import one or more projects
    src/ui/importwinresdlg.cpp      # Dialog for Importing a Windows resource file
    src/ui/insertwidget.cpp         # Dialog to lookup and insert a widget
    src/ui/newdialog.cpp            # Dialog for creating a new project dialog
    src/ui/newframe.cpp             # Dialog for creating a new project wxFrame
    src/ui/newribbon.cpp            # Dialog for creating a new wxRibbonBar
    src/ui/optionsdlg.cpp           # Dialog containing special Debugging commands

    ${wxue_generated_code}

    # ttLib submodule files (see https://github.com/KeyWorksRW/ttLib repository)

    ttLib/src/ttcstr.cpp             # Class for handling zero-terminated char strings.
    ttLib/src/ttcview.cpp            # string_view functionality on a zero-terminated char string.
    ttLib/src/ttsview.cpp            # std::string_view with additional methods
    ttLib/src/ttmultistr.cpp         # Breaks a single string into multiple strings
    ttLib/src/ttparser.cpp           # Command line parser
    ttLib/src/ttstr.cpp              # Enhanced version of wxString
    ttLib/src/ttlibspace.cpp         # ttlib namespace functions
    ttLib/src/ttstrings.cpp          # Class for handling zero-terminated char strings.
    ttLib/src/tttextfile.cpp         # Classes for reading and writing text files.

    # Debug-only files

    $<$<CONFIG:Debug>:src/debugging/debugsettings.cpp>  # Settings while running the Debug version of wxUiEditor
    $<$<CONFIG:Debug>:src/debugging/dbg_code_diff.cpp>  # Compare code generationg
    $<$<CONFIG:Debug>:src/debugging/msg_logging.cpp>    # Message logging class
    $<$<CONFIG:Debug>:src/debugging/msgframe.cpp>       # Stores messages
    $<$<CONFIG:Debug>:src/debugging/nodeinfo.cpp>       # Node memory usage dialog

    $<$<CONFIG:Debug>:src/debugging/msgframe_base.cpp>  # wxUiEditor generated file
    $<$<CONFIG:Debug>:src/debugging/nodeinfo_base.cpp>  # wxUiEditor generated file
    $<$<CONFIG:Debug>:src/debugging/dbg_code_diff_base.cpp>
    $<$<CONFIG:Debug>:src/debugging/debugsettingsBase.cpp>  # wxUiEditor generated file

    $<$<CONFIG:Debug>:ttLib/src/winsrc/ttdebug_min.cpp>  # ttAssertionMsg
)

if (MSVC)
    # /GL -- combined with the Linker flag /LTCG to perform whole program optimization in Release build
    # /FC -- Full path to source code file in diagnostics
    target_compile_options(wxUiEditor PRIVATE "$<$<CONFIG:Release>:/GL>" "/FC" "/W4" "/Zc:__cplusplus" "/utf-8")
    target_link_options(wxUiEditor PRIVATE "$<$<CONFIG:Release>:/LTCG>")

    target_link_options(wxUiEditor PRIVATE "$<$<CONFIG:Debug>:/natvis:../src/wxui.natvis>")

    # Assume the manifest is in the resource file
    target_link_options(wxUiEditor PRIVATE "/manifest:no")
endif()

target_precompile_headers(wxUiEditor PRIVATE "src/pch.h")

target_include_directories(wxUiEditor PRIVATE
    src/
    src/nodes
    src/generate
    src/utils
    src/customprops
    src/ui
    ttLib/include
)

# Packaging Instructions

set(CPACK_PACKAGE_VENDOR "KeyWorks Software")
set(CPACK_VERBATIM_VARIABLES YES)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/EULA.TXT)
set(CPACK_PACKAGE_INSTALL_DIRECTORY wxUiEditor)

set(CPACK_WIX_UPGRADE_GUID 67974CB4-5812-40A7-8031-8FA861ED303E)
set(CPACK_WIX_PRODUCT_ICON ${CMAKE_CURRENT_LIST_DIR}/src/wxUiEditor.ico)

if (WIN32)
    set(CPACK_GENERATOR ZIP WIX)
else()
    set(CPACK_GENERATOR TGZ)
endif()

include(GNUInstallDirs)
include(CPack)

install(TARGETS wxUiEditor
    CONFIGURATIONS Release
)

set_property(INSTALL "bin/$<TARGET_FILE_NAME:wxUiEditor>"
  PROPERTY CPACK_START_MENU_SHORTCUTS "wxUiEditor"
)
