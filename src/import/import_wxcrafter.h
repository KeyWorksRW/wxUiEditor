/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxCrafter project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements WxCrafter, an importer for wxCrafter JSON project files using
// RapidJSON for parsing. The class extends ImportXML (despite processing JSON) and converts
// wxCrafter's JSON object structure to wxUiEditor Nodes via ProcessForm (top-level), ProcessChild
// (recursive), ProcessProperties (property arrays), ProcessStyles/ProcessSizerFlags (style arrays),
// ProcessEvents (event arrays), ProcessFont/ProcessScintillaProperty (complex properties), and
// ProcessBitmapProperty (image handling). UnknownProperty/KnownProperty/ValueProperty handle
// property mapping with wxCrafter-specific conversions. State tracking includes m_output_name (form
// output file), m_is_output_name_used (prevents duplicate use), and m_generate_ids (project-wide ID
// generation setting). Only supports GEN_LANG_CPLUSPLUS. The importer navigates wxCrafter's nested
// JSON arrays/objects to extract component hierarchies, properties, and events into wxUiEditor's
// XML-based Node tree structure.

#pragma once

#include "gen_enums.h"   // Enumerations for generators
#include "import_xml.h"  // ImportXML -- Base class for XML importing

#include "rapidjson/document.h"

class Node;
class wxImage;

using ImportNameMap = std::unordered_map<std::string, const char*>;

class WxCrafter : public ImportXML
{
public:
    WxCrafter();
    ~WxCrafter() {};

    auto Import(const std::string& filename, bool write_doc = true) -> bool override;
    auto CreateFbpNode(pugi::xml_node& xml_prop, Node* parent, Node* sizeritem = nullptr)
        -> NodeSharedPtr;

    // wxCrafter only supports C++ code generation
    int GetLanguage() const override { return GEN_LANG_CPLUSPLUS; }

protected:
    auto ProcessFont(Node* node, const rapidjson::Value& object) -> bool;
    auto ProcessScintillaProperty(Node* node, const rapidjson::Value& object) -> bool;
    auto ProcessBitmapProperty(Node* parent, const rapidjson::Value& object) -> void;
    auto ProcessChild(Node* parent, const rapidjson::Value& object) -> void;
    auto ProcessEvents(Node* parent, const rapidjson::Value& array) -> void;
    auto ProcessForm(const rapidjson::Value& value) -> void;
    auto ProcessProperties(Node* node, const rapidjson::Value& array) -> void;
    auto ProcessSizerFlags(Node* node, const rapidjson::Value& array) -> void;
    auto ProcessStdBtnChildren(Node* parent, const rapidjson::Value& array) -> void;
    auto ProcessStyles(Node* parent, const rapidjson::Value& array) -> void;

    // Called when the property isn't recognized. Will return prop_processed if it was
    // processed, or a valid prop_name if it was converted, but needs further handling.
    auto UnknownProperty(Node* node, const rapidjson::Value& value, std::string& name)
        -> GenEnum::PropName;

    // Called when prop_name is a valid property. This will set the property's value after
    // any possible additional processing.
    auto KnownProperty(Node* node, const rapidjson::Value& value, GenEnum::PropName prop_name)
        -> void;

    // Called to handle prop_value which may get converted to a different property before
    // saving.
    auto ValueProperty(Node* node, const rapidjson::Value& value) -> void;

private:
    std::string m_output_name;

    bool m_is_output_name_used { false };

    // wxCrafter sets this for everything in the entire project
    bool m_generate_ids { true };
};
