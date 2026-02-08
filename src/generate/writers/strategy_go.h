/////////////////////////////////////////////////////////////////////////////
// Purpose:   Go language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: GoStrategy implements the LanguageStrategy interface for Go code generation
// via kwxFFI. Go uses: `import` for packages, `:=` short variable declaration, `func` for
// functions, `// ` comments, `nil` for null, no classes (struct + methods), `{`/`}` blocks,
// and tab indentation (generated as 4 spaces). Go has no inheritance — composition is used.

#pragma once

#include "strategy_ffi.h"

class GoStrategy : public FFIStrategy
{
public:
    explicit GoStrategy(const LanguageTraits& traits);

    void EmitPlatformBegin(Code& code, std::string_view platforms) override;
    void EmitImport(Code& code, std::string_view module) override;
    void EmitVarDecl(Code& code, std::string_view type, std::string_view name) override;
};
