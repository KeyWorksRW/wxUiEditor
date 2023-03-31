About
-----

wxWidgets is a free and open source cross-platform C++ framework
for writing advanced GUI applications using native controls.

![wxWidgets Logo](https://www.wxwidgets.org/assets/img/header-logo.png)

wxWidgets allows you to write native-looking GUI applications for
all the major desktop platforms and also helps with abstracting
the differences in the non-GUI aspects between them. It is free
for the use in both open source and commercial applications, comes
with the full, easy to read and modify, source and extensive
documentation and a collection of more than a hundred examples.
You can learn more about wxWidgets at https://www.wxwidgets.org/
and read its documentation online at https://docs.wxwidgets.org/

Licence
-------

[wxWidgets licence](https://github.com/wxWidgets/wxWidgets/blob/master/docs/licence.txt)
is a modified version of LGPL explicitly allowing not distributing the sources
of an application using the library even in the case of static linking.

Directory
-------

This directory contains a copy of the source code and header files necessary to build the wxWidgets libaries. This is _not_ the full directory tree of the official wxWidgets repository. Only the files needed to build **wxUiEditor** are included here.

There are several sub-folders containing custom `CMakeLists.txt`, `wxWidgets.cmake` and `setup.h` files used to build either from this folder, or from clones or forks of official wxWidgets repositories. All of these files are designed to allow building using CMake from the root of the wxUiEditor directory.
