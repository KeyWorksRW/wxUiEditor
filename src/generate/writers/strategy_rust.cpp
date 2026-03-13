/////////////////////////////////////////////////////////////////////////////
// Purpose:   Rust language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "strategy_rust.h"

#include "code.h"        // Code -- Helper class for generating code
#include "write_code.h"  // WriteCode class

RustStrategy::RustStrategy(const LanguageTraits& traits) : FFIStrategy(traits) {}

void RustStrategy::EmitPlatformBegin(Code& code, std::string_view platforms)
{
    // Rust: cfg! macro for runtime checks
    bool has_prior = false;

    auto emit_condition = [&](std::string_view platform_name, std::string_view target_os)
    {
        if (platforms.find(platform_name) != std::string_view::npos)
        {
            if (has_prior)
            {
                code << " || ";
            }
            else
            {
                code.Eol() << "if ";
                has_prior = true;
            }
            code << "cfg!(target_os = \"" << target_os << "\")";
        }
    };

    emit_condition("Windows", "windows");
    emit_condition("Unix", "linux");
    emit_condition("Mac", "macos");

    if (has_prior)
    {
        code << " {";
    }
}

void RustStrategy::EmitImport(Code& code, std::string_view module)
{
    // Rust: use module;
    code.Str("use ").Str(module);
}

void RustStrategy::EmitVarDecl(Code& code, std::string_view type, std::string_view name)
{
    // Rust: let name: Type  or  let name (type inferred)
    if (type.empty())
    {
        code.Str("let ").Str(name);
    }
    else
    {
        code.Str("let ").Str(name).Str(": ").Str(type);
    }
}
