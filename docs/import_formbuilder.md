# Importing wxFormBUilder projects

**wxUiEditor** can import **wxFormBuilder** project files. Because **wxUiEditor** generates source code for each top level form, you can import multiple **wxFormBuilder** into a single **wxUiEditor** file. Note that you may need to make some changes after a project is imported due to the differences in the way forms and widgets are handled.

**wxUiEditor** projects are roughly 90% smaller than a **wxFormBuilder** project. Only data needed to construct your UI gets saved.

## Images

When you import a **wxFormBuilder** project, **wxUiEditor** will `#include` XPM files in the generated source code. All other external images are first converted into a compressed PNG header file and a `#include` will be added to the generated source code. If you set the project's `converted_art` property before importing, then the PNG header files will be placed in this location. Otherwise, they will be created in the same directory as the **wxUiEditor** project file. Note that you can re-generate these files at any time using the "Create Embedded Image" command under the **Tools** menu. This command also allows you to convert to/from XPM and PNG header files.

**wxUiEditor** does not support generating code that loads images from a Windows resource file (since this is non-portable). You can either set the image in your derived code, or use the "Create Embedded Image" command under the **Tools** menu to convert it into an XPM or PNG header file.

## Code Generation

The code that **wxUiEditor** generates will often look quite different than the code that **wxFormBuilder** generates. Your UI will usually work the same in spite of the differences in the code. However, there are some differences you should be aware of:

- **wxUiEditor** will connect to events using **Bind()** rather than **Connect()**
- **wxUiEditor** will not disconnect events in a destructor since **wxWidgets** already does this

If you import a **wxFormBuilder** project, **wxUiEditor** will default to generating the same filename that was generated before. If you would like to compare the differences, then in **wxUiEditor** change the default source and header extensions to `.cc/.hh` or `.cxx/.hxx` so the the new code won't orerwrite the old code that **wxFormBuilder** generated.
