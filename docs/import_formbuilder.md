# Importing wxFormBUilder projects

**wxUiEditor** can import most **wxFormBuilder** project files. Because **wxUiEditor** generates source code for each top level form, you can import multiple **wxFormBuilder** files into a single **wxUiEditor** file. You may need to make some changes after a project is imported due to the differences in the way forms and widgets are handled.

Note that **wxUiEditor** projects are roughly 90% smaller than a **wxFormBuilder** project due to optimization in what gets written to the project file.

## Images

When you import a **wxFormBuilder** project, **wxUiEditor** will `#include` XPM files in the generated source code. All other external images are first converted into a C++ array and generated in the source file of the first form in your project that uses it. All further references to the same image will use a `extern` reference.

## Code Generation

The code that **wxUiEditor** generates will often look quite different than the code that **wxFormBuilder** generates. Your UI will usually work the same in spite of the differences in the code. However, there are some differences you should be aware of:

- **wxUiEditor** will connect to events using **Bind()** rather than **Connect()**
- **wxUiEditor** will not disconnect events in a destructor since **wxWidgets** already does this

If you inserted a static line before a **wxStdDialogButtonSizer**, the import will remove that line and instead generate the code in a platform independent way (on Mac, there should not be a line above the standard buttons).

When you import **wxFormBuilder** projects, **wxUiEditor** will default to generating the same filename that was generated before. If you would like to compare the differences, then in **wxUiEditor** change the default source and header extensions to `.cc/.hh` or `.cxx/.hxx` so that the new code won't overwrite the old code that **wxFormBuilder** generated.

## Pasting from wxFormBuilder

If you select an object in **wxFormBuilder** and from the **Edit** menu choose "Copy Object to Clipboard" then you can select a node in **wxUiEditor** and paste that node directly into an existing project.
