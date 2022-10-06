# Change Log
All notable changes to this project will be documented in this file.

## [Unreleased (1.0.1)]

Release summary...

### Added

- Stretchable space can now be added to toolbars.
- Added support from dropdown toolbar items that contain menu items.
- You can now set the style and width of each field in a wxStatusBar.
- XRC now generates XML for toolbar separators.

### Changed

- A user can no longer enter an invalid C++ variable name.
- wxStatusBar `fields` property now supports unique width and style for each field.
- `contents` property for wxCheckListBox and wxRearrangeCtrl now supports setting initial checked state for each string.
- When a form is duplicated, the class name and any filenames specified for the form are now unique within the project.
- Menu items can now contain multiple accelerators (requires wxWidgets 3.1.6 or later)
- XRC files now include a comment warning that regeneration will eliminate any hand edits.
- `Generate XRC files...` on Tools menu now launches a dialog allowing you to choose between combined or individually generated XRC files.
- Improved XRC generation of optional static line above a standard dialog button -- this now matches what the C++ version would look like.

### Fixed

- Code generation for button without a label fixed.
- Code generation for wxGrid with positive row label size fixed.
- Code generation for wxGrid with non-default SetSelectionMode fixed.
- Code generation for wxPanel events fixed.
- Code generation for setting custom foreground colour for wxStaticText fixed.
- Fixed browsing for an XRC output file to generate XRC to -- it now defaults to the current project directory, and can handle a relative path.
- Fixed non-combined XRC generation.
- Fixed several problems when importing XRC/wxSmith files.
