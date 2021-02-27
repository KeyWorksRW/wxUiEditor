/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxFormBuider project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include <ttstr.h>  // ttString -- wxString with additional methods similar to ttlib::cstr

#include "node_classes.h"  // Forward defintions of Node classes

#include "import_interface.h"

class wxImage;

using ImportNameMap = std::unordered_map<std::string, const char*>;

class FormBuilder : public ImportInterface
{
public:
    FormBuilder();
    ~FormBuilder() {};

    bool Import(const ttString& filename) override;

    pugi::xml_document& GetDocument() override { return m_docOut; }

protected:
    void ConvertNameSpaceProp(NodeProperty* prop, ttlib::cview org_names);
    // Call when the style name is the same, but may need additional processing
    void ProcessStyle(pugi::xml_node& xml_prop, Node* object, NodeProperty* prop);

    // Called when a property is unknown and has a value set.
    void ProcessPropValue(pugi::xml_node& xml_prop, ttlib::cview prop_name, ttlib::cview class_name, Node* newobject);

    void ConvertSizerProperties(pugi::xml_node& xml_prop, Node* object, Node* parent, NodeProperty* prop);
    void BitmapProperty(pugi::xml_node& xml_obj, NodeProperty* prop);
    NodeSharedPtr CreateFbpNode(pugi::xml_node& xml_prop, Node* parent, Node* sizeritem = nullptr);
    void CreateProjectNode(pugi::xml_node& xml_obj, Node* new_node);

private:
    Node* m_project;
    pugi::xml_document m_docOut;

    ImportNameMap m_mapEventNames;

    ttString m_importProjectFile;

    ttlib::cstr m_embedPath;
    ttlib::cstr m_eventGeneration;
    ttlib::cstr m_baseFile;

    int m_VerMinor { 0 };
};
