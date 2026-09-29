# Internal functionality

There are two types of internal functionality.
One is enabled by specifying `-test_menu` on the command line that launches wxUiEditor.
This will add a Testing menu to the right of the Help menu.
This allows you to run some tests even on a normal release build of wxUiEditor.

Additional testing functionality is only available if you are building wxUiEditor.
All testing functionality is available in Debug builds.
You can also add testing functionality to release builds by setting INTERNAL_BLD_TESTING to ON in CMake.
The additional internal testing shows up on the Internal menu, to the right of the Testing menu mentioned in the above
paragraph.

## Compare Code Generation

Select the Project, a Folder, or a Form in the Navigation panel and choose **Tools → Compare Code Generation...** (it is
also available from the Navigation panel’s context menu).
The dialog lists the classes whose generated code would change if regenerated, for the selected language (C++, Python,
Ruby, Fortran, GO, Julia, LuaJIT, TypeScript, or XRC).

If there are any differences, the **View Diffs...** button is enabled.
Clicking it opens wxUiEditor’s own built-in diff viewer, which shows the on-disk and newly generated versions of each
file side-by-side with syntax highlighting and Previous/Next navigation.

This command is cross-platform (Windows, Linux, and macOS) and does not require any external diff tool such as WinMerge.
(The button’s icon is still the WinMerge art, but WinMerge itself is no longer needed.)
