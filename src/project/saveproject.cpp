//////////////////////////////////////////////////////////////////////////
// Purpose:   Save a wxUiEditor project file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "base_generator.h"  // BaseGenerator class
#include "mainapp.h"         // App -- Main application class
#include "node.h"            // Node class
#include "prop_decl.h"       // PropChildDeclaration and PropDeclaration classes

#include "pugixml.hpp"

using namespace GenEnum;

void Node::CreateDoc(pugi::xml_document& doc)
{
    auto root = doc.append_child("wxUiEditorData");
    auto node = root.append_child("node");

    int project_version = minRequiredVer;
    AddNodeToDoc(node, project_version);
    root.append_attribute("data_version") = project_version;
}

void Node::AddNodeToDoc(pugi::xml_node& node, int& project_version)
{
    if (project_version < curSupportedVer)
    {
        // Don't check if the version is alreay as high as we support -- this speeds up the process
        if (auto gen = GetGenerator(); gen)
        {
            if (gen->GetRequiredVersion(this) > project_version)
                project_version = gen->GetRequiredVersion(this);
        }
    }

    node.append_attribute("class") = DeclName();

    for (auto& iter: m_properties)
    {
        auto& value = iter.as_string();
        if (value.size())
        {
            auto info = iter.GetPropDeclaration();

            // If the value hasn't changed from the default, don't save it
            if (info->GetDefaultValue() == value)
                continue;

            auto attr = node.append_attribute(iter.DeclName());
            if (iter.type() == type_bool)
                attr.set_value(iter.as_bool());
            else
            {
                if (iter.isType(type_image) || iter.isType(type_animation))
                {
                    // Normalize using forward slashes, no spaces after ';' and no size info unless it is an SVG file

                    ttlib::multistr parts(value, ';', tt::TRIM::both);
                    if (parts.size() < 2)
                        continue;

                    ttlib::cstr description(parts[0]);
                    parts[1].backslashestoforward();
                    description << ';' << parts[1];

                    if (parts.size() > 2 && parts[0].starts_with("SVG"))
                    {
                        description << ';' << parts[2];
                    }
                    attr.set_value(description.c_str());
                }
                else
                {
                    attr.set_value(value.c_str());
                }
            }
        }
        else
        {
            // Some properties need to be saved with empty values

            if (iter.isProp(prop_label) || iter.isProp(prop_borders))
            {
                node.append_attribute(iter.DeclName());
            }
        }
    }

    for (auto& iter: m_map_events)
    {
        auto& value = iter.second.get_value();
        if (value.size())
        {
            node.append_attribute(iter.second.get_name()) = value;
        }
    }

    for (const auto& child: m_children)
    {
        auto child_element = node.append_child("node");
        child->AddNodeToDoc(child_element, project_version);
    }
}
