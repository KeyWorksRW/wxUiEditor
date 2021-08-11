/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxSmith or XRC file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "import_wxsmith.h"

#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "mainapp.h"         // App -- Main application class
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "pjtsettings.h"     // ProjectSettings -- Hold data for currently loaded project
#include "uifuncs.h"         // Miscellaneous functions for displaying UI
#include "utils.h"           // Utility functions that work with properties

WxSmith::WxSmith() {}

bool WxSmith::Import(const ttString& filename, bool write_doc)
{
    auto result = LoadDocFile(filename);
    if (!result)
    {
        return false;
    }
    auto root = result.value().first_child();

    if (!ttlib::is_sameas(root.name(), "wxsmith", tt::CASE::either) &&
        !ttlib::is_sameas(root.name(), "resource", tt::CASE::either))
    {
        appMsgBox(filename.wx_str() + _ttc(" is not a wxSmith or XRC file"), _tt("Import"));
        return false;
    }

    // Using a try block means that if at any point it becomes obvious the project file is invalid and we cannot recover,
    // then we can throw an error and give a standard response about an invalid file.

    try
    {
        m_project = g_NodeCreator.CreateNode(gen_Project, nullptr);
        for (auto& iter: root.children())
        {
            CreateXrcNode(iter, m_project.get());
        }

        if (!m_project->GetChildCount())
        {
            appMsgBox(filename.wx_str() + _ttc(" does not contain any top level forms."), _tt("Import"));
            return false;
        }

        if (write_doc)
            m_project->CreateDoc(m_docOut);
    }

    catch (const std::exception& DBG_PARAM(e))
    {
        MSG_ERROR(e.what());
        appMsgBox(ttlib::cstr("This project file is invalid and cannot be loaded: ") << filename.wx_str(), "Import Project");
        return false;
    }

    return true;
}

NodeSharedPtr WxSmith::CreateXrcNode(pugi::xml_node& xml_obj, Node* parent, Node* sizeritem)
{
    auto object_name = xml_obj.attribute("class").as_cstr();
    if (object_name.empty())
        return NodeSharedPtr();

    bool isBitmapButton = (object_name == "wxBitmapButton");
    auto result = ConvertToGenName(object_name, parent);
    if (!result)
    {
        MSG_INFO(ttlib::cstr() << "Unrecognized object: " << object_name);
        return NodeSharedPtr();
    }
    auto gen_name = result.value();

    if (gen_name == gen_wxCheckBox)
    {
        for (auto& iter: xml_obj.children())
        {
            if (iter.cname().is_sameas("style"))
            {
                if (iter.text().as_cview().contains("wxCHK_3STATE"))
                    gen_name = gen_Check3State;
                break;
            }
        }
    }

    auto new_node = g_NodeCreator.CreateNode(gen_name, parent);
    while (!new_node)
    {
        if (sizeritem && sizeritem->isGen(gen_oldbookpage))
        {
            if (auto page = g_NodeCreator.CreateNode(gen_PageCtrl, parent); page)
            {
                if (sizeritem->HasValue(prop_label))
                {
                    page->prop_set_value(prop_label, sizeritem->prop_as_string(prop_label));
                }
                parent->Adopt(page);
                return CreateXrcNode(xml_obj, page.get(), sizeritem);
            }
        }

        // parent will be null if pasting from the clipboard
        if (parent)
        {
            MSG_INFO(ttlib::cstr() << "Unable to create " << map_GenNames[gen_name] << " as a child of "
                                   << parent->DeclName());
        }
        else
        {
            MSG_INFO(ttlib::cstr() << "Unable to create " << map_GenNames[gen_name]);
        }
        return NodeSharedPtr();
    }

    if (isBitmapButton)
    {
        new_node->prop_set_value(prop_label, "");
        isBitmapButton = false;
    }

    if (parent)
    {
        if (auto prop = new_node->get_prop_ptr(prop_var_name); prop)
        {
            auto original = prop->as_string();
            auto new_name = parent->GetUniqueName(prop->as_string());
            if (new_name.size() && new_name != prop->as_string())
                prop->set_value(new_name);
        }
    }

    if (new_node->isGen(gen_wxStdDialogButtonSizer))
    {
        if (parent)
        {
            parent->Adopt(new_node);
        }
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get());

        for (auto& button: xml_obj.children())
        {
            for (auto& btn_id: button.children())
            {
                auto id = btn_id.attribute("name").as_cview();
                if (id.is_sameas("wxID_OK"))
                    new_node->get_prop_ptr(prop_OK)->set_value("1");
                else if (id.is_sameas("wxID_YES"))
                    new_node->get_prop_ptr(prop_Yes)->set_value("1");
                else if (id.is_sameas("wxID_SAVE"))
                    new_node->get_prop_ptr(prop_Save)->set_value("1");
                else if (id.is_sameas("wxID_APPLY"))
                    new_node->get_prop_ptr(prop_Apply)->set_value("1");
                else if (id.is_sameas("wxID_NO"))
                    new_node->get_prop_ptr(prop_No)->set_value("1");
                else if (id.is_sameas("wxID_CANCEL"))
                    new_node->get_prop_ptr(prop_Cancel)->set_value("1");
                else if (id.is_sameas("wxID_CLOSE"))
                    new_node->get_prop_ptr(prop_Close)->set_value("1");
                else if (id.is_sameas("wxID_HELP"))
                    new_node->get_prop_ptr(prop_Help)->set_value("1");
                else if (id.is_sameas("wxID_CONTEXT_HELP"))
                    new_node->get_prop_ptr(prop_ContextHelp)->set_value("1");
            }
        }

        new_node->get_prop_ptr(prop_alignment)->set_value("wxALIGN_RIGHT");
        return new_node;
    }

    auto child = xml_obj.child("object");
    if (g_NodeCreator.IsOldHostType(new_node->DeclName()))
    {
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get(), parent);
        new_node = CreateXrcNode(child, parent, new_node.get());
        // ASSERT(new_node);
        if (!new_node)
            return NodeSharedPtr();
        if (new_node->isGen(gen_wxStdDialogButtonSizer))
            new_node->get_prop_ptr(prop_static_line)->set_value(false);
        child = child.next_sibling("object");
    }
    else if (sizeritem)
    {
        for (auto& iter: sizeritem->get_props_vector())
        {
            auto prop = new_node->AddNodeProperty(iter.GetPropDeclaration());
            prop->set_value(iter.as_string());
        }
        if (parent)
        {
            parent->Adopt(new_node);
        }
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get());
    }
    else if (parent)
    {
        parent->Adopt(new_node);

        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get());
    }

    if (new_node->isGen(gen_wxGridSizer))
    {
        if (new_node->prop_as_int(prop_rows) > 0 && new_node->prop_as_int(prop_cols) > 0)
            new_node->prop_set_value(prop_rows, 0);
    }

    while (child)
    {
        CreateXrcNode(child, new_node.get());
        child = child.next_sibling("object");
    }

    return new_node;
}
