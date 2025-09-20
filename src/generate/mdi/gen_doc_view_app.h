// Purpose:   Generates base class for wxDocument/wView applications
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class DocViewAppGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code& code) override;
    bool AfterConstructionCode(Code& code) override;

    bool BaseClassNameCode(Code& code) override;
    bool PreClassHeaderCode(Code& code) override;
    bool HeaderCode(Code& code) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang language) override;
};
