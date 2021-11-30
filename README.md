# ![logo](src/art_src/logo64.png) wxUiEditor

This Windows GUI tool is used to create and maintain [wxWidgets](https://docs.wxwidgets.org/trunk/index.html) UI elements (dialogs, menus, etc.), generating C++ output code. Minimal requirement for compiling output files is a C++11 compliant compiler and **wxWidgets** 3.0 (version 3.1 needed for full functionality).

In addition to creating new projects, the following project types can be imported:

- **wxFormBuilder** (Click [here](docs/import_formbuilder.md) for more information)
- **wxGlade**
- **wxSmith**
- **XRC** (including exports from **DialogBlocks**)
- **Windows Resource Dialogs** (Click [here](docs/import_winres.md) for more information)

## Building

To build the Windows version of **wxUiEditor**, run the following two commands from the root of the repository:

```
    cmake -G "Ninja Multi-Config" . -B build
    cmake.exe --build build --config Release
```

Note that the linking stage of the Release build will take quite a bit of time.

See [Developer notes](docs/DEV_NOTES.md) for more information about the code.

## Licenses

All KeyWorks Software code uses the Apache License 2.0 [LICENSE](LICENSE).

The pugixml code uses the MIT [LICENSE](pugixml/LICENSE.md).

## Screen shots

The following is a screen shot of **wxUiEditor** being used to edit one of it's own dialogs:

![image](screenshot.jpg)

The following is a screen shot of the code that wxUiEditor would generate for the same dialog:

![image](code_screenshot.jpg)
