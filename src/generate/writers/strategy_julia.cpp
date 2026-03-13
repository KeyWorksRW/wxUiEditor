/////////////////////////////////////////////////////////////////////////////
// Purpose:   Julia language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "strategy_julia.h"

#include "code.h"        // Code -- Helper class for generating code
#include "write_code.h"  // WriteCode class

JuliaStrategy::JuliaStrategy(const LanguageTraits& traits) : FFIStrategy(traits) {}

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
                code << " || ";
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
