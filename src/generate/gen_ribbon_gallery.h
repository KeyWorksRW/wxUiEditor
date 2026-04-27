/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRibbonGallery generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class wxRibbonBarEvent;
class Code;
class Node;
class WriteCode;

class RibbonGalleryGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    void AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* /* node */,
                       bool /* is_preview */) override;

    bool ConstructionCode(Code& code) override;

    // Generate construction code for all gallery items, including size checking
    // against prop_gallery_size and rescaling when needed.
    static void GenerateGalleryItems(Node* gallery_node, WriteCode* source, GenLang language);

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

    int GenXrcObject(Node* /* node */, pugi::xml_node& /* object */,
                     size_t /* xrc_flags */) override;
};

class RibbonGalleryItemGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code& code) override;

    int GenXrcObject(Node* /* node */, pugi::xml_node& /* object */,
                     size_t /* xrc_flags */) override;
};
