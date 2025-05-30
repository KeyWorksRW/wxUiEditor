/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate wxHaskell code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_base.h"  // BaseCodeGenerator

class HaskellCodeGenerator : public BaseCodeGenerator
{
public:
    HaskellCodeGenerator(Node* form_node);

    // All language generators must implement this method.
    void GenerateClass(PANEL_PAGE panel_type = NOT_PANEL) override;

};
