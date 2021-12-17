/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxCrafter project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include "ttstr.h"  // ttString -- wxString with additional methods similar to ttlib::cstr

#include "node_classes.h"  // Forward defintions of Node classes

#include "import_xml.h"  // ImportXML -- Base class for XML importing

class wxImage;

namespace Json
{
    class Value;
}

using ImportNameMap = std::unordered_map<std::string, const char*>;

class WxCrafter : public ImportXML
{
public:
    WxCrafter();
    ~WxCrafter() {};

    bool Import(const ttString& filename, bool write_doc = true) override;
    NodeSharedPtr CreateFbpNode(pugi::xml_node& xml_prop, Node* parent, Node* sizeritem = nullptr);

protected:
    bool GetBoolValue(Json::Value& value, ttlib::cview name, bool def_return = false);

    void ConvertNameSpaceProp(NodeProperty* prop, ttlib::cview org_names);

    // Called when a property is unknown and has a value set.
    void ProcessPropValue(pugi::xml_node& xml_prop, ttlib::cview prop_name, ttlib::cview class_name, Node* newobject);
    void ProcessStyleProperty(pugi::xml_node& xml_prop, ttlib::cview class_name, Node* newobject);

    void ConvertSizerProperties(pugi::xml_node& xml_prop, Node* object, Node* parent, NodeProperty* prop);
    void BitmapProperty(pugi::xml_node& xml_obj, NodeProperty* prop);
    void CreateProjectNode(pugi::xml_node& xml_obj, Node* new_node);

private:
    ImportNameMap m_mapEventNames;

    ttlib::cstr m_embedPath;
    ttlib::cstr m_eventGeneration;
    ttlib::cstr m_baseFile;

    int m_VerMinor { 0 };

    // wxCrafter sets this for everything in the entire project
    bool m_generate_ids { true };
};
