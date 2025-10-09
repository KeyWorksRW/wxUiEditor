# CMakeLists.txt for ttwx tests
# This file defines the test executable for ttwx namespace functions

# Test executable for ttwx functions
add_executable( test_ttwx
    test_ttwx.cpp
    ../ttwx.cpp
)

target_compile_features( test_ttwx PRIVATE cxx_std_20 )

if( MSVC )
    target_compile_options( test_ttwx PRIVATE "/W4" "/Zc:__cplusplus" "/utf-8" )
elseif( UNIX )
    target_compile_definitions( test_ttwx PRIVATE __WXGTK__ )
endif()

target_include_directories( test_ttwx PRIVATE
    ${wxWidgets_SOURCE_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/../
    ${CMAKE_CURRENT_LIST_DIR}
)

if( WIN32 )
    target_include_directories( test_ttwx PRIVATE
        ${wxWidgets_BUILD_DIR}/lib/vc_x64_lib/mswu
    )
elseif( APPLE )
    target_include_directories( test_ttwx PRIVATE
        ${wxWidgets_BUILD_DIR}/lib/osx_cocoa-unicode-static
    )
elseif( UNIX )
    target_include_directories( test_ttwx PRIVATE
        ${wxWidgets_BUILD_DIR}/lib/gtk3_unixu-3.3/include
    )
endif()

if( BUILD_SHARED_LIBS )
    target_compile_definitions( test_ttwx PRIVATE WXUSINGDLL )
endif()

# Register the test_ttwx executable as a test
add_test(NAME test_ttwx COMMAND $<TARGET_FILE:test_ttwx>)

target_link_libraries( test_ttwx PRIVATE
    Catch2::Catch2WithMain
    ${wxWidgets_LIBRARIES}
    ${SYSTEM_LIBS}
)

# Register the test with CTest
include( ${Catch2_SOURCE_DIR}/extras/Catch.cmake )
catch_discover_tests( test_ttwx )
