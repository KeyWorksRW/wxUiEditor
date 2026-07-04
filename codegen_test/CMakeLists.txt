cmake_minimum_required(VERSION 4.2...4.3)

project(codegen_test
    LANGUAGES CXX
    VERSION 1.0.0.0
    DESCRIPTION "wxUiEditor"
    HOMEPAGE_URL "https://github.com/KeyWorksRW/wxUiEditor")

####################### Check for Multi-Config Generator #######################

get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (NOT IS_MULTI_CONFIG)
    message("\nSingle-config generator (${CMAKE_GENERATOR}).")
    message("    CMAKE_BUILD_TYPE defaulting to Debug. Override with -DCMAKE_BUILD_TYPE=Release.\n")

    set(allowedBuildTypes Debug Release)

    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
    elseif (NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
        message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
    endif()

    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")
else()
    message("\nMulti-config generator (${CMAKE_GENERATOR}).")
    message("    Use \"--config [Debug|Release]\" with the cmake --build command.\n")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

set (widget_dir ${CMAKE_CURRENT_LIST_DIR}/../wxWidgets)
set (setup_dir ${CMAKE_CURRENT_LIST_DIR}/../wxWidgets/win/)

####################### Libraries and Executables #######################

include( generated/codegen.cmake )  # This will set ${wxue_generated_code} with list of generated files

add_library(check_build STATIC EXCLUDE_FROM_ALL
    ${wxue_generated_code}
)

target_precompile_headers(check_build PRIVATE "src/pch.h")

target_include_directories(check_build PRIVATE
    ${setup_dir}
    ${widget_dir}/include
)
