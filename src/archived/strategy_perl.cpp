/////////////////////////////////////////////////////////////////////////////
// Purpose:   Perl language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "strategy_perl.h"

#include "code.h"        // Code -- Helper class for generating code
#include "write_code.h"  // WriteCode class

PerlStrategy::PerlStrategy(const LanguageTraits& traits) : FFIStrategy(traits) {}

void PerlStrategy::EmitPlatformBegin(Code& code, std::string_view platforms)
{
    // Perl: $^O check
    bool has_prior = false;

    auto emit_condition = [&](std::string_view platform_name, std::string_view os_value)
    {
        if (platforms.find(platform_name) != std::string_view::npos)
        {
            if (has_prior)
            {
                code << " || ";
            }
            else
            {
                code.Eol() << "if (";
                has_prior = true;
            }
            code << "$^O eq \"" << os_value << "\"";
        }
    };

    emit_condition("Windows", "MSWin32");
    emit_condition("Unix", "linux");
    emit_condition("Mac", "darwin");

    if (has_prior)
    {
        code << ") {";
    }
}

void PerlStrategy::EmitImport(Code& code, std::string_view module)
{
    // Perl: use Module;
    code.Str("use ").Str(module);
}

void PerlStrategy::EmitVarDecl(Code& code, std::string_view /* type */, std::string_view name)
{
    // Perl: my $name
    code.Str("my $").Str(name);
}
