# Contributor notes

Contributions are welcome! If you are interested in contributing, please read the following sections.

Note that in addition to this file, some of the directories also have README files that provide explanations specific to the files in that directory. See [generate](../../src/generate/README.md) and [internal](../../src/internal/README.md). Also see [build_notes.md](build_notes.md) for information about building the project.

## GitHub Copilot Agents

If you are using a current version of VS Code with GitHub Copilot, the project includes custom agents to assist with development. See [.github/agents.json](../../.github/agents.json) for a list of available custom agents. Additionally, there is a comprehensive [.github/copilot-instructions.md](../../.github/copilot-instructions.md) file that provides general guidance for agents working in this project.

## Debug builds

When you create a Debug build, there will be an additional `Testing` and `Internal` menu to the right of the **Help** menu that will give you access to additional functionality for easier debugging. You can also get these menus in a Release build if you set INTERNAL_BLD_TESTING to true in your CMake configuration.

## Strings

This project is built using wxWidgets 3.3 or higher with UTF-8 encoding for wxString and other string types.

When converting wxString to std types, it's important to understand the difference between the two main conversion methods:
- `wxString::utf8_string()` creates a **copy** of the string
- `wxString::ToStdString()` returns a **const reference** to the underlying string

Because `ToStdString()` returns a reference, you can pass it directly to functions expecting `std::string_view` parameters without creating an unnecessary copy.

### Debugging macros

The `ASSERT`, `ASSERT_MSG` and `FAIL_MSG` macros are the preferred macros for debug checks. On Windows, they provide the option to break into a debugger if running a Debug build.

The `MSG_...` macros allow for display information in the custom logging window. The custom logging window has filters so that you can limit which messages are displayed. Unlike the `wxLog...` macros, none of these messages will ever be displayed to the user -- they are for your debugging use only.

## clang-format

All PRs get run through a github action that runs clang-format. This will report a failure if clang-format would have changed your code formatting. To ensure a successful PR submission, run your code through clang-format before committing it.

If you are adding a comment to a function or variable in a header file, please wrap the comment to a maximum of 93 characters -- that makes the comment more likely to display correctly when displayed in a Intellisense popup.
