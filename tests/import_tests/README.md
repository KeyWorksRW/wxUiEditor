# Import Testing

The files in the subfolders of this directory are used to test the ability of **wxUiEditor** to import different types
of projects. Each sample project file has a matching `.wxui` golden beside it, produced by importing that sample.

## Regenerating the goldens

Never hand-edit a `.wxui` golden — regenerate it with the Python script (works unchanged on Windows and Unix):

```
python tests/import_tests/regenerate.py debug
```

Omit the argument (or pass `release`) to use the Release build.
The script runs `--save_import` for every sample, reports failures, prints `git diff --stat -- tests/import_tests`, and
exits non-zero if any import failed.
Importers are deliberately lossy, so a changed golden means importer output changed — review the diff.

Add `--verify` to re-import each sample with `--verify_import` after regenerating, proving the goldens are reproducible
(idempotent):

```
python tests/import_tests/regenerate.py debug --verify
```

## Windows Resource files

The file `main_test.rc` is maintained by Visual Studio 6.0. This is the last version of Visual Studio that used property
dialogs making it very easy to work with.
The file will load fine with current versions of Visual Studio, but please do _not_ save over this file with a
**devenv**-generated version.
The older style is needed to ensure that **wxUiEditor** can pick up and convert legacy Windows projects.

## Visual Studio Code launch templates

If you are using Visual Studio Code for debugging, you might find the following templates useful.

#### Windows Resource testing

```json
     {
         "name": "WinTest Resource",
         "type": "cppvsdbg",
         "request": "launch",
         "cwd" : "${workspaceRoot}/tests/import_tests/winres/",
         "program": "${workspaceRoot}/bin/Debug/wxUiEditorD.exe",
         "args": [
             "main_test.rc"
         ],
         "logging": {
            "moduleLoad": false,
         },
         "visualizerFile": "${workspaceRoot}/src/wxui.natvis",
         "stopAtEntry": false,
     },
```
