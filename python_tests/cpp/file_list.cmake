set (file_list_dir ${CMAKE_CURRENT_LIST_DIR})

set (file_list

    cpptest.rc
    mainapp.cpp

    custom_ctrls/split_button.cpp  # Split Button Custom widget (button + menu)

)

# Manually maintained from wxui_code.cmake
set(wxui_file_list

    booktest_dlg.cpp
    commonctrls.cpp
    form_toolbar_base.cpp
    images.cpp
    mainframe.cpp
    maintestdialog.cpp
    popupwin.cpp
    python_dlg.cpp
    wizard.cpp

)

# Currently, these are not used in CMakeLists.txt, it's just a reference to which python files
# are being generated.

set (python_files

    ../python/images.py
    ../python/mainframe.py
    ../python/main_test_dlg.py
    ../python/popwin.py
    ../python/python_dlg.py
    ../python/wizard.py
    ../python/dlgissue_956.py

)

# These aren't actually used in CMAkeLists.txt, they are here just to provide a quick way to
# load them into an IDE that supports loading a file from the filenmae underneath the cursor.

set (other_files

    ../COVERAGE.md
    ../README.md

    ../pytest.py

    ../run_pytest.cmd
    ../run_cpp_build.cmd

)
