/////////////////////////////////////////////////////////////////////////////
// Purpose:   LuaJIT language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-30-2026]

#include "strategy_luajit.h"

#include "code.h"        // Code -- Helper class for generating code
#include "write_code.h"  // WriteCode class

LuaJITStrategy::LuaJITStrategy(const LanguageTraits& traits) : FFIStrategy(traits) {}

std::string LuaJITStrategy::MapClassName(std::string_view wx_class_name)
{
    // wxButton → Button (LuaJIT uses module-qualified: wx.Frame)
    if (!wx_class_name.starts_with("wx"))
    {
        return std::string(wx_class_name);
    }

    return std::string(wx_class_name.substr(2));
}

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
                code << m_traits.logical_or;
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
    auto alias = module;
    if (auto dot = alias.rfind('.'); dot != std::string_view::npos)
        alias = alias.substr(dot + 1);
    code.Str("local ").Str(alias).Str(" = require(\"").Str(module).Str("\")");
}

void LuaJITStrategy::EmitVarDecl(Code& code, std::string_view /* type */, std::string_view name)
{
    // LuaJIT: local name
    code.Str("local ").Str(name);
}
