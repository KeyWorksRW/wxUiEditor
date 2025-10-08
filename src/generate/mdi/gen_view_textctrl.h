/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTextCtrl view class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class TextViewGenerator : public BaseGenerator
{
public:
    auto ConstructionCode(Code& code) -> bool override;

    auto HeaderCode(Code& code) -> bool override;
    auto BaseClassNameCode(Code& code) -> bool override;
    auto CollectMemberVariables(Node* /* node unused */,
                                std::set<std::string>& /* code_lines unused */) -> void override;

    auto GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang language) -> bool override;
};
