# Importing wxCrafter projects

**wxUiEditor** can import most **wxCrafter** project files. Because **wxUiEditor** generates source code for each top level form, you can import multiple **wxCrafter** files into a single **wxUiEditor** file. If multiple forms are specified in a single **wxCrafter** project, only the first form will have the generated filename set. All additional forms will use a placeholder name (`filename_base`) -- you will not be able to generate code for the additional forms until you specify a filename to generate.

Note that **wxUiEditor** projects are roughly 90% smaller than a **wxCrafter** project due to optimization in what gets written to the project file.

## Images

When you import a **wxCrafter** project, **wxUiEditor** will `#include` XPM files in the generated source code. All other external images are first converted into a C++ array and generated in the source file of the first form in your project that uses it. All further references to the same image will use a `extern` reference.

## Code Generation

The code that **wxUiEditor** generates will often look quite different than the code that **wxCrafter** generates. Your UI will usually work the same in spite of the differences in the code. However, there are some differences you should be aware of:

- **wxUiEditor** will connect to events using **Bind()** rather than **Connect()**
- **wxUiEditor** will not disconnect events in a destructor since **wxWidgets** already does this

**wxUiEditor** provides a lot more options -- for example it supports validators, high dpi-aware values (e.g. Dialog Units), lambdas as event handlers, etc. You may find that using the options provided by **wxUiEditor** will reduce the amount of code needed in your derived class -- or eliminate the need for the derived class entirely.
