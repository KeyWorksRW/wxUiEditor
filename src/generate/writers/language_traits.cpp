/////////////////////////////////////////////////////////////////////////////
// Purpose:   Language traits and strategy infrastructure for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "language_traits.h"

#include "strategy_cpp.h"
#include "strategy_fortran.h"
#include "strategy_go.h"
#include "strategy_julia.h"
#include "strategy_luajit.h"
#include "strategy_perl.h"
#include "strategy_python.h"
#include "strategy_ruby.h"
#include "strategy_rust.h"

// clang-format off
namespace
{

// ---- Active language traits (connected via GetLanguageTraits) ----

constexpr LanguageTraits cpp_traits
{
    .language = GEN_LANG_CPLUSPLUS,
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
    .uses_snake_case_methods = false,
    .removes_empty_parens = false,
    .supports_namespaces = true,
    .supports_classes = true,
};



constexpr LanguageTraits python_traits
{
    .language = GEN_LANG_PYTHON,
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
    .uses_snake_case_methods = false,
    .removes_empty_parens = false,
    .supports_namespaces = true,
    .supports_classes = true,
};

constexpr LanguageTraits ruby_traits
{
    .language = GEN_LANG_RUBY,
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
    .uses_snake_case_methods = true,
    .removes_empty_parens = true,
    .supports_namespaces = true,
    .supports_classes = true,
};

// ---- FFI language trait stubs ----
// Phase 5 will assign GenLang values and wire these into GetLanguageTraits().
// All FFI languages share identical constant/function naming via kwxFFI.

constexpr LanguageTraits fortran_traits
{
    .language = GEN_LANG_FORTRAN,
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
    .uses_snake_case_methods = true,
    .removes_empty_parens = false,
    .supports_namespaces = true,
    .supports_classes = true,
};

constexpr LanguageTraits go_traits
{
    .language = GEN_LANG_GO,
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
    .wx_prefix = "wx_",
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
    .uses_snake_case_methods = true,
    .removes_empty_parens = false,
    .supports_namespaces = true,
    .supports_classes = false,
};

constexpr LanguageTraits julia_traits
{
    .language = GEN_LANG_JULIA,
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
    .wx_prefix = "wx_",
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
    .uses_snake_case_methods = true,
    .removes_empty_parens = false,
    .supports_namespaces = true,
    .supports_classes = true,
};

constexpr LanguageTraits luajit_traits
{
    .language = GEN_LANG_LUAJIT,
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
    .wx_prefix = "wx_",
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
    .uses_snake_case_methods = true,
    .removes_empty_parens = false,
    .supports_namespaces = false,
    .supports_classes = true,
};

constexpr LanguageTraits perl_traits
{
    .language = GEN_LANG_PERL,
    .true_literal = "1",
    .false_literal = "0",
    .null_literal = "undef",
    .empty_string = "\"\"",
    .self_reference = "$self",
    .member_operator = "->",
    .scope_operator = "::",
    .local_var_keyword = "my ",
    .line_comment = "# ",
    .indent_size = 4,
    .line_offset = 1,
    .stmt_end = ";",
    .construction_suffix = "",
    .wx_prefix = "wx_",
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
    .supports_lambda_events = false,
    .needs_explicit_types = false,
    .has_header_files = false,
    .uses_snake_case_methods = true,
    .removes_empty_parens = false,
    .supports_namespaces = true,
    .supports_classes = true,
};

constexpr LanguageTraits rust_traits
{
    .language = GEN_LANG_RUST,
    .true_literal = "true",
    .false_literal = "false",
    .null_literal = "std::ptr::null_mut()",
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
    .wx_prefix = "wx_",
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
    .supports_lambda_events = true,
    .needs_explicit_types = true,
    .has_header_files = false,
    .uses_snake_case_methods = true,
    .removes_empty_parens = false,
    .supports_namespaces = true,
    .supports_classes = false,
};

    // clang-format on

}  // anonymous namespace

auto GetLanguageTraits(GenLang language) -> const LanguageTraits*
{
    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
            return &cpp_traits;

        case GEN_LANG_PYTHON:
            return &python_traits;
        case GEN_LANG_RUBY:
            return &ruby_traits;

        case GEN_LANG_FORTRAN:
            return &fortran_traits;
        case GEN_LANG_GO:
            return &go_traits;
        case GEN_LANG_JULIA:
            return &julia_traits;
        case GEN_LANG_LUAJIT:
            return &luajit_traits;
        case GEN_LANG_PERL:
            return &perl_traits;
        case GEN_LANG_RUST:
            return &rust_traits;

        default:
            return nullptr;
    }
}

auto CreateLanguageStrategy(GenLang language) -> std::unique_ptr<LanguageStrategy>
{
    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
            return std::make_unique<CppStrategy>(cpp_traits);

        case GEN_LANG_PYTHON:
            return std::make_unique<PythonStrategy>(python_traits);

        case GEN_LANG_RUBY:
            return std::make_unique<RubyStrategy>(ruby_traits);

        case GEN_LANG_FORTRAN:
            return std::make_unique<FortranStrategy>(fortran_traits);

        case GEN_LANG_GO:
            return std::make_unique<GoStrategy>(go_traits);

        case GEN_LANG_JULIA:
            return std::make_unique<JuliaStrategy>(julia_traits);

        case GEN_LANG_LUAJIT:
            return std::make_unique<LuaJITStrategy>(luajit_traits);

        case GEN_LANG_PERL:
            return std::make_unique<PerlStrategy>(perl_traits);

        case GEN_LANG_RUST:
            return std::make_unique<RustStrategy>(rust_traits);

        default:
            return nullptr;
    }
}
