/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a WxGlade file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
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

    if (auto language = root.attribute("language").as_string(); language.size())
    {
        if (language == "XRC")
        {
            m_language = GEN_LANG_XRC;
        }
        else if (language == "python")
        {
            m_language = GEN_LANG_PYTHON;
        }
        else if (language == "C++")
        {
            m_language = GEN_LANG_CPLUSPLUS;
        }
    }

    // Using a try block means that if at any point it becomes obvious the project file is invalid and we cannot recover,
    // then we can throw an error and give a standard response about an invalid file.

    try
    {
        m_project = NodeCreation.CreateNode(gen_Project, nullptr);
        if (auto src_ext = root.attribute("source_extension").as_string(); src_ext.size())
        {
            if (src_ext == ".cpp" || src_ext == ".cc" || src_ext == ".cxx")
            {
                m_project->prop_set_value(prop_source_ext, src_ext);
            }
        }
        if (auto hdr_ext = root.attribute("header_extension").as_string(); hdr_ext.size())
        {
            if (hdr_ext == ".h" || hdr_ext == ".hh" || hdr_ext == ".hpp" || hdr_ext == ".hxx")
            {
                m_project->prop_set_value(prop_header_ext, hdr_ext);
            }
        }
        if (root.attribute("use_gettext").as_bool())
        {
            m_project->prop_set_value(prop_internationalize, true);
        }

        for (auto& iter: root.children())
        {
            auto new_node = CreateGladeNode(iter, m_project.get());
            // In wxGlade, if option is true, then the class name is used for each individual
            // file name
            if (auto single_files = root.attribute("option").as_bool())
            {
                if (single_files)
                {
                    // wxGlade uses the class name as the filename if each class has it's own file.
                    if (new_node->HasValue(prop_class_name))
                    {
                        switch (m_language)
                        {
                            case GEN_LANG_CPLUSPLUS:
                                new_node->prop_set_value(prop_base_file, new_node->value(prop_class_name));
                                break;

                            case GEN_LANG_PYTHON:
                                new_node->prop_set_value(prop_python_file, new_node->value(prop_class_name));
                                break;

                            case GEN_LANG_XRC:
                                new_node->prop_set_value(prop_xrc_file, new_node->value(prop_class_name));
                                break;
                        }
                    }
                }
                else
                {
                    if (m_language == GEN_LANG_PYTHON)
                    {
                        m_project->prop_set_value(prop_python_combine_forms, true);
                        tt_string combined_filename = root.attribute("path").as_string();
                        tt_cwd cwd;
                        combined_filename.make_relative(cwd.utf8_string());
                        m_project->prop_set_value(prop_python_combined_file, combined_filename);
                    }
                }
            }
        }

        if (!m_project->GetChildCount())
        {
            wxMessageBox(wxString() << filename << " does not contain any top level forms.", "Import");
            return false;
        }

        if (!root.attribute("option").as_bool())
        {
            tt_string combined_filename = root.attribute("path").as_string();
            tt_cwd cwd;
            combined_filename.make_relative(cwd.utf8_string());

            if (m_project->GetChildCount() > 1)
            {
                if (m_language == GEN_LANG_PYTHON)
                {
                    m_project->prop_set_value(prop_python_combine_forms, true);
                    m_project->prop_set_value(prop_python_combined_file, combined_filename);
                }
                else if (m_language == GEN_LANG_XRC)
                {
                    m_project->prop_set_value(prop_combine_all_forms, true);
                    m_project->prop_set_value(prop_combined_xrc_file, combined_filename);
                }
            }
            else
            {
                if (m_language == GEN_LANG_PYTHON)
                {
                    m_project->GetChild(0)->set_value(prop_python_file, combined_filename);
                }
                else if (m_language == GEN_LANG_XRC)
                {
                    m_project->GetChild(0)->set_value(prop_xrc_file, combined_filename);
                }
            }
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

        tt_string last_handler;

        for (auto& child: xml_obj.children())
        {
            if (child.name() != "object")
                continue;
            ASSERT(child.attribute("class").as_string() == "sizeritem");

            for (auto& button: child.children())
            {
                if (button.name() != "object")
                    continue;
                last_handler.clear();
                for (auto& btn_props: button.children())
                {
                    auto SetBtnAndHandler = [&](PropName prop_name, tt_string_view event_name)
                    {
                        new_node->get_prop_ptr(prop_name)->set_value("1");
                        if (last_handler.size())
                        {
                            if (auto* event = new_node->GetEvent(event_name); event)
                            {
                                event->set_value(last_handler);
                            }
                        }
                    };

                    if (btn_props.name() == "events")
                    {
                        for (auto& handler: btn_props.children())
                        {
                            last_handler = handler.text().as_string();
                        }
                    }
                    else if (btn_props.name() == "id")
                    {
                        if (auto id = btn_props.text().as_string(); tt::is_sameprefix(id, "wxID_"))
                        {
                            if (id == "wxID_OK")
                                SetBtnAndHandler(prop_OK, "OKButtonClicked");
                            else if (id == "wxID_YES")
                                SetBtnAndHandler(prop_Yes, "YesButtonClicked");
                            else if (id == "wxID_SAVE")
                                SetBtnAndHandler(prop_Save, "SaveButtonClicked");
                            else if (id == "wxID_APPLY")
                                SetBtnAndHandler(prop_Apply, "ApplyButtonClicked");
                            else if (id == "wxID_NO")
                                SetBtnAndHandler(prop_No, "NoButtonClicked");
                            else if (id == "wxID_CANCEL")
                                SetBtnAndHandler(prop_Cancel, "CancelButtonClicked");
                            else if (id == "wxID_CLOSE")
                                SetBtnAndHandler(prop_Close, "CloseButtonClicked");
                            else if (id == "wxID_HELP")
                                SetBtnAndHandler(prop_Help, "HelpButtonClicked");
                            else if (id == "wxID_CONTEXT_HELP")
                                SetBtnAndHandler(prop_ContextHelp, "ContextHelpButtonClicked");
                        }
                    }
                    else if (btn_props.name() == "stockitem")
                    {
                        if (auto id = btn_props.text().as_string(); id.size())
                        {
                            if (id == "OK")
                                SetBtnAndHandler(prop_OK, "OKButtonClicked");
                            else if (id == "YES")
                                SetBtnAndHandler(prop_Yes, "YesButtonClicked");
                            else if (id == "SAVE")
                                SetBtnAndHandler(prop_Save, "SaveButtonClicked");
                            else if (id == "APPLY")
                                SetBtnAndHandler(prop_Apply, "ApplyButtonClicked");
                            else if (id == "NO")
                                SetBtnAndHandler(prop_No, "NoButtonClicked");
                            else if (id == "CANCEL")
                                SetBtnAndHandler(prop_Cancel, "CancelButtonClicked");
                            else if (id == "CLOSE")
                                SetBtnAndHandler(prop_Close, "CloseButtonClicked");
                            else if (id == "HELP")
                                SetBtnAndHandler(prop_Help, "HelpButtonClicked");
                        }
                    }
                }
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

// Called by ImportXML -- return true if the property is processed.
bool WxGlade::HandleUnknownProperty(const pugi::xml_node& xml_obj, Node* node, Node* /* parent */)
{
    auto node_name = xml_obj.name();
    if (node_name == "attribute")
    {
        // Technically, this is a bool value, but currently wxGlade only outputs it if the
        // value is 1. It is used to indicate that the variable name should be prefixed with
        // "self." to make it a class member variable.
        node->set_value(prop_class_access, "protected:");
        return true;
    }
    else if (node_name == "events")
    {
        for (auto& handler: xml_obj.children())
        {
            tt_string event_name("wx");
            event_name += handler.attribute("event").as_string();
            if (auto* event = node->GetEvent(event_name); event)
            {
                event->set_value(handler.text().as_string());
            }
        }

        return true;
    }
    else if (node_name == "extracode_post")
    {
        // wxGlade adds this after the class is constructed, but before any Bind functions are called.
        // Currently, wxUiEditor doesn't support this, so just ignore it.

        return true;
    }
    else if (node_name == "affirmative" || node_name == "escape")
    {
        // wxGlade adds these even when the exact same buttons
    }
    return false;
}

// Called by ImportXML -- return true if the property is processed. Use this when the property conversion
// is incorrect for the type of note being processed.
bool WxGlade::HandleNormalProperty(const pugi::xml_node& xml_obj, Node* node, Node* parent, GenEnum::PropName wxue_prop)
{
    if (node->isGen(gen_sizeritem))
    {
        // wxGlade sizeritems use slightly different property names then we do, so handle those
        // here.
        if (wxue_prop == prop_border)
        {
            // wxGlade uses border for border_size in a sizer
            node->prop_set_value(prop_border_size, xml_obj.text().as_string());
            return true;
        }
        else if (wxue_prop == prop_flag)
        {
            HandleSizerItemProperty(xml_obj, node, parent);
            return true;
        }
    }
    else if (wxue_prop == prop_id)
    {
        tt_string id = xml_obj.text().as_string();
        id.erase_from('=');
        id.trim();
        node->prop_set_value(prop_id, id);
        return true;
    }

    return false;
}
