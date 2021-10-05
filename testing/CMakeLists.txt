cmake_minimum_required(VERSION 3.20)

project(wxUiTesting LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
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

include(src/ui/wxui_code.cmake)  # This will set ${wxue_generated_code} with list of generated files

add_executable(wxUiTesting WIN32
    src/wxUiTesting.rc
    src/mainapp.cpp

    src/ui/mainframe.cpp       # Main window
    src/ui/mainframe_base.cpp  # wxUiEditor generated file

    src/ui/commonctrls.cpp
    src/ui/dlgmultitest.cpp
    src/ui/other_ctrls.cpp
    src/ui/popupwin.cpp
    src/ui/ribbondlg.cpp
    src/ui/wizard.cpp

    ${wxue_generated_code}

)

if (MSVC)
    # /GL -- combined with the Linker flag /LTCG to perform whole program optimization in Release build
    # /FC -- Full path to source code file in diagnostics
    target_compile_options(wxUiTesting PRIVATE "$<$<CONFIG:Release>:/GL>" "/FC" "/W4" "/Zc:__cplusplus" "/utf-8")
    target_link_options(wxUiTesting PRIVATE "$<$<CONFIG:Release>:/LTCG>")

    target_link_options(wxUiTesting PRIVATE "$<$<CONFIG:Debug>:/natvis:../../src/wxui.natvis>")

    # Assume the manifest is in the resource file
    target_link_options(wxUiTesting PRIVATE "/manifest:no")
endif()

target_precompile_headers(wxUiTesting PRIVATE "src/pch.h")

target_include_directories(wxUiTesting PRIVATE
    src/
    src/ui
)
