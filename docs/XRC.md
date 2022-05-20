# XRC

**wxUiEditor** can generate XRC either as individual files, or a combined file. Many of the properties that would automatically generate the necessary code for you are not available when using an XRC file. You are _strongly_ encouraged _NOT_ to use XRC for C++ projects!

If you are using a programming language other than C++, then generating XRC is the only way you can use **wxUiEditor** to design your UI. You can use the `Preview XRC...` command under the Tools menu if you want to see what a Dialog will look like. If this looks different than it does in the Mockup panel, it's because you set properties that cannot be passed to XRC files. If you look at the XRC panel to the right of the Mockup panel, you will see comments about every property that you have set which is not available in XRC.

**wxCrafter** (CodeLite), **wxFormBuilder** and **wxGlade** can import XRC files. However, none of them fully support all XRC objects and properties. All of these designers will warn you if an entire object is not supported, but only **wxGlade** will warn you if there are unsupported properited (the other designers will simply ignore them). If the designer's preview of the XRC does not look the same as **wxUiEditor**'s preview, it's because the other designer is missing support for one or more objects or properties that you are using.
