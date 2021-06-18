/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxFormBuider project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <fstream>
#include <iostream>
#include <set>

#include <wx/mstream.h>  // Memory stream classes

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "import_formblder.h"

#include "mainapp.h"       // App -- Main application class
#include "mainframe.h"     // Main window frame
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator class
#include "pjtsettings.h"   // ProjectSettings -- Hold data for currently loaded project
#include "uifuncs.h"       // Miscellaneous functions for displaying UI

#include "import_arrays.cpp"  // Array of formbuilder/wxuieditor event name pairs

// clang-format off
constexpr const IMPORT_NAME_PAIR prop_pair[] = {

    { "bg", "background_colour" },
    { "fg", "background_colour" },
    { "bitmapsize", "image_size" },
    { "permission", "class_access" },
    { "hover", "current" },

    { nullptr, nullptr },
};

const auto g_lstIgnoreProps = {

    "xrc_skip_sizer",   // used for XRC code generation which we don't support
    "class_decoration", // used for adding a DLL export macro

    "event_handler",  // all events are now declared as virtual

    // The following are AUI properties. Until AUI gets re-implemented, these will all be ignored

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
    "use_enum",

};
// clang-format on

// The following gets initialized once if FormBuilder is constructed. They do not get destroyed since they are a) quite
// small, and b) might be used again. This is in contrast to m_mapEventNames which is part of the FormBuilder class.

std::unordered_map<std::string, const char*> g_PropMap;
std::unordered_set<std::string> g_setIgnoreProps;

FormBuilder::FormBuilder()
{
    for (size_t pos = 0; evt_pair[pos].wxfb_name; ++pos)
    {
        m_mapEventNames[evt_pair[pos].wxfb_name] = evt_pair[pos].wxui_name;
    }

    if (g_PropMap.empty())
    {
        for (size_t pos = 0; prop_pair[pos].wxfb_name; ++pos)
        {
            g_PropMap[prop_pair[pos].wxfb_name] = prop_pair[pos].wxui_name;
        }
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
        appMsgBox(filename.wx_str() + _ttc(" is not a wxFormBuilder file"), _tt(strIdImportFormBuilder));
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
        appMsgBox(ttlib::cstr("This wxFormBuilder project file is invalid and cannot be loaded: ") << filename.wx_str(),
                  "Import wxFormBuilder project");
        return false;
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
                    m_embedPath = xml_prop.text().as_string();
                }
                else if (prop_name.as_cview().is_sameas("file"))
                {
                    m_baseFile = xml_prop.text().as_string();
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

    if (class_name.is_sameas("wxScintilla"))
    {
        class_name = "wxStyledTextCtrl";
    }
    else if (class_name.is_sameas("wxListCtrl"))
    {
        class_name = "wxListView";
    }
    else if (class_name.is_sameas("wxBitmapButton"))
    {
        class_name = "wxButton";
    }
    else if (class_name.is_sameas("wxPanel"))
    {
        if (!parent)
        {
            auto owner = wxGetFrame().GetSelectedNode();
            while (owner->gen_type() == type_sizer)
                owner = owner->GetParent();
            if (owner->DeclName().contains("book"))
            {
                class_name = "BookPage";
            }
        }
        else if (parent->DeclName().contains("book"))
        {
            class_name = "BookPage";
        }
    }
    else if (class_name.contains("bookpage"))
    {
        class_name = "oldbookpage";
    }
    else if (class_name.is_sameas("Dialog"))
    {
        class_name = "wxDialog";
    }
    else if (class_name.is_sameas("Wizard"))
    {
        class_name = "wxWizard";
    }
    else if (class_name.is_sameas("Frame"))
    {
        class_name = "wxFrame";
    }
    else if (class_name.is_sameas("Panel"))
    {
        class_name = "PanelForm";
    }

    if (class_name.is_sameas("wxCheckBox"))
    {
        for (auto& iter: xml_obj.children())
        {
            if (iter.attribute("name").as_cview().is_sameas("style"))
            {
                if (iter.text().as_cview().contains("wxCHK_3STATE"))
                    class_name = "Check3State";
                break;
            }
        }
    }

    auto newobject = g_NodeCreator.CreateNode(class_name, parent);
    if (!newobject)
    {
        FAIL_MSG(ttlib::cstr() << "Unable to create " << class_name);
        throw std::runtime_error("Invalid project file -- object could not be created!");
    }

    auto xml_prop = xml_obj.child("property");
    while (xml_prop)
    {
        if (auto prop_name = xml_prop.attribute("name").as_cview(); prop_name.size())
        {
            NodeProperty* prop = nullptr;
            if (auto result = rmap_PropNames.find(prop_name.c_str()); result != rmap_PropNames.end())
            {
                prop = newobject->get_prop_ptr(result->second);
            }
            if (prop)
            {
                if (prop->isProp(prop_bitmap))
                {
                    if (!xml_prop.text().empty())
                        BitmapProperty(xml_prop, prop);
                }
                else
                {
                    // wxFormBuilder uses older style names from wxWidgets 2.x. Rename them to the 3.x names, and remove
                    // the ones that are no longer used.
                    auto value = xml_prop.text().as_cstr();
                    if (value.is_sameas("wxST_SIZEGRIP"))
                        value = "wxSTB_SIZEGRIP";
                    else if (value.contains("wxTE_CENTRE"))
                        value.Replace("wxTE_CENTRE", "wxTE_CENTER");
                    else if (value.contains("wxSIMPLE_BORDER"))
                        value.Replace("wxSIMPLE_BORDER", "wxBORDER_SIMPLE");
                    else if (value.contains("wxSUNKEN_BORDER"))
                        value.Replace("wxSUNKEN_BORDER", "wxBORDER_SUNKEN");
                    else if (value.contains("wxRAISED_BORDER"))
                        value.Replace("wxRAISED_BORDER", "wxBORDER_RAISED");
                    else if (value.contains("wxSTATIC_BORDER"))
                        value.Replace("wxSTATIC_BORDER", "wxBORDER_STATIC");
                    else if (value.contains("wxNO_BORDER"))
                        value.Replace("wxNO_BORDER", "wxBORDER_NONE");
                    else if (value.contains("wxNO_BORDER"))
                        value.Replace("wxNO_BORDER", "wxBORDER_NONE");
                    else if (value.contains("wxDOUBLE_BORDER"))
                        value.Replace("wxDOUBLE_BORDER", "");  // this style is obsolete
                    else if (value.contains("wxBU_AUTODRAW"))
                        value.Replace("wxBU_AUTODRAW", "");  // this style is obsolete
                    else if (value.contains("wxRA_USE_CHECKBOX"))
                        value.Replace("wxRA_USE_CHECKBOX", "");  // this style is obsolete
                    else if (value.contains("wxRB_USE_CHECKBOX"))
                        value.Replace("wxRB_USE_CHECKBOX", "");  // this style is obsolete
                    else if (value.contains("wxNB_FLAT"))
                        value.Replace("wxNB_FLAT", "");  // this style is obsolete

                    if (prop->isProp(prop_style))
                    {
                        ProcessStyle(xml_prop, newobject.get(), prop);
                    }
                    else
                    {
                        prop->set_value(value);
                    }
                }
                xml_prop = xml_prop.next_sibling("property");
                continue;
            }

            if (prop_name.is_sameas("bitmapsize"))
            {
                if (class_name.contains("book"))
                {
                    if (prop = newobject->get_prop_ptr(prop_image_size); prop)
                    {
                        prop->set_value(xml_prop.text().as_cview());
                        auto size = prop->as_size();
                        if (size.x != -1 || size.y != -1)
                        {
                            if (prop = newobject->get_prop_ptr(prop_display_images); prop)
                            {
                                prop->set_value(true);
                            }
                        }
                        xml_prop = xml_prop.next_sibling("property");
                        continue;
                    }
                }
            }

            if (prop_name.is_sameas("name"))
            {
                if (newobject->IsForm())
                {
                    prop = newobject->get_prop_ptr(prop_class_name);
                }
                else
                {
                    prop = newobject->get_prop_ptr(prop_var_name);
                }

                prop->set_value(xml_prop.text().as_cview());
                xml_prop = xml_prop.next_sibling("property");
                continue;
            }

            // We get here if the object doesn't have a property with the same name as the wxFormBuilder version.

            if (auto result = g_PropMap.find(prop_name.c_str()); result != g_PropMap.end())
            {
                // Some properties do the same thing but have a different name in wxUiEditor, so we just need to
                // change the name.
                prop_name = result->second;
                if (auto find_prop = rmap_PropNames.find(prop_name.c_str()); find_prop != rmap_PropNames.end())
                {
                    prop = newobject->get_prop_ptr(find_prop->second);
                    prop->set_value(xml_prop.text().as_cview());
                    xml_prop = xml_prop.next_sibling("property");
                    continue;
                }
            }

            // If the property actually has a value, then we need to see if we can convert it. We ignore unknown
            // properties that don't have a value.
            if (auto value = xml_prop.text().as_cview(); value.size())
            {
                ProcessPropValue(xml_prop, prop_name, class_name, newobject.get());
            }
        }
        xml_prop = xml_prop.next_sibling("property");
    }

    auto xml_event = xml_obj.child("event");
    while (xml_event)
    {
        if (auto event_name = xml_event.attribute("name").as_cview(); event_name.size())
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
        if (newobject->isGen(gen_wxStdDialogButtonSizer))
            newobject->get_prop_ptr(prop_static_line)->set_value(false);
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
    else if (prop_name.is_sameas(txt_class_access))
    {
        return;
    }

    // The label property in a wxMenuBar is not supported (since it can't actually be used)
    else if (prop_name.is_sameas(txt_label))
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
        ttlib::multistr styles(xml_prop.text().as_string());
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

    else
    {
        MSG_INFO(ttlib::cstr() << prop_name << " property in " << class_name << " class not supported");
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
            value << filename << "; ; [-1; -1]";
            prop->set_value(value);
        }
        else
        {
            {
                ttSaveCwd saveCwd;
                ttString newDir(m_importProjectFile);
                newDir.remove_filename();
                newDir.ChangeDir();

                if (!filename.file_exists())
                {
                    ttlib::cstr tmp_filename = filename;
                    tmp_filename.make_absolute();
                    if (tmp_filename.file_exists())
                    {
                        filename = tmp_filename;
                    }
                    else
                    {
                        if (m_eventGeneration.size())
                        {
                            tmp_filename = m_eventGeneration;
                            tmp_filename.append_filename(filename);
                            if (tmp_filename.file_exists())
                            {
                                filename = tmp_filename;
                            }
                        }
                    }
                }

                // It needs to be absolute to the current directory since we're about to switch back to the previous
                // directory
                filename.make_absolute();
            }

            ttlib::cstr value("XPM; ; ");
            value << filename << "; [-1; -1]";
            prop->set_value(value);
        }
    }
    else if (org_value.contains("Load From Art"))
    {
        ttlib::cstr value(xml_prop.text().as_cview());
        value.Replace("Load From Art Provider", "Art", false, tt::CASE::either);
        value << "; [-1; -1]";
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

void FormBuilder::ConvertSizerProperties(pugi::xml_node& xml_prop, Node* object, Node* parent, NodeProperty* prop)
{
    auto flag_value = xml_prop.text().as_cview();

    ttlib::cstr border_value;
    if (flag_value.contains("wxALL"))
        border_value = "wxALL";
    else
    {
        if (flag_value.contains("wxLEFT"))
        {
            if (border_value.size())
                border_value << '|';
            border_value << "wxLEFT";
        }
        if (flag_value.contains("wxRIGHT"))
        {
            if (border_value.size())
                border_value << '|';
            border_value << "wxRIGHT";
        }
        if (flag_value.contains("wxTOP"))
        {
            if (border_value.size())
                border_value << '|';
            border_value << "wxTOP";
        }
        if (flag_value.contains("wxBOTTOM"))
        {
            if (border_value.size())
                border_value << '|';
            border_value << "wxBOTTOM";
        }
    }
    if (border_value.size())
    {
        object->prop_set_value(prop_borders, border_value);
    }

    ttlib::cstr align_value;
    if (flag_value.contains("wxALIGN_LEFT"))
    {
        align_value << "wxALIGN_LEFT";
    }
    if (flag_value.contains("wxALIGN_TOP"))
    {
        if (align_value.size())
            align_value << '|';
        align_value << "wxALIGN_TOP";
    }
    if (flag_value.contains("wxALIGN_RIGHT"))
    {
        if (align_value.size())
            align_value << '|';
        align_value << "wxALIGN_RIGHT";
    }
    if (flag_value.contains("wxALIGN_BOTTOM"))
    {
        if (align_value.size())
            align_value << '|';
        align_value << "wxALIGN_BOTTOM";
    }

    if (parent && (flag_value.contains("wxALIGN_CENTER") || flag_value.contains("wxALIGN_CENTRE")))
    {
        // wxFormBuilder allows the user to add alignment flags that conflict with a
        // parent's orientation flags. We check for that here, and only add the flag if it
        // is valid.
        bool isIgnored = false;
        if (flag_value.contains("wxALIGN_CENTER_VERTICAL") || flag_value.contains("wxALIGN_CENTRE_VERTICAL"))
        {
            if (!parent->IsSizer() || !parent->prop_as_string(prop_orientation).is_sameas("wxVERTICAL"))
            {
                if (align_value.size())
                    align_value << '|';
                align_value << "wxALIGN_CENTER_VERTICAL";
            }
            else
                isIgnored = true;
        }
        else if (flag_value.contains("wxALIGN_CENTER_HORIZONTAL") || flag_value.contains("wxALIGN_CENTRE_HORIZONTAL"))
        {
            if (!parent->IsSizer() || !parent->prop_as_string(prop_orientation).is_sameas("wxHORIZONTAL"))
            {
                if (align_value.size())
                    align_value << '|';
                align_value << "wxALIGN_CENTER_HORIZONTAL";
            }
            else
                isIgnored = true;
        }

        // Because we use contains(), all we know is that a CENTER flag was used, but not which one.
        // If we get here and no CENTER flag has been added, then assume that "wxALIGN_CENTER" or
        // "wxALIGN_CENTRE" was specified.

        if (!isIgnored && !align_value.contains("wxALIGN_CENTER"))
        {
            if (align_value.size())
                align_value << '|';
            align_value << "wxALIGN_CENTER";
        }
    }
    if (align_value.size())
    {
        prop = object->get_prop_ptr(prop_alignment);
        prop->set_value(align_value);
    }

    ttlib::cstr flags_value;
    if (flag_value.contains("wxEXPAND"))
    {
        flags_value << "wxEXPAND";
    }
    if (flag_value.contains("wxSHAPED"))
    {
        if (flags_value.size())
            flags_value << '|';
        flags_value << "wxSHAPED";
    }
    if (flag_value.contains("wxFIXED_MINSIZE"))
    {
        if (flags_value.size())
            flags_value << '|';
        flags_value << "wxFIXED_MINSIZE";
    }
    if (flag_value.contains("wxRESERVE_SPACE_EVEN_IF_HIDDEN"))
    {
        if (flags_value.size())
            flags_value << '|';
        flags_value << "wxRESERVE_SPACE_EVEN_IF_HIDDEN";
    }
    if (flags_value.size())
    {
        prop = object->get_prop_ptr(prop_flags);
        prop->set_value(flags_value);
    }
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
