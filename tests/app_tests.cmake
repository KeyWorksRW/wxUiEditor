####################### sdi_test #######################

include( ${CMAKE_CURRENT_LIST_DIR}/sdi/cpp/wxui_code.cmake )  # This will set ${wxue_generated_code} with a list of generated files
include( ${CMAKE_CURRENT_LIST_DIR}/sdi/cpp/file_list.cmake )  # This will set ${file_list} with a list of files
list(TRANSFORM sdi_file_list PREPEND "${sdi_test_dir}/")

add_executable(sdi_test WIN32 EXCLUDE_FROM_ALL
    ${sdi_file_list}
    ${sdi_generated_code}
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(sdi_test PRIVATE WXUSINGDLL)
endif()

if (WIN32)
    target_link_libraries(sdi_test PRIVATE wxWidgets ${CLib} comctl32 Imm32 Shlwapi Version UxTheme)
else()
    target_link_libraries(sdi_test PRIVATE wxWidgets ${CLib})
endif()

if (MSVC)
    # /GL -- combined with the Linker flag /LTCG to perform whole program optimization in Release build
    # /FC -- Full path to source code file in diagnostics
    target_compile_options(sdi_test PRIVATE "$<$<CONFIG:Release>:/GL>" "/FC" "/W4" "/Zc:__cplusplus" "/utf-8")
    target_link_options(sdi_test PRIVATE "$<$<CONFIG:Release>:/LTCG>")

    target_link_options(sdi_test PRIVATE "$<$<CONFIG:Debug>:/natvis:${CMAKE_CURRENT_LIST_DIR}/../src/wxui.natvis>")

    # Assume the manifest is in the resource file
    target_link_options(sdi_test PRIVATE "/manifest:no")
endif()

target_precompile_headers(sdi_test PRIVATE "${CMAKE_CURRENT_LIST_DIR}/sdi/cpp/pch.h")

target_include_directories(sdi_test PRIVATE
    ${setup_dir}
    ${BUILD_ROOT}/include
    ${sdi_test_dir}/
)

####################### mdi_test #######################
