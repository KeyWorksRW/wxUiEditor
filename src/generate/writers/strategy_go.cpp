/////////////////////////////////////////////////////////////////////////////
// Purpose:   Go language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-30-2026]

#include "strategy_go.h"

#include "code.h"        // Code -- Helper class for generating code
#include "write_code.h"  // WriteCode class

GoStrategy::GoStrategy(const LanguageTraits& traits) : FFIStrategy(traits) {}

std::string GoStrategy::MapClassName(std::string_view wx_class_name)
{
    // wxButton → Button (Go uses package-qualified: wx.Button)
    if (!wx_class_name.starts_with("wx"))
    {
        return std::string(wx_class_name);
    }

    return std::string(wx_class_name.substr(2));
}

void GoStrategy::EmitPlatformBegin(Code& code, std::string_view platforms)
{
    // Go: runtime.GOOS check
    bool has_prior = false;

    auto emit_condition = [&](std::string_view platform_name, std::string_view goos_value)
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
            code << "runtime.GOOS == \"" << goos_value << "\"";
        }
    };

    emit_condition("Windows", "windows");
    emit_condition("Unix", "linux");
    emit_condition("Mac", "darwin");

    if (has_prior)
    {
        code << " {";
    }
}

void GoStrategy::EmitImport(Code& code, std::string_view module)
{
    // Go: import "module"
    code.Str("import \"").Str(module).Str("\"");
}

void GoStrategy::EmitVarDecl(Code& code, std::string_view type, std::string_view name)
{
    // Go: var name type  or  name := value (short declaration handled by caller)
    if (type.empty())
    {
        code.Str("var ").Str(name);
    }
    else
    {
        code.Str("var ").Str(name).Str(" ").Str(type);
    }
}
