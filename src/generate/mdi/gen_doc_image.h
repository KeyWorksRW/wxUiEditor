/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxImage document class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class ImageDocGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code&) override;
    auto BaseClassNameCode(Code& code) -> bool override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;
};
