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
            auto msg = GatherErrorDetails(child_xml, getGenName);
            msg << ", Type: " << ExtractQuotedString(type);
            FAIL_MSG(tt_string() << "Unrecognized class in \"proxy-type\" property: " << ExtractQuotedString(type) << "\n"
                                 << msg)
            m_errors.emplace(tt_string("Unrecognized class in \"proxy-type\" property: ") << ExtractQuotedString(type));
        }
        return;
    }

    // DialogBlocks uses wbToolBarButtonProxy for all toolbar buttons, so MapClassName() always
    // turns it into gen_tool.
    if (getGenName == gen_tool && parent->isGen(gen_wxAuiToolBar))
    {
        getGenName = gen_auitool;
    }

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

    parent->adoptChild(node);

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

auto lst_window_styles = std::to_array({
    "wxBORDER_DEFAULT",
    "wxBORDER_NONE",
    "wxBORDER_STATIC",
    "wxBORDER_SIMPLE",
    "wxBORDER_RAISED",
    "wxBORDER_SUNKEN",
    "wxBORDER_DOUBLE",
    "wxBORDER_THEME",
    "wxTRANSPARENT_WINDOW",
    "wxTAB_TRAVERSAL",
    "wxWANTS_CHARS",
    "wxVSCROLL",
    "wxHSCROLL",
    "wxALWAYS_SHOW_SB",
    "wxCLIP_CHILDREN",
    "wxNO_FULL_REPAINT_ON_RESIZE",
});

auto lst_exwindow_styles = std::to_array({

    "wxWS_EX_VALIDATE_RECURSIVELY",
    "wxWS_EX_BLOCK_EVENTS",
    "wxWS_EX_TRANSIENT",
    "wxWS_EX_PROCESS_IDLE",
    "wxWS_EX_PROCESS_UI_UPDATES",

});

auto lst_dialog_styles = std::to_array({

    "wxCAPTION",
    "wxMINIMIZE_BOX",
    "wxMAXIMIZE_BOX",
    "wxSYSTEM_MENU",
    "wxRESIZE_BORDER",
    "wxCLOSE_BOX",
    "wxDEFAYLT_DIALOG_STYLE",
    "wxSTAY_ON_TOP",
    "wxDIALOG_NO_PARENT",
    "wxWANTS_CHARS",

});

auto lst_exdialog_styles = std::to_array({

    "wxDIALOG_EX_CONTEXTHELP",
    "wxDIALOG_EX_METAL",
    "wxWS_EX_BLOCK_EVENTS",
    "wxWS_EX_PROCESS_IDLE",
    "wxWS_EX_PROCESS_UI_UPDATES",
    "wxWS_EX_VALIDATE_RECURSIVELY",

});

auto lst_alignment_styles = std::to_array({

    "wxALIGN_TOP",
    "wxALIGN_BOTTOM",
    "wxALIGN_CENTER_VERTICAL",
    "wxALIGN_LEFT",
    "wxALIGN_RIGHT",
    "wxALIGN_CENTER_HORIZONTAL",
    "wxALIGN_CENTER",

});

auto lst_layout_flags = std::to_array({

    "wxEXPAND",
    "wxSHAPED",
    "wxFIXED_MINSIZE",
    "wxRESERVE_SPACE_EVEN_IF_HIDDEN",

});

auto lst_borders_flags = std::to_array({

    "wxALL",
    "wxLEFT",
    "wxRIGHT",
    "wxTOP",
    "wxBOTTOM",

});

// These are used to set prop_style
auto lst_style = std::to_array({

    "wxLI_HORIZONTAL",
    "wxLI_VERTICAL",

});

static const view_map s_map_old_borders = {

    { "wxDOUBLE_BORDER", "wxBORDER_DOUBLE" },
    { "wxSUNKEN_BORDER", "wxBORDER_SUNKEN" },
    { "wxRAISED_BORDER", "wxBORDER_RAISED" },
    { "wxBORDER", "wxBORDER_SIMPLE" },
    { "wxSIMPLE_BORDER", "wxBORDER_SIMPLE" },
    { "wxSTATIC_BORDER", "wxBORDER_STATIC" },
    {
        "wxNO_BORDER",
        "wxBORDER_NONE",
    },

};

void DialogBlocks::ProcessStyles(pugi::xml_node& node_xml, const NodeSharedPtr& new_node)
{
    std::set<std::string> styles;

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
        if (auto result = s_map_old_borders.find(name); result != s_map_old_borders.end())
        {
            name = result->second;
        }

        styles.emplace(name);
    }

    if (styles.empty())
        return;

    tt_string style_str;
    // Not all styles are valid for all forms, but we'll assume that DialogBlocks didn't add
    // any invalid ones. I *think* unused ones will be ignored...
    for (auto& style: lst_window_styles)
    {
        if (styles.contains(style))
        {
            if (style_str.size())
                style_str << '|';
            style_str << style;
        }
    }
    if (style_str.size())
    {
        new_node->set_value(prop_window_style, style_str);
    }

    style_str.clear();
    for (auto& style: lst_exwindow_styles)
    {
        if (styles.contains(style))
        {
            if (style_str.size())
                style_str << '|';
            style_str << style;
        }
    }
    if (style_str.size())
    {
        new_node->set_value(prop_window_extra_style, style_str);
    }

    style_str.clear();
    for (auto& style: lst_style)
    {
        if (styles.contains(style))
        {
            if (style_str.size())
                style_str << '|';
            style_str << style;
        }
    }
    if (style_str.size())
    {
        new_node->set_value(prop_style, style_str);
    }

    if (new_node->isGen(gen_wxDialog))
    {
        style_str.clear();
        for (auto& style: lst_dialog_styles)
        {
            if (styles.contains(style))
            {
                if (style_str.size())
                    style_str << '|';
                style_str << style;
            }
        }
        if (style_str.size())
        {
            new_node->set_value(prop_style, style_str);
        }

        style_str.clear();
        for (auto& style: lst_exdialog_styles)
        {
            if (styles.contains(style))
            {
                if (style_str.size())
                    style_str << '|';
                style_str << style;
            }
        }
        if (style_str.size())
        {
            new_node->set_value(prop_extra_style, style_str);
        }
    }

    // For wxUE, these are sizer_child settings, however DialogBlocks doesn't use any form of sizerchild, so
    // we look for alignment before attempting to set these. That's because the wxLEFT, wxRIGHT, etc. can be used
    // for other things besides where the border should be.

    if (new_node->hasProp(prop_alignment))
    {
        style_str.clear();
        for (auto& style: lst_alignment_styles)
        {
            if (styles.contains(style))
            {
                if (style_str.size())
                    style_str << '|';
                style_str << style;
            }
        }
        if (auto value = node_xml.find_child_by_attribute("string", "name", "proxy-AlignH"); value)
        {
            auto alignment = ExtractQuotedString(value);
            if (alignment.is_sameas("Right", tt::CASE::either))
            {
                if (style_str.size())
                    style_str << '|';
                style_str << "wxRIGHT";
            }
            else if (alignment.is_sameas("Centre", tt::CASE::either))
            {
                if (style_str.size())
                    style_str << '|';
                style_str << "wxALIGN_CENTER";
            }
        }
        if (auto value = node_xml.find_child_by_attribute("string", "name", "proxy-AlignV"); value)
        {
            auto alignment = ExtractQuotedString(value);
            if (alignment.is_sameas("Bottom", tt::CASE::either))
            {
                if (style_str.size())
                    style_str << '|';
                style_str << "wxBOTTOM";
            }
            else if (alignment.is_sameas("Centre", tt::CASE::either))
            {
                if (style_str.size())
                    style_str << '|';
                style_str << "wxALIGN_CENTER";
            }
        }
        if (style_str.size())
        {
            new_node->set_value(prop_alignment, style_str);
        }

        // Check for layout flags
        style_str.clear();
        for (auto& style: lst_layout_flags)
        {
            if (styles.contains(style))
            {
                if (style_str.size())
                    style_str << '|';
                style_str << style;
            }
        }

        if (auto value = node_xml.find_child_by_attribute("string", "name", "proxy-AlignH"); value)
        {
            if (ExtractQuotedString(value).is_sameas("Expand", tt::CASE::either))
            {
                if (style_str.size())
                    style_str << '|';
                style_str << "wxEXPAND";
            }
        }
        if (auto value = node_xml.find_child_by_attribute("string", "name", "proxy-AlignV"); value)
        {
            if (ExtractQuotedString(value).is_sameas("Expand", tt::CASE::either))
            {
                if (style_str.size())
                    style_str << '|';
                style_str << "wxEXPAND";
            }
        }

        if (style_str.size())
        {
            new_node->set_value(prop_flags, style_str);
        }

        // Check for border flags
        style_str.clear();
        for (auto& style: lst_borders_flags)
        {
            if (styles.contains(style))
            {
                if (style_str.size())
                    style_str << '|';
                style_str << style;
            }
        }
        if (style_str.size())
        {
            if (style_str.contains("wxLEFT") && style_str.contains("wxRIGHT") && style_str.contains("wxTOP") &&
                style_str.contains("wxBOTTOM"))
            {
                style_str.clear();
                style_str << "wxALL";
            }
            new_node->set_value(prop_borders, style_str);
        }
    }

    // REVIEW: [Randalphwa - 05-07-2023] What happens when something like wxRIGHT is used to indicate a bitmap position?
    // wxBannerWindow has a direction property that also uses wxLEFT, wxRIGHT etc.
}

static const std::map<std::string_view, GenEnum::PropName, std::less<>> map_proxy_names = {

    { "Background colour", prop_background_colour },
    { "Foreground colour", prop_foreground_colour },
    { "Hover colour", prop_hover_color },
    { "Normal colour", prop_normal_color },
    { "Visited colour", prop_visited_color },

    { "Column width", prop_default_col_size },
    { "ColumnSpacing", prop_hgap },
    { "Columns", prop_cols },
    { "Default filter", prop_defaultfilter },
    { "Default folder", prop_defaultfolder },
    { "Filter", prop_filter },
    { "Gravity", prop_sashgravity },
    { "GrowableColumns", prop_growablecols },
    { "GrowableRows", prop_growablerows },
    { "HTML code", prop_html_content },
    { "Help text", prop_context_help },
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
    { "Thumb size", prop_thumbsize },
    { "Tool packing", prop_packing },
    { "Tool separation", prop_separation },
    { "Tooltip text", prop_tooltip },
    { "URL", prop_html_url },

    { "Initial value", prop_value },  // In DialogBlocks used for all sorts of properties

};

void DialogBlocks::ProcessMisc(pugi::xml_node& node_xml, const NodeSharedPtr& node)
{
    for (auto& string_xml: node_xml.children("string"))
    {
        auto name = string_xml.attribute("name").as_sview();
        if (name.starts_with("proxy-"))
            name.remove_prefix(sizeof("proxy-") - 1);
        if (auto result = map_proxy_names.find(name); result != map_proxy_names.end())
        {
            auto str = ExtractQuotedString(string_xml);
            if (str.size())
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
                        break;

                    case prop_background_colour:
                    case prop_foreground_colour:
                    case prop_hover_color:
                    case prop_normal_color:
                    case prop_visited_color:
                        str.insert(0, "#");
                        node->set_value(result->second, str);
                        break;

                    default:
                        if (auto prop = node->getPropPtr(result->second); prop)
                        {
                            prop->set_value(str);
                        }
                        break;
                }
            }
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

tt_string DialogBlocks::GatherErrorDetails(pugi::xml_node& xml_node, GenEnum::GenName getGenName)
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
