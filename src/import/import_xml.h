/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base class for XML importing
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <optional>

#include "pugixml.hpp"

#include "node.h"  // Node class

// This class is used to import both XRC files, and XML files that are loosely based on XRC
// (such as wxFormBuilder projects).

class ImportXML
{
public:
    virtual bool Import(const tt_wxString& filename, bool write_doc = true) = 0;

    // Valid return ONLY if Import specified with write_doc == true, and parsing worked.
    pugi::xml_document& GetDocument() { return m_docOut; }

    NodeSharedPtr GetProjectPtr() { return m_project; }

    auto GetErrors() { return m_errors; }

    // Returns a GEN_LANG_* value -- default is GEN_LANG_NONE
    auto GetLanguage() const { return m_language; }

    // This will check for an obsolete event name, and if found, it will return the 3.x
    // version of the name. Otherwise, it returns name unmodified.
    static tt_string_view GetCorrectEventName(tt_string_view name);

    // Only call this from an XRC importer (e.g., wxSMITH)
    NodeSharedPtr CreateXrcNode(pugi::xml_node& xml_obj, Node* parent, Node* sizeritem = nullptr);

    // Caller should return true if it is able to handle this unknown property
    virtual bool HandleUnknownProperty(const pugi::xml_node& /* xml_obj */, Node* /* node */, Node* /* parent */)
    {
        return false;
    }

    // Caller should return true if it is able to handle this known property. This is used
    // when the property name is knonwn, but it's actually the wrong property for the node
    // type. E.g., prop_border for a sizer really should be prop_border_size.
    virtual bool HandleNormalProperty(const pugi::xml_node& /* xml_obj */, Node* /* node */, Node* /* parent */,
                                      GenEnum::PropName /* wxue_prop */)
    {
        return false;
    }

    void HandleSizerItemProperty(const pugi::xml_node& xml_prop, Node* node, Node* parent = nullptr);

protected:
    void ProcessUnknownProperty(const pugi::xml_node& xml_obj, Node* node, Node* parent);
    std::optional<pugi::xml_document> LoadDocFile(const tt_wxString& file);
    GenEnum::GenName ConvertToGenName(const tt_string& object_name, Node* parent);

    void ProcessStyle(pugi::xml_node& xml_prop, Node* node, NodeProperty* prop);
    void ProcessAttributes(const pugi::xml_node& xml_obj, Node* node);
    void ProcessContent(const pugi::xml_node& xml_obj, Node* node);
    void ProcessNotebookTabs(const pugi::xml_node& xml_obj, Node* node);
    void ProcessBitmap(const pugi::xml_node& xml_obj, Node* node, GenEnum::PropName node_prop = prop_bitmap);
    void ProcessHandler(const pugi::xml_node& xml_obj, Node* node);
    void ProcessProperties(const pugi::xml_node& xml_obj, Node* node, Node* parent = nullptr);

    // Returns prop_unknown if the property name has no equivalent in wxUiEditor
    GenEnum::PropName MapPropName(std::string_view name) const;

    // Returns gen_unknown if the property name has not equivalent in wxUiEditor
    GenEnum::GenName MapClassName(std::string_view name) const;

    pugi::xml_document m_docOut;
    tt_wxString m_importProjectFile;
    NodeSharedPtr m_project;
    std::map<std::string, std::string, std::less<>> m_notebook_tabs;

    std::set<tt_string> m_errors;

    int m_language = GEN_LANG_NONE;
};
