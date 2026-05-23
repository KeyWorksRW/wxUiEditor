/////////////////////////////////////////////////////////////////////////////
// Purpose:   TypeScript language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "strategy_ffi.h"

class TypeScriptStrategy : public FFIStrategy
{
public:
    explicit TypeScriptStrategy(const LanguageTraits& traits);

    void EmitPlatformBegin(Code& code, std::string_view platforms) override;
    void EmitImport(Code& code, std::string_view module) override;
    void EmitVarDecl(Code& code, std::string_view type, std::string_view name) override;
};
