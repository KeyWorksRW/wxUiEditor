/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxFormBuilder project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements FormBuilder, an importer for wxFormBuilder .fbp XML project
// files. The class extends ImportXML and converts FormBuilder's object/property XML structure to
// wxUiEditor Nodes using CreateFbpNode (recursive node creation), ProcessPropValue (unknown
// property handler), BitmapProperty (image path resolution), and ConvertNameSpaceProp (namespace
// conversion). Import() loads the .fbp file, extracts project-level settings (m_embedPath,
// m_eventGeneration, m_baseFile, m_class_decoration), and delegates to CreateFbpNode for tree
// building. FormBuilder uses different property naming conventions than wxUiEditor, requiring
// extensive mapping via ImportNameMap lookups and custom handling for version-specific differences
// (m_VerMinor). The importer handles FormBuilder's unique bitmap embedding, event generation
// settings, and class decoration patterns to produce compatible wxUiEditor project structures.

#pragma once

#include <string>
class Node;

#include "import_xml.h"  // ImportXML -- Base class for XML importing

class wxImage;

using ImportNameMap = std::unordered_map<std::string, const char*>;

class FormBuilder : public ImportXML
{
public:
    FormBuilder() = default;
    ~FormBuilder() = default;

    auto Import(const std::string& filename, bool write_doc = true) -> bool override;
    auto CreateFbpNode(pugi::xml_node& xml_obj, Node* parent, Node* sizeritem = nullptr)
        -> NodeSharedPtr;

protected:
    void ConvertNameSpaceProp(NodeProperty* prop, std::string_view org_names);

    // Called when a property is unknown and has a value set.
    void ProcessPropValue(pugi::xml_node& xml_prop, std::string_view prop_name,
                          std::string_view class_name, Node* newobject, Node* parent);

    void BitmapProperty(pugi::xml_node& xml_prop, NodeProperty* prop);
    void createProjectNode(pugi::xml_node& xml_obj, Node* new_node);

private:
    // Helper methods for CreateFbpNode complexity reduction
    void ProcessXmlProperties(pugi::xml_node& xml_obj, Node* newobject, std::string_view class_name,
                              Node* parent);
    void ProcessXmlEvents(pugi::xml_node& xml_obj, Node* newobject, Node* parent);
    void ValidateAndFixNodeProperties(Node* newobject, Node* parent);
    auto ProcessChildNodes(pugi::xml_node& xml_obj, NodeSharedPtr& newobject, Node* parent,
                           Node* sizeritem) -> NodeSharedPtr;

    // Helper methods for ProcessXmlProperties complexity reduction
    auto HandleBitmapProperty(pugi::xml_node& xml_prop, NodeProperty* prop_ptr) -> void;
    static auto ConvertLegacyWindowStyles(std::string_view text_value) -> std::string;
    auto HandleNameProperty(pugi::xml_node& xml_prop, Node* newobject) -> void;
    auto HandleIncludeProperty(pugi::xml_node& xml_prop, Node* newobject, Node* parent) -> void;

    std::string m_embedPath;
    std::string m_eventGeneration;
    std::string m_baseFile;
    std::string m_class_decoration;

    int m_VerMinor { 0 };
};
