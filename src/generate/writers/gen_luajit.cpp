/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate LuaJIT code via kwxFFI
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_luajit.h"

#include "node.h"  // Node class

LuaJITCodeGenerator::LuaJITCodeGenerator(Node* form_node) :
    BaseCodeGenerator(GEN_LANG_LUAJIT, form_node)
{
}

void LuaJITCodeGenerator::GenerateClass(GenLang /* language */, PANEL_PAGE /* panel_type */,
                                        wxProgressDialog* /* progress */)
{
    // Phase 6: LuaJIT code generation will be implemented during cross-language verification
}
