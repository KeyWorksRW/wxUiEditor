/////////////////////////////////////////////////////////////////////////////
// Purpose:   Julia language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: JuliaStrategy implements the LanguageStrategy interface for Julia code
// generation via kwxFFI. Julia uses: `using`/`import` for modules, `local` for variables,
// `function`/`end` blocks, `# ` comments, `nothing` for null, multiple dispatch instead of
// classes, and dynamic typing with optional type annotations.

#pragma once

#include "strategy_ffi.h"

class JuliaStrategy : public FFIStrategy
{
public:
    explicit JuliaStrategy(const LanguageTraits& traits);

    void EmitPlatformBegin(Code& code, std::string_view platforms) override;
    void EmitImport(Code& code, std::string_view module) override;
    void EmitVarDecl(Code& code, std::string_view type, std::string_view name) override;
};
