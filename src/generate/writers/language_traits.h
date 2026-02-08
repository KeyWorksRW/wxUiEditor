/////////////////////////////////////////////////////////////////////////////
// Purpose:   Language traits and strategy infrastructure for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file defines the LanguageTraits struct and LanguageStrategy base class,
// forming the foundation for scaling code generation from 4 to 9+ languages. LanguageTraits
// provides declarative data about each language's syntax and characteristics (literals,
// operators, formatting, feature flags), replacing direct language identity checks (is_cpp(),
// is_python()) with trait queries (get_traits().true_literal). LanguageStrategy is an abstract
// base for language-specific behavioral logic (construction, event binding, imports) that
// can't be reduced to simple data lookups. Together they implement a two-tier system:
// traits (data-driven) for "what does this language use?" and strategies (polymorphic) for
// "how does this language do X?".

#pragma once

#include <memory>

#include "gen_enums.h"  // Enumerations for generators

class Code;
class EmbeddedImage;
class Node;
class NodeEvent;
class WriteCode;

// Shut of lint warnings which don't want structure members to be public
// NOLINTBEGIN
// cppcheck-suppress-begin *

// Describes a language's static characteristics declaratively.
// Used by the Code class to replace direct language identity checks with trait queries.
struct LanguageTraits
{
    GenLang language;

    // Literal values used in code generation
    std::string_view true_literal;   // "true", "True", "1", ".TRUE."
    std::string_view false_literal;  // "false", "False", "0", ".FALSE."
    std::string_view null_literal;   // "nullptr", "None", "nil", "undef", "C_NULL_PTR"
    std::string_view empty_string;   // "wxEmptyString", "\"\""

    // Object/member access
    std::string_view self_reference;   // "this", "self", "$self"
    std::string_view member_operator;  // "->", ".", "%"
    std::string_view scope_operator;   // "::", "."

    // Variable declaration keyword for local variables
    std::string_view local_var_keyword;  // "auto* ", "my $", "", "let "

    // Comment prefix (includes trailing space)
    std::string_view line_comment;  // "// ", "# ", "! ", "-- "

    // Code formatting defaults
    int indent_size;     // 2 for Ruby, 4 for most
    size_t line_offset;  // Indent multiplier for line break calculation

    // Statement terminator appended after function calls/declarations
    std::string_view stmt_end;  // ";", "" (most scripting languages)

    // Construction suffix appended after class name in object creation
    std::string_view construction_suffix;  // ".new" (Ruby), "" (most languages)

    // Language prefix for wxWidgets identifiers
    std::string_view wx_prefix;  // "wx", "Wx::", "wx.", "wx_"

    // Logical operators (with surrounding spaces)
    std::string_view logical_and;  // " && ", " and "
    std::string_view logical_or;   // " || ", " or "

    // Block structure delimiters
    std::string_view block_begin;  // "{", ""
    std::string_view block_end;    // "}", "end", ""

    // Conditional statement syntax
    std::string_view conditional_begin;  // "if (", "if "
    std::string_view conditional_end;    // ")", ":", ""

    // Language family — reflects calling convention to wxWidgets
    enum class Family : std::uint8_t
    {
        native_cpp,         // C++ (only language with direct wxWidgets access)
        wx_binding,         // wxPython, wxRuby (C wrapper, near-full parity)
        wx_binding_legacy,  // wxPerl (legacy, replaced by Perl via kwxFFI)
        ffi                 // All kwxFFI languages (identical naming/constants)
    };
    Family family;

    // Feature coverage relative to the C++ reference implementation
    enum class FeatureParity : std::uint8_t
    {
        full,       // C++ and kwxFFI languages — everything supported
        near_full,  // Python, Ruby — wrapper gaps exist
        declining   // wxPerl — being replaced
    };
    FeatureParity feature_parity;

    // How widgets are instantiated
    enum class ConstructionStyle : std::uint8_t
    {
        cpp_new,       // new wxButton(parent, ...)
        binding_call,  // wx.Button(parent, ...) or Wx::Button.new(parent, ...)
        ffi_function   // wx_button_create(parent, ...)
    };
    ConstructionStyle construction_style;

    // Feature flags
    bool supports_markup;          // SetLabelMarkup support
    bool supports_lambda_events;   // Inline event handlers
    bool needs_explicit_types;     // Variable type declarations required
    bool has_header_files;         // Separate .h/.hpp files
    bool uses_snake_case_methods;  // Method names converted to snake_case
    bool removes_empty_parens;     // Ruby: remove trailing "()" from method calls
    bool supports_namespaces;      // Language has namespace/module support
    bool supports_classes;         // Language has class/object support

    // Convenience queries

    [[nodiscard]] auto is_cpp_family() const -> bool { return family == Family::native_cpp; }
    [[nodiscard]] auto is_ffi_family() const -> bool { return family == Family::ffi; }

    [[nodiscard]] auto is_binding_family() const -> bool
    {
        return family == Family::wx_binding || family == Family::wx_binding_legacy;
    }

    [[nodiscard]] auto has_full_parity() const -> bool
    {
        return feature_parity == FeatureParity::full;
    }
};

// NOLINTEND
// cppcheck-suppress-end *

// Returns traits for the specified language. Returns nullptr for non-code-generation
// languages (XRC, XML) or unrecognized values.
[[nodiscard]] auto GetLanguageTraits(GenLang language) -> const LanguageTraits*;

// Abstract base class for language-specific code generation strategies.
// Concrete implementations are created per-language (CppStrategy, PythonStrategy, etc.)
// via CreateLanguageStrategy(). The hierarchy is:
//
//   LanguageStrategy (abstract)
//   ├── CppStrategy                     (direct C++ — unique)
//   └── CWrapperStrategy (abstract)     (shared: all non-C++ go through C wrappers)
//       ├── WxBindingStrategy (abstract)(CPython/Ruby C APIs, near-full parity)
//       │   ├── PythonStrategy
//       │   └── RubyStrategy
//       └── FFIStrategy (abstract)      (kwxFFI C API, full C++ parity)
//           ├── FortranStrategy
//           ├── GoStrategy
//           ├── JuliaStrategy
//           ├── LuaJITStrategy
//           ├── PerlStrategy
//           └── RustStrategy
//
// The strategy pattern handles complex behavioral differences that can't be reduced to
// simple data lookups in LanguageTraits — such as construction patterns, event binding,
// import systems, platform conditionals, and feature-gap workarounds.
class LanguageStrategy
{
public:
    explicit LanguageStrategy(const LanguageTraits& traits) : m_traits(traits) {}
    virtual ~LanguageStrategy() = default;

    [[nodiscard]] auto get_traits() const -> const LanguageTraits& { return m_traits; }

    // ---- Core code generation (Phase 4 stubs, migrated incrementally) ----

    // Emit widget construction code (new/create/init patterns vary per language family)
    virtual void EmitConstruction(Code& code, Node* node) = 0;

    // Emit event binding code (Bind/connect/signal patterns vary per language)
    virtual void EmitEventBinding(Code& code, Node* node, NodeEvent* event) = 0;

    // Emit import/module/use statement for the given module name
    virtual void EmitImport(Code& code, std::string_view module) = 0;

    // Emit variable declaration with language-appropriate syntax
    virtual void EmitVarDecl(Code& code, std::string_view type, std::string_view name) = 0;

    // Query whether a specific feature/property is supported in this language
    [[nodiscard]] virtual auto IsFeatureSupported(Node* node, GenEnum::PropName prop) -> bool = 0;

    // ---- Platform conditionals ----

    // Emit platform-conditional begin (e.g., #if defined(__WINDOWS__), if wx.Platform == "msw")
    virtual void EmitPlatformBegin(Code& code, std::string_view platforms) = 0;

    // Emit platform-conditional end (e.g., #endif, unindent, "end")
    virtual void EmitPlatformEnd(WriteCode* writer) = 0;

    // ---- Class name mapping ----

    // Map a wxWidgets class name to language-appropriate form.
    // e.g., "wxButton" → "wxButton" (C++), "wx.Button" (Python), "Wx::Button" (Ruby)
    [[nodiscard]] virtual auto MapClassName(std::string_view wx_class_name) -> std::string = 0;

    // ---- Image data emission ----

    // Write embedded image data for a single image (raw bytes, base64, etc.)
    virtual void EmitImageData(Code& code, const EmbeddedImage* embed, WriteCode* writer) = 0;

protected:
    const LanguageTraits& m_traits;

    LanguageStrategy(const LanguageStrategy&) = default;
    auto operator=(const LanguageStrategy&) -> LanguageStrategy& = default;
    LanguageStrategy(LanguageStrategy&&) = default;
    auto operator=(LanguageStrategy&&) -> LanguageStrategy& = default;
};

// Factory: create the appropriate LanguageStrategy for the given language.
// Returns nullptr for non-code-generation languages (XRC, XML).
[[nodiscard]] auto CreateLanguageStrategy(GenLang language) -> std::unique_ptr<LanguageStrategy>;
