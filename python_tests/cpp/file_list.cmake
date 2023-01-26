set (file_list_dir ${CMAKE_CURRENT_LIST_DIR})

set (file_list

    cpptest.rc
    mainapp.cpp

    custom_ctrls/split_button.cpp  # Split Button Custom widget (button + menu)

)

# Manually maintained from wxui_generated.cmake
set(wxui_file_list

    choicebook.cpp
    commonctrls.cpp
    dlgmultitest.cpp
    form_toolbar_base.cpp
    import_test.cpp
    listbook.cpp
    mainframe.cpp
    my_images.cpp
    notebook.cpp
    other_ctrls.cpp
    popupwin.cpp
    python_dlg.cpp
    ribbondlg.cpp
    toolbook.cpp
    treebook.cpp
    wizard.cpp

)

set (python_files

    ../python/mainframe.py
    ../python/main_test_dlg.py
    ../python/python_dlg.py

)
