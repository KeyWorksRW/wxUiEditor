/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Julia code via kwxFFI
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_julia.h"

#include "node.h"  // Node class

JuliaCodeGenerator::JuliaCodeGenerator(Node* form_node) :
    BaseCodeGenerator(GEN_LANG_JULIA, form_node)
{
}

void JuliaCodeGenerator::GenerateClass(GenLang /* language */, PANEL_PAGE /* panel_type */,
                                       wxProgressDialog* /* progress */)
{
    // Phase 6: Julia code generation will be implemented during cross-language verification
}
