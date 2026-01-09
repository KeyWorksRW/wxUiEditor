# ![logo](src/art_src/wxUiEditor.svg) wxUiEditor

This WYSIWYG tool is used to create and maintain [wxWidgets](https://docs.wxwidgets.org/trunk/index.html) UI elements (dialogs, menus, etc.), generating either C++, [wxPython](https://wxpython.org/), [wxRuby3](https://github.com/mcorino/wxRuby3) or XRC code. It supports almost all of the wxWidgets UI controls along with bitmap bundles and SVG files.

In addition to creating new projects, the following project types can be imported:

- **wxCrafter** (Click [here](docs/users/import_crafter.md) for more information)
- **wxFormBuilder** (Click [here](docs/users/import_formbuilder.md) for more information)
- **wxGlade**
- **wxSmith**
- **DialogBlocks** (Click [here](docs/users/import_dialogblocks.md) for more information)
- **XRC** (including exports from **DialogBlocks**)
- **Windows Resource Dialogs** (Click [here](docs/users/import_winres.md) for more information)
## Installing

For Windows, you can either run the wxUiEditor-_ver_-win64.exe installer from [releases](https://github.com/KeyWorksRW/wxUiEditor/releases) or run `winget install wxUiEditor` from a command line. For Linux, wxUiEditor-_ver_-Linux.deb and wxUiEditor-_ver_-Linux.rpm are available in the [releases](https://github.com/KeyWorksRW/wxUiEditor/releases) section.

## Status

[![CodeQL](https://github.com/KeyWorksRW/wxUiEditor/workflows/CodeQL/badge.svg)](https://github.com/KeyWorksRW/wxUiEditor/actions?query=workflow:"CodeQL") [![GitHub tag](https://img.shields.io/github/tag/KeyWorksRW/wxUiEditor?include_prereleases=&sort=semver&color=blue)](https://github.com/KeyWorksRW/wxUiEditor/releases/)
![GitHub Downloads (all assets, all releases)](https://img.shields.io/github/downloads/KeyWorksRW/wxUiEditor/total)

## Building

You will need a C++23 compliant compiler to build **wxUiEditor**. To build using CMake 3.20 or higher, run the following two commands from the root of the repository:

```
    cmake -G "Ninja Multi-Config" . -B build
    cmake --build build --config Release --target wxUiEditor
```

For Unix builds, you currently need a minimum of gcc 15.1 or clang 20 to be able to compile the sources.

## Licenses

- All KeyWorks Software code uses the Apache License 2.0 [LICENSE](LICENSE).
- The frozen code uses the Apache License 2.0 [LICENSE](frozen/LICENSE).
- The pugixml code uses the MIT [LICENSE](pugixml/LICENSE.md).
- The rapidjson code uses the MIT [LICENSE](src/import/rapidjson/license.txt).
- The wxWidgets code uses the wxWindows [LICENSE](wxWidgets/License.txt).

## Screen shots

The following is a sample screen shot of **wxUiEditor** showing the Event handler dialog and an example of one of the drop-down lists of controls:

![image](screenshot.jpg)
