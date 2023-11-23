// Purpose:   Generates base class for wxDocument/wView applications
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class DocViewAppGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code&) override;
    bool AfterConstructionCode(Code&) override;
    bool HeaderCode(Code&) override;
    bool PreClassHeaderCode(Code&) override;
    void AddProtectedHdrMembers(std::set<std::string>&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     int /* language */) override;
};
