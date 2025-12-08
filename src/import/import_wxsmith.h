/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxSmith file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements WxSmith, an importer for Code::Blocks wxSmith XML project files.
// The class extends ImportXML leveraging its CreateXrcNode method since wxSmith uses XRC-based XML
// format with extensions. HandleUnknownProperty provides wxSmith-specific property conversion for
// Code::Blocks-specific attributes not present in standard XRC. Import() loads the .wxs file and
// delegates to base class XRC processing with wxSmith- specific overrides. Only supports
// GEN_LANG_CPLUSPLUS output matching Code::Blocks' C++-only code generation. The importer handles
// wxSmith's XRC dialect including Code::Blocks-specific property extensions, custom control
// definitions, and layout attributes to produce compatible wxUiEditor project structures from
// Code::Blocks IDE projects.

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
