/////////////////////////////////////////////////////////////////////////////
// Purpose:   TypeScript language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: TypeScriptStrategy implements the LanguageStrategy interface for
// TypeScript code generation via kwxFFI. TypeScript uses: `import` for imports,
// `let`/`const` for variables, `function` for functions, `// ` comments,
// `null` for null, `.` member access, `{`/`}` blocks, optional `;` terminators,
// and `this.` for self-reference. Arrow functions provide lambda-like event handlers.

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