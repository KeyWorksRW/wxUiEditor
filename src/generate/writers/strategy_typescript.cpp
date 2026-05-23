/////////////////////////////////////////////////////////////////////////////
// Purpose:   TypeScript language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "strategy_typescript.h"

#include "code.h"        // Code -- Helper class for generating code
#include "write_code.h"  // WriteCode class

TypeScriptStrategy::TypeScriptStrategy(const LanguageTraits& traits) : FFIStrategy(traits) {}

void TypeScriptStrategy::EmitPlatformBegin(Code& code, std::string_view platforms)
{
    // TypeScript: process.platform checks
    bool has_prior = false;

    auto emit_condition = [&](std::string_view platform_name, std::string_view node_platform)
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
            code << "process.platform === \"" << node_platform << "\"";
        }
    };

    emit_condition("Windows", "win32");
    emit_condition("Unix", "linux");
    emit_condition("Mac", "darwin");

    if (has_prior)
    {
        code << ") {";
    }
}

void TypeScriptStrategy::EmitImport(Code& code, std::string_view module)
{
    // TypeScript: import * as module from 'path';
    code.Str("import * as ").Str(module);
}

void TypeScriptStrategy::EmitVarDecl(Code& code, std::string_view type, std::string_view name)
{
    // TypeScript: let name: Type  or  const name
    if (type.empty())
    {
        code.Str("const ").Str(name);
    }
    else
    {
        code.Str("let ").Str(name).Str(": ").Str(type);
    }
}