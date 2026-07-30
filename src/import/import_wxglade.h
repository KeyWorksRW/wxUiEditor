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
    // Helper for CreateGladeNode()
    GenEnum::GenName TryResolveUnknownGenerator(std::string_view object_name,
                                                pugi::xml_node& xml_obj, Node* parent);

    // Helper for CreateGladeNode()
    NodeSharedPtr HandleNotebookPageCreation(GenEnum::GenName get_GenName, pugi::xml_node& xml_obj,
                                             Node* parent);

    // Helper for CreateGladeNode()
    void ProcessStdDialogButtonSizer(pugi::xml_node& xml_obj, Node* new_node);
};
