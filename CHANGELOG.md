# Change Log
All notable changes to this project will be documented in this file.

## [Unreleased (1.0.1)]

Release summary...

### Added

- Stretchable space can now be added to toolbars.
- XRC now generates XML for toolbar separators.

### Changed

- A user can no longer enter an invalid C++ variable name.
- XRC files now include a comment warning that regeneration will eliminate any hand edits.
- `Generate XRC files...` on Tools menu now launches a dialog allowing you to choose between combined or individually generated XRC files.
- wxStatusBar `fields` property now supports unique width and style for each field.
- `contents` property for wxCheckListBox and wxRearrangeCtrl now supports setting initial checked state for each string.

### Fixed

- Code generation for button without a label fixed.
- Code generation for wxGrid with positive row label size fixed.
- Code generation for wxGrid with non-default SetSelectionMode fixed.
- Code generation for wxPanel events fixed.
