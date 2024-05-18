# Internal functionality

There are two types of internal functionality. One is enabled by specifying `-test_menu` on the command line that launches wxUiEditor. This will add a Testing menu to the right of the Help menu. This allows you to run some tests even on a normal release build of wxUiEditor.

Additional testing functionality is only available if you are building wxUiEditor. All testing functionality is available in Debug builds. You can also add testing functionality to release builds by setting INTERNAL_BLD_TESTING to ON in CMake. The additional internal testing shows up on the Internal menu, to the right of the Testing menu mentioned in the above paragraph.

## Compare Code Generation

This testing command only works on Windows, and only if you have [WinMerge](https://github.com/WinMerge/winmerge) installed and in your $PATH. This will first display a dialog listing any files that would be changed if regenerated (this part doesn't require WinMerge). If there are any files that would change, the WinMerge button will be enabled.
