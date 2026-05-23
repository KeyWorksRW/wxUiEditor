/////////////////////////////////////////////////////////////////////////////
// Purpose:   Python language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

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
