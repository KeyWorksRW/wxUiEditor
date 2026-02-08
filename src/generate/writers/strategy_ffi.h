/////////////////////////////////////////////////////////////////////////////
// Purpose:   FFI language strategy base class for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: FFIStrategy is the abstract base for all 6 kwxFFI-based languages (Fortran,
// Go, Julia, LuaJIT, Perl, Rust). These languages access wxWidgets through the kwxFFI C
// API, which provides complete wxWidgets coverage. All 6 share identical constant names and
// function names (one shared convention), full C++ feature parity, and an opaque handle
// model. The only per-language differences are pure syntax: variable declarations, callbacks,
// imports, string handling. FFIStrategy inherits from CWrapperStrategy, capturing shared
// non-C++ patterns (no headers, single output file), and adds FFI-specific behaviors:
// wx_<classname> construction, wx_ prefix mapping, and full feature support.

#pragma once

#include "strategy_cwrapper.h"

class FFIStrategy : public CWrapperStrategy
{
public:
    explicit FFIStrategy(const LanguageTraits& traits);

    // Maps wxWidgets class names to FFI convention: wxFooBar → wx_foo_bar
    [[nodiscard]] auto MapClassName(std::string_view wx_class_name) -> std::string override;

    // FFI languages target full C++ feature parity — all features supported
    [[nodiscard]] auto IsFeatureSupported(Node* node, GenEnum::PropName prop) -> bool override;

    // Default FFI platform conditional (per-language strategies override for syntax)
    void EmitPlatformBegin(Code& code, std::string_view platforms) override;

    // Default FFI platform end (per-language strategies override for syntax)
    void EmitPlatformEnd(WriteCode* writer) override;

    // Default FFI import (per-language strategies override)
    void EmitImport(Code& code, std::string_view module) override;

    // Default FFI variable declaration (per-language strategies override)
    void EmitVarDecl(Code& code, std::string_view type, std::string_view name) override;
};
