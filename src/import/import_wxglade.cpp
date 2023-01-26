/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a WxGlade file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "import_wxglade.h"

#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "utils.h"           // Utility functions that work with properties

WxGlade::WxGlade() {}

bool WxGlade::Import(const tt_wxString& filename, bool write_doc)
{
    auto result = LoadDocFile(filename);
    if (!result)
    {
        return false;
    }
    auto root = result.value().first_child();

    if (!tt::is_sameas(root.name(), "application", tt::CASE::either))
    {
        wxMessageBox(wxString() << filename << " is not a wxGlade file", "Import");
        return false;
    }

    // Using a try block means that if at any point it becomes obvious the project file is invalid and we cannot recover,
    // then we can throw an error and give a standard response about an invalid file.

    try
    {
        m_project = NodeCreation.CreateNode(gen_Project, nullptr);
        for (auto& iter: root.children())
        {
            CreateGladeNode(iter, m_project.get());
        }

        if (!m_project->GetChildCount())
        {
            wxMessageBox(wxString() << filename << " does not contain any top level forms.", "Import");
            return false;
        }

        if (write_doc)
            m_project->CreateDoc(m_docOut);
    }

    catch (const std::exception& TESTING_PARAM(e))
    {
        MSG_ERROR(e.what());
        wxMessageBox(wxString("This project file is invalid and cannot be loaded: ") << filename, "Import Project");
        return false;
    }

    return true;
}

NodeSharedPtr WxGlade::CreateGladeNode(pugi::xml_node& xml_obj, Node* parent, Node* sizeritem)
{
    auto object_name = xml_obj.attribute("class").as_cstr();
    if (object_name.empty())
        return NodeSharedPtr();

    bool isBitmapButton = (object_name == "wxBitmapButton");
    auto gen_name = ConvertToGenName(object_name, parent);
    if (gen_name == gen_unknown)
    {
        // If we don't recognize the class, then try the base= attribute
        auto base = xml_obj.attribute("base").as_string();
        if (base == "EditFrame")
        {
            gen_name = ConvertToGenName("wxFrame", parent);
        }
        else if (base == "EditDialog")
        {
            gen_name = ConvertToGenName("wxDialog", parent);
        }
        else if (base == "EditTopLevelPanel")
        {
            gen_name = ConvertToGenName("Panel", parent);
        }

        if (gen_name == gen_unknown)
        {
            // TODO: [KeyWorks - 08-10-2021] wxGlade supports wxMDIChildFrame using a base name of "EditMDIChildFrame"

            // This appears to be a placeholder to reserve a spot. We just ignore it.
            if (object_name == "sizerslot")
                return NodeSharedPtr();

            MSG_INFO(tt_string() << "Unrecognized object: " << object_name);
            return NodeSharedPtr();
        }
    }

    if (gen_name == gen_wxCheckBox)
    {
        for (auto& iter: xml_obj.children())
        {
            if (iter.value() == "style")
            {
                if (iter.text().as_sview().contains("wxCHK_3STATE"))
                    gen_name = gen_Check3State;
                break;
            }
        }
    }

    auto new_node = NodeCreation.CreateNode(gen_name, parent);
    if (gen_name == gen_BookPage && new_node)
    {
        if (!xml_obj.attribute("name").empty())
        {
            if (auto tab = m_notebook_tabs.find(xml_obj.attribute("name").as_string()); tab != m_notebook_tabs.end())
            {
                new_node->prop_set_value(prop_label, tab->second);
            }
        }
    }

    while (!new_node)
    {
        if (parent->isGen(gen_wxNotebook))
        {
            if (gen_name == gen_wxPanel)
            {
                new_node = NodeCreation.CreateNode(gen_BookPage, parent);
                if (new_node)
                {
                    if (!xml_obj.attribute("name").empty())
                    {
                        if (auto tab = m_notebook_tabs.find(xml_obj.attribute("name").as_string());
                            tab != m_notebook_tabs.end())
                        {
                            new_node->prop_set_value(prop_label, tab->second);
                        }
                    }
                    continue;
                }
            }
            else
            {
                if (auto page = NodeCreation.CreateNode(gen_PageCtrl, parent); page)
                {
                    parent->Adopt(page);
                    if (!xml_obj.attribute("name").empty())
                    {
                        if (auto tab = m_notebook_tabs.find(xml_obj.attribute("name").as_string());
                            tab != m_notebook_tabs.end())
                        {
                            page->prop_set_value(prop_label, tab->second);
                        }
                    }

                    new_node = NodeCreation.CreateNode(gen_name, page.get());
                    if (new_node)
                        continue;
                }
            }
        }
        MSG_INFO(tt_string() << "Unable to create " << map_GenNames[gen_name] << " as a child of " << parent->DeclName());
        return NodeSharedPtr();
    }

    if (isBitmapButton)
    {
        new_node->prop_set_value(prop_label, "");
    }

    if (auto prop = new_node->get_prop_ptr(prop_var_name); prop)
    {
        auto original = prop->as_string();
        auto new_name = parent->GetUniqueName(prop->as_string());
        if (new_name.size() && new_name != prop->as_string())
            prop->set_value(new_name);
    }

    if (new_node->isGen(gen_wxStdDialogButtonSizer))
    {
        parent->Adopt(new_node);
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get());

        for (auto& button: xml_obj.children())
        {
            for (auto& btn_id: button.children())
            {
                auto id = btn_id.attribute("name").as_string();
                if (id == "wxID_OK")
                    new_node->get_prop_ptr(prop_OK)->set_value("1");
                else if (id == "wxID_YES")
                    new_node->get_prop_ptr(prop_Yes)->set_value("1");
                else if (id == "wxID_SAVE")
                    new_node->get_prop_ptr(prop_Save)->set_value("1");
                else if (id == "wxID_APPLY")
                    new_node->get_prop_ptr(prop_Apply)->set_value("1");
                else if (id == "wxID_NO")
                    new_node->get_prop_ptr(prop_No)->set_value("1");
                else if (id == "wxID_CANCEL")
                    new_node->get_prop_ptr(prop_Cancel)->set_value("1");
                else if (id == "wxID_CLOSE")
                    new_node->get_prop_ptr(prop_Close)->set_value("1");
                else if (id == "wxID_HELP")
                    new_node->get_prop_ptr(prop_Help)->set_value("1");
                else if (id == "wxID_CONTEXT_HELP")
                    new_node->get_prop_ptr(prop_ContextHelp)->set_value("1");
            }
        }

        new_node->get_prop_ptr(prop_alignment)->set_value("wxALIGN_RIGHT");
        return new_node;
    }

    auto child = xml_obj.child("object");
    if (NodeCreation.IsOldHostType(new_node->DeclName()))
    {
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get(), parent);
        new_node = CreateGladeNode(child, parent, new_node.get());
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
        parent->Adopt(new_node);
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
        CreateGladeNode(child, new_node.get());
        child = child.next_sibling("object");
    }

    return new_node;
}
