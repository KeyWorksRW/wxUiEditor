/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxSplitterWindow view class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class SplitterViewGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code&) override;

    bool HeaderCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;
};
