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

    auto Import(const std::string& filename, bool write_doc = true) -> bool override;

    // wxSmith only supports C++ code generation
    auto GetLanguage() const -> GenLang override { return GenLang::cplusplus; }

    auto HandleUnknownProperty(const pugi::xml_node& /* xml_obj */, Node* /* node */,
                               Node* /* parent */) -> bool override;
};
