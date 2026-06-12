/////////////////////////////////////////////////////////////////////////////
// Purpose:   Fortran language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "strategy_ffi.h"

class FortranStrategy : public FFIStrategy
{
public:
    explicit FortranStrategy(const LanguageTraits& traits);

    void EmitPlatformBegin(Code& code, std::string_view platforms) override;
    void EmitPlatformEnd(WriteCode* writer) override;
    void EmitImport(Code& code, std::string_view module) override;
    void EmitVarDecl(Code& code, std::string_view type, std::string_view name) override;

    // wxButton → wx_Button (Fortran uses wx_ prefix as part of the type name)
    [[nodiscard]] std::string MapClassName(std::string_view wx_class_name) override;
};
