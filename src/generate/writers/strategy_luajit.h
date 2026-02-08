/////////////////////////////////////////////////////////////////////////////
// Purpose:   LuaJIT language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: LuaJITStrategy implements the LanguageStrategy interface for LuaJIT code
// generation via kwxFFI. LuaJIT uses: `require` for modules, `local` for variables,
// `function`/`end` blocks, `-- ` comments, `nil` for null, `:` for method calls,
// `.` for module access, and metatables for OOP patterns.

#pragma once

#include "strategy_ffi.h"

class LuaJITStrategy : public FFIStrategy
{
public:
    explicit LuaJITStrategy(const LanguageTraits& traits);

    void EmitPlatformBegin(Code& code, std::string_view platforms) override;
    void EmitImport(Code& code, std::string_view module) override;
    void EmitVarDecl(Code& code, std::string_view type, std::string_view name) override;
};
