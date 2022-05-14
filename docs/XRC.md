# XRC

**wxUiEditor** can generate XRC either as individual files, or a combined file. Many of the properties that would automatically generate the necessary code for you are not available when using an XRC file. You are _strongly_ encouraged _NOT_ to use XRC for C++ projects!

If you are using a programming language other than C++, then generating XRC is the only way you can use **wxUiEditor** to design your UI. You can use the `Preview XRC...` command under the Tools menu if you want to see what a Dialog will look like. If this looks different than it does in the Mockup panel, it's because you set properties that cannot be passed to XRC files. If you look at the XRC panel to the right of the Mockup panel, you will see comments about every property that you have set which is not available in XRC.

Several other designers import XRC files. Technically you could export your XRC files and import it into another designer. However, not only will you lose any of the properties mentioned above, but the other designers generally don't support all of the options that XRC does. That means you can lose additional functionality simply because they don't support the XRC setting that **wxUiEditor** generated.
