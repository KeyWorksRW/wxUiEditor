/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxCrafter project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include "ttstr.h"  // ttString -- wxString with additional methods similar to ttlib::cstr

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

    bool Import(const ttString& filename, bool write_doc = true) override;
    NodeSharedPtr CreateFbpNode(pugi::xml_node& xml_prop, Node* parent, Node* sizeritem = nullptr);

protected:
    void ProcessSizerFlags(Node* node, const rapidjson::Value& array);
    void ProcessProperties(Node* node, const rapidjson::Value& array);
    void ProcessChild(Node* parent, const rapidjson::Value& object);
    void ProcessStyles(Node* parent, const rapidjson::Value& array);
    void ProcessEvents(Node* parent, const rapidjson::Value& array);
    void ProcessStdBtnChildren(Node* parent, const rapidjson::Value& array);
    void ProcessBitmapPropety(Node* parent, const rapidjson::Value& object);

    void ProcessForm(const rapidjson::Value& value);

private:
    ttlib::cstr m_output_name;

    bool m_is_output_name_used { false };

    // wxCrafter sets this for everything in the entire project
    bool m_generate_ids { true };
};
