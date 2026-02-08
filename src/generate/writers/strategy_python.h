/////////////////////////////////////////////////////////////////////////////
// Purpose:   Python language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: PythonStrategy implements the LanguageStrategy interface for wxPython code
// generation. Python accesses wxWidgets through CPython C extensions (wxPython). The strategy
// handles: wx. prefix mapping (wxButton → wx.Button, with module-specific prefixes like
// wx.adv., wx.grid.), Python-style platform conditionals (wx.Platform == "msw"),
// and feature-gap workarounds for wxPython limitations.

#pragma once

#include "strategy_cwrapper.h"

class PythonStrategy : public WxBindingStrategy
{
public:
    explicit PythonStrategy(const LanguageTraits& traits);

    void EmitPlatformBegin(Code& code, std::string_view platforms) override;
    void EmitPlatformEnd(WriteCode* writer) override;
    [[nodiscard]] auto MapClassName(std::string_view wx_class_name) -> std::string override;
};
