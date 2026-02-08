/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Perl code via kwxFFI
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gen_base.h"  // BaseCodeGenerator

class PerlCodeGenerator : public BaseCodeGenerator
{
public:
    PerlCodeGenerator(Node* form_node);

    void GenerateClass(GenLang language = GEN_LANG_PERL,
                       PANEL_PAGE panel_type = PANEL_PAGE::NOT_PANEL,
                       wxProgressDialog* progress = nullptr) override;
};
