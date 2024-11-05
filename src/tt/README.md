# tt classes general info

This repository is designed to be added as a subrepostory in other repositories. It's used in multiple KeyWorks repositories. You are welcome to use it in your own repositories if you find the classes useful (see [License](LICENSE)).

## tt_ prefix

Normally, a set of classes like the ones in this repository would be placed into a namespace, such as `tt`. However, the names of these classes are designed to make it clear what class they inherit from, and if they were in a `tt` namespace instead of the `tt_` prefix, then adding `using namespace tt` along with `using namespace std` would mean that `string` could refer to either the tt or std version. Using the `tt_` prefix avoids that potential conflict.

## class list

- tt_string: derived from `std::string` with additional methods
- tt_stringview: derived from `std::stringview` with additional methods
- tt_string_vector: derived from `std::vector<tt_string>` with additional methods including the ability to read and write to line-oriented text files.
- tt_view_vector: same as `tt_string_vector`, only using `tt_string_view` which means you cannot modify any of the lines.

