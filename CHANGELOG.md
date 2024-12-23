# Change Log
All notable changes to this project will be documented in this file.

## [unreleased (1.3.0)]

### Added

- New Add menu provides an alternative to the main toolbar for adding widgets to your project.
- By default, all Position and Size properties are scaled when your generated code is run under Windows with a high DPI setting.
- You can now change the initial size of Art images instead of using their default size.
- File menu now has a Reload Project command to reload the current project file from disk.
- Dialogs now support the variant property
- Added support for wxPopupWindow as a form

### Changed

- All coordinates using dialog units will automatically be converted to physical pixels. For C++ using wxWidgets 3.2 or higher along with wxPython and wxRuby, FromDIP() will be called to scale the UI as needed on high DPI displays.
- In most cases, you no longer need to select a sizer or container before adding a widget. wxUiEditor will now attempt to find an appropriate parent for what you want to add starting from your current selection.
- Generic option removed for wxAnimationCtrl. The generic version is automatically generated when a ANI animation file is specified. This will correctly display the file on wxGTK when generating C++ and wxPthon code. wxRuby3 does not support Wx::GenericAnimationCtrl in version 1.0, so only the regular version is generated.
- derived_class property in the wxWindows category has been changed to subclass to better reflect that the generated class derives from your specified class. The derived_class_name in the various language categories remain unchanged to indicate you derive your class from the generated class.
- wxRuby3 generated code now supports persistence property (`Wx.persistent_register_and_restore`)

### Fixed
- Fixed generation of event handlers in C++ derived classes.
- Fixed inability to hand-edit the location of a generated file
- Add missing events to wxPropertyGrid and wxPropertyGridManager
- wxPython now correctly loads embedded animation files for wx.adv.AnimationCtrl
- XRC generation now includes variant property settings for forms
- wxPython now places wxTreeListCtrl in the dataview library
- Fixed code generation for Python and Ruby wxStdDialogButtonSizer events
- Ruby code generation no longer writes the class `end` line before the final generated comment block
- You can add a menubar or toolbar to a wxFrame that was created without them
- Fixed inconsistent generation of wxScrolledWindow versus wxScrolled<wxPanel> -- wxScrolledWindow is now always used.

## [Released (1.2.1)]

### Added

- Dark Mode and High Contrast Dark Mode are now available in the Prefences dialog on Windows.
- The font used in the code panels can now be set in the Preferences dialog.
- XPM files are now supported in wxPython and wxRuby3.
- Setting a static bitmap's scale mode will now use wxGenericStaticBitmap in wxPython (4.2.1) and wxRuby3 (0.9.3) to ensure that all platforms will support the scaling.
- Added wxBitmapToggleButton for all languages.
- Added wxContextHelpButton for all languages.
- Added support for all four validators in wxRuby3 0.9.3 and up.
- Non-derived C++ class headers now support protected: class methods in addition to the public: ones that were already supported.
- New Data List form allowing you to embed text, xml and binary files into your application.
- Any generated .cmake file will also include a separate list of data files that can be used in the `add_custom_command` and `add_custom_target` commands of a CMakeLists.txt file.
- Images list has a new auto_add property that will automatically add new images to the list when they are added to any control.
- The size of the Icons in the tree control (Navigation Panel) can be set in the Preferences dialog. On Windows, these settings will be scaled to the current DPI setting.

### Changed

- The graphics in the Navigation panel and the Ribbon toolbar have been replaced with new SVG images that will scale better on high DPI displays. Because the new graphics were created from scratch, they may look quite different from the old ones.
- Additional colors for the Code display panels can be set in Preferences (support for C++, Python, Ruby and XRC).
- The `hide_children` property in the `wxStaticBoxSizer` has been removed since the `hidden` property does exactly the same thing. Projects where `hide_children` was set will be automatically converted to use `hidden` instead.
- The Image List file now includes wxue_img::image_ functions for each image in the list, replacing the `extern` declarations that were previously used. A new `add_externs` property has been added -- check that if you still need the extern declarations.
- wxIntegerValidator and wxFloatingPointValidator now support setting either min or max, and are generated as separate calls. Previously, you had to set both of them to have either one generated.
- The ability to generate only a C++ header file without a source file has been removed.
- Books now display images on pages by default. If you don't want the images, you can uncheck the `display_images` property.
- Before generating code, any images that you used are checked to see if they have been modified since you created them, and if so, the new file is loaded before generating the code.
- When using the context menu to change a widgets from one type to another, all identical events will have their handler names preserved, and if the var_name is used for class access, that will be preserved as well.

### Fixed

- The Show Hidden on the toolbar now shows (in the Mockup panel) the children of sizers that have their children hidden.
- Setting `hidden` in a `wxStaticBoxSizer` did not generate any code to hide the wxStaticBox and it's children (you had to check `hide_children` to get the code). This has been fixed.
- The Context Help button in wxStdDialogButtonSizer did not have a label. This now generates a `wxContextHelpButton` which uses a bitmap rather than a label, and automatically places the dialog in context-help mode when clicked.
- Fixed wxPython and wxRuby3 code generation for creating a wxPropertySheetDialog class.
- When importing XRC files, a form's class name is correctly set if specified in the XRC file.
- wxStyledTextCtrl::SetTabWidth() now generates code correctly (generated if tabs are enabled and tab width is not the default value of 8)
- wxStaticBoxSizers that use a wxRadioButton now work correctly in wxUiEditor's UI on Linux, and code generation has been updated to work on wxGTK allowing the button to be unchecked when the app is built with wxWidgets 3.3 or later.

### Additional Note

Starting with version 1.2.1, wxUiEditor uses LunaSVG instead of NanoSVG to display SVG images in the Mockup Panel. In most cases, you won't see a difference between what wxUiEditor displays and what your own code displays. However, there are times where NanoSVG displays an SVG image incorrectly and LunaSVG displays it correctly. LunaSVG will be integrated into wxWidgets 3.3 in the near future, but until you can use that version, then it's possible your SVG images won't display the way you intended them to in your application, even though they look fine in wxUiEditor.

## [Released (1.2.0)]

### Added

- Code can now be generated for wxRuby3 -- see https://github.com/mcorino/wxRuby3
- New Doc panel (Windows version only) will display a URL containing documentation for the currently selected widget. A toolbar in this panel lets you choose between C++, Python and Ruby documentation.
- New Preferences dialog available under the File menu lets you set several global preferences
- wxTimer is now supported as a child of a dialag, frame or form version of wxPanel
- DialogBlocks projects saved as XML can be imported
- wxAuiToolBar can now be created as a form (it creates a class instead of a child widget)
- Added support for additional integer and floating point validators for wxTextCtrl
- Folders now have their own code language preference that can override the Project's code preference
- Added support for wxPropertySheetDialog as a form
- Tooltips set in wxAuiNotebook pages are now displayed when hovering over the tab display rather than the page itself
- You can now set selected and non-selected fonts for wxAuiNotebook tabs
- Added `dialog_units` property to the Project properties list to change default units for new dimensions
- Custom controls now have a `construction` property allowing you to replace the normal wxUiEditor-generated construction of the control with your own code.

### Changed

- Controls that have one or more default window styles set (e.g., wxPanel has wxTAB_TRAVERSAL set) can now have that style unchecked and that state will be stored with the project file.
- Color properties dialog now supports _all_ CSS color names
- Generated code for colors now uses a CSS HTML String (#RRGGBB) instead of numerical values for red, green and blue.
- Generated code for wxRibbonToolBar images are now scaled on high DPI displays
- A control within a wxDialog or wxFrame with the focus property set will now call SetFocus() after all other controls have been created.
- Limiting the platforms for a container (such as a wxPanel) will now also limit the platforms for any child widgets.
- C++ src_preamble contents is now placed at the top of the file instead of after the includes.
- Custom font point size can be set to -1 to indicate that a default point size should be used.
- Importing wxFormBuilder and wxGlade projects now supports converting a construction property for a custom control.

### Fixed

- You can now change the Window styles in a wxRichTextCtrl without generating an invalid constructor.
- Color properties are correctly saved in a project
- Widgets set to specific platforms will also place events in a conditional block
- For C++, widgets set to specific platforms will have the header member declarations in a conditional block
- C++ code generation for fonts with a negative point size fixed

## [Released (1.1.2)]

### Added

- The C++ Settings in forms now have a `initial_enum_string` property that allows you to set the initial enumeration value to something other than the default "wxID_HIGHEST + 1".
- There is a new 'Global Edit Custom IDs` under the Tools menu for setting prefixes and suffixes for your custom IDs.
- The C++ Settings in forms now have properties for local and system header files to include in either the generated source or header files.
- wxMenu and wxMenu items now have a stock_id property allowing you to choose from wxWidgets stock items.
- Added support for wxAUI_BUTTON_STATE flags when creating a wxAuiToolBar tool.
- **wxPopupTransientWindow** now supports variant, minimum, maximum, and size properties

### Changed

- Improved generation of default filenames for a class when the class name is changed
- wxFrame and the form version of wxPanel now also support 2-step construction.
- Embedded image filenames can now contain characters that are invalid as part of a variable name.
- Allow custom ids to have an assignment to a value as part of the id. In C++, the id will then be generated as a `static const int` instead of an enumerated value. In Python, this will be added verbatim after any auto-generated ids.
- If you previously used either base_hdr_includes or base_src_includes properties, then when the project is loaded they will be automatically converted to header_preamble and source_preamble properties to better reflect what it's for, and to avoid confusion with the new local and system includes properties.
- Removed position, size, and foreground_colour properties from **wxPropertySheetDialog** since the dialog doesn't support them.
- Removed size properties from wxWizard since they cannot be used

### Fixed

- Code generation for Dialogs with an extra_style property fixed.
- Code generation for wxToolBar drop-down tools with menu items fixed.
- Code generation for wxGridBagSizer when growablecols or growablerows specified
- Code generation for Wizard using wxWidgets 3.1 corrected
- Code generation for adding an item to a wxPropertyGrid has corrected
- Code is now generated for a sub-menu that has a bitmap property set
- Code generation for event handler of a wxStdDialogButtonSizer Close button corrected
- Header file code generation could create uninitialized wxStdDialogButtonSizer buttons under special circumstances -- this has been fixed.
- Fixed code generation for variant, font, and background color in **wxPropertySheetDialog**
- Add `no` as a Center option to prevent any wxWizard centering so that it will honor the `pos` property.
- Fixed `variant` property code generation in wxWizard.
- Changing a sizer or widget type no longer changes validator names

## [Released (1.1.0)]

The biggest change is the ability to generate wxPython code. In addition, several improvements have been made for C++ code generation. For both languages, you can now add your own code at the end of the generated files, often eliminating the need to create an inherited class.

### Added

- Ability to generate wxPython code (supports wxPython 4.2 including wx.BitmapBundle and roughly 140 widgets).
- Ability to add your own code at the end of generated files. This code will be preserved even when the file is re-generated.
- A line length property has been added to the C++ and Python Project properties, and most code will be wrapped so that it is less than this limit.
- `--gen_cpp` and `--gen_python` command line switches added. Using one of these switches will generate any code that needs to be updated, and then exit. A log file will be created containing any code generation messages. This will be the project file with the extension replaced by `.log`.
- Stretchable space can now be added to toolbars.
- Added support for dropdown toolbar items that contain menu items.
- wxToolBar and wxAuiToolBar individual tools now have a disabled property.
- wxStatusBar `fields` property now supports unique width and style for each field.
- New `platforms` property for most controls that inherit from **wxWindow**. This gives you the option of limiting the creation of a control to specific platforms.
- Menu items can now contain multiple accelerators (requires wxWidgets 3.1.6 or later)
- XRC now generates XML for toolbar separators.

#### Folders

You can now place forms inside of folders to make it easier to organize your project when you have a large number of forms. Besides organizing the Navigation pane, folders also let you override properties such as the output folder for just the forms contains within that folder. The new folder command is available on the right side of the Forms panel of the toolbar, or from the Move sub-menu when you right-click on a form.

#### Context menu

The right-click menu for a widget in the Navigation Panel will sometimes have a `Change to` option. The 1.0 release supported changing most sizers. New to 1.1 is the ability to change some widgets as well. The following widgets can be changed to another widget in the group:

- 2-state wxCheckBox, 3-state wxCheckBox, wxRadioBox
- wxChoice, wxComboBox, wxListBox
- wxAuibook, wxChoicebook, wxListbook, wxNotebook, wxSimplebook

### Changed

- The Generate Code command now let's you choose between C++, Python or XRC code generation.
- A user can no longer enter an invalid C++ variable name.
- All C++ generated files include clang-format off and on comments around generated code so that if you add your own code at the end, you can run it through clang-format.
- `contents` property for wxCheckListBox and wxRearrangeCtrl now supports setting initial checked state for each string.
- When a form is duplicated, the class name and any filenames specified for the form are now unique within the project.
- Improved XRC generation of optional static line above a standard dialog button -- this now matches what the C++ version would look like.
- XRC files now include a comment warning that regeneration will eliminate any hand edits.

### Fixed

- Code generation for button without a label fixed.
- Code generation for wxGrid with positive row label size fixed.
- Code generation for wxGrid with non-default SetSelectionMode fixed.
- Code generation for wxPanel events fixed.
- Code generation for setting custom foreground colour for wxStaticText fixed.
- Fixed browsing for an XRC output file to generate XRC to -- it now defaults to the current project directory, and can handle a relative path.
- Fixed non-combined XRC generation.
- Fixed several problems when importing XRC files.
- #include \<wx/aui/auibar.h\> now added when creating a wxAuiToolBar
