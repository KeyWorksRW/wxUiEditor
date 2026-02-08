/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Julia code via kwxFFI
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gen_base.h"  // BaseCodeGenerator

class JuliaCodeGenerator : public BaseCodeGenerator
{
public:
    JuliaCodeGenerator(Node* form_node);

    void GenerateClass(GenLang language = GEN_LANG_JULIA,
                       PANEL_PAGE panel_type = PANEL_PAGE::NOT_PANEL,
                       wxProgressDialog* progress = nullptr) override;
};
