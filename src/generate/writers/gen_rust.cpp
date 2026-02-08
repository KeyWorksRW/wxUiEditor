/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Rust code via kwxFFI
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_rust.h"

#include "node.h"  // Node class

RustCodeGenerator::RustCodeGenerator(Node* form_node) : BaseCodeGenerator(GEN_LANG_RUST, form_node)
{
}

void RustCodeGenerator::GenerateClass(GenLang /* language */, PANEL_PAGE /* panel_type */,
                                      wxProgressDialog* /* progress */)
{
    // Phase 6: Rust code generation will be implemented during cross-language verification
}
