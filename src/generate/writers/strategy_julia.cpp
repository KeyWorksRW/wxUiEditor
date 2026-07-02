/////////////////////////////////////////////////////////////////////////////
// Purpose:   Julia language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-30-2026]

#include "strategy_julia.h"

#include "code.h"        // Code -- Helper class for generating code
#include "write_code.h"  // WriteCode class

JuliaStrategy::JuliaStrategy(const LanguageTraits& traits) : FFIStrategy(traits) {}

std::string JuliaStrategy::MapClassName(std::string_view wx_class_name)
{
    // wxButton → Button (Julia uses module-qualified: wx.Frame)
    if (!wx_class_name.starts_with("wx"))
    {
        return std::string(wx_class_name);
    }

    return std::string(wx_class_name.substr(2));
}

void JuliaStrategy::EmitPlatformBegin(Code& code, std::string_view platforms)
{
    // Julia: Sys.iswindows(), Sys.islinux(), Sys.isapple()
    bool has_prior = false;

    auto emit_condition = [&](std::string_view platform_name, std::string_view sys_check)
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
            code << sys_check;
        }
    };

    emit_condition("Windows", "Sys.iswindows()");
    emit_condition("Unix", "Sys.islinux()");
    emit_condition("Mac", "Sys.isapple()");
}

void JuliaStrategy::EmitImport(Code& code, std::string_view module)
{
    // Julia: using Module
    code.Str("using ").Str(module);
}

void JuliaStrategy::EmitVarDecl(Code& code, std::string_view /* type */, std::string_view name)
{
    // Julia is dynamically typed — no type declaration needed for local variables
    code.Str(name);
}
