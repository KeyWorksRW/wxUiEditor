/////////////////////////////////////////////////////////////////////////////
// Purpose:   Rust language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "strategy_rust.h"

#include <string>
#include <string_view>

RustStrategy::RustStrategy(const LanguageTraits& traits) : FFIStrategy(traits) {}

std::string RustStrategy::MapClassName(std::string_view wx_class_name)
{
    // wxButton -> Button (Rust wrapper uses bare type names)
    if (!wx_class_name.starts_with("wx"))
    {
        return std::string(wx_class_name);
    }

    return std::string(wx_class_name.substr(2));
}
