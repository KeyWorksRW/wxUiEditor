# Images

Most Windows applications store their images in the resource file which then gets linked into the executable. While **wxWidgets** does support reading these images, it only works on Windows, so you have to conditionalize the code if you want your app to be cross-platform. **wxUiEditor** takes a different approach. By default, when you specify an image file, that file will be converted into an unsigned character array, and placed into the first class source file that uses it. This allows the image to be compiled directly into your executable, which works on all platforms.

# wxBitmapBundle

**wxUiEditor** supports the **wxBitmapBundle** class introduced in **wxWidgets** 3.1.6. If your project settings specify a **wxWidgets** 3.1 version, then all bitmap code will be generated in conditionals so that it will use **wxBitmapBundle** if you compile with **wxWidgets** 3.1.6 or later, and the regular **wxImage/wxBitmap** code if you are compiling using an earlier version of **wxWidgets**.

The exception to the above paragraph is if you specify a SVG file for a bitmap -- since these images require **wxWidgets** 3.1.6 or later, the entire class will be marked as requiring **wxWidgets** 3.1.6 or later (if you use 3.1 as your project wxWidgets library setting), and will not compile if you use an earlier version of **wxWidgets**.

Multiple images will be added automatically if you using one of the following suffixes:

```
    basename + _1_5x
    basename + @1_5x
    basename + _2x
    basename + @2x
```

Example:

```
    my_bitmap.png

    // will also add the following if they exist

    my_bitmap_1_5x.png
    my_bitmap_2x.png
```

If the basename contains the suffix `_16x16` then a search will also be made for files with the suffix `_24x24` and `_32x32`. If the basename contains the suffix `_24x24`, then a search will also be made for files with the suffix `_36x36` and `_48x48`.

Example:

```
    my_bitmap_16x16.png

    // will also add the following if they exist

    my_bitmap_24x24.png
    my_bitmap_32x32.png
```

# SVG images

Starting with **wxWidgets** 3.1.6, many SVG images can be used as bitmaps that will accuratele scale to any dimension. Support for SVG is somewhat limited, so it's generally a good idea to preview the image before you build you application execting the SVG images to work. If you are adding it to a control inside **wxUiEditor** you can simply check how it looks in the Mockup panel. Otherwise, you can use the Preview SVG command under the Tools menu to see what it looks at (this will also allow you to see it at different scaling sizes).

When you specify an SVG file, **wxUiEdiutor** will read the file, strip out some content that **wxWidgets** won't use, compress it, and then store it as an unsigned character array in one of your source files. This will typically reduce the size of the image in your executable file by 80%. When your program runs, the generated code that displays the image will automatically decompress the data and pass it to **wxBitMapBundle**.

# XPM images

Several graphics applications will save an XPM file using a `static char` array. This will generate a compiler error when compiled with a c++20 compiler since it is invalid to assign a literal string to a non-const char array pointer. In addition, while most applications create an array name consisting of the base filename with a `_xpm` suffix, not all do. If you add an XPM file using the XPM image type, then **wxUiEditor** expects the `_xpm` suffix, and your code will not compile if it does not have the expected array name.

Both of these problems can be avoided by using the Embed command to specify your filename. Internally, **wxUiEditor** will convert the image to a PNG format (using any mask as the alpha channel) and then embed the image in the generated source code. The image should appear the same as it would if you used the XPM type, but without the potential problems mentioned above.
