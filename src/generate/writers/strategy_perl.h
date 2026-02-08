/////////////////////////////////////////////////////////////////////////////
// Purpose:   Perl language strategy for code generation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: PerlStrategy implements the LanguageStrategy interface for Perl code
// generation via kwxFFI. This replaces legacy wxPerl with FFI-based bindings. It uses:
// `use` for modules, `my $`/`my @` for variables, `sub` for functions, `# ` comments,
// `undef` for null, `->` member access, `::` scope operator, `{`/`}` blocks with `;`
// statement terminators.

#pragma once

#include "strategy_ffi.h"

class PerlStrategy : public FFIStrategy
{
public:
    explicit PerlStrategy(const LanguageTraits& traits);

    void EmitPlatformBegin(Code& code, std::string_view platforms) override;
    void EmitImport(Code& code, std::string_view module) override;
    void EmitVarDecl(Code& code, std::string_view type, std::string_view name) override;
};
