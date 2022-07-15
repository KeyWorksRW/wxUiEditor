/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxFormBuider project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include "node_classes.h"  // Forward defintions of Node classes

#include "import_xml.h"  // ImportXML -- Base class for XML importing

class wxImage;

using ImportNameMap = std::unordered_map<std::string, const char*>;

class FormBuilder : public ImportXML
{
public:
    FormBuilder();
    ~FormBuilder() {};

    bool Import(const ttString& filename, bool write_doc = true) override;
    NodeSharedPtr CreateFbpNode(pugi::xml_node& xml_prop, Node* parent, Node* sizeritem = nullptr);

protected:
    void ConvertNameSpaceProp(NodeProperty* prop, std::string_view org_names);

    // Called when a property is unknown and has a value set.
    void ProcessPropValue(pugi::xml_node& xml_prop, ttlib::sview prop_name, ttlib::sview class_name, Node* newobject);

    void BitmapProperty(pugi::xml_node& xml_obj, NodeProperty* prop);
    void CreateProjectNode(pugi::xml_node& xml_obj, Node* new_node);

private:
    ImportNameMap m_mapEventNames;

    ttlib::cstr m_embedPath;
    ttlib::cstr m_eventGeneration;
    ttlib::cstr m_baseFile;
    ttlib::cstr m_class_decoration;

    int m_VerMinor { 0 };
};
