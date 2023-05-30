These are notes for building a release version that can be uploaded to github for a beta or final release.

## Version change

Updating the version of wxUiEditor means changing the version number in the following files:

- pch.h
- wxUiEditor.rc
- wxUiEditor.exe.manifest
- CMakeLists.txt (change VERSION at the top of the file)
- build/CpackConfig.cmake (CPACK_PACKAGE_VERSION, CPACK_PACKAGE_VERSION_PATCH and CPACK_PACKAGE_FILE_NAME)

If new controls, properties, or other changes have been made that change the project file in a way that previous versions will not understand, then the project number needs to be increated -- `curSupportedVer` in pch.h.

## CMake changes

If you use cgui to manage CMake options, then in the `BUILD` category, uncheck `BUILD_FORK` and `BUILD_SHARED_LIBS`. All options in the `INTERRNAL` category need to be unchecked.

## CPack

To build the installe program, change to the build/ directory and run:

```
    cpack -G NSIS
```

This will create the installer executable in the build directory. The filename will be created using `wxUiEditor-` + version + `-win64.exe` (e.g. wxUiEditor-1.1.0-win64.exe).
