# Building

To build the C++ apps, you will need build from the root of the repository (_not_ the directory this README file is in) using a `--target` command to build a specific app. For example, to build sdi_test.exe, you would run (from the root):

```
    cmake.exe --build build --config Debug --target sdi_test
```

This will create the executable: `build/stage/bin/Debug/sdi_test.exe`

The file `app_tests.cmake` is used to specify everything needed to build a target using whatever configuration for the wxWidgets library was specified in the root.
