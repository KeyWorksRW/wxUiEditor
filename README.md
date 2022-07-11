# ![logo](src/art_src/logo64.png) wxUiEditor

This WYSIWYG tool is used to create and maintain [wxWidgets](https://docs.wxwidgets.org/trunk/index.html) UI elements (dialogs, menus, etc.), generating either C++ or XRC code. Minimal requirement for compiling C++ output files is a C++11 compliant compiler and **wxWidgets** 3.0 (version 3.2 needed for full functionality).

In addition to creating new projects, the following project types can be imported:

- **wxCrafter** (Click [here](docs/import_crafter.md) for more information)
- **wxFormBuilder** (Click [here](docs/import_formbuilder.md) for more information)
- **wxGlade**
- **wxSmith**
- **XRC** (including exports from **DialogBlocks**)
- **Windows Resource Dialogs** (Click [here](docs/import_winres.md) for more information)

**wxUiEditor** is a 64-bit app that runs natively on Windows 10 or later, and with **Wine** on Unix systems.

## Status

[![CodeQL](https://github.com/KeyWorksRW/wxUiEditor/workflows/CodeQL/badge.svg)](https://github.com/KeyWorksRW/wxUiEditor/actions?query=workflow:"CodeQL") [![GitHub tag](https://img.shields.io/github/tag/KeyWorksRW/wxUiEditor?include_prereleases=&sort=semver&color=blue)](https://github.com/KeyWorksRW/wxUiEditor/releases/)

## Building

You will need a C++20 compliant compiler to build **wxUiEditor**. Currently only Windows builds are supported. To build, run the following two commands from the root of the repository:

```
    cmake -G "Ninja Multi-Config" . -B build
    cmake --build build --config Release
```

Note that the linking stage of the Release build will take quite a bit of time.

See [Developer notes](docs/DEV_NOTES.md) for more information about the current branches, and some high level overview of the code.

## Licenses

All KeyWorks Software code uses the Apache License 2.0 [LICENSE](LICENSE).

The pugixml code uses a MIT [LICENSE](pugixml/LICENSE.md). The rapidjson code also uses a MIT [LICENSE](src/import/rapidjson/license.txt).

## Screen shots

The following is a screen shot of **wxUiEditor**:

![image](screenshot.jpg)
