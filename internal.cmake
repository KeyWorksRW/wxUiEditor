# Everything in this file is designed for internal testing by the maintainers.
# A lot of it is specifically designed to run on a maintainer's machine, and
# will almost certainly not work for anyone else. It's in this separate file
# in order to make the normal build process in CMakeLists.txt easier to read.

if (INTERNAL_BLD_FORK)
    message(NOTICE "Building with forked wxWidgets libraries")

    add_compile_definitions(INTERNAL_BLD_FORK)

    set (widget_dir ../wxWidgets)
    set (widget_cmake_dir ../wxWidgets/bld)

    # Note that for INTERNAL_BLD_FORK with INTERNAL_BLD_WX_CMAKE to work, you must build the wxWidgets libraries yourself
    # (add_subdirectory is not used in this case). You will need to set the cmake configuration for wxWidgets to build mondo
    # static (including static runtime). Do *not* use this for an official release build as you cannot optimize or profile
    # the wxWidgets library when using their cmake build.

    # Alternaticely, you can use the following -- but that means creating the CMakeLists.txt file yourself (you can use the
    # version in wxSnapshot as a template).

    if (NOT INTERNAL_BLD_WX_CMAKE)
        # This will build wxCLib and wxWidgets. wxCLib can NOT be used in a UNIX build
        add_subdirectory(../wxWidgets/bld ../wxWidgets/bld/build)
    endif()

    # Using MSVC, this will build in lib/vc_x64_lib:
    #     wxmsw33ud wxexpatd wxjpegd wxpngd wxregexud wxscintillad wxtiffd wxzlibd
    # add_subdirectory(../wxWidgets /../wxWidgets/bld)

    if (MSVC)
        # Both debug and release libraries are in this directory
        if (INTERNAL_BLD_WX_CMAKE)
            set (widget_lib_dir ../wxWidgets/bld/lib/vc_x64_lib)
        else()
            set (widget_lib_dir ../wxWidgets/bld/build)
        endif()
    else()
        set (widget_lib_dir ../wxWidgets/bld/lib)
    endif()
endif()

if (WIN32)
    if (INTERNAL_BLD_FORK)
        if (INTERNAL_BLD_WX_CMAKE)
            set(setup_dir ${widget_cmake_dir}/lib/vc_x64_lib/mswud)
        else()
            set(setup_dir ${widget_cmake_dir}/win)
        endif()
    endif()
else()
    if (INTERNAL_BLD_FORK)
        set(setup_dir ${widget_cmake_dir}/lib/wx/include/gtk3-unicode-static-3.3)
    endif()
endif()

if (INTERNAL_BLD_TESTING)
    include( src/internal/wxui_internal.cmake )  # This will set ${wxui_internal} with a list of source files

    # Note that setting the INTERNAL_BLD_TESTING doesn't just add these modules, it also enables the three assertion macros
    # in a Release build, and may change other functionality as well. It should NEVER be used for a production build!

    set (wxui_internal ${wxui_internal}
        # These are always included in Debug builds, with INTERNAL_BLD_TESTING we also need them
        # in Release builds.
        $<$<CONFIG:Release>:src/internal/code_compare.cpp>
        $<$<CONFIG:Release>:src/internal/convert_img.cpp>
        $<$<CONFIG:Release>:src/internal/import_panel.cpp>
        $<$<CONFIG:Release>:src/internal/node_info.cpp>
        $<$<CONFIG:Release>:src/internal/xrcpreview.cpp>

        $<$<CONFIG:Release>:src/internal/msg_logging.cpp>
        $<$<CONFIG:Release>:src/internal/msgframe.cpp>
        $<$<CONFIG:Release>:src/internal/debugsettings.cpp>
    )

    if (WIN32)
        if (INTERNAL_BLD_WX_CMAKE)
            # Both Debug and Release libaries are in the same location
            set(fork_wxlibraries
                $<$<CONFIG:Debug>:wxmsw33ud>
                $<$<CONFIG:Debug>:wxexpatd>
                $<$<CONFIG:Debug>:wxjpegd>
                $<$<CONFIG:Debug>:wxpngd>
                $<$<CONFIG:Debug>:wxregexud>
                $<$<CONFIG:Debug>:wxscintillad>
                $<$<CONFIG:Debug>:wxtiffd>
                $<$<CONFIG:Debug>:wxzlibd>

                $<$<CONFIG:Release>:wxmsw33u>
                $<$<CONFIG:Release>:wxexpat>
                $<$<CONFIG:Release>:wxjpeg>
                $<$<CONFIG:Release>:wxpng>
                $<$<CONFIG:Release>:wxregexu>
                $<$<CONFIG:Release>:wxscintilla>
                $<$<CONFIG:Release>:wxtiff>
                $<$<CONFIG:Release>:wxzlib>

                comctl32 Imm32 Shlwapi Version UxTheme
            )
        endif()
    endif()
endif()
