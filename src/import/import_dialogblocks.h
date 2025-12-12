/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a DialogBlocks project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements DialogBlocks, an importer for Anthemion DialogBlocks XML project
// files. The class extends ImportXML base class and converts DialogBlocks-specific XML structures
// (proxy attributes, quoted strings, DialogBlocks-style validators) to wxUiEditor Node trees. Key
// methods: CreateFormNode/ CreateFolderNode parse top-level structures,
// CreateChildNode/CreateCustomNode recursively build component hierarchies, FindGenerator maps
// proxy-Base class/proxy-type to GenName enums, ProcessStyles/ProcessEvents/ ProcessMisc extract
// node properties from XML attributes. Helper methods (SetNodeValidator, SetNodeID, SetNodeVarname,
// SetNodeDimensions, SetNodeState) populate Node properties, and ExtractQuotedString handles
// DialogBlocks' inconsistent quoting. State flags (m_use_enums for ID format,
// m_class_uses_dlg_units for coordinate system) affect conversion. Only supports GEN_LANG_CPLUSPLUS
// output.

#pragma once

#include "gen_enums.h"   // Enumerations for generators
#include "import_xml.h"  // ImportXML -- Base class for XML importing

class Node;
using NodeSharedPtr = std::shared_ptr<Node>;

class DialogBlocks : public ImportXML
{
public:
    DialogBlocks();
    virtual ~DialogBlocks() = default;

    DialogBlocks(const DialogBlocks&) = delete;
    auto operator=(const DialogBlocks&) -> DialogBlocks& = delete;
    DialogBlocks(DialogBlocks&&) = delete;
    auto operator=(DialogBlocks&&) -> DialogBlocks& = delete;

    bool Import(const std::string& filename, bool write_doc = true) override;

    [[nodiscard]] auto GetLanguage() const -> int override { return GEN_LANG_CPLUSPLUS; }

protected:
    // Sets validator variable name and variable handler type
    void SetNodeValidator(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // Sets the node id (and optionally assigns it a value) if it isn't wxID_ANY
    void SetNodeID(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // Sets var_name and class access for a node
    void SetNodeVarname(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // Sets pos and size
    void SetNodeDimensions(pugi::xml_node& node_xml, const NodeSharedPtr& new_node) const;

    // Sets disabled and hidden states for a node
    void SetNodeState(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    auto CreateFormNode(pugi::xml_node& form_xml, const NodeSharedPtr& parent) -> bool;
    auto CreateFolderNode(pugi::xml_node& form_xml, const NodeSharedPtr& parent) -> bool;
    void CreateChildNode(pugi::xml_node& child_node, Node* parent);
    void CreateCustomNode(pugi::xml_node& child_xml, Node* parent);

    // Process all the style-like attributes for the current node
    void ProcessStyles(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // Add all events for the current node
    void ProcessEvents(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // This will walk through all the immediate children of the current node, and process any
    // known proxy settings.
    void ProcessMisc(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // This will try to determine the generator to use based on either "proxy-Base class" or
    // "proxy-type" attributes.
    auto FindGenerator(pugi::xml_node& node, Node* parent) -> GenEnum::GenName;

    // Most strings in a DialogBlocks project are quoted, but some are not. This will return
    // the string without quotes.
    auto ExtractQuotedString(pugi::xml_node& str_xml) -> wxString;

    auto GatherErrorDetails(pugi::xml_node& xml_node, GenEnum::GenName get_GenName) -> wxString;

private:
    bool m_use_enums { true };
    bool m_class_uses_dlg_units { false };
};
