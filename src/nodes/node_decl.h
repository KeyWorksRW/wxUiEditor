/////////////////////////////////////////////////////////////////////////////
// Purpose:   NodeDeclaration class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include <wx/bitmap.h>  // wxBitmap class interface

#include "node_classes.h"  // Forward defintions of Node classes

#include "category.h"
#include "node_types.h"  // NodeType -- Class for storing component types and allowable child count

using DblStrMap = std::map<std::string, std::string, std::less<>>;
using PropertyInfoPtr = std::shared_ptr<PropertyInfo>;
using PropertyInfoMap = std::map<std::string, PropertyInfoPtr>;

namespace pugi
{
    class xml_node;
}

class NodeDeclaration
{
public:
    NodeDeclaration(ttlib::cview class_name, NodeType* type);

    NodeCategory& GetCategory() { return m_category; }

    size_t GetPropertyCount() const { return m_properties.size(); }
    size_t GetEventCount() const { return m_events.size(); }

    PropertyInfo* GetPropertyInfo(size_t idx) const;

    NodeEventInfo* GetEventInfo(ttlib::cview name);
    const NodeEventInfo* GetEventInfo(size_t idx) const;

    PropertyInfoMap& GetPropInfoMap() { return m_properties; }

    void AddBaseClassDefaultPropertyValue(size_t baseIndex, ttlib::cview propertyName, ttlib::cview defaultValue);

    const std::string& GetBaseClassDefaultPropertyValue(size_t baseIndex, const std::string& propertyName) const;

    const ttlib::cstr& GetNodeTypeName() const { return m_type->get_name(); }

    NodeType* GetNodeType() const { return m_type; }

    const ttlib::cstr& GetClassName() const { return m_classname; }

    size_t AddBaseClass(NodeDeclaration* base)
    {
        m_base.emplace_back(base);
        return m_base.size() - 1;
    }

    bool IsSubclassOf(const std::string& classname) const;
    bool isSubclassOf(std::string_view classname) const;

    NodeDeclaration* GetBaseClass(size_t idx, bool inherited = true) const;

    void GetBaseClasses(std::vector<NodeDeclaration*>& classes, bool inherited = true) const;

    size_t GetBaseClassCount(bool inherited = true) const;
    bool hasBaseClasses() const { return m_base.size(); }

    void SetImage(wxImage image) { m_image = image; }
    wxImage GetImage() const { return m_image; }

    void SetGenerator(BaseGenerator* component) { m_component = component; }
    BaseGenerator* GetGenerator() const { return m_component; }

    void ParseEvents(pugi::xml_node& elem_obj, NodeCategory& category);

private:
    ttlib::cstr m_classname;

    wxImage m_image;  // The node's image, primarily used in the navigation pane

    NodeType* m_type;

    NodeCategory m_category;

    PropertyInfoMap m_properties;  // std::map<std::string, PropertyInfoPtr>
    std::map<std::string, std::unique_ptr<NodeEventInfo>> m_events;
    std::map<size_t, DblStrMap> m_baseClassDefaultPropertyValues;

    std::vector<NodeDeclaration*> m_base;  // base classes

    BaseGenerator* m_component { nullptr };
};
