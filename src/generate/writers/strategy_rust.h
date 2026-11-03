/////////////////////////////////////////////////////////////////////////////
// Purpose:   Rust language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "strategy_ffi.h"

class RustStrategy : public FFIStrategy
{
public:
    explicit RustStrategy(const LanguageTraits& traits);

    // wxButton -> Button (Rust wrapper uses bare type names)
    [[nodiscard]] std::string MapClassName(std::string_view wx_class_name) override;
};
