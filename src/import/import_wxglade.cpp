/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a WxGlade file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "import_wxglade.h"

#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "dlg_msgs.h"        // wxMessageDialog dialogs
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "utils.h"           // Utility functions that work with properties

WxGlade::WxGlade() {}

bool WxGlade::Import(const tt_string& filename, bool write_doc)
{
    auto result = LoadDocFile(filename);
    if (!result)
    {
        return false;
    }
    auto root = result.value().first_child();

    if (!tt::is_sameas(root.name(), "application", tt::CASE::either))
    {
        dlgInvalidProject(filename, "wxGlade", "Import wxGlade project");
        return false;
    }

    if (auto language = root.attribute("language").as_view(); language.size())
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
        else if (language == "Perl")
        {
            m_language = GEN_LANG_PERL;
        }
    }
    else
    {
        // We don't support Perl or Lisp
        m_language = GEN_LANG_CPLUSPLUS;
    }

    // Using a try block means that if at any point it becomes obvious the project file is invalid and we cannot recover,
    // then we can throw an error and give a standard response about an invalid file.

    try
    {
        m_project = NodeCreation.createNode(gen_Project, nullptr).first;
        if (auto src_ext = root.attribute("source_extension").as_view(); src_ext.size())
        {
            if (src_ext == ".cpp" || src_ext == ".cc" || src_ext == ".cxx")
            {
                m_project->set_value(prop_source_ext, src_ext);
            }
        }
        if (auto hdr_ext = root.attribute("header_extension").as_view(); hdr_ext.size())
        {
            if (hdr_ext == ".h" || hdr_ext == ".hh" || hdr_ext == ".hpp" || hdr_ext == ".hxx")
            {
                m_project->set_value(prop_header_ext, hdr_ext);
            }
        }
        if (root.attribute("use_gettext").as_bool())
        {
            m_project->set_value(prop_internationalize, true);
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
                    if (new_node->hasValue(prop_class_name))
                    {
                        switch (m_language)
                        {
                            case GEN_LANG_CPLUSPLUS:
                                new_node->set_value(prop_base_file, new_node->as_string(prop_class_name));
                                break;

                            case GEN_LANG_PYTHON:
                                new_node->set_value(prop_python_file, new_node->as_string(prop_class_name));
                                break;

                            case GEN_LANG_XRC:
                                new_node->set_value(prop_xrc_file, new_node->as_string(prop_class_name));
                                break;
                        }
                    }
                }
                else
                {
                    if (m_language == GEN_LANG_PYTHON)
                    {
                        m_project->set_value(prop_python_combine_forms, true);
                        tt_string combined_filename = root.attribute("path").as_view();
                        tt_cwd cwd;
                        combined_filename.make_relative(cwd);
                        m_project->set_value(prop_python_combined_file, combined_filename);
                    }
                }
            }
        }

        if (!m_project->getChildCount())
        {
            wxMessageBox(filename.make_wxString() << " does not contain any top level forms.", "Import");
            return false;
        }

        if (!root.attribute("option").as_bool())
        {
            tt_string combined_filename = root.attribute("path").as_view();
            tt_cwd cwd;
            combined_filename.make_relative(cwd);

            if (m_project->getChildCount() > 1)
            {
                if (m_language == GEN_LANG_PYTHON)
                {
                    m_project->set_value(prop_python_combine_forms, true);
                    m_project->set_value(prop_python_combined_file, combined_filename);
                }
                else if (m_language == GEN_LANG_XRC)
                {
                    m_project->set_value(prop_combine_all_forms, true);
                    m_project->set_value(prop_combined_xrc_file, combined_filename);
                }
            }
            else
            {
                if (m_language == GEN_LANG_PYTHON)
                {
                    m_project->getChild(0)->set_value(prop_python_file, combined_filename);
                }
                else if (m_language == GEN_LANG_XRC)
                {
                    m_project->getChild(0)->set_value(prop_xrc_file, combined_filename);
                }
            }
        }

        if (write_doc)
            m_project->createDoc(m_docOut);
    }

    catch (const std::exception& err)
    {
        MSG_ERROR(err.what());
        dlgImportError(err, filename, "Import wxGlade project");
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
    auto getGenName = ConvertToGenName(object_name, parent);
    bool object_not_generator = false;
    if (getGenName == gen_unknown)
    {
        object_not_generator = true;
        // If we don't recognize the class, then try the base= attribute
        auto base = xml_obj.attribute("base").as_view();
        if (base == "EditFrame")
        {
            getGenName = ConvertToGenName("wxFrame", parent);
        }
        else if (base == "EditDialog")
        {
            getGenName = ConvertToGenName("wxDialog", parent);
        }
        else if (base == "EditTopLevelPanel")
        {
            getGenName = ConvertToGenName("Panel", parent);
        }

        if (getGenName == gen_unknown)
        {
            // TODO: [KeyWorks - 08-10-2021] wxGlade supports wxMDIChildFrame using a base name of "EditMDIChildFrame"

            // This appears to be a placeholder to reserve a spot. We just ignore it.
            if (object_name == "sizerslot")
                return NodeSharedPtr();

            MSG_INFO(tt_string() << "Unrecognized object: " << object_name);
            return NodeSharedPtr();
        }
    }

    if (getGenName == gen_wxCheckBox)
    {
        for (auto& iter: xml_obj.children())
        {
            if (iter.value() == "style")
            {
                if (iter.text().as_sview().contains("wxCHK_3STATE"))
                    getGenName = gen_Check3State;
                break;
            }
        }
    }

    auto new_node = NodeCreation.createNode(getGenName, parent).first;
    if (new_node && object_not_generator)
    {
        new_node->set_value(prop_class_name, object_name);
    }

    if (new_node)
    {
        if (getGenName == gen_wxMenuBar)
        {
            parent->adoptChild(new_node);
            CreateMenus(xml_obj, new_node.get());
            return new_node;
        }
        else if (getGenName == gen_wxToolBar)
        {
            parent->adoptChild(new_node);
            CreateToolbar(xml_obj, new_node.get());
            return new_node;
        }

        else if (getGenName == gen_BookPage)
        {
            if (!xml_obj.attribute("name").empty())
            {
                if (auto tab = m_notebook_tabs.find(xml_obj.attribute("name").as_view()); tab != m_notebook_tabs.end())
                {
                    new_node->set_value(prop_label, tab->second);
                }
            }
        }
    }

    while (!new_node)
    {
        if (parent->isGen(gen_wxNotebook))
        {
            if (getGenName == gen_wxPanel)
            {
                new_node = NodeCreation.createNode(gen_BookPage, parent).first;
                if (new_node)
                {
                    if (!xml_obj.attribute("name").empty())
                    {
                        if (auto tab = m_notebook_tabs.find(xml_obj.attribute("name").as_view());
                            tab != m_notebook_tabs.end())
                        {
                            new_node->set_value(prop_label, tab->second);
                        }
                    }
                    continue;
                }
            }
            else
            {
                if (auto page = NodeCreation.createNode(gen_PageCtrl, parent).first; page)
                {
                    parent->adoptChild(page);
                    if (!xml_obj.attribute("name").empty())
                    {
                        if (auto tab = m_notebook_tabs.find(xml_obj.attribute("name").as_view());
                            tab != m_notebook_tabs.end())
                        {
                            page->set_value(prop_label, tab->second);
                        }
                    }

                    new_node = NodeCreation.createNode(getGenName, page.get()).first;
                    if (new_node)
                        continue;
                }
            }
        }
        MSG_INFO(tt_string() << "Unable to create " << map_GenNames[getGenName] << " as a child of " << parent->declName());
        return NodeSharedPtr();
    }

    if (isBitmapButton)
    {
        new_node->set_value(prop_label, "");
    }

    if (auto prop = new_node->getPropPtr(prop_var_name); prop)
    {
        auto original = prop->as_string();
        auto new_name = parent->getUniqueName(prop->as_string());
        if (new_name.size() && new_name != prop->as_string())
            prop->set_value(new_name);
    }

    if (new_node->isGen(gen_wxStdDialogButtonSizer))
    {
        parent->adoptChild(new_node);
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get());

        tt_string last_handler;

        for (auto& child: xml_obj.children())
        {
            if (child.name() != "object")
                continue;
            ASSERT(child.attribute("class").as_view() == "sizeritem");

            for (auto& button: child.children())
            {
                if (button.name() != "object")
                    continue;
                last_handler.clear();
                for (auto& btn_props: button.children())
                {
                    auto SetBtnAndHandler = [&](PropName prop_name, tt_string_view event_name)
                    {
                        new_node->getPropPtr(prop_name)->set_value("1");
                        if (last_handler.size())
                        {
                            if (auto* event = new_node->getEvent(event_name); event)
                            {
                                event->set_value(last_handler);
                            }
                        }
                    };

                    if (btn_props.name() == "events")
                    {
                        for (auto& handler: btn_props.children())
                        {
                            last_handler = handler.text().as_view();
                        }
                    }
                    else if (btn_props.name() == "id")
                    {
                        if (auto id = btn_props.text().as_view(); tt::is_sameprefix(id, "wxID_"))
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
                        if (auto id = btn_props.text().as_view(); id.size())
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

        new_node->getPropPtr(prop_alignment)->set_value("wxALIGN_RIGHT");
        return new_node;
    }

    auto child = xml_obj.child("object");
    if (!child && new_node->isGen(gen_wxMenuBar))
        child = xml_obj.child("menus");
    if (NodeCreation.isOldHostType(new_node->declName()))
    {
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get(), parent);
        new_node = CreateGladeNode(child, parent, new_node.get());
        // ASSERT(new_node);
        if (!new_node)
            return NodeSharedPtr();
        if (new_node->isGen(gen_wxStdDialogButtonSizer))
            new_node->getPropPtr(prop_static_line)->set_value(false);
        child = child.next_sibling("object");
    }
    else if (sizeritem)
    {
        for (auto& iter: sizeritem->getPropsVector())
        {
            auto prop = new_node->addNodeProperty(iter.getPropDeclaration());
            prop->set_value(iter.as_string());
        }
        parent->adoptChild(new_node);
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get());
    }
    else if (parent)
    {
        parent->adoptChild(new_node);
        ProcessAttributes(xml_obj, new_node.get());
        ProcessProperties(xml_obj, new_node.get());
    }

    if (new_node->isGen(gen_wxGridSizer))
    {
        if (new_node->as_int(prop_rows) > 0 && new_node->as_int(prop_cols) > 0)
            new_node->set_value(prop_rows, 0);
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
            event_name += handler.attribute("event").as_view();
            if (auto* event = node->getEvent(event_name); event)
            {
                event->set_value(handler.text().as_view());
            }
        }

        return true;
    }
    else if (node_name == "arguments" && node->isGen(gen_CustomControl))
    {
        tt_string parameters;
        for (auto& argument: xml_obj.children())
        {
            tt_string param = argument.text().as_view();
            param.Replace("$parent", "${parent}");
            param.Replace("$id", "${id}");
            if (parameters.size())
                parameters += ", ";
            parameters += param;
        }

        if (parameters.size())
            node->set_value(prop_parameters, parameters);

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
    else if (node_name == "option" && node->isGen(gen_sizeritem))
    {
        node->set_value(prop_proportion, xml_obj.text().as_view());
        return true;
    }
    else if (node_name == "scroll_rate")
    {
        tt_string param = xml_obj.text().as_view();
        tt_view_vector params(param, ',');
        node->set_value(prop_scroll_rate_x, params[0]);
        node->set_value(prop_scroll_rate_y, params[1]);
        return true;
    }
    else if (node_name == "extracode_post")
    {
        if (m_language == GEN_LANG_PYTHON)
            node->set_value(prop_python_insert, xml_obj.text().as_view());
        else if (m_language == GEN_LANG_CPLUSPLUS)
            node->set_value(prop_source_preamble, xml_obj.text().as_view());
        return true;
    }
    else if (node_name == "stockitem" && node->isGen(gen_wxButton))
    {
        if (node->as_string(prop_id).empty() || node->as_string(prop_id) == "wxID_ANY")
        {
            tt_string id("wxID_");
            id << xml_obj.text().as_view();
            node->set_value(prop_id, id);

            if (node->as_string(prop_label).empty() || node->as_string(prop_label) == "MyButton")
            {
                // This is a stock button, so let wxWidgets set the label
                node->set_value(prop_label, "");
            }

            return true;
        }
    }
    else if (node_name == "scrollable")
    {
        // [Randalphwa - 10-11-2023]
        // wxGlade will set this to 1 for wxScrolledWindow. In the wxGlade interface (1.1.0) if
        // you uncheck this it will generate an Error in wxGlade, but will generate code and
        // XML file using wxPanel without this property. Unless it's used for something besides
        // wxScrolledWindow, I think we can just ignore it.
        return true;
    }
    else if (node_name == "menubar")
    {
        // This gets set to 1 if the form has a menubar. We don't need to do anything with it.
        return true;
    }
    else if (node_name == "focused" && node->isForm())
    {
        // This is an option for dialogs -- no idea what it is supposed to do...
        return true;
    }
    else if (node_name == "custom_constructor" && node->isGen(gen_CustomControl))
    {
        // wxGlade specifes the construction code on the right side of the = sign, so we need to
        // insert what should be on the left side.
        tt_string construction;
        if (m_language == GEN_LANG_PYTHON)
        {
            construction = "self." + node->as_string(prop_var_name) + " = ";
            construction += xml_obj.text().as_view();
        }
        else if (m_language == GEN_LANG_CPLUSPLUS)
        {
            construction = node->as_string(prop_var_name) + " = ";
            construction += xml_obj.text().as_view();
        }
        else
        {
            // Construction is not supported in any other language
            return true;
        }

        node->set_value(prop_construction, construction);
        return true;
    }
    return false;
}

// Called by ImportXML -- return true if the property is processed. Use this when the property conversion
// is different in wxGlade then for other XML projects for the type of node being processed.
bool WxGlade::HandleNormalProperty(const pugi::xml_node& xml_obj, Node* node, Node* parent, GenEnum::PropName wxue_prop)
{
    if (node->isGen(gen_sizeritem))
    {
        // wxGlade sizeritems use slightly different property names then we do, so handle those
        // here.
        if (wxue_prop == prop_border)
        {
            // wxGlade uses border for border_size in a sizer
            node->set_value(prop_border_size, xml_obj.text().as_view());
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
        tt_string id = xml_obj.text().as_view();
        id.erase_from('=');
        id.trim();
        node->set_value(prop_id, id);
        return true;
    }

    return false;
}

void WxGlade::CreateMenus(pugi::xml_node& xml_obj, Node* parent)
{
    auto menus = xml_obj.child("menus");
    ASSERT(menus);
    if (!menus)
        return;

    for (auto& menu: menus.children("menu"))
    {
        auto menu_node = NodeCreation.createNode(gen_wxMenu, parent).first;
        parent->adoptChild(menu_node);
        for (auto& iter: menu.attributes())
        {
            if (iter.name() == "name")
            {
                menu_node->set_value(prop_var_name, iter.value());
            }
            else if (iter.name() == "label")
            {
                menu_node->set_value(prop_label, iter.value());
            }
        }

        for (auto& item: menu.children("item"))
        {
            auto id = item.child("id");

            auto new_item =
                NodeCreation.createNode(id.text().as_view() == "---" ? gen_separator : gen_wxMenuItem, menu_node.get())
                    .first;
            menu_node->adoptChild(new_item);

            for (auto& iter: item.children())
            {
                if (iter.name() == "label")
                {
                    new_item->set_value(prop_label, iter.text().as_view());
                }
                else if (iter.name() == "id")
                {
                    tt_string id_value = iter.text().as_view();
                    if (m_language == GEN_LANG_PYTHON)
                    {
                        id_value.Replace(".", "", true);
                    }
                    new_item->set_value(prop_id, id_value);
                }
                else if (iter.name() == "name")
                {
                    new_item->set_value(prop_var_name, iter.text().as_view());
                }
                else if (iter.name() == "help_str")
                {
                    new_item->set_value(prop_help, iter.text().as_view());
                }
                else if (iter.name() == "checkable")
                {
                    new_item->set_value(prop_checked, iter.text().as_view());
                    new_item->set_value(prop_kind, "wxITEM_CHECK");
                }
                else if (iter.name() == "radio")
                {
                    new_item->set_value(prop_checked, iter.text().as_view());
                    new_item->set_value(prop_kind, "wxITEM_RADIO");
                }
                else if (iter.name() == "handler")
                {
                    if (auto* event = new_item->getEvent("wxEVT_MENU"); event)
                    {
                        event->set_value(iter.text().as_view());
                    }
                }
            }
        }
    }
}

void WxGlade::CreateToolbar(pugi::xml_node& xml_obj, Node* parent)
{
    auto tools = xml_obj.child("tools");
    ASSERT(tools);
    if (!tools)
        return;

    for (auto& tool: tools.children("tool"))
    {
        auto id = tool.child("id");

        auto new_tool = NodeCreation.createNode(id.text().as_view() == "---" ? gen_separator : gen_wxMenuItem, parent).first;
        parent->adoptChild(new_tool);
        for (auto& iter: tool.children())
        {
            if (iter.name() == "label")
            {
                new_tool->set_value(prop_label, iter.text().as_view());
            }
            else if (iter.name() == "id")
            {
                tt_string id_value = iter.text().as_view();
                if (m_language == GEN_LANG_PYTHON)
                {
                    id_value.Replace(".", "", true);
                }
                new_tool->set_value(prop_id, id_value);
            }
            else if (iter.name() == "short_help")
            {
                new_tool->set_value(prop_tooltip, iter.text().as_view());
            }
            else if (iter.name() == "handler")
            {
                if (auto* event = new_tool->getEvent("wxEVT_TOOL"); event)
                {
                    event->set_value(iter.text().as_view());
                }
            }
        }
    }
}
