/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a DialogBlocks project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gen_enums.h"     // Enumerations for generators
#include "node_classes.h"  // Forward defintions of Node classes

#include "import_xml.h"  // ImportXML -- Base class for XML importing

class DialogBlocks : public ImportXML
{
public:
    DialogBlocks();
    ~DialogBlocks() {};

    bool Import(const tt_wxString& filename, bool write_doc = true) override;

    int GetLanguage() const override { return GEN_LANG_CPLUSPLUS; }

protected:
    void CreateFormNode(pugi::xml_node& form_xml);
    void CreateChildNode(pugi::xml_node& child_node, const NodeSharedPtr& parent);

    // This will try to determine the generator to use based on either "proxy-Base class" or
    // "proxy-type" attributes.
    GenEnum::GenName FindGenerator(pugi::xml_node& node);

    // Most strings in a DialogBlocks project are quoted, but some are not. This will return
    // the string without quotes.
    tt_string ExtractQuotedString(pugi::xml_node& str_node);

private:
    bool m_use_enums { true };
};
