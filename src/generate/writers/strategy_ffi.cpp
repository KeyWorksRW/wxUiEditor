/////////////////////////////////////////////////////////////////////////////
// Purpose:   FFI language strategy base class for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <cctype>

#include "strategy_ffi.h"

#include "code.h"        // Code -- Helper class for generating code
#include "write_code.h"  // WriteCode class

FFIStrategy::FFIStrategy(const LanguageTraits& traits) : CWrapperStrategy(traits) {}

auto FFIStrategy::MapClassName(std::string_view wx_class_name) -> std::string
{
    if (!wx_class_name.starts_with("wx"))
    {
        return std::string(wx_class_name);
    }

    // Convert wxFooBar → wx_foo_bar
    // All kwxFFI languages use identical snake_case naming with wx_ prefix
    std::string result("wx_");
    auto name_part = wx_class_name.substr(2);  // Remove "wx" prefix

    for (size_t i = 0; i < name_part.size(); ++i)
    {
        auto ch = name_part[i];
        if (std::isupper(static_cast<unsigned char>(ch)))
        {
            if (i > 0)
            {
                result += '_';
            }
            result += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }
        else
        {
            result += ch;
        }
    }

    return result;
}

auto FFIStrategy::IsFeatureSupported(Node* /* node */, GenEnum::PropName /* prop */) -> bool
{
    // kwxFFI provides full C++ parity — all features are supported
    return true;
}

void FFIStrategy::EmitPlatformBegin(Code& code, std::string_view platforms)
{
    // Default FFI platform conditional — uses kwxFFI runtime platform check.
    // Per-language strategies override with language-appropriate syntax.
    bool has_prior = false;

    auto emit_condition = [&](std::string_view platform_name, std::string_view ffi_constant)
    {
        if (platforms.find(platform_name) != std::string_view::npos)
        {
            if (has_prior)
            {
                code << m_traits.logical_or;
            }
            else
            {
                code.Eol() << m_traits.conditional_begin;
                has_prior = true;
            }
            code << "wx_get_platform() == " << ffi_constant;
        }
    };

    emit_condition("Windows", "WX_PLATFORM_WINDOWS");
    emit_condition("Unix", "WX_PLATFORM_UNIX");
    emit_condition("Mac", "WX_PLATFORM_MAC");

    if (has_prior)
    {
        code << m_traits.conditional_end;
    }
}

void FFIStrategy::EmitPlatformEnd(WriteCode* writer)
{
    // Default: unindent and emit block_end if the language uses one
    writer->Unindent();
    if (!m_traits.block_end.empty())
    {
        writer->writeLine(std::string(m_traits.block_end));
    }
}

void FFIStrategy::EmitImport(Code& code, std::string_view module)
{
    // Default FFI import — per-language strategies override with proper syntax
    code.Str(module);
}

void FFIStrategy::EmitVarDecl(Code& code, std::string_view /* type */, std::string_view name)
{
    // Default FFI variable declaration — per-language strategies override
    code.Str(name);
}
