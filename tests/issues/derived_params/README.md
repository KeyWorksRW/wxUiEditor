# Issue 1484: derived_params not used during code generation

[Issue 1484](https://github.com/KeyWorksRW/wxUiEditor/issues/1484)

## Problem

A main frame whose generated base class inherits from `wxDocParentFrame` did not compile because
the generated constructor and `Create()` omitted the `wxDocManager*` parameter that
`wxDocParentFrame::Create()` requires, and the `derived_params` value was ignored during
generation. The supported fix is to use the `wxDocParentFrame` form class: the generated base class
now derives from `wxDocParentFrame` and takes `wxDocManager* manager, wxFrame* parent` in both the
constructor and `Create()`.

## Test project

`tests/issues/derived_params/derived_params.wxui` — one `wxDocParentFrame` form
(`DerivedParamsBase`, derived class `DerivedParams`) containing a `wxBoxSizer` and a
`wxStaticText`. The trigger is the form class `wxDocParentFrame` together with
`use_derived_class=1`, `derived_class_name`, and `derived_file`.

## How to verify

1. Run `bin\Debug\wxUiEditorD.exe tests\issues\derived_params\derived_params.wxui`.
   The project must load and the form must show `wxDocParentFrame` as its base class.
2. Inspect `derived_params_base.h`: it must contain
   `class DerivedParamsBase : public wxDocParentFrame` and both the constructor and
   `bool Create(...)` must begin with `wxDocManager* manager, wxFrame* parent`.
   Pre-fix: the class still derived from `wxDocParentFrame`, but the generated signature had only
   `wxWindow* parent` (no manager), so `wxDocParentFrame::Create()` did not compile.
3. Run `bin\Debug\wxUiEditorD.exe --gen_cpp tests\issues\derived_params\derived_params.wxui`.
   It must exit 0 and leave `derived_params_base.h` / `.cpp` unchanged.
4. Run `bin\Debug\wxUiEditorD.exe --verify_cpp tests\issues\derived_params\derived_params.wxui`.
   It must exit 0, confirming the generated code matches the committed golden files.

## Files

- `derived_params.wxui` — the project (one wxDocParentFrame form with a derived class).
- `derived_params_base.h` — golden generated header.
- `derived_params_base.cpp` — golden generated source.
- `README.md` — this file.
