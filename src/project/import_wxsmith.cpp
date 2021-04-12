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

bool WxSmith::Import(const ttString& filename)
{
    pugi::xml_document doc;

    if (auto result = doc.load_file(filename.wx_str()); !result)
    {
        appMsgBox(_ttc(strIdCantOpen) << filename.wx_str() << "\n\n" << result.description(), _tt(stdImportFile));
        return false;
    }

    auto root = doc.first_child();
    if (!ttlib::is_sameas(root.name(), "wxsmith", tt::CASE::either) &&
        !ttlib::is_sameas(root.name(), "resource", tt::CASE::either))
    {
        appMsgBox(filename.wx_str() + _ttc(" is not a wxSmith or XRC file"), _tt("Import"));
        return false;
    }

    auto project = g_NodeCreator.CreateNode("Project", nullptr);
    for (auto& iter: root.children())
    {
        CreateXrcNode(iter, project.get());
    }

    if (!project->GetChildCount())
    {
        appMsgBox(filename.wx_str() + _ttc(" does not contain any top level forms."), _tt("Import"));
        return false;
    }

    project->CreateDoc(m_docOut);

    return true;
}

NodeSharedPtr WxSmith::CreateXrcNode(pugi::xml_node& xml_obj, Node* parent, Node* sizeritem)
{
    auto class_name = xml_obj.attribute("class").as_cstr();
    if (class_name.empty())
        return NodeSharedPtr();

    if (class_name == "wxBitmapButton")
    {
        class_name = "wxButton";
    }
    else if (class_name.is_sameas("wxPanel") && parent->DeclName().contains("book"))
    {
        class_name = "BookPage";
    }
    else if (class_name.contains("bookpage"))
    {
        class_name = "oldbookpage";
    }

    if (auto declaration = g_NodeCreator.GetNodeDeclaration(class_name); !declaration)
    {
        MSG_INFO(ttlib::cstr() << "Unrecognized class: " << class_name);
        // TODO: [KeyWorks - 02-25-2021] Need a way to let the user know about this
        return NodeSharedPtr();
    }

    auto new_node = g_NodeCreator.CreateNode(class_name, parent);
    while (!new_node)
    {
        if (parent->isGen(gen_Project))
        {
            if (class_name == "wxPanel")
            {
                class_name = "PanelForm";
                new_node = g_NodeCreator.CreateNode(class_name, parent);
                continue;
            }
            else if (class_name == "wxMenuBar")
            {
                class_name = "MenuBar";
                new_node = g_NodeCreator.CreateNode(class_name, parent);
                continue;
            }
            else if (class_name == "wxToolBar")
            {
                class_name = "ToolBar";
                new_node = g_NodeCreator.CreateNode(class_name, parent);
                continue;
            }
        }

        return NodeSharedPtr();
    }

    if (auto prop = new_node->get_prop_ptr("var_name"); prop)
    {
        auto original = prop->as_string();
        auto new_name = parent->GetUniqueName(prop->as_string());
        if (new_name.size() && new_name != prop->as_string())
            prop->set_value(new_name);
    }

    if (new_node->isGen(gen_wxStdDialogButtonSizer))
    {
        parent->AddChild(new_node);
        new_node->SetParent(parent->GetSharedPtr());
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get());

        for (auto& button: xml_obj.children())
        {
            for (auto& btn_id: button.children())
            {
                auto id = btn_id.attribute("name").as_cview();
                if (id.is_sameas("wxID_OK"))
                    new_node->get_prop_ptr("OK")->set_value("1");
                else if (id.is_sameas("wxID_YES"))
                    new_node->get_prop_ptr("Yes")->set_value("1");
                else if (id.is_sameas("wxID_SAVE"))
                    new_node->get_prop_ptr("Save")->set_value("1");
                else if (id.is_sameas("wxID_APPLY"))
                    new_node->get_prop_ptr("Apply")->set_value("1");
                else if (id.is_sameas("wxID_NO"))
                    new_node->get_prop_ptr("No")->set_value("1");
                else if (id.is_sameas("wxID_CANCEL"))
                    new_node->get_prop_ptr("Cancel")->set_value("1");
                else if (id.is_sameas("wxID_CLOSE"))
                    new_node->get_prop_ptr("Close")->set_value("1");
                else if (id.is_sameas("wxID_HELP"))
                    new_node->get_prop_ptr("Help")->set_value("1");
                else if (id.is_sameas("wxID_CONTEXT_HELP"))
                    new_node->get_prop_ptr("ContextHelp")->set_value("1");
            }
        }

        new_node->get_prop_ptr("alignment")->set_value("wxALIGN_RIGHT");
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
            new_node->get_prop_ptr("static_line")->set_value(false);
        child = child.next_sibling("object");
    }
    else if (sizeritem)
    {
        for (auto& iter: sizeritem->get_props_vector())
        {
            auto prop = new_node->AddNodeProperty(iter.GetPropDeclaration());
            prop->set_value(iter.as_string());
        }
        parent->AddChild(new_node);
        new_node->SetParent(parent->GetSharedPtr());
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get());
    }
    else if (parent)
    {
        parent->AddChild(new_node);
        new_node->SetParent(parent->GetSharedPtr());

        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get());
    }

    while (child)
    {
        CreateXrcNode(child, new_node.get());
        child = child.next_sibling("object");
    }

    return new_node;
}

// Call this AFTER the node has been hooked up to it's parent to prevent duplicate var_names.
void WxSmith::ProcessAttributes(const pugi::xml_node& xml_obj, Node* new_node)
{
    for (auto& iter: xml_obj.attributes())
    {
        if (iter.cname().is_sameas("name"))
        {
            if (new_node->IsForm())
            {
                if (auto prop = new_node->get_prop_ptr("class_name"); prop)
                {
                    prop->set_value(iter.value());
                }
            }
            else if (iter.as_cview().is_sameprefix("wxID_"))
            {
                auto prop = new_node->get_prop_ptr("id");
                if (prop)
                {
                    prop->set_value(iter.value());
                }
                else if (prop = new_node->get_prop_ptr("var_name"); prop)
                {
                    prop->set_value(iter.value());
                }
            }

            else
            {
                // In a wxSmith file, name is the ID and variable is the var_name
                if (!xml_obj.attribute("variable").empty())
                {
                    if (auto prop = new_node->get_prop_ptr("id"); prop)
                    {
                        prop->set_value(iter.value());
                    }
                    continue;
                }

                if (auto prop = new_node->get_prop_ptr("var_name"); prop)
                {
                    ttlib::cstr org_name(iter.value());
                    auto new_name = new_node->GetUniqueName(org_name);
                    prop->set_value(new_name);
                }
            }
        }
        else if (iter.cname().is_sameas("variable"))
        {
            if (auto prop = new_node->get_prop_ptr("var_name"); prop)
            {
                ttlib::cstr org_name(iter.value());
                auto new_name = new_node->GetUniqueName(org_name);
                prop->set_value(new_name);
            }
        }
    }
}

void WxSmith::ProcessProperties(const pugi::xml_node& xml_obj, Node* node, Node* parent)
{
    for (auto& iter: xml_obj.children())
    {
        if (iter.cname().is_sameas("object"))
        {
            continue;
        }

        // Start by processing names that wxUiEditor might use but that need special processing when importing.

        if (iter.cname().is_sameas("bitmap"))
        {
            ProcessBitmap(iter, node);
            continue;
        }
        else if (iter.cname().is_sameas("value"))
        {
            auto escaped = ConvertEscapeSlashes(iter.text().as_string());
            if (auto prop = node->get_prop_ptr(iter.cname()); prop)
            {
                prop->set_value(escaped);
            }
            continue;
        }
        else if (iter.cname().is_sameas("label"))
        {
            ttlib::cstr label = iter.text().as_string();
            label.Replace("_", "&");
            auto pos = label.find("\\t");
            if (ttlib::is_found(pos))
            {
                label[pos] = 0;
                if (auto prop_shortcut = node->get_prop_ptr("shortcut"); prop_shortcut)
                    prop_shortcut->set_value(label.subview(pos + 2));
            }
            if (auto prop = node->get_prop_ptr("label"); prop)
            {
                prop->set_value(label);
            }
            continue;
        }
        else if (iter.cname().is_sameas("option"))
        {
            if (auto prop = node->get_prop_ptr("proportion"); prop)
            {
                prop->set_value(iter.text().as_string());
                continue;
            }
        }

        // Now process names that are identical.

        auto prop = node->get_prop_ptr(iter.cname());
        if (prop)
        {
            prop->set_value(iter.text().as_string());
            continue;
        }

        // Finally, process names that are unique to XRC/wxSmith

        if (iter.cname().is_sameas("orient"))
        {
            prop = node->get_prop_ptr("orientation");
            if (prop)
            {
                prop->set_value(iter.text().as_string());
            }
        }
        else if (iter.cname().is_sameas("border"))
        {
            node->prop_set_value(prop_border_size, iter.text().as_string());
        }
        else if (iter.cname().is_sameas("flag") &&
                 (node->isGen(gen_sizeritem) || node->isGen(gen_gbsizeritem)))
        {
            HandleSizerItemProperty(iter, node, parent);
        }
        else if (iter.cname().is_sameas("handler"))
        {
            ProcessHandler(iter, node);
        }
    }
}

void WxSmith::ProcessBitmap(const pugi::xml_node& xml_obj, Node* node)
{
    if (!xml_obj.attribute("stock_id").empty())
    {
        ttlib::cstr bitmap("Art; ");
        bitmap << xml_obj.attribute("stock_id").value() << "; ";
        if (!xml_obj.attribute("stock_client").empty())
            bitmap << xml_obj.attribute("stock_client").value();
        bitmap << "; [-1; -1]";

        if (auto prop = node->get_prop_ptr("bitmap"); prop)
        {
            prop->set_value(bitmap);
        }
    }
}

void WxSmith::ProcessHandler(const pugi::xml_node& xml_obj, Node* node)
{
    if (xml_obj.attribute("function").empty() || xml_obj.attribute("entry").empty())
        return;

    ttlib::cstr event_name("wx");
    event_name << xml_obj.attribute("entry").value();
    auto event = node->GetEvent(event_name);
    if (event)
    {
        event->set_value(xml_obj.attribute("function").value());
        return;
    }
}
