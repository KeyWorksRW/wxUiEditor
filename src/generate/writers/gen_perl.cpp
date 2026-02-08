/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Perl code via kwxFFI
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_perl.h"

#include "node.h"  // Node class

PerlCodeGenerator::PerlCodeGenerator(Node* form_node) : BaseCodeGenerator(GEN_LANG_PERL, form_node)
{
}

void PerlCodeGenerator::GenerateClass(GenLang /* language */, PANEL_PAGE /* panel_type */,
                                      wxProgressDialog* /* progress */)
{
    // Phase 6: Perl code generation will be implemented during cross-language verification
}
