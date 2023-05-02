# Change Log
All notable changes to this project will be documented in this file.

## [Unreleased (1.1.1)]

### Added

- The C++ Settings in forms now have a `initial_enum_string` property that allows you to set the initial enumeration value to something other than the default "wxID_HIGHEST + 1".
- wxMenu and wxMenu items now have a stock_id property allowing you to choose from wxWidgets stock items.

### Changed

- Improved generation of default filenames for a class when the class name is changed
- wxFrame and the form version of wxPanel now also support 2-step construction
- Handle embedded filenames that contain characters that are invalid as variable names
- Allow custom ids to have an assignment to a value as part of the id. In C++, the id will then be generated as a `static const int` instead of an enumerated value. In Python, this will be added verbatim after any auto-generated ids.

### Fixed

- Code generation for Dialogs with an extra_style property fixed.
- Code generation for wxToolBar drop-down tools with menu items fixed.
- Code generation for wxGridBagSizer when growablecols or growablerows specified
- Code generation for Wizard using wxWidgets 3.1 corrected

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
