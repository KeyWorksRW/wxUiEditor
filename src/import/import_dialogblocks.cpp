/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a DialogBlocks project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/*
 * DialogBlocks uses <document> for objects, and all properties are stored as <string>, <long>
 * or <bool> nodes.
 *
 * Note that the text for <strings> is typically in quotes, so call ExtractQuotedString() to
 * get the string without quotes.
 *
 * Class names are stored in either "proxy-Base class" or "proxy-type" attributes. For the
 * latter, change "wb" to "wx" and remove "Proxy" from the end to get the wxWidgets class name.
 *
 * Styles are typically stored as a series of <bool> notes with the name containing a prefix of
 * "proxy-" followed by the style name. For example, "proxy-wxTAB_TRAVERSAL" would be the name
 * for the wxTAB_TRAVERSAL style.
 *
 * The styles are not separated into individual properties -- they can aply to prop_style,
 * prop_ex_style, prop_window_style, prop_window_ex_style, prop_alignment, prop_borders, etc.
*/

#include "import_dialogblocks.h"

#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator class

DialogBlocks::DialogBlocks() {}

bool DialogBlocks::Import(const tt_wxString& filename, bool write_doc)
{
    auto result = LoadDocFile(filename);
    if (!result)
    {
        wxMessageBox(wxString() << "Unable to load " << filename << " -- was it saved as a binary file?",
                     "Import DialogBlocks project");
        return false;
    }

    auto root = result.value().first_child();

    if (!tt::is_sameas(root.name(), "anthemion-project", tt::CASE::either))
    {
        wxMessageBox(wxString() << filename << " is not a DialogBlocks file", "Import DialogBlocks project");
        return false;
    }

    // Using a try block means that if at any point it becomes obvious the formbuilder file is invalid and we cannot recover,
    // then we can throw an error and give a standard response about an invalid file.

    try
    {
        auto header = root.child("header");
        if (!header)
        {
            FAIL_MSG("DialogBlocks project file does not have a root \"header\" node.")
            throw std::runtime_error("Invalid project file");
        }

        m_project = NodeCreation.CreateNode(gen_Project, nullptr);
        m_project->set_value(prop_code_preference, "C++");

        auto option = header.find_child_by_attribute("string", "name", "target_wx_version");
        if (option)
        {
            auto version = ExtractQuotedString(option);
            if (version == "3.1.0")
                m_project->set_value(prop_wxWidgets_version, "3.1");
            else if (version == "3.2.0")
                m_project->set_value(prop_wxWidgets_version, "3.2");
        }

        option = header.find_child_by_attribute("bool", "name", "translate_strings");
        if (option && option.text().as_bool())
        {
            m_project->set_value(prop_internationalize, true);
        }

        option = header.find_child_by_attribute("bool", "name", "use_enums");
        if (option)
        {
            m_use_enums = option.text().as_bool();
        }

        option = header.find_child_by_attribute("string", "name", "xrc_filename");
        if (option)
        {
            m_project->set_value(prop_combined_xrc_file, ExtractQuotedString(option));
        }

        if (auto documents = root.child("documents"); documents)
        {
            if (auto project = documents.child("document"); project)
            {
                if (auto Windows = project.child("document"); Windows)
                {
                    for (auto& form: Windows.children("document"))
                    {
                        CreateFormNode(form);
                    }
                }
            }
        }

        if (write_doc)
            m_project->CreateDoc(m_docOut);
        return true;
    }

    catch (const std::exception& TESTING_PARAM(e))
    {
        MSG_ERROR(e.what());
        wxMessageBox(wxString("This DialogBlocks project file is invalid and cannot be loaded: ") << filename,
                     "Import DialogBlocks project");
        return false;
    }

    return true;
}

void DialogBlocks::CreateFormNode(pugi::xml_node& form_xml)
{
    GenEnum::GenName gen_name = gen_unknown;
    if (auto base_class = form_xml.find_child_by_attribute("string", "name", "proxy-Base class"); base_class)
    {
        gen_name = MapClassName(ExtractQuotedString(base_class));
        auto form = NodeCreation.CreateNode(gen_name, m_project.get());
        if (!form)
        {
            m_errors.emplace(tt_string("Unable to create ") << ExtractQuotedString(base_class));
            return;
        }
        m_project->Adopt(form);

        // Start be setting properties common to most forms

        if (auto prop = form->get_prop_ptr(prop_class_name); prop)
        {
            if (auto value = form_xml.find_child_by_attribute("string", "name", "proxy-Class"); value)
            {
                prop->set_value(ExtractQuotedString(value));
            }
        }

        if (auto prop = form->get_prop_ptr(prop_base_file); prop)
        {
            if (auto value = form_xml.find_child_by_attribute("string", "name", "proxy-Implementation filename"); value)
            {
                auto file = ExtractQuotedString(value);
                file.remove_extension();
                prop->set_value(file);
            }
        }

        if (auto prop = form->get_prop_ptr(prop_title); prop)
        {
            if (auto value = form_xml.find_child_by_attribute("string", "name", "proxy-Title"); value)
            {
                prop->set_value(ExtractQuotedString(value));
            }
        }

        if (auto prop = form->get_prop_ptr(prop_center); prop)
        {
            if (auto value = form_xml.find_child_by_attribute("bool", "name", "proxy-Centre");
                value && value.text().as_bool())
            {
                prop->set_value("wxBOTH");
            }
            else
            {
                prop->set_value("no");
            }
        }

        for (auto& child_xml: form_xml.children("document"))
        {
            CreateChildNode(child_xml, form);
        }
    }
    else
    {
        FAIL_MSG("Missing proxy-Base class -- unable to determine class to create")
    }
}

void DialogBlocks::CreateChildNode(pugi::xml_node& child_xml, const NodeSharedPtr& parent)
{
    auto gen = FindGenerator(child_xml);
    if (gen == gen_unknown)
    {
        auto type = child_xml.find_child_by_attribute("string", "name", "proxy-type");
        if (!type)
        {
            m_errors.emplace(tt_string("Unable to determine class due to missing \"proxy-type\" property."));
        }
        else
        {
            m_errors.emplace(tt_string("Unrecognized class in \"proxy-type\" property: ") << ExtractQuotedString(type));
        }
        return;
    }

    auto node = NodeCreation.CreateNode(gen, parent.get());
    ASSERT(node);
    if (!node)
    {
        m_errors.emplace(tt_string("Unable to create ") << map_GenNames[gen]);
        return;
    }

    parent->Adopt(node);

    for (auto& grand_child_xml: child_xml.children("document"))
    {
        CreateChildNode(grand_child_xml, node);
    }
}

GenEnum::GenName DialogBlocks::FindGenerator(pugi::xml_node& node)
{
    GenEnum::GenName gen = gen_unknown;

    if (auto class_name = node.find_child_by_attribute("string", "name", "proxy-Base class"); class_name)
    {
        gen = MapClassName(ExtractQuotedString(class_name));
    }

    if (gen == gen_unknown)
    {
        // proxy-type starts with "wb" and ends with "Proxy". Change the "wb" to "wx" and remove
        // the "Proxy" to get the class name.

        if (auto type = node.find_child_by_attribute("string", "name", "proxy-type"); type)
        {
            auto type_name = ExtractQuotedString(type);
            if (type_name.starts_with("wb"))
            {
                type_name[1] = 'x';
            }
            type_name.Replace("Proxy", "");
            gen = MapClassName(type_name);
        }
    }

    return gen;
}

tt_string DialogBlocks::ExtractQuotedString(pugi::xml_node& str_node)
{
    tt_string str;
    auto view = str_node.text().as_sview();
    if (view.starts_with("\""))
        str.ExtractSubString(view);
    else
        str = view;
    return str;
}
