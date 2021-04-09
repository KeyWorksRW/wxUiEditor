/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains the declarations for a node (properties, events, etc.)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "node_decl.h"

#include "node.h"       // Contains the user-modifiable node
#include "prop_info.h"  // PropDefinition and PropertyInfo classes

NodeDeclaration::NodeDeclaration(ttlib::cview class_name, NodeType* type) :
    m_classname(class_name), m_type(type), m_category(class_name)
{
    m_class_enum = rmap_ClassNames[class_name.c_str()];
    m_class_type = rmap_ClassTypes[type->get_name()];
    m_name = NodeEnums::map_ClassNames[m_class_enum];
}

PropertyInfo* NodeDeclaration::GetPropertyInfo(size_t idx) const
{
    ASSERT(idx < m_properties.size());

    auto it = m_properties.begin();
    size_t i = 0;
    while (i < idx && it != m_properties.end())
    {
        i++;
        it++;
    }

    if (it != m_properties.end())
        return it->second.get();

    return nullptr;
}

NodeEventInfo* NodeDeclaration::GetEventInfo(ttlib::cview name)
{
    if (auto it = m_events.find(name.c_str()); it != m_events.end())
        return it->second.get();

    return nullptr;
}

const NodeEventInfo* NodeDeclaration::GetEventInfo(size_t idx) const
{
    ASSERT(idx < m_events.size());

    auto it = m_events.begin();
    size_t i = 0;
    while (i < idx && it != m_events.end())
    {
        i++;
        it++;
    }

    if (it != m_events.end())
        return it->second.get();

    return nullptr;
}

void NodeDeclaration::AddBaseClassDefaultPropertyValue(size_t baseIndex, ttlib::cview propertyName,
                                                       ttlib::cview defaultValue)
{
    if (auto baseClassMap = m_baseClassDefaultPropertyValues.find(baseIndex);
        baseClassMap != m_baseClassDefaultPropertyValues.end())
    {
        baseClassMap->second[propertyName.c_str()] = defaultValue.c_str();
    }
    else
    {
        DblStrMap propertyDefaultValues;
        propertyDefaultValues[propertyName.c_str()] = defaultValue;
        m_baseClassDefaultPropertyValues[baseIndex] = propertyDefaultValues;
    }
}

const std::string& NodeDeclaration::GetBaseClassDefaultPropertyValue(size_t baseIndex, const std::string& propertyName) const
{
    if (auto baseClassMap = m_baseClassDefaultPropertyValues.find(baseIndex);
        baseClassMap != m_baseClassDefaultPropertyValues.end())
    {
        if (auto defaultValue = baseClassMap->second.find(propertyName); defaultValue != baseClassMap->second.end())
        {
            return defaultValue->second;
        }
    }
    return ttlib::emptystring;
}

NodeDeclaration* NodeDeclaration::GetBaseClass(size_t idx, bool inherited) const
{
    if (inherited)
    {
        std::vector<NodeDeclaration*> classes;
        GetBaseClasses(classes);

        ASSERT(idx < classes.size());
        return classes.at(idx);
    }
    else
    {
        ASSERT(idx < m_base.size());
        return m_base.at(idx);
    }
}

size_t NodeDeclaration::GetBaseClassCount(bool inherited) const
{
    if (inherited && m_base.size())
    {
        std::vector<NodeDeclaration*> classes;

        // Do the first loop here to avoid recursion if we're only one deep
        for (auto& iter: m_base)
        {
            classes.push_back(iter);

            if (iter->hasBaseClasses())
            {
                iter->GetBaseClasses(classes);
            }
        }

        return classes.size();
    }
    else
        return m_base.size();
}

void NodeDeclaration::GetBaseClasses(std::vector<NodeDeclaration*>& classes, bool inherited) const
{
    for (auto& iter: m_base)
    {
        classes.emplace_back(iter);

        if (inherited && iter->hasBaseClasses())
        {
            iter->GetBaseClasses(classes);
        }
    }
}

bool NodeDeclaration::isSubclassOf(ClassName class_name) const
{
    if (class_name == m_class_enum)
    {
        return true;
    }
    else
    {
        std::vector<NodeDeclaration*> classes;
        GetBaseClasses(classes);
        for (auto& iter: classes)
        {
            if (iter->isSubclassOf(class_name))
                return true;
        }
    }
    return false;
}
