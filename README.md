# ![logo](src/art_src/logo64.png) wxUiEditor

This Windows GUI tool is used to create and maintain [wxWidgets](https://docs.wxwidgets.org/trunk/index.html) UI elements (dialogs, menus, etc.), generating C++ output code. Minimal requirement for compiling output files is a c++11 compliant compiler and **wxWidgets** 3.0 (version 3.1 needed for full functionality).

In addition to creating new projects, the following project types can be imported:

- **wxFormBuilder** (Click [here](docs/import_formbuilder.md) for more information)
- **wxGlade**
- **wxSmith**
- **XRC**

## Building

In order to build the project, you must have version 1.7.2 or newer of [ttBld.exe](https://github.com/KeyWorksRW/ttBld) either in your path or in the `src/` directory. If you don't already have it, you can copy the version in `ttBld/win`.

In addition you will need to have the following in your $PATH:

- [Ninja.exe](https://github.com/ninja-build/ninja)

- Either of the following compilers (_must_ be able to generate c++17 code):
  - [CLANG](https://clang.llvm.org/) (version 9 or higher)
  - MSVC
- Either `nmake.exe` (Microsoft) or `mingw32-make.exe`
- wxWidgets headers and libraries version 3.1.5 or higher. Debug builds expect a dll version in your `LIB` environment paths, release builds expect a static library version.

If you are building on Windows, you will need to start one of the Visual Studio command shells, preferably one that builds 64-bit targets. From within that shell, you can use the following commands to build **wxUiEditor.exe**:

	cd src
	nmake

See [Developer notes](docs/DEV_NOTES.md) for more information about the code.

## Licenses

All KeyWorks Software code uses the Apache License 2.0 [LICENSE](LICENSE).

The pugixml code uses the MIT [LICENSE](pugixml/LICENSE.md).
