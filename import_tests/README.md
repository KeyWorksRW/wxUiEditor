# Import Testing

The files in the subfolders of this directory are used to test the ability of **wxUiEditor** to import different types of projects.

## Windows Resource files

The file `main_test.rc` is maintained by Visual Studio 6.0. This is the last version of Visual Studio that used property dialogs making it very easy to work with. The file will load fine with current versions of Visual Studio, but please do _not_ save over this file with a **devenv**-generated version. The older style is needed to ensure that **wxUiEditor** can pick up and convert legacy Windows projects.

## Visual Studio Code launch templates

If you are using Visual Studio Code for debugging, you might find the following templates useful.

#### Windows Resource testing

```json
     {
         "name": "WinTest Resource",
         "type": "cppvsdbg",
         "request": "launch",
         "cwd" : "${workspaceRoot}/../import_tests/winres/",
         "program": "${workspaceRoot}/../bin/wxUiEditorD.exe",
         "args": [
             "main_test.rc"
         ],
         "logging": {
            "moduleLoad": false,
         },
         "visualizerFile": "${workspaceRoot}/wxui.natvis",
         "stopAtEntry": false,
     },
```
