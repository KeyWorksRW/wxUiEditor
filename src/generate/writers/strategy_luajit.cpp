/////////////////////////////////////////////////////////////////////////////
// Purpose:   LuaJIT language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "strategy_luajit.h"

#include "code.h"        // Code -- Helper class for generating code
#include "write_code.h"  // WriteCode class

LuaJITStrategy::LuaJITStrategy(const LanguageTraits& traits) : FFIStrategy(traits) {}

void LuaJITStrategy::EmitPlatformBegin(Code& code, std::string_view platforms)
{
    // LuaJIT: ffi.os check
    bool has_prior = false;

    auto emit_condition = [&](std::string_view platform_name, std::string_view os_value)
    {
        if (platforms.find(platform_name) != std::string_view::npos)
        {
            if (has_prior)
            {
                code << " or ";
            }
            else
            {
                code.Eol() << "if ";
                has_prior = true;
            }
            code << "ffi.os == \"" << os_value << "\"";
        }
    };

    emit_condition("Windows", "Windows");
    emit_condition("Unix", "Linux");
    emit_condition("Mac", "OSX");

    if (has_prior)
    {
        code << " then";
    }
}

void LuaJITStrategy::EmitImport(Code& code, std::string_view module)
{
    // LuaJIT: local module = require("module")
    code.Str("local ").Str(module).Str(" = require(\"").Str(module).Str("\")");
}

void LuaJITStrategy::EmitVarDecl(Code& code, std::string_view /* type */, std::string_view name)
{
    // LuaJIT: local name
    code.Str("local ").Str(name);
}
