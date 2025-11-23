# Importing Windows Resources

**wxUiEditor** can import Menus and Dialogs from a Windows Resource file. While menus will look essentially the same, Dialogs will usually appear somewhat differently. This is due to the original Windows controls having absolution positions and sizes. **wxWidgets** controls have positions and sizes determined relative to the dialog, other controls in the dialog, the language and font currently in use by the user, and the operating system the dialog is being displayed on. The conversion algorithm attempts to replicate your intent for how your dialog should appear, but given all the possible ways absolute positioned controls can be specified, the conversion may not correctly replicate the original layout.

After conversion, you can select each dialog to display it in the Mockup Panel and determine whether or not you need to make adjustments. The following sections cover some of the changes you will see.

## Columns and Rows

In some cases, **wxUiEditor** will not recognize a control as being part of a row or column, and the control will be displayed out of position relative to the other controls. In many cases you can simply drag and drop the control into the sizer (using the Navigation panel) containing the other controls it should be aligned with. Once in the correct sizer, you can move the control up or down so that the order of the controls is correct.

## Buttons

Depending on the label and id of your buttons, **wxUiEditor** may convert some of your buttons into a **wxStdDialogButtonSizer**. The advantage of this is that the dialog will have a consistent button placement on all platforms -- but the button placement may be quite different from your original dialog.

## Text

If you use the Windows version of **wxUiEditor**, then text for captions and labels will be converted into UTF8 based on the current code page specified in the resource file. The conversion relies on a Windows API, and as such, it is not currently available on non-Windows platforms (which means the text will be assumed to be UTF8 already).

Note that if you are using a code editor that supports UTF8 text to edit your resource files, then you can specify `#pragma code_page(65001)` at the top of the resource file to indicate all text below it is in UTF8. Be aware that if you then edit the file with Visual Studio (any version through 17.0) then it will remove the code page and replace it with the current default Windows code page.

## Bitmaps and Icons

When a bitmap or icon is specified for a Dialog, **wxUiEditor** will convert it into an embedded form that will be generated as part of the class's source file. This allows the image to work on non-Windows platforms without needing to take any additional steps, or needing to ship external files.

## MFC controls

**wxUiEditor** can convert most Mfc controls into a **wxWidgets** equivalent, but it will not have access to many of the properties. For example, `MfcLink` will be converted to `wxHyperlinkCtrl` with the correct label, but it will not have the URL that the link points to.
