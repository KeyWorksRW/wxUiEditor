/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxSmith file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <vector>

#include "import_xml.h"  // ImportXML -- Base class for XML importing

class Node;

class WxSmith : public ImportXML
{
public:
    WxSmith();

    bool Import(const std::string& filename, bool write_doc = true, bool allow_ui = true) override;

    // wxSmith only supports C++ code generation
    GenLang GetLanguage() const override { return GenLang::cplusplus; }

    bool HandleUnknownProperty(const pugi::xml_node& /* xml_obj */, Node* /* node */,
                               Node* /* parent */) override;

private:
    // Replaces every <object_ref> element in the document with a copy of the named <object>
    // element it references, in the same location.
    void ExpandObjectRefs(pugi::xml_node root);

    // Expands the <object_ref> children of parent, then recurses into the remaining children.
    // ref_chain holds the names already expanded and is used to detect circular references.
    void ExpandObjectRefsIn(pugi::xml_node parent,
                            const std::map<std::string, pugi::xml_node>& named_objects,
                            const std::vector<std::string>& ref_chain, int depth);
};
