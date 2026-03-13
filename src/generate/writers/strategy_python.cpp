/////////////////////////////////////////////////////////////////////////////
// Purpose:   Python language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "strategy_python.h"

#include "code.h"        // Code -- Helper class for generating code
#include "write_code.h"  // WriteCode class

PythonStrategy::PythonStrategy(const LanguageTraits& traits) : WxBindingStrategy(traits) {}

void PythonStrategy::EmitPlatformBegin(Code& code, std::string_view platforms)
{
    bool has_prior = false;

    if (platforms.find("Windows") != std::string_view::npos)
    {
        code.Eol() << "if wx.Platform == \"msw\"";
        has_prior = true;
    }
    if (platforms.find("Unix") != std::string_view::npos)
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
        code << "wx.Platform == \"unix\"";
    }
    if (platforms.find("Mac") != std::string_view::npos)
    {
        if (has_prior)
        {
            code << " or ";
        }
        else
        {
            code.Eol() << "if ";
        }
        code << "wx.Platform == \"mac\"";
    }
    code << ':';
}

void PythonStrategy::EmitPlatformEnd(WriteCode* writer)
{
    // Python uses indentation-based blocks — just unindent
    writer->Unindent();
}

auto PythonStrategy::MapClassName(std::string_view wx_class_name) -> std::string
{
    if (!wx_class_name.starts_with("wx"))
    {
        return std::string(wx_class_name);
    }

    // Check the full-name prefix map first for module-specific classes
    // (e.g., wxCalendarCtrl → wx.adv.CalendarCtrl)
    if (auto iter = g_map_python_prefix.find(wx_class_name); iter != g_map_python_prefix.end())
    {
        std::string result(iter->second);
        result += wx_class_name.substr(2);
        return result;
    }

    // Default: wxFoo → wx.Foo
    std::string result("wx.");
    result += wx_class_name.substr(2);
    return result;
}
