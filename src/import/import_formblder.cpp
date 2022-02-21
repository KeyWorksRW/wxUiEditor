/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxFormBuider project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iostream>
#include <set>

#include <wx/mstream.h>  // Memory stream classes

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "import_formblder.h"

#include "base_generator.h"  // BaseGenerator -- Base widget generator class
#include "font_prop.h"       // FontProperty class
#include "mainapp.h"         // App -- Main application class
#include "mainframe.h"       // Main window frame
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "pjtsettings.h"     // ProjectSettings -- Hold data for currently loaded project
#include "utils.h"           // Utility functions that work with properties

#include "import_arrays.cpp"  // Array of formbuilder/wxuieditor event name pairs

// clang-format off

const auto g_lstIgnoreProps = {

    "xrc_skip_sizer",   // used for XRC code generation which we don't support

    "event_handler",  // all events are now declared as virtual

    // The following are wxFormBuilder properties for wxAuiToolBar

    "label_visible",
    "toolbar_label",
    "use_explicit_ids",

    // The following are AUI properties. Unless AUI frame windows gets implemented, these will all be ignored

    "BottomDockable",
    "LeftDockable",
    "RightDockable",
    "TopDockable",
    "aui_layer",
    "aui_managed",
    "aui_manager_style",
    "aui_name",
    "aui_position",
    "aui_row",
    "best_size",
    "caption",
    "caption_visible",
    "center_pane",
    "close_button",
    "context_menu",
    "default_pane",
    "dock",
    "dock_fixed",
    "docking",
    "event_generation",
    "first_id",
    "floatable",
    "gripper",
    "max_size",
    "maximize_button",
    "min_size",
    "minimize_button",
    "moveable",
    "pane_border",
    "pane_position",
    "pane_size",
    "parent",
    "pin_button",
    "resize",
    "show",
    "toolbar_pane",

    // This are miscellanious properties that we don't support

    "two_step_creation",
    "use_enum",

};
// clang-format on

// The following gets initialized once if FormBuilder is constructed. They do not get destroyed since they are a) quite
// small, and b) might be used again. This is in contrast to m_mapEventNames which is part of the FormBuilder class.

std::unordered_set<std::string> g_setIgnoreProps;

FormBuilder::FormBuilder()
{
    for (size_t pos = 0; evt_pair[pos].wxfb_name; ++pos)
    {
        m_mapEventNames[evt_pair[pos].wxfb_name] = evt_pair[pos].wxui_name;
    }

    if (g_setIgnoreProps.empty())
    {
        for (auto& iter: g_lstIgnoreProps)
        {
            g_setIgnoreProps.insert(iter);
        }
    }
}

bool FormBuilder::Import(const ttString& filename, bool write_doc)
{
    auto result = LoadDocFile(filename);
    if (!result)
    {
        return false;
    }
    auto root = result.value().first_child();

    if (!ttlib::is_sameas(root.name(), "wxFormBuilder_Project", tt::CASE::either))
    {
        wxMessageBox(wxString() << filename << " is not a wxFormBuilder file", "Import wxFormBuilder project");
        return false;
    }

    if (root.text().as_string() != std::string("object"))
    {
        auto fileVersion = root.child("FileVersion");
        if (fileVersion)
        {
            m_VerMinor = fileVersion.attribute("minor").as_int();
        }
    }

    // Using a try block means that if at any point it becomes obvious the formbuilder file is invalid and we cannot recover,
    // then we can throw an error and give a standard response about an invalid file.

    try
    {
        auto object = root.child("object");
        if (!object)
        {
            FAIL_MSG("formbuilder project file does not have a root \"object\" node.")
            throw std::runtime_error("Invalid project file");
        }

        m_project = g_NodeCreator.CreateNode(gen_Project, nullptr);

        CreateProjectNode(object, m_project.get());

        if (write_doc)
            m_project->CreateDoc(m_docOut);
    }

    catch (const std::exception& DBG_PARAM(e))
    {
        MSG_ERROR(e.what());
        wxMessageBox(wxString("This wxFormBuilder project file is invalid and cannot be loaded: ") << filename,
                     "Import wxFormBuilder project");
        return false;
    }

    if (m_errors.size())
    {
        ttlib::cstr errMsg("Not everything in the wxFormBuilder project could be converted:\n\n");
        MSG_ERROR(ttlib::cstr() << "------  " << m_importProjectFile.filename().wx_str() << "------");
        for (auto& iter: m_errors)
        {
            MSG_ERROR(iter);
            errMsg << iter << '\n';
        }

        wxMessageBox(errMsg, "Import wxFormBuilder project");
    }

    return true;
}

void FormBuilder::CreateProjectNode(pugi::xml_node& xml_obj, Node* new_node)
{
    for (auto& xml_prop: xml_obj.children("property"))
    {
        if (auto prop_name = xml_prop.attribute("name"); prop_name)
        {
            if (!xml_prop.text().empty())
            {
                // In wxUE, a lot of properties are specific to the form. For example, it's perfectly fine to
                // connect to events using Bind for a Dialog and a table macro for a Frame.

                if (prop_name.as_cview().is_sameas("internationalize"))
                {
                    new_node->get_prop_ptr(prop_internationalize)->set_value(xml_prop.text().as_bool() ? "1" : "0");
                }
                else if (prop_name.as_cview().is_sameas("help_provider"))
                {
                    new_node->get_prop_ptr(prop_help_provider)->set_value(xml_prop.text().as_string());
                }
                else if (prop_name.as_cview().is_sameas("precompiled_header"))
                {
                    // wxFormBuilder calls it a precompiled header, but uses it as a preamble.
                    new_node->get_prop_ptr(prop_src_preamble)->set_value(xml_prop.text().as_string());
                }

                else if (prop_name.as_cview().is_sameas("embedded_files_path"))
                {
                    ttString path(xml_prop.text().as_cstr().wx_str());
                    ttString root(m_importProjectFile);
                    root.remove_filename();
                    path.make_relative_wx(root);
                    m_project->prop_set_value(prop_art_directory, path);
                }
                else if (prop_name.as_cview().is_sameas("path"))
                {
                    ttString path(xml_prop.text().as_cstr().wx_str());
                    ttString root(m_importProjectFile);
                    root.remove_filename();
                    path.make_relative_wx(root);
                    m_project->prop_set_value(prop_base_directory, path);
                }
                else if (prop_name.as_cview().is_sameas("file"))
                {
                    m_baseFile = xml_prop.text().as_string();
                }
                else if (prop_name.as_cview().is_sameas("class_decoration"))
                {
                    m_class_decoration = xml_prop.text().as_string();
                    // Current formbuild uses "; " as the default property value
                    if (m_class_decoration.is_sameprefix((";")))
                    {
                        m_class_decoration.clear();
                    }
                }
                else if (prop_name.as_cview().is_sameas("namespace") && xml_prop.text().as_cview().size())
                {
                    ConvertNameSpaceProp(new_node->get_prop_ptr(prop_name_space), xml_prop.text().as_cview());
                }
            }
        }
    }

    auto child = xml_obj.child("object");
    while (child)
    {
        CreateFbpNode(child, new_node);
        child = child.next_sibling("object");
    }
}

NodeSharedPtr FormBuilder::CreateFbpNode(pugi::xml_node& xml_obj, Node* parent, Node* sizeritem)
{
    auto class_name = xml_obj.attribute("class").as_cview();
    if (class_name.empty())
        return NodeSharedPtr();

    auto gen_name = MapClassName(xml_obj.attribute("class").value());
    if (gen_name == gen_unknown)
    {
        if (class_name.contains("bookpage"))
        {
            gen_name = gen_oldbookpage;
        }
        else if (class_name.is_sameprefix("ribbon"))
        {
            if (class_name.contains("Tool"))
            {
                gen_name = gen_ribbonTool;
            }
            else
            {
                gen_name = gen_ribbonButton;
            }
        }

        else
        {
            m_errors.emplace(ttlib::cstr() << class_name << " is not supported in wxUiEditor");
            return {};
        }
    }

    if (gen_name == gen_wxPanel)
    {
        if (!parent)
        {
            // This gets called when pasting a formbuilder node from the clipboard
            auto owner = wxGetFrame().GetSelectedNode();
            while (owner->gen_type() == type_sizer)
                owner = owner->GetParent();
            if (owner->DeclName().contains("book"))
            {
                gen_name = gen_BookPage;
            }
        }
        else if (parent->DeclName().contains("book"))
        {
            gen_name = gen_BookPage;
        }
    }
    else if (gen_name == gen_wxCheckBox)
    {
        for (auto& iter: xml_obj.children())
        {
            if (iter.attribute("name").as_cview().is_sameas("style"))
            {
                if (iter.text().as_cview().contains("wxCHK_3STATE"))
                    gen_name = gen_Check3State;
                break;
            }
        }
    }
    else if (gen_name == gen_tool && parent->isGen(gen_wxAuiToolBar))
    {
        gen_name = gen_auitool;
    }

    auto newobject = g_NodeCreator.CreateNode(gen_name, parent);
    if (!newobject)
    {
        ttlib::cstr msg("Unable to create ");
        msg << class_name;
        if (parent)
        {
            // We can't use the class name because that won't necessarily be the wxWidgets class name. E.g., PanelForm might
            // be the class name, but what we want to display to the user is wxPanel. GetHelpText() will give us something
            // that makes sense to the user.

            auto name = parent->GetGenerator()->GetHelpText(parent);
            if (name.size() && name != "wxWidgets")
            {
#if defined(_DEBUG)
                // Currently, Debug builds also include the filename that gets passed to the browser if Help is requested.
                // That's not useful in a message box, so we remove it.

                name.erase_from('(');
#endif  // _DEBUG
                msg << " as a child of " << name;
            }
        }
        m_errors.emplace(msg);
        return {};
    }
    if (m_class_decoration.size() && newobject->IsForm())
        newobject->prop_set_value(prop_class_decoration, m_class_decoration);

    if (gen_name == gen_ribbonButton || gen_name == gen_ribbonTool)
    {
        // wxFormBuilder uses a control for each type (8 total controls). wxUiEditor only uses 2 controls, and instead
        // uses prop_kind to specify the type of button to use.

        if (class_name.contains("Dropdown"))
        {
            newobject->prop_set_value(prop_kind, "wxRIBBON_BUTTON_DROPDOWN");
        }
        else if (class_name.contains("Hybrid"))
        {
            newobject->prop_set_value(prop_kind, "wxRIBBON_BUTTON_HYBRID");
        }
        else if (class_name.contains("Toggle"))
        {
            newobject->prop_set_value(prop_kind, "wxRIBBON_BUTTON_TOGGLE");
        }
    }

    for (auto xml_prop = xml_obj.child("property"); xml_prop; xml_prop = xml_prop.next_sibling("property"))
    {
        if (auto prop_name = xml_prop.attribute("name").as_cview(); prop_name.size())
        {
            auto wxue_prop = MapPropName(xml_prop.attribute("name").value());
            auto prop_ptr = newobject->get_prop_ptr(wxue_prop);

            if (prop_ptr)
            {
                if (wxue_prop == prop_bitmap)
                {
                    if (!xml_prop.text().empty())
                        BitmapProperty(xml_prop, prop_ptr);
                }
                else if (wxue_prop == prop_inactive_bitmap)
                {
                    if (!xml_prop.text().empty())
                        BitmapProperty(xml_prop, prop_ptr);
                }
                else if (wxue_prop == prop_view_whitespace)
                {
                    // There are 4 possible values, but wxFormBuilder only supports this as a bool
                    if (xml_prop.text().as_bool())
                    {
                        prop_ptr->set_value("always visible");
                    }
                    continue;
                }
                else if (wxue_prop == prop_bitmapsize)
                {
                    if (class_name.contains("book"))
                    {
                        if (prop_ptr = newobject->get_prop_ptr(prop_image_size); prop_ptr)
                        {
                            prop_ptr->set_value(xml_prop.text().as_cview());
                            auto size = prop_ptr->as_size();
                            if (size != wxDefaultSize)
                            {
                                if (prop_ptr = newobject->get_prop_ptr(prop_display_images); prop_ptr)
                                {
                                    prop_ptr->set_value(true);
                                }
                            }
                            continue;
                        }
                    }
                }
                else if (wxue_prop == prop_animation)
                {
                    if (!xml_prop.text().empty())
                    {
                        ttlib::cstr animation("Embed;");
                        animation << xml_prop.text().as_cview() << ";[-1,-1]";
                        prop_ptr->set_value(animation);
                    }
                }
                else if (prop_ptr->isProp(prop_style))
                {
                    ProcessStyle(xml_prop, newobject.get(), prop_ptr);
                }
                else if (wxue_prop == prop_font)
                {
                    if (!xml_prop.text().empty())
                    {
                        FontProperty font_prop;
                        font_prop.Convert(xml_prop.text().as_string());
                        prop_ptr->set_value(font_prop.as_string());
                    }
                }
                else if (wxue_prop == prop_window_style)
                {
                    // wxFormBuilder uses older style names from wxWidgets 2.x. Rename them to the 3.x names, and remove
                    // the ones that are no longer used.
                    auto value = xml_prop.text().as_cstr();
                    if (value.contains("wxSIMPLE_BORDER"))
                        value.Replace("wxSIMPLE_BORDER", "wxBORDER_SIMPLE");
                    else if (value.contains("wxRAISED_BORDER"))
                        value.Replace("wxRAISED_BORDER", "wxBORDER_RAISED");
                    else if (value.contains("wxSTATIC_BORDER"))
                        value.Replace("wxSTATIC_BORDER", "wxBORDER_STATIC");
                    else if (value.contains("wxNO_BORDER"))
                        value.Replace("wxNO_BORDER", "wxBORDER_NONE");
                    else if (value.contains("wxDOUBLE_BORDER"))
                        value.Replace("wxDOUBLE_BORDER", "");  // this style is obsolete

                    prop_ptr->set_value(value);
                }
                else if (!xml_prop.text().empty())
                {
                    prop_ptr->set_value(xml_prop.text().as_string());
                }
                continue;
            }

            // If we get here, wxue_prop will be prop_unknown and prop_ptr will be null.

            if (prop_name.is_sameas("name"))
            {
                if (newobject->IsForm())
                {
                    prop_ptr = newobject->get_prop_ptr(prop_class_name);
                }
                else if (newobject->isGen(gen_ribbonTool) || newobject->isGen(gen_ribbonButton) ||
                         newobject->isGen(gen_ribbonGalleryItem))
                {
                    // FormBuilder has a property for this but doesn't use it, nor do we.
                    continue;
                }

                else
                {
                    prop_ptr = newobject->get_prop_ptr(prop_var_name);
                }

                ASSERT(prop_ptr);

                if (prop_ptr)
                {
                    prop_ptr->set_value(xml_prop.text().as_cview());
                }
                continue;
            }
            else if (prop_name.is_sameas("declaration"))
            {
                // This property is for a custom control, and we don't use this specific property
                continue;
            }
            else if (prop_name.is_sameas("construction"))
            {
                ttlib::cstr copy = xml_prop.text().as_string();
                if (auto pos = copy.find('('); ttlib::is_found(pos))
                {
                    copy.erase(0, pos);
                }
                if (auto pos = copy.find(';'); ttlib::is_found(pos))
                {
                    copy.erase_from(';');
                }

                newobject->prop_set_value(prop_parameters, copy);
                continue;
            }
            else if (prop_name.is_sameas("settings"))
            {
                newobject->prop_set_value(prop_settings_code, xml_prop.text().as_string());
            }
            else if (prop_name.is_sameas("include"))
            {
                ttlib::cstr header;
                header.ExtractSubString(xml_prop.text().as_cview().view_stepover());
                if (header.size())
                {
                    newobject->prop_set_value(prop_header, header);
                }
                continue;
            }

            // If the property actually has a value, then we need to see if we can convert it. We ignore unknown
            // properties that don't have a value.
            if (auto value = xml_prop.text().as_cview(); value.size())
            {
                ProcessPropValue(xml_prop, prop_name, class_name, newobject.get());
            }
        }
    }

    // At this point, all properties have been processed.

    if (newobject->isGen(gen_wxGridSizer) || newobject->isGen(gen_wxFlexGridSizer))
    {
        if (newobject->prop_as_int(prop_rows) > 0 && newobject->prop_as_int(prop_cols) > 0)
            newobject->prop_set_value(prop_rows, 0);
    }

    // wxFormBuilder allows the users to create settings that will generate an assert if compiled on a debug version of
    // wxWidgets. We fix some of the more common invalid settings here.

    if (newobject->HasValue(prop_flags) && newobject->prop_as_string(prop_flags).contains("wxEXPAND"))
    {
        if (newobject->HasValue(prop_alignment))
        {
            // wxWidgets will ignore all alignment flags if wxEXPAND is set.
            newobject->prop_set_value(prop_alignment, "");
        }
    }

    if (parent && parent->IsSizer())
    {
        if (parent->prop_as_string(prop_orientation).contains("wxHORIZONTAL"))
        {
            auto currentValue = newobject->prop_as_string(prop_alignment);
            if (currentValue.size() && (currentValue.contains("wxALIGN_LEFT") || currentValue.contains("wxALIGN_RIGHT") ||
                                        currentValue.contains("wxALIGN_CENTER_HORIZONTAL")))
            {
                auto fixed = ClearMultiplePropFlags("wxALIGN_LEFT|wxALIGN_RIGHT|wxALIGN_CENTER_HORIZONTAL", currentValue);
                newobject->prop_set_value(prop_alignment, fixed);
            }
        }
        else if (parent->prop_as_string(prop_orientation).contains("wxVERTICAL"))
        {
            auto currentValue = newobject->prop_as_string(prop_alignment);
            if (currentValue.size() && (currentValue.contains("wxALIGN_TOP") || currentValue.contains("wxALIGN_BOTTOM") ||
                                        currentValue.contains("wxALIGN_CENTER_VERTICAL")))
            {
                auto fixed = ClearMultiplePropFlags("wxALIGN_TOP|wxALIGN_BOTTOM|wxALIGN_CENTER_VERTICAL", currentValue);
                newobject->prop_set_value(prop_alignment, fixed);
            }
        }
    }

    auto xml_event = xml_obj.child("event");
    while (xml_event)
    {
        if (auto event_name = xml_event.attribute("name").as_cview();
            event_name.size() && xml_event.text().as_cview().size())
        {
            if (auto result = m_mapEventNames.find(event_name.c_str()); result != m_mapEventNames.end())
            {
                event_name = result->second;
                if (event_name.is_sameas("wxEVT_MENU") && newobject->isGen(gen_tool))
                    event_name = "wxEVT_TOOL";
            }
            else
            {
                // There is nothing in the wxWidgets source code that actually generates wxEVT_HIBERNATE that I can find as
                // of wxWidgets 3.1.3. It was removed in wxFormBuilder (but I couldn't find a reason as to why).
                // Documentation states it's part of WinCE which we don't support.
                if (event_name.is_sameas("OnHibernate"))
                {
                    xml_event = xml_event.next_sibling("event");
                    continue;
                }

                // REVIEW: [KeyWorks - 10-28-2020] We don't support this, but we could convert it if it's actually used.
                else if (event_name.is_sameas("OnMouseEvents"))
                {
                    xml_event = xml_event.next_sibling("event");
                    continue;
                }

                MSG_INFO(ttlib::cstr() << event_name << " event not supported");
                xml_event = xml_event.next_sibling("event");
                continue;
            }

            if (auto event = newobject->GetEvent(event_name); event)
            {
                event->set_value(xml_event.text().as_cview());
            }
        }

        xml_event = xml_event.next_sibling("event");
    }

    auto child = xml_obj.child("object");
    if (g_NodeCreator.IsOldHostType(newobject->DeclName()))
    {
        newobject = CreateFbpNode(child, parent, newobject.get());
        if (!newobject)
        {
            return newobject;
        }

        if (newobject->isGen(gen_wxStdDialogButtonSizer))
        {
            // wxFormBuilder isn't able to add a sizer using CreateSeparatedSizer(), so the user has to add a static line
            // above wxStdDialogButtonSizer. The problem with that approach is that if the program is compiled for MAC then
            // there should *not* be a line above the standard buttons. We fix that be removing the static line -- wxUE
            // defaults to adding the line via CreateSeparatedSizer().

            auto pos = parent->GetChildPosition(newobject.get());
            if (pos > 0)
            {
                auto prior_sibling = parent->GetChild(pos - 1);
                if (prior_sibling->isGen(gen_wxStaticLine))
                {
                    parent->RemoveChild(pos - 1);
                }
                else
                {
                    newobject->get_prop_ptr(prop_static_line)->set_value(false);
                }
            }
        }
        child = child.next_sibling("object");
    }
    else if (sizeritem)
    {
        for (auto& iter: sizeritem->get_props_vector())
        {
            auto prop = newobject->AddNodeProperty(iter.GetPropDeclaration());
            prop->set_value(iter.as_string());
        }
        if (parent)
        {
            parent->Adopt(newobject);
        }
    }
    else if (parent)
    {
        parent->Adopt(newobject);
    }

    while (child)
    {
        CreateFbpNode(child, newobject.get());
        child = child.next_sibling("object");
    }

    if (newobject->isGen(gen_wxDialog) && m_baseFile.size())
    {
        if (auto prop = newobject->get_prop_ptr(prop_base_file); prop)
        {
            prop->set_value(m_baseFile);
        }
    }

    return newobject;
}

void FormBuilder::ProcessPropValue(pugi::xml_node& xml_prop, ttlib::cview prop_name, ttlib::cview class_name,
                                   Node* newobject)
{
    if (g_setIgnoreProps.find(prop_name.c_str()) != g_setIgnoreProps.end())
    {
        return;
    }

    // validator_style sets the wxFILTER flags and is only valid in a wxTextValidator, so it's
    // removed from widgets that can't use it.
    else if (prop_name.is_sameas("validator_style"))
    {
        return;
    }

    // validator_type is only valid in wxTextValidator, where it let's the user choose between wxTextValidator and
    // wxGenericValidator
    else if (prop_name.is_sameas("validator_type"))
    {
        // BUGBUG: [KeyWorks - 11-07-2020] This IS valid for text controls, so we need to process it
        return;
    }

    // This will be caused by a spacer item which isn't actually a widget, so has no access property
    else if (prop_name.is_sameas(map_PropNames[prop_class_access]))
    {
        return;
    }

    // The label property in a wxMenuBar is not supported (since it can't actually be used)
    else if (prop_name.is_sameas(map_PropNames[prop_label]))
    {
        return;
    }

    // This is most likely a Dialog class -- we don't support wxAUI in that class. We silently ignore it...
    else if (prop_name.is_sameas("aui_managed") || prop_name.is_sameas("aui_manager_style"))
    {
        return;
    }

    else if (prop_name.is_sameas("flag") && (class_name.is_sameas("sizeritem") || class_name.is_sameas("gbsizeritem")))
    {
        HandleSizerItemProperty(xml_prop, newobject);
    }

    else if (prop_name.is_sameas("name"))
    {
        if (class_name.is_sameas("Project"))
            return;  // we don't use this (and neither does wxFormBuilder for that matter)
        else if (class_name.is_sameas("wxDialog"))
            newobject->prop_set_value(prop_class_name, xml_prop.text().as_cview());
    }
    else if (prop_name.is_sameas("permission"))
    {
        auto value = xml_prop.text().as_cview();
        if (value.is_sameas("protected") || value.is_sameas("private"))
            newobject->prop_set_value(prop_class_access, "protected:");
        else if (value.is_sameas("public"))
            newobject->prop_set_value(prop_class_access, "public:");
        else
            newobject->prop_set_value(prop_class_access, "none");
    }

    else if (prop_name.is_sameas("border"))
    {
        newobject->prop_set_value(prop_border_size, xml_prop.text().as_cview());
    }

    else if (prop_name.is_sameas("enabled"))
    {
        // Form builder will apply enabled to things like a ribbon tool which cannot be enabled/disabled
        auto disabled = newobject->get_prop_ptr(prop_disabled);
        if (disabled)
            disabled->set_value(xml_prop.text().as_bool() ? 0 : 1);
    }

    else if (prop_name.is_sameas("disabled"))
    {
        if (class_name.is_sameas("wxToggleButton") || class_name.is_sameas("wxButton"))
        {
            newobject->get_prop_ptr(prop_disabled_bmp)->set_value(xml_prop.text().as_cview());
        }
    }
    else if (prop_name.is_sameas("pressed"))
    {
        if (class_name.is_sameas("wxToggleButton") || class_name.is_sameas("wxButton"))
        {
            newobject->get_prop_ptr(prop_pressed_bmp)->set_value(xml_prop.text().as_cview());
        }
    }

    else if (prop_name.is_sameas("value"))
    {
        if (class_name.is_sameas("wxRadioButton"))
        {
            newobject->get_prop_ptr(prop_checked)->set_value(xml_prop.text().as_cview());
        }
        else if (class_name.is_sameas("wxSpinCtrl"))
        {
            newobject->get_prop_ptr(prop_initial)->set_value(xml_prop.text().as_cview());
        }
        else if (class_name.is_sameas("wxToggleButton"))
        {
            newobject->get_prop_ptr(prop_pressed)->set_value(xml_prop.text().as_cview());
        }
        else if (class_name.is_sameas("wxSlider") || class_name.is_sameas("wxGauge") || class_name.is_sameas("wxScrollBar"))
        {
            newobject->get_prop_ptr(prop_position)->set_value(xml_prop.text().as_cview());
        }
        else if (class_name.is_sameas("wxComboBox") || class_name.is_sameas("wxBitmapComboBox"))
        {
            newobject->get_prop_ptr(prop_selection_string)->set_value(xml_prop.text().as_cview());
        }
        else if (class_name.is_sameas("wxFilePickerCtrl") || class_name.is_sameas("wxDirPickerCtrl"))
        {
            newobject->get_prop_ptr(prop_initial_path)->set_value(xml_prop.text().as_cview());
        }
        else if (class_name.is_sameas("wxFontPickerCtrl"))
        {
            newobject->get_prop_ptr(prop_initial_font)->set_value(xml_prop.text().as_cview());
        }
        else
        {
            auto prop = newobject->get_prop_ptr(prop_value);
            if (prop)
                prop->set_value(xml_prop.text().as_cview());
        }
    }
    else if (prop_name.is_sameas("flags") && class_name.is_sameas("wxWrapSizer"))
    {
        auto prop = newobject->get_prop_ptr(prop_wrap_flags);
        auto prop_value = xml_prop.text().as_cstr();
        if (prop_value.contains("wxWRAPSIZER_DEFAULT_FLAGS"))
            prop_value = "wxEXTEND_LAST_ON_EACH_LINE|wxREMOVE_LEADING_SPACES";
        prop->set_value(prop_value);
    }
    else if (prop_name.is_sameas("selection") && (class_name.is_sameas("wxComboBox") || class_name.is_sameas("wxChoice") ||
                                                  class_name.is_sameas("wxBitmapComboBox")))
    {
        newobject->get_prop_ptr(prop_selection_int)->set_value(xml_prop.text().as_cview());
    }
    else if (prop_name.is_sameas("style") && class_name.is_sameas("wxCheckBox"))
    {
        // wxCHK_2STATE and wxCHK_3STATE are part of the type property instead of style
        ttlib::multiview styles(xml_prop.text().as_string());
        ttlib::cstr new_style;
        for (auto& iter: styles)
        {
            if (iter.is_sameas("wxCHK_2STATE"))
                return;  // this is default, so ignore it
            else if (iter.is_sameas("wxCHK_3STATE"))
            {
                newobject->get_prop_ptr(prop_type)->set_value("wxCHK_3STATE");
            }
            else
            {
                if (new_style.size())
                    new_style += "|";
                new_style += iter;
            }
        }

        if (new_style.size())
        {
            auto prop = newobject->get_prop_ptr(prop_style);
            prop->set_value("new_style");
        }
    }
    else if (prop_name.is_sameas("style") && class_name.is_sameas("wxToolBar"))
    {
        auto prop = newobject->get_prop_ptr(prop_style);
        auto prop_value = xml_prop.text().as_cstr();
        prop_value.Replace("wxTB_DEFAULT_STYLE", "wxTB_HORIZONTAL");
        prop->set_value(prop_value);
    }
    else if (prop_name.is_sameas("orient"))
    {
        if (auto prop = newobject->get_prop_ptr(prop_orientation); prop)
        {
            prop->set_value(xml_prop.text().as_string());
        }
    }
    else if (prop_name.is_sameas("hidden") && newobject->isGen(gen_wxDialog))
    {
        return;
    }
    else if (prop_name.is_sameas("subclass"))
    {
        ttlib::multistr parts(xml_prop.text().as_string(), ';', tt::TRIM::both);
        if (parts[0].empty())
            return;
        if (auto prop = newobject->get_prop_ptr(prop_derived_class); prop)
        {
            prop->set_value(parts[0]);
            if (parts.size() > 0 && !parts[1].contains("forward_declare"))
            {
                prop = newobject->get_prop_ptr(prop_derived_header);
                if (prop)
                {
                    prop->set_value(parts[1]);
                }
            }
        }
    }
    else if (prop_name.is_sameas("folding"))
    {
        if (xml_prop.text().as_bool())
        {
            newobject->prop_set_value(prop_fold_margin, "1");
            newobject->prop_set_value(prop_fold_width, "16");
        }
    }
    else if (prop_name.is_sameas("line_numbers"))
    {
        if (xml_prop.text().as_bool())
        {
            newobject->prop_set_value(prop_line_margin, "1");
        }
    }

    else
    {
        if (xml_prop.text().as_cview().size())
        {
            if (prop_name.is_sameas("hidden") && newobject->isGen(gen_ribbonTool))
                return;
            else if (xml_prop.text().as_cview().is_sameas("wxWS_EX_VALIDATE_RECURSIVELY"))
                return;
            MSG_INFO(ttlib::cstr() << prop_name << "(" << xml_prop.text().as_string() << ") property in " << class_name
                                   << " class not supported");
        }
    }
}

void FormBuilder::BitmapProperty(pugi::xml_node& xml_prop, NodeProperty* prop)
{
    auto org_value = xml_prop.text().as_cstr();
    if (org_value.contains("Load From File") || org_value.contains("Load From Embedded File"))
    {
        auto pos_semi = org_value.find(';');
        if (!ttlib::is_found(pos_semi))
            return;
        ttlib::cstr filename;
        if (org_value.subview(pos_semi).is_sameas("; Load From File"))
        {
            // Older version of wxFB placed the filename first
            org_value.erase(pos_semi);
            filename = org_value;
        }
        else
        {
            filename = org_value.subview(org_value.stepover(pos_semi));
        }
        if (filename.empty())
            return;

        if (filename.has_extension(".xpm"))
        {
            ttlib::cstr value("XPM; ");
            value << filename << ";[-1,-1]";
            prop->set_value(value);
        }
        else
        {
            ttlib::cstr bitmap("Embed;");
            ttString relative(filename.wx_str());
            relative.make_relative_wx(wxGetCwd());
            relative.backslashestoforward();
            bitmap << relative.wx_str();
            bitmap << ";[-1,-1]";
            prop->set_value(bitmap);
        }
    }
    else if (org_value.contains("Load From Art") && !xml_prop.text().as_cview().is_sameprefix("Load From Art Provider; ;"))
    {
        ttlib::cstr value(xml_prop.text().as_cview());
        value.Replace("Load From Art Provider; ", "Art;", false, tt::CASE::either);
        value.Replace("; ", "|", false, tt::CASE::either);
        if (value.back() == '|')
        {
            value << "wxART_OTHER";
        }
        value << ";[-1,-1]";
        prop->set_value(value);
    }
}

inline bool is_printable(unsigned char ch)
{
    return (ch > 31 && ch < 128);
}

inline bool is_numeric(unsigned char ch)
{
    return (ch >= '0' && ch <= '9');
}

void FormBuilder::ConvertNameSpaceProp(NodeProperty* prop, ttlib::cview org_names)
{
    if (org_names.empty())
        return;

    ttlib::cstr names;
    size_t offset = 0;
    while (offset < org_names.size())
    {
        ttlib::cstr temp;
        offset = temp.ExtractSubString(org_names, offset) + 1;
        if (temp.empty())
            break;
        if (names.size())
            names << "::";
        names << temp;
    }

    prop->set_value(names);
}
