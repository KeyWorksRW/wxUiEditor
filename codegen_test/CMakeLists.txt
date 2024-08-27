cmake_minimum_required(VERSION 3.20)

project(codegen_test
    LANGUAGES CXX
    VERSION 1.0.0.0
    DESCRIPTION "wxUiEditor"
    HOMEPAGE_URL "https://github.com/KeyWorksRW/wxUiEditor")

####################### Check for Multi-Config Generator #######################

get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
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
