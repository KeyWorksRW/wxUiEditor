/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains the declarations for a node (properties, events, etc.)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "node_decl.h"

// BaseGenerator -- Base widget generator class
#include "base_generator.h"  // IWYU pragma: keep
#include "prop_decl.h"       // PropChildDeclaration and PropDeclaration classes

NodeDeclaration::NodeDeclaration(wxue::string_view class_name, NodeType* type) :
    m_type(type), m_category(class_name), m_gen_name(rmap_GenNames[class_name]),
    m_gen_type(type->get_GenType()), m_name(GenEnum::map_GenNames.at(m_gen_name))
{
}

NodeDeclaration::~NodeDeclaration()
{
    delete m_generator;
}

auto NodeDeclaration::get_PropDeclaration(size_t idx) const -> PropDeclaration*
{
    ASSERT(idx < m_properties.size());

    auto iter = m_properties.begin();
    size_t i = 0;
    while (i < idx && iter != m_properties.end())
    {
        i++;
        iter++;
    }

    if (iter != m_properties.end())
    {
        return iter->second;
    }

    return nullptr;
}

const NodeEventInfo* NodeDeclaration::get_EventInfo(wxue::string_view name) const
{
    if (auto iter = m_events.find(name); iter != m_events.end())
    {
        return iter->second;
    }

    return nullptr;
}

const NodeEventInfo* NodeDeclaration::get_EventInfo(size_t idx) const
{
    ASSERT(idx < m_events.size());

    auto iter = m_events.begin();
    size_t i = 0;
    while (i < idx && iter != m_events.end())
    {
        i++;
        iter++;
    }

    if (iter != m_events.end())
    {
        return iter->second;
    }

    return nullptr;
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
    ASSERT(idx < m_base.size());
    return m_base.at(idx);
}

size_t NodeDeclaration::GetBaseClassCount(bool inherited) const
{
    if (inherited && m_base.size())
    {
        std::vector<NodeDeclaration*> classes;

        // Do the first loop here to avoid recursion if we're only one deep
        for (auto* iter: m_base)
        {
            if (iter == nullptr)
            {
                continue;
            }
            classes.push_back(iter);

            if (iter->hasBaseClasses())
            {
                iter->GetBaseClasses(classes);
            }
        }

        return classes.size();
    }
    return m_base.size();
}

auto NodeDeclaration::GetBaseClasses(std::vector<NodeDeclaration*>& classes, bool inherited) const
    -> void
{
    for (auto* iter: m_base)
    {
        if (iter == nullptr)
        {
            continue;
        }
        classes.emplace_back(iter);

        if (inherited && iter->hasBaseClasses())
        {
            iter->GetBaseClasses(classes);
        }
    }
}

bool NodeDeclaration::isSubclassOf(GenName get_GenName) const
{
    if (get_GenName == m_gen_name)
    {
        return true;
    }
    std::vector<NodeDeclaration*> classes;
    GetBaseClasses(classes);
    for (const auto& iter: classes)
    {
        if (iter->isSubclassOf(get_GenName))
        {
            return true;
        }
    }
    return false;
}

ptrdiff_t NodeDeclaration::get_AllowableChildren(GenType child_gen_type) const
{
    if (m_gen_name == gen_wxFrame)
    {
        if (child_gen_type == type_menubar || child_gen_type == type_statusbar ||
            child_gen_type == type_toolbar)
        {
            return 1;
        }
    }

    return m_type->get_AllowableChildren(child_gen_type);
}

std::optional<wxue::string> NodeDeclaration::GetOverRideDefValue(GenEnum::PropName prop_name)
{
    if (auto result = m_override_def_values.find(prop_name); result != m_override_def_values.end())
    {
        return result->second;
    }
    return {};
}

wxBitmapBundle NodeDeclaration::GetBitmapBundle(int width, int height) const
{
    if (m_bundle_function)
    {
        return m_bundle_function(width, height);
    }

    return wxBitmapBundle();
}
