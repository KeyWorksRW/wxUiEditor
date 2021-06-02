# Importing Windows Resources

**wxUiEditor** can import Dialogs from a Windows Resource file. The resulting dialog in **wxWidgets** will be similar to your original version, but you will almost certainly need to modify the dialogs in **wxUiEditor** after conversion. After conversion, you can select each dialog to display it in the Mockup Panel and determine whether or not you need to make adjustments. The following sections cover the types of changes you will see.

## Layout

Within the resource file, all the dialog's' controls within it have absolute positions and size. Even if you used a library such as **MFC** to dynamically change positions or sizes at runtime, the resource file won't contain that information. When **wxUiEditor** converts the dialog, _all_ controls are placed within sizers and their position and size is automatically calculated by **wxWidgets**. The conversion will attempt to figure out your intent and use alignment and horizontal or vertical sizers, however you will probably need to make adjustments to get closer to your original layout design.

If you used IDOK as the id for a dialog's default button, then **wxUiEditor** will convert that into a **wxStdDialogButtonSizer**. The advantage of this is that the dialog will have a consistent button placement on all platforms -- but it may look quite different from your original dialog.

## Bitmaps and Icons

**wxWidgets** supports reading bitmaps and icons from a resource file, however it means that your program can only be compiled for Windows unless you write conditional code for non-Windows compilation. **wxUiEditor** takes a different approach and converts your images into a header file that can be compiled directly into your program. This method works on all platforms and doesn't require shipping external images. The executable size will be roughly the same (or smaller if you used BMP files).

After conversion, you will see a question mark for all your images until you tell **wxUiEditor** to generate code. You should check all images and make certain that both the original image and the name and location the header file is what you want. Do this _before_ you tell **wxUiEditor** to generate code!

In a Windows Resource file, ICONS and BITMAPS can have different id names with identical id values. For example, you might define the following:

```
    #define IDICON_MAINFRAME                100
    #define IDR_MAINFRAME                   100
```

In your resource file, you might have:

```
    IDR_MAINFRAME    BITMAP "res/MyBitmap.bmp"
    IDICON_MAINFRAME ICON   "res/MyIcon.ico"
```

If you then try to add a picture control in Visual Studio specifying `IDR_MAINFRAME` as your bitmap, Visual Studio will write the control as:

```
    CONTROL IDICON_MAINFRAME,IDC_STATIC,"Static",SS_BITMAP
```

When the resource compiler parses the above line, it will convert `IDICON_MAINFRAME` into it's numerical value and use that to locate the bitmap rather than the icon.

Because **wxUiEditor** isn't a resource compiler, it can only see the name of the id -- and as a result, it will get the image that the name refers to (in the above case this would be the icon).

This problem only occurs if you use numerical ids that are the same for both ICONS and BITMAPS. If you do, the bitmap or icon may be wrong, and you will need to change it in the **wxWidgets** properties for the image.
