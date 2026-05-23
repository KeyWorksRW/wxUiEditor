/////////////////////////////////////////////////////////////////////////////
// Purpose:   FFI language strategy base class for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

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
