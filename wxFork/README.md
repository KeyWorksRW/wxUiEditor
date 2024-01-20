This folder assumes a wxWidgets repository at the same level as wxUiEditor, which is presumably a fork of wxWidgets. By default, all source and header files will be build from the ../../wxWidgets directory.

wxCLib.cmake and wxWidgets.cmake contain the list of sources. Note that these lists are _not_ a part of wxWidgets itself, so care must be taken to ensure that the list of files matches the list of files in the wxWidgets repository. This _will_ be an issue whenever a new file is added to wxWidgets, since until it gets added to one of these lists it won't be built.

Overriding a wxWidgets file is fairly straightforward. All you need to do is create the file somewhere within this directory and then replace or add the file to the appropriate list in wxCLib.cmake or wxWidgets.cmake. The file will then be built instead of the one in the wxWidgets repository.

There is no License file in this directory because there is normally no origianal wxWidgets code here, unless it's a copy. That means folders within this directory could either be under the wxWidgets license or some other license -- check the directory itself for a license, and assume it is under the wxWidgets license if there is no license file.
