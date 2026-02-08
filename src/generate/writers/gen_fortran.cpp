/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Fortran code via kwxFFI
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_fortran.h"

#include "node.h"  // Node class

FortranCodeGenerator::FortranCodeGenerator(Node* form_node) :
    BaseCodeGenerator(GEN_LANG_FORTRAN, form_node)
{
}

void FortranCodeGenerator::GenerateClass(GenLang /* language */, PANEL_PAGE /* panel_type */,
                                         wxProgressDialog* /* progress */)
{
    // Phase 6: Fortran code generation will be implemented during cross-language verification
}
