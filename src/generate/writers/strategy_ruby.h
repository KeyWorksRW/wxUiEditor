/////////////////////////////////////////////////////////////////////////////
// Purpose:   Ruby language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: RubyStrategy implements the LanguageStrategy interface for wxRuby3 code
// generation. Ruby accesses wxWidgets through Ruby C extensions (wxRuby3). The strategy
// handles: Wx:: prefix mapping (wxButton → Wx::Button), Ruby-style platform conditionals
// (Wx::PLATFORM == 'WXMSW'), "end" block terminators, .new construction suffix,
// snake_case method names, and feature-gap workarounds for wxRuby3 limitations.

#pragma once

#include "strategy_cwrapper.h"

class RubyStrategy : public WxBindingStrategy
{
public:
    explicit RubyStrategy(const LanguageTraits& traits);

    void EmitPlatformBegin(Code& code, std::string_view platforms) override;
    void EmitPlatformEnd(WriteCode* writer) override;
    [[nodiscard]] auto MapClassName(std::string_view wx_class_name) -> std::string override;
};
