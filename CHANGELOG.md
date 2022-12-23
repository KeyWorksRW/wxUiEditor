# Change Log
All notable changes to this project will be documented in this file.

## [Unreleased (1.1.0)]

Release summary...

### Added

- You can now add your own code at the end of generated files. This code will be preserved even when the file is re-generated.
- Stretchable space can now be added to toolbars.
- Added support for dropdown toolbar items that contain menu items.
- wxToolBar and wxAuiToolBar individual tools now have a disabled property.
- wxStatusBar `fields` property now supports unique width and style for each field.
- New `platforms` property for most controls that inherit from **wxWindow**. This gives you the option of limiting the creation of a control to specific platforms.
- Menu items can now contain multiple accelerators (requires wxWidgets 3.1.6 or later)
- XRC now generates XML for toolbar separators.

You can now place forms inside of folders to make it easier to organize your project when you have a large number of forms. Besides organizing the Navigation pane, folders also let you override properties such as the output folder for just the forms contains within that folder. The new folder command is available on the right side of the Forms panel of the toolbar, or from the Move sub-menu when you right-click on a form.

The right-click menu for a widget in the Navigation Panel will sometimes have a `Change to` option. The 1.0 release supported changing most sizers. New to 1.1 is the ability to change some widgets as well. The following widgets can be changed to another widget in the group:

- 2-state wxCheckBox, 3-state wxCheckBox, wxRadioBox
- wxChoice, wxComboBox, wxListBox
- wxAuibook, wxChoicebook, wxListbook, wxNotebook, wxSimplebook

### Changed

- A user can no longer enter an invalid C++ variable name.
- `contents` property for wxCheckListBox and wxRearrangeCtrl now supports setting initial checked state for each string.
- When a form is duplicated, the class name and any filenames specified for the form are now unique within the project.
- `Generate XRC files...` on Tools menu now launches a dialog allowing you to choose between combined or individually generated XRC files.
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
