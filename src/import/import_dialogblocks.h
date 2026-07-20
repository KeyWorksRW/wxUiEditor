/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a DialogBlocks project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

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
    DialogBlocks& operator=(const DialogBlocks&) = delete;
    DialogBlocks(DialogBlocks&&) = delete;
    DialogBlocks& operator=(DialogBlocks&&) = delete;

    bool Import(const std::string& filename, bool write_doc = true) override;

    [[nodiscard]] GenLang GetLanguage() const override { return GenLang::cplusplus; }

protected:
    // Sets validator variable name and variable handler type
    static void SetNodeValidator(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // Sets the node id (and optionally assigns it a value) if it isn't wxID_ANY
    static void SetNodeID(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // Sets var_name and class access for a node
    static void SetNodeVarname(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // Sets pos and size
    void SetNodeDimensions(pugi::xml_node& node_xml, const NodeSharedPtr& new_node) const;

    // Sets disabled and hidden states for a node
    static void SetNodeState(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    bool CreateFormNode(pugi::xml_node& form_xml, const NodeSharedPtr& parent);
    bool CreateFolderNode(pugi::xml_node& form_xml, const NodeSharedPtr& parent);
    void CreateChildNode(pugi::xml_node& child_xml, Node* parent);
    void CreateCustomNode(pugi::xml_node& child_xml, Node* parent);

    // Process all the style-like attributes for the current node
    static void ProcessStyles(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // Add all events for the current node
    static void ProcessEvents(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // This will walk through all the immediate children of the current node, and process any
    // known proxy settings.
    static void ProcessMisc(pugi::xml_node& node_xml, const NodeSharedPtr& new_node);

    // This will try to determine the generator to use based on either "proxy-Base class" or
    // "proxy-type" attributes.
    GenEnum::GenName FindGenerator(pugi::xml_node& node, Node* parent);

    // Most strings in a DialogBlocks project are quoted, but some are not. This will return
    // the string without quotes.
    static wxString ExtractQuotedString(pugi::xml_node& str_xml);

    static wxString GatherErrorDetails(pugi::xml_node& xml_node, GenEnum::GenName get_GenName);

private:
    // Helper methods for CreateFormNode
    GenEnum::GenName DetermineFormGenName(pugi::xml_node& form_xml);
    static GenEnum::GenName HandleDialogToPanelConversion(pugi::xml_node& form_xml,
                                                          GenEnum::GenName gen_name);
    static NodeSharedPtr TryRecreateFormNode(GenEnum::GenName& gen_name,
                                             const NodeSharedPtr& parent, pugi::xml_node& form_xml);
    static void SetFormCommonProperties(pugi::xml_node& form_xml, const NodeSharedPtr& form);

    // Helper methods for ProcessMisc
    static void ProcessMiscStringChildren(pugi::xml_node& node_xml, const NodeSharedPtr& node);
    static void ProcessMiscLongChildren(pugi::xml_node& node_xml, const NodeSharedPtr& node);
    static void ProcessMiscBoolChildren(pugi::xml_node& node_xml, const NodeSharedPtr& node);

    bool m_use_enums { true };
    bool m_class_uses_dlg_units { false };
};
