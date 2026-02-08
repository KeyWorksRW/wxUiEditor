/////////////////////////////////////////////////////////////////////////////
// Purpose:   Rust language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: RustStrategy implements the LanguageStrategy interface for Rust code
// generation via kwxFFI. Rust uses: `use` for imports, `let`/`let mut` for variables,
// `fn` for functions, `// ` comments, `std::ptr::null_mut()` for null, `.` member access,
// `::` scope operator, `{`/`}` blocks with `;` statement terminators, and ownership/
// borrowing for memory management. Rust closures provide lambda-like event handlers.

#pragma once

#include "strategy_ffi.h"

class RustStrategy : public FFIStrategy
{
public:
    explicit RustStrategy(const LanguageTraits& traits);

    void EmitPlatformBegin(Code& code, std::string_view platforms) override;
    void EmitImport(Code& code, std::string_view module) override;
    void EmitVarDecl(Code& code, std::string_view type, std::string_view name) override;
};
