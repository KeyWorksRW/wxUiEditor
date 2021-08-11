/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base class for XML importing
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <optional>

#include "../../pugixml/pugixml.hpp"

#include "node.h"  // Node class

class ImportXML
{
public:
    virtual bool Import(const ttString& filename, bool write_doc = true) = 0;

    // Valid return ONLY if Import specified with write_doc == true, and parsing worked.
    pugi::xml_document& GetDocument() { return m_docOut; }

    NodeSharedPtr GetProjectPtr() { return m_project; }

protected:
    std::optional<pugi::xml_document> LoadDocFile(const ttString& file);
    std::optional<GenName> ConvertToGenName(const ttlib::cstr& object_name, Node* parent);

    void HandleSizerItemProperty(const pugi::xml_node& xml_prop, Node* node, Node* parent = nullptr);
    void ProcessStyle(pugi::xml_node& xml_prop, Node* node, NodeProperty* prop);
    void ProcessAttributes(const pugi::xml_node& xml_obj, Node* node);
    void ProcessContent(const pugi::xml_node& xml_obj, Node* node);
    void ProcessNotebookTabs(const pugi::xml_node& xml_obj, Node* node);
    void ProcessBitmap(const pugi::xml_node& xml_obj, Node* node);
    void ProcessHandler(const pugi::xml_node& xml_obj, Node* node);
    void ProcessProperties(const pugi::xml_node& xml_obj, Node* node, Node* parent = nullptr);

    pugi::xml_document m_docOut;
    ttString m_importProjectFile;
    NodeSharedPtr m_project;
    std::map<std::string, std::string> m_notebook_tabs;
};
