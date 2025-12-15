/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxGlade file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements WxGlade, an importer for wxGlade XML project files. The class
// extends ImportXML and converts wxGlade's XRC-like XML format to wxUiEditor Nodes using
// CreateGladeNode (recursive node creation with wxGlade-specific property handling), CreateMenus
// (wxMenuBar hierarchy processing), and CreateToolbar (wxToolBar tool extraction). Override methods
// HandleUnknownProperty and HandleNormalProperty provide wxGlade-specific property conversion for
// cases where wxGlade's property names/values differ from standard XRC or wxUiEditor conventions.
// Import() orchestrates file loading and tree building. The importer handles wxGlade's unique
// property naming, widget hierarchies, and XRC deviations to produce compatible wxUiEditor project
// structures, supporting Python-oriented wxGlade projects with multi-language output.

#pragma once

#include "import_xml.h"  // ImportXML -- Base class for XML importing

class Node;

class WxGlade : public ImportXML
{
public:
    WxGlade();

    bool Import(const std::string& filename, bool write_doc = true) override;

    bool HandleUnknownProperty(const pugi::xml_node& /* xml_obj */, Node* /* node */,
                               Node* /* parent */) override;

    bool HandleNormalProperty(const pugi::xml_node& /* xml_obj */, Node* /* node */,
                              Node* /* parent */, GenEnum::PropName /* wxue_prop */) override;

protected:
    NodeSharedPtr CreateGladeNode(pugi::xml_node& xml_obj, Node* parent, Node* sizeritem = nullptr);

    // parent should be a wxMenuBar
    void CreateMenus(pugi::xml_node& xml_obj, Node* parent);

    // parent should be a wxToolBar
    void CreateToolbar(pugi::xml_node& xml_obj, Node* parent);

private:
};
