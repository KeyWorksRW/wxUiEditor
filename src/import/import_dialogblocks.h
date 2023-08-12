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

    bool Import(const tt_string& filename, bool write_doc = true) override;

    int GetLanguage() const override { return GEN_LANG_CPLUSPLUS; }

protected:
    // Sets validator variable name and variable handler type
    void SetNodeValidator(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // Sets the node id (and optionally assigns it a value) if it isn't wxID_ANY
    void SetNodeID(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // Sets var_name and class access for a node
    void SetNodeVarname(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // Sets pos and size
    void SetNodeDimensions(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // Sets disabled and hidden states for a node
    void SetNodeState(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    bool CreateFormNode(pugi::xml_node& form_xml, const NodeSharedPtr& parent);
    bool CreateFolderNode(pugi::xml_node& form_xml, const NodeSharedPtr& parent);
    void createChildNode(pugi::xml_node& child_node, Node* parent);

    // Process all the style-like attributes for the current node
    void ProcessStyles(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // Add all events for the current node
    void ProcessEvents(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // This will walk through all the immediate children of the current node, and process any
    // known proxy settings.
    void ProcessMisc(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // This will try to determine the generator to use based on either "proxy-Base class" or
    // "proxy-type" attributes.
    GenEnum::GenName FindGenerator(pugi::xml_node& node, Node* parent);

    // Most strings in a DialogBlocks project are quoted, but some are not. This will return
    // the string without quotes.
    tt_string ExtractQuotedString(pugi::xml_node& str_node);

    tt_string GatherErrorDetails(pugi::xml_node& xml_node, GenEnum::GenName getGenName);

private:
    bool m_use_enums { true };
    bool m_class_uses_dlg_units { false };
};
