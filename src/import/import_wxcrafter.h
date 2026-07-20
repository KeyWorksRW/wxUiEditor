/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxCrafter project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <glaze/json/generic.hpp>

#include "gen_enums.h"   // Enumerations for generators
#include "import_xml.h"  // ImportXML -- Base class for XML importing

class Node;
class wxImage;

using ImportNameMap = std::unordered_map<std::string, const char*>;

class WxCrafter : public ImportXML
{
public:
    WxCrafter();
    ~WxCrafter() {};

    bool Import(const std::string& filename, bool write_doc = true) override;
    NodeSharedPtr CreateFbpNode(pugi::xml_node& xml_prop, Node* parent, Node* sizeritem = nullptr);

    // wxCrafter only supports C++ code generation
    GenLang GetLanguage() const override { return GenLang::cplusplus; }

protected:
    bool ProcessFont(Node* node, const glz::generic& object);
    bool ProcessScintillaProperty(Node* node, const glz::generic& object);
    void ProcessBitmapProperty(Node* parent, const glz::generic& object);
    void ProcessChild(Node* parent, const glz::generic& object);
    void ProcessEvents(Node* parent, const glz::generic& array);
    void ProcessForm(const glz::generic& value);
    void ProcessProperties(Node* node, const glz::generic& array);
    void ProcessSizerFlags(Node* node, const glz::generic& array);
    void ProcessStdBtnChildren(Node* parent, const glz::generic& array);
    void ProcessStyles(Node* parent, const glz::generic& array);

    // Called when the property isn't recognized. Will return prop_processed if it was
    // processed, or a valid prop_name if it was converted, but needs further handling.
    GenEnum::PropName UnknownProperty(Node* node, const glz::generic& value, std::string& name);

    // Called when prop_name is a valid property. This will set the property's value after
    // any possible additional processing.
    void KnownProperty(Node* node, const glz::generic& value, GenEnum::PropName prop_name);

    // Called to handle prop_value which may get converted to a different property before
    // saving.
    void ValueProperty(Node* node, const glz::generic& value);

private:
    std::string m_output_name;

    bool m_is_output_name_used { false };

    // wxCrafter sets this for everything in the entire project
    bool m_generate_ids { true };
};
