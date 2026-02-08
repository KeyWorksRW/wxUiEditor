/////////////////////////////////////////////////////////////////////////////
// Purpose:   Ruby language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "strategy_ruby.h"

#include "code.h"        // Code -- Helper class for generating code
#include "write_code.h"  // WriteCode class

RubyStrategy::RubyStrategy(const LanguageTraits& traits) : WxBindingStrategy(traits) {}

void RubyStrategy::EmitPlatformBegin(Code& code, std::string_view platforms)
{
    bool has_prior = false;

    if (platforms.find("Windows") != std::string_view::npos)
    {
        code.Eol() << "if Wx::PLATFORM == 'WXMSW'";
        has_prior = true;
    }
    if (platforms.find("Unix") != std::string_view::npos)
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
        code << "Wx::PLATFORM == 'WXUNIX'";
    }
    if (platforms.find("Mac") != std::string_view::npos)
    {
        if (has_prior)
        {
            code << " || ";
        }
        else
        {
            code.Eol() << "if ";
        }
        code << "Wx::PLATFORM == 'WXOSX'";
    }
}

void RubyStrategy::EmitPlatformEnd(WriteCode* writer)
{
    writer->Unindent();
    writer->writeLine("end");
}

auto RubyStrategy::MapClassName(std::string_view wx_class_name) -> std::string
{
    if (!wx_class_name.starts_with("wx"))
    {
        return std::string(wx_class_name);
    }

    // Check the full-name prefix map first for module-specific classes
    if (auto iter = g_map_ruby_prefix.find(wx_class_name); iter != g_map_ruby_prefix.end())
    {
        std::string result(iter->second);
        result += wx_class_name.substr(2);
        return result;
    }

    // Default: wxFoo → Wx::Foo
    std::string result("Wx::");
    result += wx_class_name.substr(2);
    return result;
}
