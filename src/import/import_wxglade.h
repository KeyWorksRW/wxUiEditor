/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxGlade file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "import_xml.h"  // ImportXML -- Base class for XML importing

class Node;

class WxGlade : public ImportXML
{
public:
    WxGlade();

    auto Import(const std::string& filename, bool write_doc = true) -> bool override;

    auto HandleUnknownProperty(const pugi::xml_node& /* xml_obj */, Node* /* node */,
                               Node* /* parent */) -> bool override;

    auto HandleNormalProperty(const pugi::xml_node& /* xml_obj */, Node* /* node */,
                              Node* /* parent */, GenEnum::PropName /* wxue_prop */)
        -> bool override;

protected:
    auto CreateGladeNode(pugi::xml_node& xml_obj, Node* parent, Node* sizeritem = nullptr)
        -> NodeSharedPtr;

    // parent should be a wxMenuBar
    auto CreateMenus(pugi::xml_node& xml_obj, Node* parent) -> void;

    // parent should be a wxToolBar
    auto CreateToolbar(pugi::xml_node& xml_obj, Node* parent) -> void;

private:
    // Helper for CreateGladeNode()
    auto TryResolveUnknownGenerator(std::string_view object_name, pugi::xml_node& xml_obj,
                                    Node* parent) -> GenEnum::GenName;

    // Helper for CreateGladeNode()
    auto HandleNotebookPageCreation(GenEnum::GenName get_GenName, pugi::xml_node& xml_obj,
                                    Node* parent) -> NodeSharedPtr;

    // Helper for CreateGladeNode()
    auto ProcessStdDialogButtonSizer(pugi::xml_node& xml_obj, Node* new_node) -> void;
};
