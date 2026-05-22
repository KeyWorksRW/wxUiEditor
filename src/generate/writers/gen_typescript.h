/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate TypeScript code via kwxFFI
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gen_base.h"  // BaseCodeGenerator

class TypeScriptCodeGenerator : public BaseCodeGenerator
{
public:
    TypeScriptCodeGenerator(Node* form_node);

    void GenerateClass(GenLang language = GEN_LANG_TYPESCRIPT,
                       PANEL_PAGE panel_type = PANEL_PAGE::NOT_PANEL,
                       wxProgressDialog* progress = nullptr) override;

private:
    auto GenerateConstructionCode(Code& code) -> void;
};