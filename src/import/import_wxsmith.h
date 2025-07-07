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

    bool Import(const tt_string& filename, bool write_doc = true) override;

    // wxSmith only supports C++ code generation
    int GetLanguage() const override { return GEN_LANG_CPLUSPLUS; }

    bool HandleUnknownProperty(const pugi::xml_node& /* xml_obj */, Node* /* node */,
                               Node* /* parent */) override;
};
