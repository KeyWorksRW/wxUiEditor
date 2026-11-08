/////////////////////////////////////////////////////////////////////////////
// Purpose:   Language traits and strategy infrastructure for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-30-2026]

#include "language_traits.h"

#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "strategy_cpp.h"
#include "strategy_fortran.h"
#include "strategy_go.h"
#include "strategy_julia.h"
#include "strategy_luajit.h"
#include "strategy_python.h"
#include "strategy_ruby.h"
#include "strategy_rust.h"
#include "strategy_typescript.h"
#include "write_code.h"  // WriteCode class

// clang-format off


// ---- Active language traits (connected via GetLanguageTraits) ----

static constexpr LanguageTraits cpp_traits
{
    .language = GenLang::cplusplus,
    .true_literal = "true",
    .false_literal = "false",
    .null_literal = "nullptr",
    .empty_string = "wxEmptyString",
    .self_reference = "this",
    .member_operator = "->",
    .scope_operator = "::",
    .local_var_keyword = "auto* ",
    .line_comment = "// ",
    .indent_size = 4,
    .line_offset = 1,
    .stmt_end = ";",
    .construction_suffix = "",
    .wx_prefix = "wx",
    .class_prefix = "wx",
    .logical_and = " && ",
    .logical_or = " || ",
    .block_begin = "{",
    .block_end = "}",
    .conditional_begin = "if (",
    .conditional_end = ")",
    .family = LanguageTraits::Family::native_cpp,
    .feature_parity = LanguageTraits::FeatureParity::full,
    .construction_style = LanguageTraits::ConstructionStyle::cpp_new,
    .supports_markup = true,
    .supports_lambda_events = true,
    .needs_explicit_types = true,
    .has_header_files = true,
    .method_case = LanguageTraits::MethodCase::pascal_case,
    .removes_empty_parens = false,
    .supports_namespaces = true,
    .supports_classes = true,
};



static constexpr LanguageTraits python_traits
{
    .language = GenLang::python,
    .true_literal = "True",
    .false_literal = "False",
    .null_literal = "None",
    .empty_string = "\"\"",
    .self_reference = "self",
    .member_operator = ".",
    .scope_operator = ".",
    .local_var_keyword = "",
    .line_comment = "# ",
    .indent_size = 4,
    .line_offset = 2,
    .stmt_end = "",
    .construction_suffix = "",
    .wx_prefix = "wx.",
    .class_prefix = "wx.",
    .logical_and = " and ",
    .logical_or = " or ",
    .block_begin = "",
    .block_end = "",
    .conditional_begin = "if ",
    .conditional_end = ":",
    .family = LanguageTraits::Family::wx_binding,
    .feature_parity = LanguageTraits::FeatureParity::near_full,
    .construction_style = LanguageTraits::ConstructionStyle::binding_call,
    .supports_markup = true,
    .supports_lambda_events = false,
    .needs_explicit_types = false,
    .has_header_files = false,
    .method_case = LanguageTraits::MethodCase::pascal_case,
    .removes_empty_parens = false,
    .supports_namespaces = true,
    .supports_classes = true,
};

static constexpr LanguageTraits ruby_traits
{
    .language = GenLang::ruby,
    .true_literal = "true",
    .false_literal = "false",
    .null_literal = "nil",
    .empty_string = "\"\"",
    .self_reference = "self",
    .member_operator = ".",
    .scope_operator = "::",
    .local_var_keyword = "",
    .line_comment = "# ",
    .indent_size = 2,
    .line_offset = 2,
    .stmt_end = "",
    .construction_suffix = ".new",
    .wx_prefix = "Wx::",
    .class_prefix = "Wx::",
    .logical_and = " && ",
    .logical_or = " || ",
    .block_begin = "",
    .block_end = "end",
    .conditional_begin = "if ",
    .conditional_end = "",
    .family = LanguageTraits::Family::wx_binding,
    .feature_parity = LanguageTraits::FeatureParity::near_full,
    .construction_style = LanguageTraits::ConstructionStyle::binding_call,
    .supports_markup = true,
    .supports_lambda_events = false,
    .needs_explicit_types = false,
    .has_header_files = false,
    .method_case = LanguageTraits::MethodCase::snake_case,
    .removes_empty_parens = true,
    .supports_namespaces = true,
    .supports_classes = true,
};

// ---- FFI language trait stubs ----
// Phase 5 will assign GenLang values and wire these into GetLanguageTraits().
// All FFI languages share identical constant/function naming via kwxFFI.

static constexpr LanguageTraits fortran_traits
{
    .language = GenLang::fortran,
    .true_literal = ".TRUE.",
    .false_literal = ".FALSE.",
    .null_literal = "C_NULL_PTR",
    .empty_string = "\"\"",
    .self_reference = "",
    .member_operator = "%",
    .scope_operator = "%",
    .local_var_keyword = "",
    .line_comment = "! ",
    .indent_size = 3,
    .line_offset = 1,
    .stmt_end = "",
    .construction_suffix = "",
    .wx_prefix = "wx_",
    .class_prefix = "wx_",
    .logical_and = " .AND. ",
    .logical_or = " .OR. ",
    .block_begin = "",
    .block_end = "",
    .conditional_begin = "if (",
    .conditional_end = ") then",
    .family = LanguageTraits::Family::ffi,
    .feature_parity = LanguageTraits::FeatureParity::full,
    .construction_style = LanguageTraits::ConstructionStyle::ffi_function,
    .supports_markup = true,
    .supports_lambda_events = false,
    .needs_explicit_types = true,
    .has_header_files = false,
    .method_case = LanguageTraits::MethodCase::snake_case,
    .removes_empty_parens = false,
    .supports_namespaces = true,
    .supports_classes = true,
};

static constexpr LanguageTraits go_traits
{
    .language = GenLang::go,
    .true_literal = "true",
    .false_literal = "false",
    .null_literal = "nil",
    .empty_string = "\"\"",
    .self_reference = "",
    .member_operator = ".",
    .scope_operator = ".",
    .local_var_keyword = "",
    .line_comment = "// ",
    .indent_size = 4,
    .line_offset = 1,
    .stmt_end = "",
    .construction_suffix = "",
    .wx_prefix = "wx.",
    .class_prefix = "wx.",
    .logical_and = " && ",
    .logical_or = " || ",
    .block_begin = "{",
    .block_end = "}",
    .conditional_begin = "if ",
    .conditional_end = "",
    .family = LanguageTraits::Family::ffi,
    .feature_parity = LanguageTraits::FeatureParity::full,
    .construction_style = LanguageTraits::ConstructionStyle::ffi_function,
    .supports_markup = true,
    .supports_lambda_events = false,
    .needs_explicit_types = true,
    .has_header_files = false,
    .method_case = LanguageTraits::MethodCase::pascal_case,
    .removes_empty_parens = false,
    .supports_namespaces = true,
    .supports_classes = false,
};

static constexpr LanguageTraits julia_traits
{
    .language = GenLang::julia,
    .true_literal = "true",
    .false_literal = "false",
    .null_literal = "nothing",
    .empty_string = "\"\"",
    .self_reference = "",
    .member_operator = ".",
    .scope_operator = ".",
    .local_var_keyword = "local ",
    .line_comment = "# ",
    .indent_size = 4,
    .line_offset = 1,
    .stmt_end = "",
    .construction_suffix = "",
    .wx_prefix = "wx.",
    .class_prefix = "wx.",
    .logical_and = " && ",
    .logical_or = " || ",
    .block_begin = "",
    .block_end = "end",
    .conditional_begin = "if ",
    .conditional_end = "",
    .family = LanguageTraits::Family::ffi,
    .feature_parity = LanguageTraits::FeatureParity::full,
    .construction_style = LanguageTraits::ConstructionStyle::ffi_function,
    .supports_markup = true,
    .supports_lambda_events = false,
    .needs_explicit_types = false,
    .has_header_files = false,
    .method_case = LanguageTraits::MethodCase::snake_case,
    .removes_empty_parens = false,
    .supports_namespaces = true,
    .supports_classes = true,
};

static constexpr LanguageTraits luajit_traits
{
    .language = GenLang::luajit,
    .true_literal = "true",
    .false_literal = "false",
    .null_literal = "nil",
    .empty_string = "\"\"",
    .self_reference = "self",
    .member_operator = ":",
    .scope_operator = ".",
    .local_var_keyword = "local ",
    .line_comment = "-- ",
    .indent_size = 4,
    .line_offset = 1,
    .stmt_end = "",
    .construction_suffix = "",
    .wx_prefix = "wx.",
    .class_prefix = "wx.",
    .logical_and = " and ",
    .logical_or = " or ",
    .block_begin = "",
    .block_end = "end",
    .conditional_begin = "if ",
    .conditional_end = " then",
    .family = LanguageTraits::Family::ffi,
    .feature_parity = LanguageTraits::FeatureParity::full,
    .construction_style = LanguageTraits::ConstructionStyle::ffi_function,
    .supports_markup = true,
    .supports_lambda_events = false,
    .needs_explicit_types = false,
    .has_header_files = false,
    .method_case = LanguageTraits::MethodCase::snake_case,
    .removes_empty_parens = false,
    .supports_namespaces = false,
    .supports_classes = true,
};

static constexpr LanguageTraits rust_traits
{
    .language = GenLang::rust,
    .true_literal = "true",
    .false_literal = "false",
    .null_literal = "None",
    .empty_string = "\"\"",
    .self_reference = "self",
    .member_operator = ".",
    .scope_operator = "::",
    .local_var_keyword = "let ",
    .line_comment = "// ",
    .indent_size = 4,
    .line_offset = 1,
    .stmt_end = ";",
    .construction_suffix = "",
    .wx_prefix = "wx",
    .class_prefix = "wx",
    .logical_and = " && ",
    .logical_or = " || ",
    .block_begin = "{",
    .block_end = "}",
    .conditional_begin = "if ",
    .conditional_end = "",
    .family = LanguageTraits::Family::ffi,
    .feature_parity = LanguageTraits::FeatureParity::full,
    .construction_style = LanguageTraits::ConstructionStyle::ffi_function,
    .supports_markup = true,
    .supports_lambda_events = false,
    .needs_explicit_types = false,
    .has_header_files = false,
    .method_case = LanguageTraits::MethodCase::snake_case,
    .removes_empty_parens = false,
    .supports_namespaces = true,
    .supports_classes = true,
};

static constexpr LanguageTraits typescript_traits
{
    .language = GenLang::typescript,
    .true_literal = "true",
    .false_literal = "false",
    .null_literal = "null",
    .empty_string = "\"\"",
    .self_reference = "this",
    .member_operator = ".",
    .scope_operator = ".",
    .local_var_keyword = "let ",
    .line_comment = "// ",
    .indent_size = 4,
    .line_offset = 1,
    .stmt_end = ";",
    .construction_suffix = "",
    .wx_prefix = "",
    .class_prefix = "wx",
    .logical_and = " && ",
    .logical_or = " || ",
    .block_begin = "{",
    .block_end = "}",
    .conditional_begin = "if (",
    .conditional_end = ")",
    .family = LanguageTraits::Family::ffi,
    .feature_parity = LanguageTraits::FeatureParity::full,
    .construction_style = LanguageTraits::ConstructionStyle::ffi_function,
    .supports_markup = true,
    .supports_lambda_events = true,
    .needs_explicit_types = true,
    .has_header_files = false,
    .method_case = LanguageTraits::MethodCase::pascal_case,
    .removes_empty_parens = false,
    .supports_namespaces = true,
    .supports_classes = true,
};

// clang-format on

const LanguageTraits* GetLanguageTraits(GenLang language)
{
    switch (language)
    {
        case GenLang::cplusplus:
            return &cpp_traits;

        case GenLang::python:
            return &python_traits;
        case GenLang::ruby:
            return &ruby_traits;

        case GenLang::fortran:
            return &fortran_traits;
        case GenLang::go:
            return &go_traits;
        case GenLang::julia:
            return &julia_traits;
        case GenLang::luajit:
            return &luajit_traits;
        case GenLang::rust:
            return &rust_traits;
        case GenLang::typescript:
            return &typescript_traits;

        default:
            return nullptr;
    }
}

// Maps a language to the project properties that control its generated formatting.
struct LanguageFormatProps
{
    GenEnum::PropName indent_prop;
    GenEnum::PropName function_case_prop;
};

static LanguageFormatProps GetLanguageFormatProps(GenLang language)
{
    switch (language)
    {
        case GenLang::cplusplus:
            return { GenEnum::prop_cpp_indent, GenEnum::prop_cpp_function_case };

        case GenLang::python:
            return { GenEnum::prop_python_indent, GenEnum::prop_python_function_case };

        case GenLang::ruby:
            return { GenEnum::prop_ruby_indent, GenEnum::prop_ruby_function_case };

        case GenLang::fortran:
            return { GenEnum::prop_fortran_indent, GenEnum::prop_fortran_function_case };

        case GenLang::go:
            return { GenEnum::prop_go_indent, GenEnum::prop_go_function_case };

        case GenLang::julia:
            return { GenEnum::prop_julia_indent, GenEnum::prop_julia_function_case };

        case GenLang::luajit:
            return { GenEnum::prop_lua_indent, GenEnum::prop_lua_function_case };

        case GenLang::rust:
            return { GenEnum::prop_rust_indent, GenEnum::prop_rust_function_case };

        case GenLang::typescript:
            return { GenEnum::prop_typescript_indent, GenEnum::prop_typescript_function_case };

        default:
            return { GenEnum::prop_unknown, GenEnum::prop_unknown };
    }
}

// Returns the stored project property value, or nullptr when the project node is missing or the
// property does not exist.
static const wxue::string* GetProjectPropValue(GenEnum::PropName prop_name)
{
    if (prop_name == GenEnum::prop_unknown)
    {
        return nullptr;
    }

    Node* project_node = Project.get_ProjectNode();
    if (!project_node || !project_node->HasProp(prop_name))
    {
        return nullptr;
    }

    return &project_node->as_string(prop_name);
}

int GetLanguageIndentSize(GenLang language)
{
    const LanguageTraits* traits = GetLanguageTraits(language);
    const int default_indent = traits ? traits->indent_size : 4;

    const wxue::string* value = GetProjectPropValue(GetLanguageFormatProps(language).indent_prop);
    if (!value || value->is_sameas("tab"))
    {
        return default_indent;
    }

    const int indent_spaces = value->atoi();
    if (indent_spaces < 2 || indent_spaces > 8)
    {
        return default_indent;
    }

    return indent_spaces;
}

bool GetLanguageIndentWithTabs(GenLang language)
{
    const wxue::string* value = GetProjectPropValue(GetLanguageFormatProps(language).indent_prop);
    return value && value->is_sameas("tab");
}

LanguageTraits::MethodCase GetLanguageMethodCase(GenLang language)
{
    const LanguageTraits* traits = GetLanguageTraits(language);
    const LanguageTraits::MethodCase default_case =
        traits ? traits->method_case : LanguageTraits::MethodCase::pascal_case;

    const wxue::string* value =
        GetProjectPropValue(GetLanguageFormatProps(language).function_case_prop);
    if (!value)
    {
        return default_case;
    }

    if (value->is_sameas("camelCase"))
    {
        return LanguageTraits::MethodCase::camel_case;
    }
    if (value->is_sameas("snake_case"))
    {
        return LanguageTraits::MethodCase::snake_case;
    }

    // "PascalCase" and any unrecognized value fall back to the language default.
    return default_case;
}

void SetLanguageIndent(WriteCode* source, GenLang language)
{
    if (!source)
    {
        return;
    }

    source->SetTabToSpaces(GetLanguageIndentSize(language), GetLanguageIndentWithTabs(language));
}

std::unique_ptr<LanguageStrategy> CreateLanguageStrategy(GenLang language)
{
    switch (language)
    {
        case GenLang::cplusplus:
            return std::make_unique<CppStrategy>(cpp_traits);

        case GenLang::python:
            return std::make_unique<PythonStrategy>(python_traits);

        case GenLang::ruby:
            return std::make_unique<RubyStrategy>(ruby_traits);

        case GenLang::fortran:
            return std::make_unique<FortranStrategy>(fortran_traits);

        case GenLang::go:
            return std::make_unique<GoStrategy>(go_traits);

        case GenLang::julia:
            return std::make_unique<JuliaStrategy>(julia_traits);

        case GenLang::luajit:
            return std::make_unique<LuaJITStrategy>(luajit_traits);

        case GenLang::rust:
            return std::make_unique<RustStrategy>(rust_traits);

        case GenLang::typescript:
            return std::make_unique<TypeScriptStrategy>(typescript_traits);

        default:
            return nullptr;
    }
}
