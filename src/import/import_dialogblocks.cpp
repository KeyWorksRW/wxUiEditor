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

#include <set>

#include <frozen/map.h>
#include <frozen/set.h>
#include <frozen/string.h>

#include "import_dialogblocks.h"

#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator class

DialogBlocks::DialogBlocks() {}

bool DialogBlocks::Import(const tt_string& filename, bool write_doc)
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

    // Using a try block means that if at any point it becomes obvious the file is invalid and
    // we cannot recover, then we can throw an error and give a standard response about an
    // invalid file.

    try
    {
        auto header = root.child("header");
        if (!header)
        {
            FAIL_MSG("DialogBlocks project file does not have a root \"header\" node.")
            throw std::runtime_error("Invalid project file");
        }

        m_project = NodeCreation.createNode(gen_Project, nullptr);
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
                    NodeSharedPtr parent = m_project;
                    for (auto& form: Windows.children("document"))
                    {
                        if (CreateFormNode(form, parent))
                            continue;
                        else if (CreateFolderNode(form, parent))
                            continue;
// clang_analyzer will complain about assigning variables that aren't used
#if defined(_DEBUG) && !defined(__clang_analyzer__)
                        if (auto first_child = form.first_child(); first_child)
                        {
                            auto first_attr = first_child.first_attribute();
                            if (first_attr)
                            {
                                auto sv_name = first_attr.name();
                                auto sv_value = first_attr.value();
                                auto data = first_child.text().as_sview();
                                auto name = first_attr.name();
                            }
                        }
#endif  // _DEBUG
                        FAIL_MSG("Missing proxy-Base class -- unable to determine class to create")
                    }
                }
            }
        }

        if (write_doc)
            m_project->createDoc(m_docOut);
    }

    catch (const std::exception& TESTING_PARAM(e))
    {
        MSG_ERROR(e.what());
        wxMessageBox(wxString("This DialogBlocks project file is invalid and cannot be loaded: ")
                         << filename.make_wxString(),
                     "Import DialogBlocks project");
        return false;
    }

    if (m_errors.size())
    {
        tt_string errMsg("Not everything in the DialogBlocks project could be converted:\n\n");
        MSG_ERROR(tt_string() << "------  " << m_importProjectFile.filename() << "------");
        for (auto& iter: m_errors)
        {
            MSG_ERROR(iter);
            errMsg << iter << '\n';
        }

        wxMessageBox(errMsg, "Import DialogBlocks project");
    }

    return true;
}

bool DialogBlocks::CreateFolderNode(pugi::xml_node& form_xml, const NodeSharedPtr& parent)
{
    if (auto folder = form_xml.find_child_by_attribute("string", "name", "type");
        folder && folder.text().as_sview() == "\"html-folder-document\"")
    {
        if (auto folder_name = form_xml.find_child_by_attribute("string", "name", "title"); folder_name)
        {
            auto gen_folder_type = parent->isGen(gen_folder) ? gen_sub_folder : gen_folder;
            if (auto new_parent = NodeCreation.createNode(gen_folder_type, parent.get()); new_parent)
            {
                new_parent->set_value(prop_label, ExtractQuotedString(folder_name));
                parent->adoptChild(new_parent);
                for (auto& form: form_xml.children("document"))
                {
                    if (CreateFormNode(form, new_parent))
                        continue;
                    else if (CreateFolderNode(form, new_parent))
                        continue;
                }
                return true;
            }
        }
    }
    return false;
}

/*
 * The wxWidgets class is determined via the proxy-type attribute. This will have a "wb"
 * prefix, which is replaced with "wx", and a "Proxy" suffix, which is removed. In most cases,
 * this will then be the same names as proxy-Base class, but if not it means the user wants the
 * base class to be a derived class that they have created.
 */

bool DialogBlocks::CreateFormNode(pugi::xml_node& form_xml, const NodeSharedPtr& parent)
{
    GenEnum::GenName getGenName = gen_unknown;
    if (auto widgets_class = form_xml.find_child_by_attribute("string", "name", "proxy-type"); widgets_class)
    {
        auto type_name = ExtractQuotedString(widgets_class);
        if (type_name.starts_with("wb"))
        {
            type_name[1] = 'x';
        }
        type_name.Replace("Proxy", "");

        getGenName = MapClassName(type_name);
        if (getGenName == gen_unknown)
        {
            if (type_name == "wxApp")
            {
                // Currently, we don't support creating an app class, but we return true since we know
                // it's not a folder.
                return true;
            }
            auto msg = GatherErrorDetails(form_xml, getGenName);
            ASSERT_MSG(getGenName != gen_unknown, tt_string("Unrecognized proxy-type class: ") << type_name << "\n" << msg);
            m_errors.emplace(tt_string("Unrecognized form class: ") << type_name);
            return false;
        }
        else if (getGenName == gen_wxDialog)
        {
            if (auto base_class = form_xml.find_child_by_attribute("string", "name", "proxy-Base class"); base_class)
            {
                auto base_name = ExtractQuotedString(base_class);
                if (base_name == "wxPanel")
                {
                    getGenName = gen_PanelForm;
                }
                else if (base_class = form_xml.find_child_by_attribute("string", "name", "proxy-Window kind"); base_class)
                {
                    base_name = ExtractQuotedString(base_class);
                    if (base_name == "wxPanel")
                    {
                        getGenName = gen_PanelForm;
                    }
                }
            }
        }

        auto form = NodeCreation.createNode(getGenName, parent.get());
        if (!form)
        {
            if (parent->isGen(gen_Project) || parent->isGen(gen_folder) || parent->isGen(gen_sub_folder))
            {
                switch (getGenName)
                {
                    default:
                        {
                            auto msg = GatherErrorDetails(form_xml, getGenName);
                            FAIL_MSG(tt_string() << "Unable to create " << type_name << "\n" << msg)
                            m_errors.emplace(tt_string("Unable to create ") << type_name);
                        }
                        return false;

                    case gen_wxPanel:
                        getGenName = gen_PanelForm;
                        break;

                    case gen_wxMenuBar:
                        getGenName = gen_MenuBar;
                        break;

                    case gen_wxToolBar:
                        getGenName = gen_ToolBar;
                        break;

                    case gen_wxRibbonBar:
                        getGenName = gen_RibbonBar;
                        break;

                    case gen_wxMenu:
                        getGenName = gen_PopupMenu;
                        break;
                }
                if (form = NodeCreation.createNode(getGenName, parent.get()); !form)
                {
                    auto msg = GatherErrorDetails(form_xml, getGenName);
                    FAIL_MSG(tt_string() << "Unable to create " << type_name << "\n" << msg)
                    m_errors.emplace(tt_string("Unable to create ") << type_name);
                    return false;
                }
            }
            else
            {
                auto msg = GatherErrorDetails(form_xml, getGenName);
                FAIL_MSG(tt_string() << "Unable to create " << type_name << "\n" << msg)
                m_errors.emplace(tt_string("Unable to create ") << type_name);
                return false;
            }
        }
        parent->adoptChild(form);
        if (auto derived_class = form_xml.find_child_by_attribute("string", "name", "proxy-Base class"); derived_class)
        {
            auto derived_name = ExtractQuotedString(derived_class);
            if (derived_name != type_name)
            {
                form->set_value(prop_derived_class, derived_name);
            }
        }

        m_class_uses_dlg_units = false;
        if (auto dlg_units = form_xml.find_child_by_attribute("bool", "name", "proxy-Dialog units");
            dlg_units && dlg_units.text().as_bool())
        {
            m_class_uses_dlg_units = true;
        }

        // Start be setting properties common to most forms

        if (auto prop = form->getPropPtr(prop_class_name); prop)
        {
            if (auto value = form_xml.find_child_by_attribute("string", "name", "proxy-Class"); value)
            {
                prop->set_value(ExtractQuotedString(value));
            }
        }

        if (auto prop = form->getPropPtr(prop_base_file); prop)
        {
            if (auto value = form_xml.find_child_by_attribute("string", "name", "proxy-Implementation filename"); value)
            {
                auto file = ExtractQuotedString(value);
                file.remove_extension();
                prop->set_value(file);
            }
        }

        if (auto prop = form->getPropPtr(prop_xrc_file); prop)
        {
            if (auto value = form_xml.find_child_by_attribute("string", "name", "proxy-XRC filename"); value)
            {
                auto file = ExtractQuotedString(value);
                // Note that unlike the base file, we do *not* remove the XRC file extension
                prop->set_value(file);
            }
        }

        if (auto prop = form->getPropPtr(prop_title); prop)
        {
            if (auto value = form_xml.find_child_by_attribute("string", "name", "proxy-Title"); value)
            {
                prop->set_value(ExtractQuotedString(value));
            }
        }

        if (auto prop = form->getPropPtr(prop_center); prop)
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

        SetNodeDimensions(form_xml, form);  // Set pos and size
        SetNodeID(form_xml, form);          // Set ID
        ProcessStyles(form_xml, form);      // Set all styles for the current node
        ProcessEvents(form_xml, form);      // Add all events for the current node

        for (auto& child_xml: form_xml.children("document"))
        {
            createChildNode(child_xml, form.get());
        }
        return true;
    }
    else
    {
        return false;
    }
}

void DialogBlocks::createChildNode(pugi::xml_node& child_xml, Node* parent)
{
    auto getGenName = FindGenerator(child_xml, parent);
    if (getGenName == gen_unknown)
    {
        auto type = child_xml.find_child_by_attribute("string", "name", "proxy-type");
        if (!type)
        {
            auto msg = GatherErrorDetails(child_xml, getGenName);
            FAIL_MSG(tt_string() << "Unable to determine class due to missing \"proxy-type\" property.\n" << msg)
            m_errors.emplace(tt_string("Unable to determine class due to missing \"proxy-type\" property."));
        }
        else
        {
            if (ExtractQuotedString(type) == "wbForeignCtrlProxy")
            {
                CreateCustomNode(child_xml, parent);
                return;
            }

            // No point complaining about custom controls
            auto msg = GatherErrorDetails(child_xml, getGenName);
            msg << ", Type: " << ExtractQuotedString(type);
            FAIL_MSG(tt_string() << "Unrecognized class in \"proxy-type\" property: " << ExtractQuotedString(type) << "\n"
                                 << msg)
        }
        return;
    }

    // DialogBlocks uses wbToolBarButtonProxy for all toolbar buttons, so MapClassName() always
    // turns it into gen_tool.
    if (getGenName == gen_tool && parent->isGen(gen_wxAuiToolBar))
    {
        getGenName = gen_auitool;
    }
    else if (getGenName == gen_wxPanel && parent->isGen(gen_wxPropertySheetDialog))
    {
        getGenName = gen_BookPage;
    }

    bool allow_adoption = true;  // set to false if node has already been adopted, e.g. a genPageCtrl was inserted
    auto node = NodeCreation.createNode(getGenName, parent);
    if (!node)
    {
        if (parent->isGen(gen_wxStdDialogButtonSizer) && getGenName == gen_wxButton)
        {
            auto add_buttons = [&](std::string_view id, GenEnum::PropName propname)
            {
                if (auto value = child_xml.find_child_by_attribute("bool", "name", id); value && value.text().as_bool())
                {
                    parent->set_value(propname, true);
                }
            };

            // Note that DialogBlocks does not use wxID_CLOSE

            add_buttons("proxy-wxID_APPLY", prop_Apply);
            add_buttons("proxy-wxID_OK", prop_OK);
            add_buttons("proxy-wxID_CANCEL", prop_Cancel);
            add_buttons("proxy-wxID_YES", prop_Yes);
            add_buttons("proxy-wxID_NO", prop_No);
            add_buttons("proxy-wxID_CONTEXT_HELP", prop_ContextHelp);
            add_buttons("proxy-wxID_HELP", prop_Help);
            add_buttons("proxy-wxID_SAVE", prop_Save);
            return;
        }

        if (parent->isSizer() && parent->getParent()->isForm())
        {
            node = NodeCreation.createNode(getGenName, parent->getParent());
            if (node)
            {
                parent = parent->getParent();
            }
        }

        // DialogBlocks will sometimes put the statusbar nested under two sizers.
        else if (getGenName == gen_wxStatusBar)
        {
            if (auto form = parent->getForm(); form)
            {
                node = NodeCreation.createNode(getGenName, form);
                if (node)
                {
                    parent = form;
                }
            }
        }
        else if (tt::contains(map_GenTypes[parent->getGenType()], "book"))
        {
            if (auto page_ctrl = NodeCreation.createNode(gen_PageCtrl, parent); page_ctrl)
            {
                if (node = NodeCreation.createNode(getGenName, page_ctrl.get()); node)
                {
                    page_ctrl->adoptChild(node);
                    parent->adoptChild(page_ctrl);
                    allow_adoption = false;
                }
            }
        }
    }

    if (!node)
    {
        auto msg = GatherErrorDetails(child_xml, getGenName);
        ASSERT_MSG(node, tt_string("Unable to create ")
                             << map_GenNames[getGenName] << " as child of " << map_GenNames[parent->getGenName()] << "\n"
                             << msg);
        m_errors.emplace(tt_string("Unable to create ") << map_GenNames[getGenName]);
        return;
    }

    if (allow_adoption)
    {
        parent->adoptChild(node);
    }

    if (auto prop = node->getPropPtr(prop_label); prop)
    {
        if (auto value = child_xml.find_child_by_attribute("string", "name", "proxy-Label"); value)
        {
            prop->set_value(ExtractQuotedString(value));
        }
    }

    if (auto prop = node->getPropPtr(prop_orientation); prop)
    {
        if (auto value = child_xml.find_child_by_attribute("string", "name", "proxy-Orientation"); value)
        {
            auto direction = ExtractQuotedString(value);
            if (direction.is_sameas("Vertical", tt::CASE::either))
            {
                prop->set_value("wxVERTICAL");
            }
            else if (direction.is_sameas("Horizontal", tt::CASE::either))
            {
                prop->set_value("wxHORIZONTAL");
            }
            else
            {
                FAIL_MSG(tt_string() << "Unrecognized orientation: " << direction << "\n"
                                     << GatherErrorDetails(child_xml, getGenName));
                m_errors.emplace(tt_string("Unrecognized orientation: ") << direction);
            }
        }
    }

    // These Set...() functions can be called whether or not the property exists, so no need to
    // check for it first.

    SetNodeState(child_xml, node);       // Set disabled and hidden states
    SetNodeDimensions(child_xml, node);  // Set pos and size
    SetNodeVarname(child_xml, node);     // Set var_name and class access
    SetNodeID(child_xml, node);          // Set ID
    SetNodeValidator(child_xml, node);   // Set validator

    ProcessStyles(child_xml, node);  // Set all styles for the current node
    ProcessEvents(child_xml, node);  // Add all events for the current node
    ProcessMisc(child_xml, node);    // Set all other properties for the current node

    // Now add all the children of this child node
    for (auto& grand_child_xml: child_xml.children("document"))
    {
        createChildNode(grand_child_xml, node.get());
    }
}

void DialogBlocks::CreateCustomNode(pugi::xml_node& child_xml, Node* parent)
{
    auto node = NodeCreation.createNode(gen_CustomControl, parent);
    if (!node)
    {
        auto msg = GatherErrorDetails(child_xml, gen_CustomControl);
        ASSERT_MSG(node, tt_string("Unable to create ") << map_GenNames[gen_CustomControl] << " as child of "
                                                        << map_GenNames[parent->getGenName()] << "\n"
                                                        << msg);
        m_errors.emplace(tt_string("Unable to create ") << map_GenNames[gen_CustomControl]);
        return;
    }

    parent->adoptChild(node);

    SetNodeState(child_xml, node);       // Set disabled and hidden states
    SetNodeDimensions(child_xml, node);  // Set pos and size
    SetNodeVarname(child_xml, node);     // Set var_name and class access
    SetNodeID(child_xml, node);          // Set ID

    ProcessStyles(child_xml, node);  // Set all styles for the current node
    ProcessEvents(child_xml, node);  // Add all events for the current node
    ProcessMisc(child_xml, node);    // Set all other properties for the current node

    if (auto value = child_xml.find_child_by_attribute("string", "name", "proxy-Class"); value)
    {
        node->set_value(prop_class_name, ExtractQuotedString(value));
    }

    if (auto value = child_xml.find_child_by_attribute("string", "name", "proxy-Custom arguments"); value)
    {
        auto result = ExtractQuotedString(value);
        result.Replace("%PARENT%", "this", true);
        result.Replace("%ID%", "${id}", true);
        result.Replace("%POSITION%", "${pos}", true);
        result.Replace("%SIZE%", "${size}", true);
        result.Replace("%STYLE%", "${window_style}", true);
        result.insert(0, "(");
        result << ")";
        node->set_value(prop_parameters, result);
    }
}

GenEnum::GenName DialogBlocks::FindGenerator(pugi::xml_node& node_xml, Node* parent)
{
    GenEnum::GenName getGenName = gen_unknown;

    // proxy-type starts with "wb" and ends with "Proxy". Change the "wb" to "wx" and remove
    // the "Proxy" to get the class name.

    if (auto type = node_xml.find_child_by_attribute("string", "name", "proxy-type"); type)
    {
        auto type_name = ExtractQuotedString(type);
        if (type_name.starts_with("wb"))
        {
            type_name[1] = 'x';
        }
        // Proxy
        type_name.Replace("Proxy", "", true);
        getGenName = MapClassName(type_name);
        if (getGenName == gen_unknown)
        {
            if (type_name == "wxWizardPage")
            {
                return gen_wxWizardPageSimple;
            }
            else if (type_name == "wxAuiToolBarButton")
            {
                getGenName = gen_auitool;
            }
            else
            {
                if (auto value = node_xml.find_child_by_attribute("string", "name", "proxy-Class"); value)
                {
                    getGenName = MapClassName(ExtractQuotedString(value));
                }
            }
        }
    }
    if (getGenName == gen_wxPanel)
    {
        if (parent->declName().contains("book"))
            getGenName = gen_BookPage;
    }
    else if (getGenName == gen_wxWindow)
    {
        getGenName = gen_CustomControl;
    }

    return getGenName;
}

// Sets var_name and class access for a node
void DialogBlocks::SetNodeVarname(pugi::xml_node& node_xml, const NodeSharedPtr& new_node)
{
    if (auto prop = new_node->getPropPtr(prop_var_name); prop)
    {
        if (auto value = node_xml.find_child_by_attribute("string", "name", "proxy-Member variable name"); value)
        {
            auto name = ExtractQuotedString(value);
            if (name.size())
            {
                prop->set_value(name);
                // DialogBlocks makes the variable public:, but we force it to protected: so
                // that only the base and any derived classes can access it directly.
                new_node->set_value(prop_class_access, "protected:");
            }
            else if (value = node_xml.find_child_by_attribute("string", "name", "identifier"); value)
            {
                name = ExtractQuotedString(value);
                if (name.size())
                {
                    prop->set_value(name);
                    new_node->set_value(prop_class_access, "none");
                }
            }
        }
    }
}

void DialogBlocks::SetNodeID(pugi::xml_node& node_xml, const NodeSharedPtr& new_node)
{
    if (auto prop = new_node->getPropPtr(prop_id); prop)
    {
        if (auto value = node_xml.find_child_by_attribute("string", "name", "proxy-Id name"); value)
        {
            auto name = ExtractQuotedString(value);
            if (name != "wxID_ANY")
            {
                if (!name.starts_with("wxID_"))
                {
                    if (value = node_xml.find_child_by_attribute("long", "name", "proxy-Id value"); value)
                    {
                        auto id_value = value.text().as_int();
                        if (id_value != -1)
                        {
                            name << " = " << id_value;
                        }
                    }
                }
                prop->set_value(name);
            }
        }
    }
}

// Sets disabled and hidden states for a node
void DialogBlocks::SetNodeState(pugi::xml_node& node_xml, const NodeSharedPtr& new_node)
{
    if (auto prop = new_node->getPropPtr(prop_disabled); prop)
    {
        if (auto value = node_xml.find_child_by_attribute("bool", "name", "proxy-Enabled"); value && !value.text().as_bool())
        {
            prop->set_value(true);
        }
    }

    if (auto prop = new_node->getPropPtr(prop_hidden); prop)
    {
        if (auto value = node_xml.find_child_by_attribute("bool", "name", "proxy-Hidden"); value && value.text().as_bool())
        {
            prop->set_value(true);
        }
    }
}

void DialogBlocks::SetNodeDimensions(pugi::xml_node& node_xml, const NodeSharedPtr& new_node)
{
    if (auto prop = new_node->getPropPtr(prop_size); prop)
    {
        wxSize size { -1, -1 };
        if (auto value = node_xml.find_child_by_attribute("long", "name", "proxy-Width"); value)
        {
            size.SetWidth(value.text().as_int());
        }
        if (auto value = node_xml.find_child_by_attribute("long", "name", "proxy-Height"); value)
        {
            size.SetHeight(value.text().as_int());
        }
        prop->set_value(size);
        if (m_class_uses_dlg_units)
        {
            prop->get_value() << 'd';
        }
    }

    if (auto prop = new_node->getPropPtr(prop_pos); prop)
    {
        wxPoint pos { -1, -1 };
        if (auto value = node_xml.find_child_by_attribute("long", "name", "proxy-X"); value)
        {
            pos.x = value.text().as_int();
        }
        if (auto value = node_xml.find_child_by_attribute("long", "name", "proxy-Y"); value)
        {
            pos.y = value.text().as_int();
        }
        prop->set_value(pos);
        if (m_class_uses_dlg_units)
        {
            prop->get_value() << 'd';
        }
    }

    if (new_node->isGen(gen_spacer))
    {
        if (auto value = node_xml.find_child_by_attribute("long", "name", "proxy-Width"); value)
        {
            new_node->set_value(prop_width, value.text().as_int());
        }
        if (auto value = node_xml.find_child_by_attribute("long", "name", "proxy-Height"); value)
        {
            new_node->set_value(prop_height, value.text().as_int());
        }
    }
}

void DialogBlocks::SetNodeValidator(pugi::xml_node& node_xml, const NodeSharedPtr& new_node)
{
    // Note that while DialogBlocks allows the user to set the properties, it doesn't actually
    // correctly set the validator in code.
    if (auto prop = new_node->getPropPtr(prop_validator_variable); prop)
    {
        if (auto value = node_xml.find_child_by_attribute("string", "name", "proxy-Data variable"); value)
        {
            prop->set_value(ExtractQuotedString(value));

            if (value = node_xml.find_child_by_attribute("string", "name", "proxy-Data validator"); value)
            {
                new_node->set_value(prop_validator_type, ExtractQuotedString(value));
            }
        }
    }
}

void DialogBlocks::ProcessEvents(pugi::xml_node& node_xml, const NodeSharedPtr& new_node)
{
    for (int event_count = 0;; ++event_count)
    {
        if (auto value = node_xml.find_child_by_attribute("string", "name", tt_string("event-handler-") << event_count);
            value)
        {
            auto event_text = ExtractQuotedString(value);
            tt_view_vector event_parts(event_text, '|');
            ASSERT(event_parts.size() > 1);
            if (event_parts.size() > 1)
            {
                if (auto node_event = new_node->getEvent(GetCorrectEventName(event_parts[0])); node_event)
                {
                    node_event->set_value(event_parts[1]);
                }
            }
        }
        else
        {
            break;
        }
    }
}

tt_string DialogBlocks::ExtractQuotedString(pugi::xml_node& str_xml)
{
    tt_string str;
    auto view = str_xml.text().as_sview();
    if (view.starts_with("\""))
        str.ExtractSubString(view);
    else
        str = view;
    return str;
}

// Performance on such small lists isn't going to be an issue, so we use a set rather than an
// unordered_set to reduce the memory footprint. Using frozen::make_set means the set is orderd
// at compile time, so it has no impact on application startup time.

constexpr auto set_window_styles = frozen::make_set<std::string_view>({
    "wxHSCROLL",
    "wxALWAYS_SHOW_SB",
    "wxBORDER_DEFAULT",
    "wxBORDER_DOUBLE",
    "wxBORDER_NONE",
    "wxBORDER_RAISED",
    "wxBORDER_SIMPLE",
    "wxBORDER_STATIC",
    "wxBORDER_SUNKEN",
    "wxBORDER_THEME",
    "wxCLIP_CHILDREN",
    "wxNO_FULL_REPAINT_ON_RESIZE",
    "wxTAB_TRAVERSAL",
    "wxTRANSPARENT_WINDOW",
    "wxVSCROLL",
    "wxWANTS_CHARS",
});

constexpr auto set_exwindow_styles = frozen::make_set<std::string_view>({
    "wxWS_EX_VALIDATE_RECURSIVELY",
    "wxWS_EX_BLOCK_EVENTS",
    "wxWS_EX_TRANSIENT",
    "wxWS_EX_PROCESS_IDLE",
    "wxWS_EX_PROCESS_UI_UPDATES",
});

constexpr auto set_dialog_styles = frozen::make_set<std::string_view>({
    "wxCAPTION",
    "wxCLOSE_BOX",
    "wxDEFAULT_DIALOG_STYLE",
    "wxDIALOG_NO_PARENT",
    "wxMAXIMIZE_BOX",
    "wxMINIMIZE_BOX",
    "wxRESIZE_BORDER",
    "wxSTAY_ON_TOP",
    "wxSYSTEM_MENU",
    "wxWANTS_CHARS",
});

constexpr auto set_dialog_exstyles = frozen::make_set<std::string_view>({
    "wxDIALOG_EX_CONTEXTHELP",
    "wxDIALOG_EX_METAL",
    "wxWS_EX_BLOCK_EVENTS",
    "wxWS_EX_PROCESS_IDLE",
    "wxWS_EX_PROCESS_UI_UPDATES",
    "wxWS_EX_VALIDATE_RECURSIVELY",
});

constexpr auto set_alignment_styles = frozen::make_set<std::string_view>({
    "wxALIGN_BOTTOM",
    "wxALIGN_CENTER",
    "wxALIGN_CENTER_HORIZONTAL",
    "wxALIGN_CENTER_VERTICAL",
    "wxALIGN_LEFT",
    "wxALIGN_RIGHT",
    "wxALIGN_TOP",
});

constexpr auto set_layout_flags = frozen::make_set<std::string_view>({
    "wxEXPAND",
    "wxFIXED_MINSIZE",
    "wxRESERVE_SPACE_EVEN_IF_HIDDEN",
    "wxSHAPED",
});

constexpr auto set_borders_flags = frozen::make_set<std::string_view>({
    "wxALL",
    "wxBOTTOM",
    "wxLEFT",
    "wxRIGHT",
    "wxTOP",
});

constexpr auto set_modes = frozen::make_set<std::string_view>({
    "wxLC_ICON",
    "wxLC_SMALL_ICON",
    "wxLC_LIST",
    "wxLC_REPORT",
    "wxDATAVIEW_CELL_INERT",
    "wxDATAVIEW_CELL_ACTIVATABLE",
    "wxDATAVIEW_CELL_EDITABLE",
});

constexpr auto set_listbox_types = frozen::make_set<std::string_view>({
    "wxLB_SINGLE",
    "wxLB_MULTIPLE",
    "wxLB_EXTENDED",
});

// These are used to set prop_style
constexpr auto set_styles = frozen::make_set<std::string_view>({
    "wxLI_HORIZONTAL",
    "wxLI_VERTICAL",

    "wxTE_AUTO_URL",
    "wxTE_CAPITALIZE",
    "wxTE_CENTRE",
    "wxTE_CHARWRAP",
    "wxTE_LEFT",
    "wxTE_MULTILINE",
    "wxTE_NOHIDESEL",
    "wxTE_PASSWORD",
    "wxTE_PROCESS_ENTER",
    "wxTE_PROCESS_TAB",
    "wxTE_READONLY",
    "wxTE_RICH",
    "wxTE_RICH2",
    "wxTE_RIGHT",
    "wxTE_WORDWRAP",

    "wxAUI_NB_BOTTOM",
    "wxAUI_NB_CLOSE_BUTTON",
    "wxAUI_NB_CLOSE_ON_ACTIVE_TAB",
    "wxAUI_NB_CLOSE_ON_ALL_TABS",
    "wxAUI_NB_MIDDLE_CLICK_CLOSE",
    "wxAUI_NB_SCROLL_BUTTONS",
    "wxAUI_NB_TAB_EXTERNAL_MOVE",
    "wxAUI_NB_TAB_FIXED_WIDTH",
    "wxAUI_NB_TAB_MOVE",
    "wxAUI_NB_TAB_SPLIT",
    "wxAUI_NB_TOP",
    "wxAUI_NB_WINDOWLIST_BUTTON",

    "wxAUI_TB_TEXT",
    "wxAUI_TB_NO_TOOLTIPS",
    "wxAUI_TB_NO_AUTORESIZE",
    "wxAUI_TB_GRIPPER",
    "wxAUI_TB_OVERFLOW",
    "wxAUI_TB_VERTICAL",
    "wxAUI_TB_HORZ_LAYOUT",
    "wxAUI_TB_PLAIN_BACKGROUND",
    "wxAUI_TB_HORZ_TEXT",
    "wxAUI_ORIENTATION_MASK",
    "wxAUI_TB_DEFAULT_STYLE",

    "wxSTB_SIZEGRIP",
    "wxSTB_SHOW_TIPS",
    "wxSTB_ELLIPSIZE_START",
    "wxSTB_ELLIPSIZE_MIDDLE",
    "wxSTB_ELLIPSIZE_END",
    "wxSTB_DEFAULT_STYLE",

    "wxCHB_DEFAULT",
    "wxCHB_TOP",
    "wxCHB_BOTTOM",
    "wxCHB_LEFT",
    "wxCHB_RIGHT",

    "wxBK_DEFAULT",
    "wxBK_TOP",
    "wxBK_BOTTOM",
    "wxBK_LEFT",
    "wxBK_RIGHT",

    "wxNB_FIXEDWIDTH",
    "wxNB_MULTILINE",
    "wxNB_NOPAGETHEME",

    "wxTBK_HORZ_LAYOUT",

    "wxCB_SIMPLE",
    "wxCB_DROPDOWN",
    "wxCB_READONLY",
    "wxCB_SORT",

    "wxLB_HSCROLL",
    "wxLB_ALWAYS_SB",
    "wxLB_NEEDED_SB",
    "wxLB_NO_SB",
    "wxLB_SORT",

    "wxHLB_DEFAULT_STYLE",
    "wxHLB_MULTIPLE",

    "wxALIGN_RIGHT",  // used by wxCheckBox -- will this conflict with the Layout version?

    "wxCHK_UNCHECKED",
    "wxCHK_CHECKED",
    "wxCHK_UNDETERMINED",

    "wxRB_GROUP",
    "wxRB_SINGLE",

    "wxBU_BOTTOM",
    "wxBU_LEFT",
    "wxBU_RIGHT",
    "wxBU_TOP",
    "wxBU_EXACTFIT",
    "wxBU_NOTEXT",
    "wxBORDER_NONE",  // used by wxButton -- will this conflict with the Window version?

    "wxSP_ARROW_KEYS",
    "wxSP_WRAP",
    "wxALIGN_CENTER_HORIZONTAL",  // used by wxSpinButton -- will this conflict with the Layout version?
    "wxALIGN_LEFT",
    // wxALIGN_RIGHT, // used by wxSpinButton -- will this conflict with the Layout version?

    "wxSP_3D",
    "wxSP_NOBORDER",
    "wxSP_3DSASH",
    "wxSP_NOSASH",
    "wxSP_3DBORDER",
    "wxSP_BORDER",
    "wxSP_NO_XP_THEME",
    "wxSP_THIN_SASH",
    "wxSP_PERMIT_UNSPLIT",
    "wxSP_LIVE_UPDATE",

    "wxCP_DEFAULT_STYLE",
    "wxCP_NO_TLW_RESIZE",

    "wxDV_SINGLE",
    "wxDV_MULTIPLE",
    "wxDV_NO_HEADER",
    "wxDV_HORIZ_RULES",
    "wxDV_VERT_RULES",
    "wxDV_ROW_LINES",
    "wxDV_VARIABLE_LINE_HEIGHT",

    "wxDEFAULT_FRAME_STYLE",
    "wxFRAME_TOOL_WINDOW",
    "wxFRAME_FLOAT_ON_PARENT",
    "wxFRAME_SHAPED",
    "wxFRAME_NO_TASKBAR",
    "wxFRAME_TOOL_WINDOW",
    "wxFRAME_FLOAT_ON_PARENT",
    "wxFRAME_SHAPED",
    "wxICONIZE",
    "wxMAXIMIZE",
    "wxSTAY_ON_TOP",
    "wxVSCROLL",
    "wxHSCROLL",

    "wxPU_CONTAINS_CONTROLS",

    "wxRIBBON_BAR_DEFAULT_STYLE",
    "wxRIBBON_BAR_SHOW_PAGE_LABELS",
    "wxRIBBON_BAR_SHOW_PAGE_ICONS",
    "wxRIBBON_BAR_SHOW_PANEL_EXT_BUTTONS",
    "wxRIBBON_BAR_SHOW_TOGGLE_BUTTON",
    "wxRIBBON_BAR_SHOW_HELP_BUTTON",
    "wxRIBBON_BAR_FOLDBAR_STYLE",

    "wxLC_VIRTUAL",
    "wxLC_ALIGN_TOP",
    "wxLC_ALIGN_LEFT",
    "wxLC_AUTOARRANGE",
    "wxLC_USER_TEXT",
    "wxLC_EDIT_LABELS",
    "wxLC_NO_HEADER",
    "wxLC_NO_SORT_HEADER",
    "wxLC_SINGLE_SEL",
    "wxLC_SORT_ASCENDING",
    "wxLC_SORT_DESCENDING",
    "wxLC_HRULES",
    "wxLC_VRULES",
    "wxLC_ALIGN_MASK",
    "wxLC_MASK_TYPE",
    "wxLC_MASK_ALIGN",
    "wxLC_MASK_SORT",
    "wxLC_MASK_STYLE",
    "wxLC_VIRTUAL",

    "wxEL_ALLOW_NEW",
    "wxEL_ALLOW_EDIT",
    "wxEL_ALLOW_DELETE",
    "wxEL_NO_REORDER",

    "wxMB_DOCKABLE",

    "wxFLP_USE_TEXTCTRL",
    "wxFLP_OPEN",
    "wxFLP_SAVE",
    "wxFLP_OVERWRITE_PROMPT",
    "wxFLP_FILE_MUST_EXIST",
    "wxFLP_CHANGE_DIR",
    "wxFLP_SMALL",

    "wxDIRP_DEFAULT_STYLE",
    "wxDIRP_DIR_MUST_EXIST",
    "wxDIRP_CHANGE_DIR",
    "wxDIRP_SMALL",

    "wxFNTP_USE_TEXTCTRL",
    "wxFNTP_FONTDESC_AS_LABEL",
    "wxFNTP_USEFONT_FOR_LABEL",

    "wxCLRP_USE_TEXTCTRL",
    "wxCLRP_SHOW_LABEL",
    "wxCLRP_SHOW_ALPHA",

    "wxDP_SPIN",
    "wxDP_DROPDOWN",
    "wxDP_SHOWCENTURY",
    "wxDP_ALLOWNONE",
    "wxDP_DEFAULT",

    "wxTP_DEFAULT",

    "wxPG_ALPHABETIC_MODE",
    "wxPG_AUTO_SORT",
    "wxPG_BOLD_MODIFIED",
    "wxPG_DEFAULT_STYLE",
    "wxPG_DESCRIPTION",
    "wxPG_HIDE_CATEGORIES",
    "wxPG_HIDE_MARGIN",
    "wxPG_NO_INTERNAL_BORDER",
    "wxPG_SHOW_ROOT",
    "wxPG_SPLITTER_AUTO_CENTER",
    "wxPG_STATIC_SPLITTER",
    "wxPG_TOOLBAR",
    "wxPG_TOOLTIPS",

    "wxST_NO_AUTORESIZE",
    "wxST_ELLIPSIZE_START",
    "wxST_ELLIPSIZE_MIDDLE",
    "wxST_ELLIPSIZE_END",

    "wxRE_CENTRE_CARET",

    "wxTB_FLAT",
    "wxTB_DOCKABLE",
    "wxTB_HORIZONTAL",
    "wxTB_VERTICAL",
    "wxTB_TEXT",
    "wxTB_NOICONS",
    "wxTB_HORZ_LAYOUT",
    "wxTB_HORZ_TEXT",
    "wxTB_NOALIGN",
    "wxTB_NODIVIDER",
    "wxTB_NO_TOOLTIPS",
    "wxTB_BOTTOM",
    "wxTB_RIGHT",
    "wxTB_DEFAULT_STYLE",

    "wxTR_EDIT_LABELS",
    "wxTR_MULTIPLE",
    "wxTR_EXTENDED",
    "wxTR_HAS_BUTTONS",
    "wxTR_NO_BUTTONS",
    "wxTR_TWIST_BUTTONS",
    "wxTR_ROW_LINES",
    "wxTR_COLUMN_LINES",
    "wxTR_NO_LINES",
    "wxTR_FULL_ROW_HIGHLIGHT",
    "wxTR_DEFAULT_STYLE",

    "wxTL_3D",
    "wxTL_3STATE",
    "wxTL_CHECKBOX",
    "wxTL_MULTIPLE",
    "wxTL_SINGLE",
    "wxTL_USER_3STATE",

    "wxAC_NO_AUTORESIZE",
    "wxAC_DEFAULT_STYLE",

    "wxSL_AUTOTICKS",
    "wxSL_MIN_MAX_LABELS",
    "wxSL_VALUE_LABEL",
    "wxSL_LABELS",
    "wxSL_LEFT",
    "wxSL_RIGHT",
    "wxSL_TOP",
    "wxSL_BOTTOM",
    "wxSL_BOTH",
    "wxSL_SELRANGE",
    "wxSL_INVERSE",

    "wxGA_SMOOTH",

    "wxSB_HORIZONTAL",
    "wxSB_VERTICAL",

    "wxHL_ALIGN_LEFT",
    "wxHL_ALIGN_RIGHT",
    "wxHL_ALIGN_CENTRE",
    "wxHL_CONTEXTMENU",
    "wxHL_DEFAULT_STYLE",

    "wxHW_SCROLLBAR_NEVER",
    "wxHW_SCROLLBAR_AUTO",
    "wxHW_NO_SELECTION",

    "wxCAL_SUNDAY_FIRST",
    "wxCAL_MONDAY_FIRST",
    "wxCAL_SHOW_HOLIDAYS",
    "wxCAL_NO_YEAR_CHANGE",
    "wxCAL_NO_MONTH_CHANGE",
    "wxCAL_SHOW_SURROUNDING_WEEKS",
    "wxCAL_SEQUENTIAL_MONTH_SELECTION",
    "wxCAL_SHOW_WEEK_NUMBERS",

});

constexpr auto map_old_borders = frozen::make_map<std::string_view, std::string_view>({
    { "wxBORDER", "wxBORDER_SIMPLE" },
    { "wxDOUBLE_BORDER", "wxBORDER_DOUBLE" },
    { "wxNO_BORDER", "wxBORDER_NONE" },
    { "wxRAISED_BORDER", "wxBORDER_RAISED" },
    { "wxSIMPLE_BORDER", "wxBORDER_SIMPLE" },
    { "wxSTATIC_BORDER", "wxBORDER_STATIC" },
    { "wxSUNKEN_BORDER", "wxBORDER_SUNKEN" },
});

void DialogBlocks::ProcessStyles(pugi::xml_node& node_xml, const NodeSharedPtr& new_node)
{
    tt_string window_styles;
    tt_string window_exstyles;
    tt_string dialog_styles;
    tt_string dialog_exstyles;
    tt_string prop_styles;
    tt_string alignment_styles;
    tt_string layout_flags;
    tt_string border_flags;

    for (auto& form: node_xml.children("bool"))
    {
        // We only collect styles that have been set, and ignore the rest
        if (!form.text().as_bool())
            continue;
        auto name = form.attribute("name").as_std_str();
        if (!name.starts_with("proxy-"))
            continue;
        name.erase(0, sizeof("proxy-") - 1);
        if (!name.starts_with("wx"))
            continue;
        if (auto result = map_old_borders.find(name); result != map_old_borders.end())
        {
            name = result->second;
        }

        if (set_window_styles.contains(name))
        {
            if (window_styles.size())
                window_styles << '|';
            window_styles << name;
        }
        else if (set_exwindow_styles.contains(name))
        {
            if (window_exstyles.size())
                window_exstyles << '|';
            window_exstyles << name;
        }
        else if (set_dialog_styles.contains(name))
        {
            if (dialog_styles.size())
                dialog_styles << '|';
            dialog_styles << name;
        }
        else if (set_dialog_exstyles.contains(name))
        {
            if (dialog_exstyles.size())
                dialog_exstyles << '|';
            dialog_exstyles << name;
        }
        else if (set_styles.contains(name))
        {
            if (prop_styles.size())
                prop_styles << '|';
            prop_styles << name;
        }
        else if (set_alignment_styles.contains(name))
        {
            if (alignment_styles.size())
                alignment_styles << '|';
            alignment_styles << name;
        }
        else if (set_layout_flags.contains(name))
        {
            if (layout_flags.size())
                layout_flags << '|';
            layout_flags << name;
        }
        else if (set_borders_flags.contains(name))
        {
            if (border_flags.size())
                border_flags << '|';
            border_flags << name;
        }
        else if (set_modes.contains(name))
        {
            // Only one mode can be set
            new_node->set_value(prop_mode, name);
            continue;
        }
        else if (set_listbox_types.contains(name))
        {
            // A wxListBox can only have one type set.
            new_node->set_value(prop_type, name);
            continue;
        }
    }

    if (window_styles.size())
    {
        new_node->set_value(prop_window_style, window_styles);
    }
    if (window_exstyles.size())
    {
        new_node->set_value(prop_window_extra_style, window_exstyles);
    }

    if (prop_styles.size())
    {
        new_node->set_value(prop_style, prop_styles);
    }

    if (new_node->isGen(gen_wxDialog))
    {
        if (dialog_styles.size())
        {
            new_node->set_value(prop_style, dialog_styles);
        }
        if (dialog_exstyles.size())
        {
            new_node->set_value(prop_extra_style, dialog_exstyles);
        }
    }

    // For wxUE, these are sizer_child settings, however DialogBlocks doesn't use any form of sizerchild, so
    // we look for alignment before attempting to set these. That's because the wxLEFT, wxRIGHT, etc. can be used
    // for other things besides where the border should be.

    if (new_node->hasProp(prop_alignment))
    {
        tt_string style_str;
        if (alignment_styles.size())
        {
            style_str << alignment_styles;
        }
        if (auto value = node_xml.find_child_by_attribute("string", "name", "proxy-AlignH"); value)
        {
            auto alignment = ExtractQuotedString(value);
            if (alignment.is_sameas("Right", tt::CASE::either))
            {
                if (style_str.size())
                    style_str << '|';
                style_str << "wxALIGN_RIGHT";
            }
            else if (alignment.is_sameas("Centre", tt::CASE::either))
            {
                if (style_str.size())
                    style_str << '|';
                style_str << "wxALIGN_CENTER_HORIZONTAL";
            }
        }
        if (auto value = node_xml.find_child_by_attribute("string", "name", "proxy-AlignV"); value)
        {
            // Vertical alignment is invalid if the sizer's orientation is wxVERTICAL
            if (auto parent = new_node->getParent();
                parent && parent->isSizer() && parent->as_string(prop_orientation) != "wxVERTICAL")
            {
                auto alignment = ExtractQuotedString(value);
                if (alignment.is_sameas("Bottom", tt::CASE::either))
                {
                    if (style_str.size())
                        style_str << '|';
                    style_str << "wxALIGN_BOTTOM";
                }
                else if (alignment.is_sameas("Centre", tt::CASE::either))
                {
                    if (style_str.size())
                        style_str << '|';
                    style_str << "wxALIGN_CENTER_VERTICAL";
                }
            }
        }
        if (style_str.size())
        {
            new_node->set_value(prop_alignment, style_str);
        }

        // Check for layout flags
        style_str.clear();
        if (layout_flags.size())
        {
            style_str << layout_flags;
        }

        if (auto value = node_xml.find_child_by_attribute("string", "name", "proxy-AlignH"); value)
        {
            if (ExtractQuotedString(value).is_sameas("Expand", tt::CASE::either))
            {
                if (auto parent = new_node->getParent();
                    parent && parent->isSizer() && parent->as_string(prop_orientation) != "wxHORIZONTAL")
                {
                    if (style_str.size())
                        style_str << '|';
                    style_str << "wxEXPAND";
                }
            }
        }
        if (auto value = node_xml.find_child_by_attribute("string", "name", "proxy-AlignV"); value)
        {
            // Vertical alignment is invalid if the sizer's orientation is wxVERTICAL
            if (ExtractQuotedString(value).is_sameas("Expand", tt::CASE::either))
            {
                if (auto parent = new_node->getParent();
                    parent && parent->isSizer() && parent->as_string(prop_orientation) != "wxVERTICAL")
                {
                    if (style_str.size())
                        style_str << '|';
                    style_str << "wxEXPAND";
                }
            }
        }

        if (style_str.size())
        {
            new_node->set_value(prop_flags, style_str);
        }

        if (border_flags.size())
        {
            if (border_flags.contains("wxLEFT") && border_flags.contains("wxRIGHT") && border_flags.contains("wxTOP") &&
                border_flags.contains("wxBOTTOM"))
            {
                new_node->set_value(prop_borders, "wxALL");
            }
            else
            {
                new_node->set_value(prop_borders, border_flags);
            }
        }
    }

    // REVIEW: [Randalphwa - 05-07-2023] What happens when something like wxRIGHT is used to indicate a bitmap position?
    // wxBannerWindow has a direction property that also uses wxLEFT, wxRIGHT etc.
}

constexpr auto map_proxy_names = frozen::make_map<std::string_view, GenEnum::PropName>({
    { "Background colour", prop_background_colour },
    { "Foreground colour", prop_foreground_colour },
    { "Hover colour", prop_hover_color },
    { "Normal colour", prop_normal_color },
    { "Visited colour", prop_visited_color },

    // wxGridBoxSizer
    { "Empty cell height", prop_empty_cell_size },
    { "Empty cell width", prop_empty_cell_size },
    { "Grid x", prop_column },
    { "Grid y", prop_row },
    { "Span x", prop_colspan },
    { "Span y", prop_rowspan },

    { "Animation", prop_animation },
    { "Bitmap", prop_bitmap },
    { "Border", prop_border_size },
    { "Column width", prop_default_col_size },
    { "ColumnSpacing", prop_hgap },
    { "Columns", prop_cols },
    { "Default filter", prop_defaultfilter },
    { "Default folder", prop_defaultfolder },
    { "Field count", prop_fields },
    { "Filter", prop_filter },
    { "Gravity", prop_sashgravity },
    { "GrowableColumns", prop_growablecols },
    { "GrowableRows", prop_growablerows },
    { "HTML code", prop_html_content },
    { "Help text", prop_context_help },
    { "Items", prop_contents },
    { "Kind", prop_kind },
    { "Label", prop_label },
    { "Max length", prop_maxlength },
    { "Maximum value", prop_max },
    { "Minimum pane size", prop_min_pane_size },
    { "Minimum value", prop_min },
    { "Page size", prop_pagesize },
    { "Range", prop_range },
    { "Row label width", prop_row_label_size },
    { "RowSpacing", prop_vgap },
    { "Rows", prop_rows },
    { "Sash position", prop_sashpos },
    { "Selection mode", prop_selection_mode },
    { "Strings", prop_contents },
    { "Stretch factor", prop_proportion },
    { "Strings", prop_contents },
    { "Tab label", prop_label },
    { "Tab icon", prop_bitmap },
    { "Thumb size", prop_thumbsize },
    { "Tool packing", prop_packing },
    { "Tool separation", prop_separation },
    { "Tooltip text", prop_tooltip },
    { "URL", prop_html_url },

    { "wxRA_SPECIFY_COLS", prop_style },
    { "wxRA_SPECIFY_ROWS", prop_style },

    { "Initial value", prop_value },  // In DialogBlocks used for all sorts of properties

});

void DialogBlocks::ProcessMisc(pugi::xml_node& node_xml, const NodeSharedPtr& node)
{
    for (auto& string_xml: node_xml.children("string"))
    {
        auto name = string_xml.attribute("name").as_sview();
        if (name.starts_with("proxy-"))
            name.remove_prefix(sizeof("proxy-") - 1);
        auto str = ExtractQuotedString(string_xml);
        if (str.empty())
            continue;
        if (auto result = map_proxy_names.find(name); result != map_proxy_names.end())
        {
            switch (result->second)
            {
                case prop_contents:
                    {
                        tt_string_vector multi(str, '|');
                        str.clear();
                        for (auto& iter: multi)
                        {
                            if (str.size())
                                str << ' ';
                            str << '"' << iter << '"';
                        }
                        if (auto prop = node->getPropPtr(result->second); prop)
                        {
                            prop->set_value(str);
                        }
                    }
                    break;

                case prop_value:
                    if (node->isGen(gen_wxChoice) || node->isGen(gen_wxComboBox) || node->isGen(gen_wxListBox) ||
                        node->isGen(gen_wxBitmapComboBox))
                    {
                        node->set_value(prop_selection_string, str);
                    }
                    else if (auto prop = node->getPropPtr(result->second); prop)
                    {
                        prop->set_value(str);
                    }
                    else if (node->isGen(gen_wxRadioBox))
                    {
                        node->set_value(prop_selection, str);
                    }
                    break;

                case prop_selection_mode:
                    if (str == "Cells")
                        node->set_value(prop_selection_mode, "wxGridSelectCells");
                    else if (str == "Rows")
                        node->set_value(prop_selection_mode, "wxGridSelectCells");
                    else if (str == "Columns")
                        node->set_value(prop_selection_mode, "wxGridSelectRows");
                    break;

                case prop_kind:
                    if (str == "Normal")
                        node->set_value(prop_selection_mode, "wxITEM_NORMAL");
                    else if (str == "Check")
                        node->set_value(prop_selection_mode, "wxITEM_CHECK");
                    else if (str == "Radio")
                        node->set_value(prop_selection_mode, "wxITEM_RADIO");
                    else if (str == "Dropdown")
                        node->set_value(prop_selection_mode, "wxITEM_DROPDOWN");
                    break;

                case prop_background_colour:
                case prop_foreground_colour:
                case prop_hover_color:
                case prop_normal_color:
                case prop_visited_color:
                    str.insert(0, "#");
                    node->set_value(result->second, str);
                    break;

                case prop_label:
                    if (node->getParent()->isGen(gen_PageCtrl))
                    {
                        node->getParent()->set_value(prop_label, str);
                    }
                    else
                    {
                        node->set_value(prop_label, str);
                    }
                    break;

                case prop_bitmap:
                    if (node->getParent()->isGen(gen_PageCtrl))
                    {
                        node->getParent()->set_value(prop_bitmap, tt_string("Embed;") << str);
                    }
                    else
                    {
                        node->set_value(prop_bitmap, tt_string("Embed;") << str);
                    }
                    break;

                default:
                    if (auto prop = node->getPropPtr(result->second); prop)
                    {
                        prop->set_value(str);
                    }
                    break;
            }
        }
        else if (name == "Field widths")
        {
            auto width_value = ExtractQuotedString(string_xml);
            tt_string_vector widths(width_value, ',');
            tt_string_vector fields(node->as_string(prop_fields), ';');
            size_t pos = 0;
            for (auto& iter: widths)
            {
                if (fields.size() < pos + 1)
                {
                    fields.push_back("wxSB_NORMAL");
                }
                if (!fields[pos].starts_with("wxSB_"))
                {
                    fields[pos] = "wxSB_NORMAL";
                }
                fields[pos] << '|' << iter;
                ++pos;
            }
            tt_string new_fields;
            for (auto& iter: fields)
            {
                if (new_fields.size())
                    new_fields << ';';
                new_fields << iter;
            }
            node->set_value(prop_fields, new_fields);
        }
    }

    for (auto& string_xml: node_xml.children("long"))
    {
        auto name = string_xml.attribute("name").as_sview();
        if (name.starts_with("proxy-"))
            name.remove_prefix(sizeof("proxy-") - 1);
        if (auto result = map_proxy_names.find(name); result != map_proxy_names.end())
        {
            if (string_xml.text().as_int() > 0)
            {
                switch (result->second)
                {
                    case prop_value:
                        if (node->isGen(gen_wxSlider) || node->isGen(gen_wxScrollBar))
                        {
                            node->set_value(prop_position, string_xml.text().as_string());
                        }
                        else if (node->isGen(gen_wxSpinButton) || node->isGen(gen_wxSpinCtrl))
                        {
                            node->set_value(prop_initial, string_xml.text().as_string());
                        }
                        else if (auto prop = node->getPropPtr(result->second); prop)
                        {
                            prop->set_value(string_xml.text().as_string());
                        }
                        break;

                    case prop_empty_cell_size:
                        {
                            auto size = node->as_wxSize(prop_empty_cell_size);
                            if (name == "Empty cell height")
                                size.y = string_xml.text().as_int();
                            else
                                size.x = string_xml.text().as_int();
                            node->set_value(prop_empty_cell_size, size);
                        }
                        break;

                    case prop_fields:
                        if (!node->hasValue(prop_fields))
                        {
                            node->set_value(prop_fields, string_xml.text().as_string());
                        }
                        // It will have a value already if Field Widths has been processed
                        break;

                    default:
                        if (auto prop = node->getPropPtr(result->second); prop)
                        {
                            // There's really no reason to convert the number, since set_value() would
                            // just convert the number back to a string.
                            prop->set_value(string_xml.text().as_string());
                        }
                        break;
                }
            }
        }
    }

    for (auto& string_xml: node_xml.children("bool"))
    {
        auto name = string_xml.attribute("name").as_sview();
        if (name.starts_with("proxy-"))
            name.remove_prefix(sizeof("proxy-") - 1);
        if (auto result = map_proxy_names.find(name); result != map_proxy_names.end())
        {
            switch (result->second)
            {
                case prop_value:
                    if (node->isGen(gen_wxRadioButton) || node->isGen(gen_wxCheckBox))
                    {
                        if (string_xml.text().as_bool())
                            node->set_value(prop_checked, true);
                    }
                    else if (node->isGen(gen_wxToggleButton))
                    {
                        if (string_xml.text().as_bool())
                            node->set_value(prop_pressed, true);
                    }
                    break;

                case prop_style:
                    if (node->isGen(gen_wxRadioBox))
                    {
                        if (name == "wxRA_SPECIFY_COLS")
                            node->set_value(prop_style, "columns");
                        else if (name == "wxRA_SPECIFY_ROWS")
                            node->set_value(prop_style, "rows");
                    }

                default:
                    if (auto prop = node->getPropPtr(result->second); prop)
                    {
                        if (string_xml.text().as_bool())
                        {
                            prop->set_value(true);
                        }
                    }
                    break;
            }
        }
    }
}

#if defined(INTERNAL_TESTING)
tt_string DialogBlocks::GatherErrorDetails(pugi::xml_node& xml_node, GenEnum::GenName getGenName)
#else
tt_string DialogBlocks::GatherErrorDetails(pugi::xml_node& /* xml_node */, GenEnum::GenName /* getGenName */)
#endif
{
#if defined(INTERNAL_TESTING)
    tt_string msg = "Name: ";
    if (getGenName != gen_unknown)
        msg << map_GenNames[getGenName];
    else
        msg << "Unknown gen_name";
    if (auto value = xml_node.find_child_by_attribute("string", "name", "proxy-Label"); value)
    {
        if (auto str = ExtractQuotedString(value); str.size())
            msg << ", Label: " << str;
    }

    if (auto value = xml_node.find_child_by_attribute("string", "name", "proxy-Member variable name"); value)
    {
        if (auto str = ExtractQuotedString(value); str.size())
            msg << ", VarName: " << str;
    }
    if (auto value = xml_node.find_child_by_attribute("string", "name", "proxy-Id name"); value)
    {
        if (auto str = ExtractQuotedString(value); str.size())
            msg << ", Id: " << str;
    }
    return msg;
#else
    return {};
#endif
}
