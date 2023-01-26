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

#include "import_formblder.h"

#include "base_generator.h"  // BaseGenerator -- Base widget generator class
#include "font_prop.h"       // FontProperty class
#include "mainframe.h"       // Main window frame
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
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

bool FormBuilder::Import(const tt_wxString& filename, bool write_doc)
{
    auto result = LoadDocFile(filename);
    if (!result)
    {
        return false;
    }
    auto root = result.value().first_child();

    if (!tt::is_sameas(root.name(), "wxFormBuilder_Project", tt::CASE::either))
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

        m_project = NodeCreation.CreateNode(gen_Project, nullptr);

        CreateProjectNode(object, m_project.get());

        if (write_doc)
            m_project->CreateDoc(m_docOut);
    }

    catch (const std::exception& TESTING_PARAM(e))
    {
        MSG_ERROR(e.what());
        wxMessageBox(wxString("This wxFormBuilder project file is invalid and cannot be loaded: ") << filename,
                     "Import wxFormBuilder project");
        return false;
    }

    if (m_errors.size())
    {
        tt_string errMsg("Not everything in the wxFormBuilder project could be converted:\n\n");
        MSG_ERROR(tt_string() << "------  " << m_importProjectFile.filename().wx_str() << "------");
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

                if (prop_name.as_string() == "internationalize")
                {
                    new_node->get_prop_ptr(prop_internationalize)->set_value(xml_prop.text().as_bool() ? "1" : "0");
                }
                else if (prop_name.as_string() == "help_provider")
                {
                    new_node->get_prop_ptr(prop_help_provider)->set_value(xml_prop.text().as_string());
                }
                else if (prop_name.as_string() == "precompiled_header")
                {
                    // wxFormBuilder calls it a precompiled header, but uses it as a preamble.
                    new_node->get_prop_ptr(prop_src_preamble)->set_value(xml_prop.text().as_string());
                }

                else if (prop_name.as_string() == "embedded_files_path")
                {
                    // Unlike wxString, this ctor will call FromUTF8() on Windows
                    tt_wxString path(xml_prop.text().as_string());
                    tt_wxString root(m_importProjectFile);
                    root.remove_filename();
                    path.make_relative_wx(root);
                    m_project->prop_set_value(prop_art_directory, path);
                }
                else if (prop_name.as_string() == "path")
                {
                    tt_wxString path(xml_prop.text().as_string());
                    tt_wxString root(m_importProjectFile);
                    root.remove_filename();
                    path.make_relative_wx(root);
                    m_project->prop_set_value(prop_base_directory, path);
                }
                else if (prop_name.as_string() == "file")
                {
                    m_baseFile = xml_prop.text().as_string();
                }
                else if (prop_name.as_string() == "class_decoration")
                {
                    m_class_decoration = xml_prop.text().as_string();
                    // Current formbuild uses "; " as the default property value
                    if (m_class_decoration.starts_with((";")))
                    {
                        m_class_decoration.clear();
                    }
                }
                else if (prop_name.as_string() == "namespace" && xml_prop.text().as_string().size())
                {
                    ConvertNameSpaceProp(new_node->get_prop_ptr(prop_name_space), xml_prop.text().as_string());
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
    auto class_name = xml_obj.attribute("class").as_sview();
    if (class_name.empty())
        return NodeSharedPtr();

    auto gen_name = MapClassName(xml_obj.attribute("class").value());
    if (gen_name == gen_unknown)
    {
        if (class_name.contains("bookpage"))
        {
            gen_name = gen_oldbookpage;
        }
        else if (class_name.starts_with("ribbon"))
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
            m_errors.emplace(tt_string() << class_name << " is not supported in wxUiEditor");
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
            if (iter.attribute("name").as_string() == "style")
            {
                if (iter.text().as_sview().contains("wxCHK_3STATE"))
                    gen_name = gen_Check3State;
                break;
            }
        }
    }
    else if (gen_name == gen_tool && parent->isGen(gen_wxAuiToolBar))
    {
        gen_name = gen_auitool;
    }

    auto newobject = NodeCreation.CreateNode(gen_name, parent);
    if (!newobject)
    {
        tt_string msg("Unable to create ");
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
        if (auto prop_name = xml_prop.attribute("name").as_string(); prop_name.size())
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
                            prop_ptr->set_value(xml_prop.text().as_string());
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
                        tt_string animation("Embed;");
                        animation << xml_prop.text().as_string() << ";[-1,-1]";
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

            if (prop_name == "name")
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
                    prop_ptr->set_value(xml_prop.text().as_string());
                }
                continue;
            }
            else if (prop_name == "declaration")
            {
                // This property is for a custom control, and we don't use this specific property
                continue;
            }
            else if (prop_name == "construction")
            {
                tt_string copy = xml_prop.text().as_string();
                if (auto pos = copy.find('('); tt::is_found(pos))
                {
                    copy.erase(0, pos);
                }
                if (auto pos = copy.find(';'); tt::is_found(pos))
                {
                    copy.erase_from(';');
                }

                newobject->prop_set_value(prop_parameters, copy);
                continue;
            }
            else if (prop_name == "settings")
            {
                newobject->prop_set_value(prop_settings_code, xml_prop.text().as_string());
            }
            else if (prop_name == "include")
            {
                tt_string header;
                header.ExtractSubString(xml_prop.text().as_sview().view_stepover());
                if (header.size())
                {
                    newobject->prop_set_value(prop_header, header);
                }
                continue;
            }

            // If the property actually has a value, then we need to see if we can convert it. We ignore unknown
            // properties that don't have a value.
            if (auto value = xml_prop.text().as_string(); value.size())
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
        if (auto event_name = xml_event.attribute("name").as_std_str();
            event_name.size() && xml_event.text().as_string().size())
        {
            if (auto result = m_mapEventNames.find(event_name); result != m_mapEventNames.end())
            {
                event_name = result->second;
                if (event_name == "wxEVT_MENU" && newobject->isGen(gen_tool))
                    event_name = "wxEVT_TOOL";
            }
            else
            {
                // There is nothing in the wxWidgets source code that actually generates wxEVT_HIBERNATE that I can find as
                // of wxWidgets 3.1.3. It was removed in wxFormBuilder (but I couldn't find a reason as to why).
                // Documentation states it's part of WinCE which we don't support.
                if (event_name == "OnHibernate")
                {
                    xml_event = xml_event.next_sibling("event");
                    continue;
                }

                // REVIEW: [KeyWorks - 10-28-2020] We don't support this, but we could convert it if it's actually used.
                else if (event_name == "OnMouseEvents")
                {
                    xml_event = xml_event.next_sibling("event");
                    continue;
                }

                MSG_INFO(tt_string() << event_name << " event not supported");
                xml_event = xml_event.next_sibling("event");
                continue;
            }

            if (auto event = newobject->GetEvent(event_name); event)
            {
                event->set_value(xml_event.text().as_string());
            }
        }

        xml_event = xml_event.next_sibling("event");
    }

    auto child = xml_obj.child("object");
    if (NodeCreation.IsOldHostType(newobject->DeclName()))
    {
        newobject = CreateFbpNode(child, parent, newobject.get());
        if (!newobject)
        {
            return newobject;
        }

        if (newobject->isGen(gen_wxStdDialogButtonSizer) && parent)
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

void FormBuilder::ProcessPropValue(pugi::xml_node& xml_prop, tt_string_view prop_name, tt_string_view class_name,
                                   Node* newobject)
{
    if (g_setIgnoreProps.contains(std::string(prop_name)))
    {
        return;
    }

    // validator_style sets the wxFILTER flags and is only valid in a wxTextValidator, so it's
    // removed from widgets that can't use it.
    else if (prop_name == "validator_style")
    {
        return;
    }

    // validator_type is only valid in wxTextValidator, where it let's the user choose between wxTextValidator and
    // wxGenericValidator
    else if (prop_name == "validator_type")
    {
        // BUGBUG: [KeyWorks - 11-07-2020] This IS valid for text controls, so we need to process it
        return;
    }

    // This will be caused by a spacer item which isn't actually a widget, so has no access property
    else if (prop_name == map_PropNames[prop_class_access])
    {
        return;
    }

    // The label property in a wxMenuBar is not supported (since it can't actually be used)
    else if (prop_name == map_PropNames[prop_label])
    {
        return;
    }

    // This is most likely a Dialog class -- we don't support wxAUI in that class. We silently ignore it...
    else if (prop_name == "aui_managed" || prop_name == "aui_manager_style")
    {
        return;
    }

    else if (prop_name == "flag" && (class_name == "sizeritem" || class_name == "gbsizeritem"))
    {
        HandleSizerItemProperty(xml_prop, newobject);
    }

    else if (prop_name == "name")
    {
        if (class_name == "Project")
            return;  // we don't use this (and neither does wxFormBuilder for that matter)
        else if (class_name == "wxDialog")
            newobject->prop_set_value(prop_class_name, xml_prop.text().as_string());
    }
    else if (prop_name == "permission")
    {
        auto value = xml_prop.text().as_string();
        if (value == "protected" || value == "private")
            newobject->prop_set_value(prop_class_access, "protected:");
        else if (value == "public")
            newobject->prop_set_value(prop_class_access, "public:");
        else
            newobject->prop_set_value(prop_class_access, "none");
    }

    else if (prop_name == "border")
    {
        newobject->prop_set_value(prop_border_size, xml_prop.text().as_string());
    }

    else if (prop_name == "enabled")
    {
        // Form builder will apply enabled to things like a ribbon tool which cannot be enabled/disabled
        auto disabled = newobject->get_prop_ptr(prop_disabled);
        if (disabled)
            disabled->set_value(xml_prop.text().as_bool() ? 0 : 1);
    }

    else if (prop_name == "disabled")
    {
        if (class_name == "wxToggleButton" || class_name == "wxButton")
        {
            newobject->get_prop_ptr(prop_disabled_bmp)->set_value(xml_prop.text().as_string());
        }
    }
    else if (prop_name == "pressed")
    {
        if (class_name == "wxToggleButton" || class_name == "wxButton")
        {
            newobject->get_prop_ptr(prop_pressed_bmp)->set_value(xml_prop.text().as_string());
        }
    }

    else if (prop_name == "value")
    {
        if (class_name == "wxRadioButton")
        {
            newobject->get_prop_ptr(prop_checked)->set_value(xml_prop.text().as_string());
        }
        else if (class_name == "wxSpinCtrl")
        {
            newobject->get_prop_ptr(prop_initial)->set_value(xml_prop.text().as_string());
        }
        else if (class_name == "wxToggleButton")
        {
            newobject->get_prop_ptr(prop_pressed)->set_value(xml_prop.text().as_string());
        }
        else if (class_name == "wxSlider" || class_name == "wxGauge" || class_name == "wxScrollBar")
        {
            newobject->get_prop_ptr(prop_position)->set_value(xml_prop.text().as_string());
        }
        else if (class_name == "wxComboBox" || class_name == "wxBitmapComboBox")
        {
            newobject->get_prop_ptr(prop_selection_string)->set_value(xml_prop.text().as_string());
        }
        else if (class_name == "wxFilePickerCtrl" || class_name == "wxDirPickerCtrl")
        {
            newobject->get_prop_ptr(prop_initial_path)->set_value(xml_prop.text().as_string());
        }
        else if (class_name == "wxFontPickerCtrl")
        {
            newobject->get_prop_ptr(prop_initial_font)->set_value(xml_prop.text().as_string());
        }
        else
        {
            auto prop = newobject->get_prop_ptr(prop_value);
            if (prop)
                prop->set_value(xml_prop.text().as_string());
        }
    }
    else if (prop_name == "flags" && class_name == "wxWrapSizer")
    {
        auto prop = newobject->get_prop_ptr(prop_wrap_flags);
        auto prop_value = xml_prop.text().as_sview();
        if (prop_value.contains("wxWRAPSIZER_DEFAULT_FLAGS"))
            prop_value = "wxEXTEND_LAST_ON_EACH_LINE|wxREMOVE_LEADING_SPACES";
        prop->set_value(prop_value);
    }
    else if (prop_name == "selection" &&
             (class_name == "wxComboBox" || class_name == "wxChoice" || class_name == "wxBitmapComboBox"))
    {
        newobject->get_prop_ptr(prop_selection_int)->set_value(xml_prop.text().as_string());
    }
    else if (prop_name == "style" && class_name == "wxCheckBox")
    {
        // wxCHK_2STATE and wxCHK_3STATE are part of the type property instead of style
        tt_view_vector styles(xml_prop.text().as_string());
        tt_string new_style;
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
    else if (prop_name == "style" && class_name == "wxToolBar")
    {
        auto prop = newobject->get_prop_ptr(prop_style);
        auto prop_value = xml_prop.text().as_cstr();
        prop_value.Replace("wxTB_DEFAULT_STYLE", "wxTB_HORIZONTAL");
        prop->set_value(prop_value);
    }
    else if (prop_name == "orient")
    {
        if (auto prop = newobject->get_prop_ptr(prop_orientation); prop)
        {
            prop->set_value(xml_prop.text().as_string());
        }
    }
    else if (prop_name == "hidden" && newobject->isGen(gen_wxDialog))
    {
        return;
    }
    else if (prop_name == "subclass")
    {
        tt_string_vector parts(xml_prop.text().as_string(), ';', tt::TRIM::both);
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
    else if (prop_name == "folding")
    {
        if (xml_prop.text().as_bool())
        {
            newobject->prop_set_value(prop_fold_margin, "1");
            newobject->prop_set_value(prop_fold_width, "16");
        }
    }
    else if (prop_name == "line_numbers")
    {
        if (xml_prop.text().as_bool())
        {
            newobject->prop_set_value(prop_line_margin, "1");
        }
    }

    else
    {
        if (xml_prop.text().as_string().size())
        {
            if (prop_name == "hidden" && newobject->isGen(gen_ribbonTool))
                return;
            else if (xml_prop.text().as_string() == "wxWS_EX_VALIDATE_RECURSIVELY")
                return;
            MSG_INFO(tt_string() << prop_name << "(" << xml_prop.text().as_string() << ") property in " << class_name
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
        if (!tt::is_found(pos_semi))
            return;
        tt_string filename;
        if (org_value.subview(pos_semi) == "; Load From File")
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
            tt_string value("XPM; ");
            value << filename << ";[-1,-1]";
            prop->set_value(value);
        }
        else
        {
            tt_string bitmap("Embed;");
            tt_wxString relative(filename.wx_str());
            relative.make_relative_wx(wxGetCwd());
            relative.backslashestoforward();
            bitmap << relative.wx_str();
            bitmap << ";[-1,-1]";
            prop->set_value(bitmap);
        }
    }
    else if (org_value.contains("Load From Art") && xml_prop.text().as_string() != "Load From Art Provider; ;")
    {
        tt_string value = xml_prop.text().as_std_str();
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

void FormBuilder::ConvertNameSpaceProp(NodeProperty* prop, std::string_view org_names)
{
    if (org_names.empty())
        return;

    tt_string names;
    size_t offset = 0;
    while (offset < org_names.size())
    {
        tt_string temp;
        offset = temp.ExtractSubString(org_names, offset) + 1;
        if (temp.empty())
            break;
        if (names.size())
            names << "::";
        names << temp;
    }

    prop->set_value(names);
}
