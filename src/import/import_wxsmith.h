/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxSmith file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "import_xml.h"  // ImportXML -- Base class for XML importing

class Node;

class WxSmith : public ImportXML
{
public:
    WxSmith();

    bool Import(const std::string& filename, bool write_doc = true) override;

    // wxSmith only supports C++ code generation
    GenLang GetLanguage() const override { return GenLang::cplusplus; }

    bool HandleUnknownProperty(const pugi::xml_node& /* xml_obj */, Node* /* node */,
                               Node* /* parent */) override;
};
