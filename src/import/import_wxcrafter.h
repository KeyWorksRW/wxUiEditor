/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxCrafter project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include "gen_enums.h"     // Enumerations for generators
#include "node_classes.h"  // Forward defintions of Node classes

#include "import_xml.h"  // ImportXML -- Base class for XML importing

#include "rapidjson/document.h"

class wxImage;

using ImportNameMap = std::unordered_map<std::string, const char*>;

class WxCrafter : public ImportXML
{
public:
    WxCrafter();
    ~WxCrafter() {};

    bool Import(const tt_wxString& filename, bool write_doc = true) override;
    NodeSharedPtr CreateFbpNode(pugi::xml_node& xml_prop, Node* parent, Node* sizeritem = nullptr);

protected:
    bool ProcessFont(Node* node, const rapidjson::Value& object);
    bool ProcessScintillaProperty(Node* node, const rapidjson::Value& object);
    void ProcessBitmapPropety(Node* parent, const rapidjson::Value& object);
    void ProcessChild(Node* parent, const rapidjson::Value& object);
    void ProcessEvents(Node* parent, const rapidjson::Value& array);
    void ProcessForm(const rapidjson::Value& value);
    void ProcessProperties(Node* node, const rapidjson::Value& array);
    void ProcessSizerFlags(Node* node, const rapidjson::Value& array);
    void ProcessStdBtnChildren(Node* parent, const rapidjson::Value& array);
    void ProcessStyles(Node* parent, const rapidjson::Value& array);

    // Called when the property isn't recognized. Will return prop_processed if it was
    // processed, or a valid prop_name if it was converted, but needs further handling.
    GenEnum::PropName UnknownProperty(Node* node, const rapidjson::Value& value, tt_string& name);

    // Called when prop_name is a valid property. This will set the property's value after
    // any possible additional processing.
    void KnownProperty(Node* node, const rapidjson::Value& value, GenEnum::PropName prop_name);

    // Called to handle prop_value which may get converted to a different property before
    // saving.
    void ValueProperty(Node* node, const rapidjson::Value& value);

private:
    tt_string m_output_name;

    bool m_is_output_name_used { false };

    // wxCrafter sets this for everything in the entire project
    bool m_generate_ids { true };
};
