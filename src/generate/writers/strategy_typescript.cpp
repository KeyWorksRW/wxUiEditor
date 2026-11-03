/////////////////////////////////////////////////////////////////////////////
// Purpose:   TypeScript language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-30-2026]

#include "strategy_typescript.h"

#include "code.h"        // Code -- Helper class for generating code
#include "write_code.h"  // WriteCode class

TypeScriptStrategy::TypeScriptStrategy(const LanguageTraits& traits) : FFIStrategy(traits) {}

std::string TypeScriptStrategy::MapClassName(std::string_view wx_class_name)
{
    // The kwx TypeScript corpus keeps the "wx" prefix on wrapper classes (wxFrame, wxButton),
    // so the name is passed through unchanged.
    return std::string(wx_class_name);
}

void TypeScriptStrategy::EmitPlatformBegin(Code& code, std::string_view platforms,
                                           std::string_view conditional)
{
    // TypeScript: compare against the runtime platform value imported from "kwx/platform".
    bool has_prior = false;
    const LanguageTraits& traits = code.get_traits();

    auto emit_condition = [&](std::string_view platform_name, std::string_view platform_value)
    {
        if (platforms.find(platform_name) != std::string_view::npos)
        {
            if (has_prior)
            {
                code << traits.logical_or;
            }
            else
            {
                code.Eol() << "if (";
                has_prior = true;
            }
            code << "platform === \"" << platform_value << "\"";
        }
    };

    emit_condition("Windows", "windows");
    emit_condition("Unix", "linux");
    emit_condition("Mac", "darwin");

    if (!conditional.empty())
    {
        if (has_prior)
        {
            code << traits.logical_and;
        }
        else
        {
            code.Eol() << "if (";
            has_prior = true;
        }
        code << conditional;
    }

    if (has_prior)
    {
        code << ") {";
    }
}

void TypeScriptStrategy::EmitConditionalOnly(Code& code, std::string_view conditional)
{
    code.Eol() << "if (" << conditional << ") {";
}

void TypeScriptStrategy::EmitImport(Code& code, std::string_view module)
{
    // TypeScript: import * as module from 'module';
    code.Str("import * as ").Str(module).Str(" from '").Str(module).Str("';");
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
