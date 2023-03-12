/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTextCtrl document class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class TextDocumentGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code&) override;

};
