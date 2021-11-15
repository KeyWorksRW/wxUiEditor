The files in this directory are used to test importing from Windows Resource Dialogs and Menus into a **wxUiEditor** project. There are two resource files and the root, and there may be others in sub-directories.

### main_test.rc

This resource file contains everything necessary to load it into Visual Studio. This allows you to compare the appearance of the original dialogs with the **wxUiEditor** imported version.

### regression.rc

This resource file can _not_ be loaded into Visual Studio as it is missing all the headers that VS needs. Each Dialog and Menu in this file needed to have special processing added before they displayed correctly. Putting them here makes it easy to quickly confirm that they still work correctly when other code processing changes are made. However, since you can't see what the original looks like, this will rely on you knowing what it used to look like before verifying that a new wxUiEditor build doesn't change it (you can compare Release and Debug builds if needed).

That caption for each dialog has been modified to indicate what problem was being addressed.
