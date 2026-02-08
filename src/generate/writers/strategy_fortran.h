/////////////////////////////////////////////////////////////////////////////
// Purpose:   Fortran language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: FortranStrategy implements the LanguageStrategy interface for Fortran code
// generation via kwxFFI. Fortran uses: `use` modules for imports, `type(c_ptr)` for opaque
// handles, subroutine/function blocks, `! ` comment prefix, `.TRUE.`/`.FALSE.` literals,
// `%` member access operator, and `C_NULL_PTR` for null. Indentation is 2 spaces.

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
};
