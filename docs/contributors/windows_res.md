# Converting a Windows Resource

Windows Dialogs use absolute positions and size which we have to convert into flexible-sized controls that are contained within sizers. Complicating this is that the resource controls may not be exactly aligned and they can appear in any order. While most resource files will be created by Visual Studio, then can also be created by tools like **RisohEditor** or even created by hand. That means there is a _lot_ of variation in what resource files look like.

The goal is not to get a perfect rendition of the original dialog, but to at least get all of the controls converted so that the user can make adjustments to get the dialog to look closer to the original Windows version. The closer we can get, the easier it will be for the user, but that will often result in special-case handling.

## Group boxes

Windows supports having the label for a group box in the center or right side, but unfortunately wxWidgets only supports that on wxGTK (as of 3.1.15).

## Testing

Two repositories are used as the main tests for conversion primarily because they have a lot of dialogs and both are mostly hand-edited:

### WinFile

[https://github.com/Microsoft/winfile]

`src/res.rc` pulls in all dialogs, including all language versions which will have the same name, but different text. Use this top open in Visual Studio to see what the original dialogs look like. Do _not_ import this resource into **wxUE** as you'll end up with multiple identically named dialogs.

`src/lang/` contains `res_*.rc` for the different languages. Visual Studio can't read these, but you can import them into **wxUE** -- or just import the matching `*.dlg` file.

### NotePadPlusPlus

[https://github.com/notepad-plus-plus/notepad-plus-plus]

`PowerEditor/gcc/resources.rc` pulls in most of the dialogs and gives the most coverage. **wxUE** can import this, but Visual Studio cannot, so to compare you'll have to open the individual resource files in Vistual Studio.
