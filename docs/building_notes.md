## Version change

Updating the version of wxUiEditor means changing the version number in the following files:

- pch.h
- wxUiEditor.rc
- wxUiEditor.exe.manifest
- CMakeLists.txt (change VERSION at the top of the file)
- CHANGELOG.md

If new controls, properties, or other changes have been made that change the project file in a way that previous versions will not understand, then the project number needs to be increated -- `curSupportedVer` in pch.h.
