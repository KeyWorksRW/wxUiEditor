/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base class for XML importing
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
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
    virtual ~ImportXML() = default;

    virtual auto Import(const std::string& filename, bool write_doc = true) -> bool = 0;

    // Valid return ONLY if Import specified with write_doc == true, and parsing worked.
    auto GetDocument() -> pugi::xml_document& { return m_docOut; }

    auto GetProjectPtr() -> NodeSharedPtr { return m_project; }

    auto GetErrors() { return m_errors; }

    // Returns a GenLang::* value -- default is GenLang::none
    [[nodiscard]] virtual auto GetLanguage() const -> GenLang { return m_language; }

    // This will check for an obsolete event name, and if found, it will return the 3.x
    // version of the name. Otherwise, it returns name unmodified.
    static auto GetCorrectEventName(std::string_view name) -> std::string_view;

    // Only call this from an XRC importer (e.g., wxSMITH)
    auto CreateXrcNode(pugi::xml_node& xml_obj, Node* parent, Node* sizeritem = nullptr)
        -> NodeSharedPtr;

    // Caller should return true if it is able to handle this unknown property
    virtual auto HandleUnknownProperty(const pugi::xml_node& /* xml_obj */, Node* /* node */,
                                       Node* /* parent */) -> bool
    {
        return false;
    }

    // Caller should return true if it is able to handle this known property. This is used
    // when the property name is knonwn, but it's actually the wrong property for the node
    // type. E.g., prop_border for a sizer really should be prop_border_size.
    virtual auto HandleNormalProperty(const pugi::xml_node& /* xml_obj */, Node* /* node */,
                                      Node* /* parent */, GenEnum::PropName /* wxue_prop */) -> bool
    {
        return false;
    }

    void HandleSizerItemProperty(const pugi::xml_node& xml_prop, Node* node,
                                 Node* parent = nullptr);

protected:
    void ProcessFont(const pugi::xml_node& xml_obj, Node* node);
    void ProcessUnknownProperty(const pugi::xml_node& xml_obj, Node* node, Node* parent);
    [[nodiscard]] auto LoadDocFile(const std::string& file) -> std::optional<pugi::xml_document>;
    auto ConvertToGenName(const std::string& object_name, Node* parent) -> GenEnum::GenName;

    void ProcessStyle(pugi::xml_node& xml_prop, Node* node, NodeProperty* prop);
    void ProcessAttributes(const pugi::xml_node& xml_obj, Node* node);
    void ProcessContent(const pugi::xml_node& xml_obj, Node* node);
    void ProcessNotebookTabs(const pugi::xml_node& xml_obj, Node* node);
    void ProcessBitmap(const pugi::xml_node& xml_obj, Node* node,
                       GenEnum::PropName node_prop = prop_bitmap);
    void ProcessHandler(const pugi::xml_node& xml_obj, Node* node);
    void ProcessProperties(const pugi::xml_node& xml_obj, Node* node, Node* parent = nullptr);

    // Returns prop_unknown if the property name has no equivalent in wxUiEditor
    [[nodiscard]] auto MapPropName(std::string_view name) const -> GenEnum::PropName;

    // Returns gen_unknown if the property name has not equivalent in wxUiEditor
    [[nodiscard]] auto MapClassName(std::string_view name) const -> GenEnum::GenName;

    pugi::xml_document m_docOut;
    std::string m_importProjectFile;
    NodeSharedPtr m_project;
    std::map<std::string, std::string, std::less<>> m_notebook_tabs;

    std::set<std::string> m_errors;

    GenLang m_language = GenLang::none;
};
