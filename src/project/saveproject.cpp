//////////////////////////////////////////////////////////////////////////
// Purpose:   Save a wxUiEditor project file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "mainapp.h"    // App -- Main application class
#include "node.h"       // Node class
#include "prop_info.h"  // PropDefinition and PropertyInfo classes

void Node::CreateDoc(pugi::xml_document& doc)
{
    auto root = doc.append_child("wxUiEditorData");
    root.append_attribute("data_version") = (curWxuiMajorVer * 10) + curWxuiMinorVer;

    auto node = root.append_child("node");

    AddNodeToDoc(node);
}

void Node::AddNodeToDoc(pugi::xml_node& node)
{
    node.append_attribute("class") = GetClassName().c_str();

    for (auto& iter: m_properties)
    {
        auto& value = iter.as_string();
        if (value.size())
        {
            auto info = iter.GetPropertyInfo();

            // If the value hasn't changed from the default, don't save it
            if (info->GetDefaultValue() == value)
                continue;

            auto attr = node.append_attribute(iter.GetPropName().c_str());
            if (auto type = iter.GetType(); type == Type::Bool)
                attr.set_value(iter.as_bool());
            else
                attr.set_value(value.c_str());
        }
        else
        {
            // Some properties need to be saved with empty values

            if (iter.GetPropName() == txt_label || iter.GetPropName() == txt_borders)
            {
                node.append_attribute(iter.GetPropName().c_str());
            }
        }
    }

    for (auto& iter: m_events)
    {
        auto& value = iter.get_value();
        if (value.size())
            node.append_attribute(iter.get_name().c_str()) = value.c_str();
    }

    for (size_t i = 0; i < GetChildCount(); i++)
    {
        auto child = GetChild(i);
        auto child_element = node.append_child("node");
        child->AddNodeToDoc(child_element);
    }
}
