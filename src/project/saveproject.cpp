//////////////////////////////////////////////////////////////////////////
// Purpose:   Save a wxUiEditor project file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-27-2026]

#include "base_generator.h"   // BaseGenerator class
#include "mainapp.h"          // App -- Main application class
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "prop_decl.h"        // PropChildDeclaration and PropDeclaration classes
#include "wxue_namespace/wxue_string.h"
#include "wxue_namespace/wxue_string_vector.h"

#include "pugixml.hpp"

using namespace GenEnum;

void Node::CreateDoc(pugi::xml_document& xml_doc)
{
    pugi::xml_node root = xml_doc.append_child("wxUiEditorData");
    pugi::xml_node node = root.append_child("node");

    int project_version = minRequiredVer;
    if (Project.is_ProjectUpdated())
    {
        project_version = Project.get_ProjectVersion();
    }
    AddNodeToDoc(node, project_version);
    root.append_attribute("data_version") = project_version;
}

void Node::AddNodeToDoc(pugi::xml_node& node, int& project_version)
{
    if (project_version < curSupportedVer)
    {
        // Don't check if the version is already as high as we support -- this speeds up the process
        if (BaseGenerator* generator = get_Generator(); generator)
        {
            if (const int required_ver = generator->GetRequiredVersion(this);
                required_ver > project_version)
            {
                project_version = required_ver;
            }
        }
    }

    node.append_attribute("class") = get_DeclName();

    for (auto& iter: m_properties)
    {
        const wxue::string& value = iter.as_string();
        if (!value.empty())
        {
            const PropDeclaration* info = iter.get_PropDeclaration();

            // If the value hasn't changed from the default, don't save it
            if (info->getDefaultValue() == value)
            {
                continue;
            }

            pugi::xml_attribute attr = node.append_attribute(iter.get_DeclName());
            if (iter.type() == type_bool)
            {
                std::ignore = attr.set_value(iter.as_bool());
            }
            else
            {
                if (iter.is_Type(type_image) || iter.is_Type(type_animation))
                {
                    // Normalize using forward slashes, no spaces after ';' and no size info unless
                    // it is an SVG file

                    wxue::StringVector parts(value, ';', wxue::TRIM::both);
                    if (parts.size() < 2)
                    {
                        node.remove_attribute(attr);
                        continue;
                    }

                    wxue::string description(parts[0]);
                    parts[1].backslashestoforward();
                    description << ';' << parts[1];

                    if (parts.size() > 2 &&
                        (parts[0].starts_with("SVG") || parts[0].starts_with("Art")))
                    {
                        description << ';' << parts[2];
                    }
                    std::ignore = attr.set_value(description.c_str());
                }
                else
                {
                    std::ignore = attr.set_value(value.c_str());
                }
            }
        }
        else
        {
            // Some properties need to be saved with empty values

            if (iter.isProp(prop_label) || iter.isProp(prop_borders))
            {
                node.append_attribute(iter.get_DeclName());
            }
        }
    }

    for (auto& iter: m_map_events)
    {
        const wxue::string& value = iter.second.get_value();
        if (!value.empty())
        {
            node.append_attribute(iter.second.get_name()) = value;
        }
    }

    for (const auto& child: m_children)
    {
        pugi::xml_node child_element = node.append_child("node");
        child->AddNodeToDoc(child_element, project_version);
    }
}
