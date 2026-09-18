# Everything in this file is designed for internal testing by the maintainers.
# A lot of it is specifically designed to run on a maintainer's machine, and
# will almost certainly not work for anyone else. It's in this separate file
# in order to make the normal build process in CMakeLists.txt easier to read.

if(INTERNAL_BLD_TESTING)
    message(NOTICE "Building internal testing version")

    include(src/wxui/wxui_internal.cmake) # These are the internal generated files

    # In addition to enabling testing-only functionality, this enables ASSERT(), ASSERT_MSG(),
    # and FAIL_MSG() in a Release build.
    add_compile_definitions(INTERNAL_TESTING)

    # REVIEW: [Randalphwa - 09-18-2026] These need to be updated, and perhaps hooked into a testing framework
    # include( src/verify/verify.cmake ) # This will set ${verify_files} with a list of source files

    set(wxui_internal_files
        src/internal/msg_logging.cpp
        src/internal/msgframe.cpp
        src/internal/import_panel.cpp
        src/internal/convert_img.cpp
        src/internal/debugsettings.cpp
        src/internal/node_info.cpp             # Node memory usage dialog
        src/internal/xrcpreview_handlers.cpp   # Handlers for XrcPreview Dialog class
        src/internal/node_search_dlg.cpp
        src/internal/msgframe.cpp              # Stores messages
        src/internal/unused_gen_dlg.cpp
        src/internal/undo_info.cpp
        src/internal/viewdetails_dlg.cpp
        src/internal/xrc_list_dlg.cpp

        ${wxue_internal_generated_code}
        ${verify_files}
    )
endif()
