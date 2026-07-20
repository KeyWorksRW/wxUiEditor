/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxFormBuilder project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

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

    bool Import(const std::string& filename, bool write_doc = true) override;
    NodeSharedPtr CreateFbpNode(pugi::xml_node& xml_obj, Node* parent, Node* sizeritem = nullptr);

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
    NodeSharedPtr ProcessChildNodes(pugi::xml_node& xml_obj, NodeSharedPtr& newobject, Node* parent,
                                    Node* sizeritem);

    // Helper methods for ProcessXmlProperties complexity reduction
    void HandleBitmapProperty(pugi::xml_node& xml_prop, NodeProperty* prop_ptr);
    static std::string ConvertLegacyWindowStyles(std::string_view text_value);
    void HandleNameProperty(pugi::xml_node& xml_prop, Node* newobject);
    void HandleIncludeProperty(pugi::xml_node& xml_prop, Node* newobject, Node* parent);

    std::string m_embedPath;
    std::string m_eventGeneration;
    std::string m_baseFile;
    std::string m_class_decoration;

    int m_VerMinor { 0 };
};
