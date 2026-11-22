# Linked projects

A linked project lets a folder in your `.wxui` file display and generate the forms of a *second* `.wxui`
file. The linked file is never modified: wxUiEditor reads it, adds its forms to the navigation tree as
read-only content, and generates their code into the owning project's output directories.

This is useful when several applications share a set of dialogs, or when a reusable form library is
maintained in its own project file.

## Creating a link

There are two ways to create a link:

- Right-click the Project node or a folder in the Navigation panel and choose **Link project...**.
  Select the `.wxui` file to link to.
- Select a folder and set its `link_project` property in the property grid. The path may be absolute or
  relative to the project file; a relative path is stored so the link keeps working when the project is
  moved together with the linked file.

The forms in the linked file appear immediately below the folder. Relinking the same folder to a
different file asks for confirmation before the existing children are replaced. To remove a link,
right-click the link folder and choose **Unlink project** — the folder stays, but its linked content is
removed.

Both actions are single undo steps.

If a link cannot be loaded (the file is missing, is not a wxUiEditor file, was created by a newer
wxUiEditor version, or contains content that cannot be linked), the reason is reported and the folder is
left empty. The same validation is repeated every time the project is loaded, whenever the link is
reloaded, and once more immediately before code is generated — an externally edited file never enters
the tree or the generated code without being revalidated.

## What is imported

Only **forms and folders** are imported from the linked file:

- Each form of the linked file becomes a form in the link folder.
- Folders in the linked file become sub-folders of the link folder, so the navigation hierarchy is
  preserved.
- Only the node subtree is imported. The linked project's own settings — code preference, output
  directories, wxWidgets version, namespaces, and so on — are **not** imported.

Class names, base file names, derived class names and per-language file names must not collide with the
forms already in the owning project. A collision rejects the whole link and reports the conflicting
names.

## Read-only rule

Everything below a link folder is **read-only**:

- Its properties are disabled in the property grid.
- Cut, delete, duplicate, paste, move, drag-and-drop, and adding new nodes are refused.
- The context menu offers only commands that do not modify the node, such as copying.
- Undo actions that would modify a linked node are discarded, so the project is not marked modified.

This is deliberate: the linked content lives in another file that wxUiEditor never writes, so a change
made in the owning project could not be saved.

The link folder itself remains fully editable. You can change its label, `link_project`,
`link_generate_code`, `art_directory`, and all of the folder output-directory overrides described below.

## Output directories

A link folder is an ordinary folder for generation purposes, so the usual folder overrides apply to every
form it contains:

- `folder_base_directory` — where generated base class files are written
- `folder_derived_directory` — where derived class files would be written (linked forms never produce
  derived files; see below)
- `folder_namespace`
- `folder_cmake_file` and `folder_cmake_varname`
- `folder_python_output_folder`, `folder_ruby_output_folder`, `folder_xrc_directory`, and the
  corresponding overrides for the other supported languages

When an override is empty, the project-level setting is used. Generated files are always written into the
owning project's directories — never into the linked project's tree.

## `link_generate_code`

By default, forms loaded from a linked file are included in code generation. Clear
`link_generate_code` on the link folder to keep the forms visible in the editor but exclude them from
**all** generated output: C++, wxPython, wxRuby, XRC, CMake files and the other supported languages.

## Art files

Image, animation and bitmap-combo-item paths inside the linked forms are resolved once, when the link is
loaded, in this order:

1. the path exactly as stored in the linked file
2. relative to the link folder's `art_directory` property, if set
3. relative to the linked project's own `art_directory` setting, if set
4. relative to the directory containing the linked `.wxui` file

The resolved path is then rewritten relative to the owning project, so the rest of the session (and the
generated code) sees a normal project-relative path. An absolute path is never stored in a property, and
`Art;` references are left untouched. Use the link folder's `art_directory` when the linked project uses
art files that live somewhere other than beside the linked file.

Paths that cannot be resolved are logged and left unchanged.

## External changes

While a project with a link is open, wxUiEditor watches the linked files. When one changes on disk, that
folder's subtree is reloaded — the file is re-read and revalidated from scratch. The undo history is
cleared for the reload, and the owning project is not marked as modified. If the edited file no longer
passes validation, the previous subtree stays in place and the reason is reported.

## Limitations

- **Forms only.** An `Images` list, a `Data` list or a `data_folder` in the linked file rejects the
  link. Those node types would write generated files or image-list sources next to the linked project,
  which this feature never does.
- **No derived-class forms.** A form that has `use_derived_class` enabled and a non-empty `derived_file`
  is rejected, because derived files are never generated for linked nodes. If you need a derived class
  for a shared form, keep that file under your own control.
- **No cycles.** A file may not link, directly or indirectly, back to a file that is already being
  loaded.
- **Maximum four linked levels.** A chain of linked files may not be nested more than four levels deep.
- **No project settings.** Only the node subtree is imported, as described above.
- **Undo history on external change.** When a linked file is changed on disk and reloaded
automatically, the undo history is cleared. Link and unlink actions performed in the editor remain
single, undoable steps.
