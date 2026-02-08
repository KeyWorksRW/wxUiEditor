/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Rust code via kwxFFI
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gen_base.h"  // BaseCodeGenerator

class RustCodeGenerator : public BaseCodeGenerator
{
public:
    RustCodeGenerator(Node* form_node);

    void GenerateClass(GenLang language = GEN_LANG_RUST,
                       PANEL_PAGE panel_type = PANEL_PAGE::NOT_PANEL,
                       wxProgressDialog* progress = nullptr) override;
};
