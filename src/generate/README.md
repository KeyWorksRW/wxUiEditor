# Generators

Each widget that wxUiEditor supports has a generator that is responsible for generating the code needed to create the widget as well as any additional settings that the user may have specified. For generating language code, the generator can provide up to 5 functions depending on what the widget needs. These 5 functions will have a version for creating C++ code, and a second set of 5 functions that can produce wxPython, wxLua and wxPHP code. While this does result in some code duplication, it can be difficult to read the multiple language versions due to all the sub-functions that are called. The C++ functions provide the base template which is easier to read and maintaing for the other languages to follow.

In some cases, the differences between the languages is too great, in which case individual generators are called for each language. For example, constructing a wxDialog has four functions:

```c++
    bool GenConstruction(Node*, BaseCodeGenerator* code_gen) override;
    std::optional<ttlib::cstr> GenPythonConstruction(Node*) override;
    std::optional<ttlib::cstr> GenLuaConstruction(Node*) override;
    std::optional<ttlib::cstr> GenPhpConstruction(Node*) override;
```

Note that XRC is not a language, and only requires a single function which provides generates XRC code for all of the widgets properties.

The following sections cover specific differences about the language versus the C++ code generation.

## wxPython

wxWidget names are prefixed with `wx.` followed by the name of the widget without the "wx" prefix (e.g. `wx.StaticText` for `wxStaticText`). wxEmptyString is special-cased to use `""` instead of `wxEmptyString`.

Python uses indentation as the equivalent to braces in C++ -- which makes correct indentation critical.

## wxLua

wxWidget names are prefixed with `wx.` followed by the full wxWidget name (e.g. `wx.wxStaticText`).

wxLua does not support internationalization -- there is no support for `wxTranslate`.

## wxPHP

wxPHP is very similar to the C++ code, so the wxWidget names are identical.
