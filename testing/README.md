# wxUiTesting

This app serves two purposes: it verifies that generated code can be compiled, and **Issues** can refer to this app for reproducing a problem or verifying a fix.

Specific dialogs can be run from the Dialogs menu. Controls in the dialog may have tooltips if certain behavior is expected so that you can verify that the control is behaving normally. In most cases, the derived dialog class will be unchanged from what `wxUiEditor` created -- normally the dialog isn't expected to do anything other than display itself.

Note that .srcfiles.yaml has a version of the `CFlags_cmn` setting commented out. If you have a C++20 compiler, then `-std:c++latest` will generate a _lot_ of warnings. The problem is that flags are being combined from multiple enumerations -- see issue [#135](https://github.com/KeyWorksRW/wxUiEditor/issues/135) for a way to resolve this problem.
