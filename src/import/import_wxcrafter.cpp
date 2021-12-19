/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxCrafter project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iostream>
#include <set>

#include <wx/mstream.h>  // Memory stream classes

#include "jsoncpp.h"     // json parser
#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "import_wxcrafter.h"

#include "base_generator.h"  // BaseGenerator -- Base widget generator class
#include "mainapp.h"         // App -- Main application class
#include "mainframe.h"       // Main window frame
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "pjtsettings.h"     // ProjectSettings -- Hold data for currently loaded project
#include "utils.h"           // Utility functions that work with properties

extern std::map<int, GenEnum::GenName> g_map_id_generator;

WxCrafter::WxCrafter() {}

bool WxCrafter::Import(const ttString& filename, bool write_doc)
{
    std::ifstream istrm(filename.wx_str(), std::ifstream::binary);
    if (!istrm.is_open())
    {
        wxMessageBox(wxString() << "Cannot open " << filename, "Import wxCrafter project");
        return false;
    }

    Json::Value json_doc;
    istrm >> json_doc;

    if (!json_doc.isObject())
    {
        wxMessageBox(wxString() << filename << " is not a valid wxCrafter file", "Import wxCrafter project");
        return false;
    }

    m_project = g_NodeCreator.CreateNode(gen_Project, nullptr);

    if (auto& metadata = json_doc["metadata"]; !metadata.isNull())
    {
        m_generate_ids = GetBoolValue(metadata, "m_useEnum", true);

        if (auto& include_files = metadata["m_includeFiles"]; !include_files.isNull())
        {
            if (include_files.isArray() && include_files.size() > 0)
            {
                // TODO: [KeyWorks - 12-16-2021]
            }
        }
    }

    if (auto& windows = json_doc["windows"]; !windows.isNull())
    {
        if (windows.isArray())
        {
            for (Json::ArrayIndex idx = 0; idx < windows.size(); ++idx)
            {
                auto& item = windows[idx];
                ProcessForm(item);
            }
        }

        if (write_doc)
            m_project->CreateDoc(m_docOut);
    }

    if (m_errors.size())
    {
        ttlib::cstr errMsg("Not everything in the wxCrafter project could be converted:\n\n");
        MSG_ERROR(ttlib::cstr() << "------  " << m_importProjectFile.filename().wx_str() << "------");
        for (auto& iter: m_errors)
        {
            MSG_ERROR(iter);
            errMsg << iter << '\n';
        }

        wxMessageBox(errMsg, "Import wxCrafter project");
    }

    return true;
}

void WxCrafter::ProcessForm(const Json::Value& form)
{
    ASSERT_MSG(form.isObject(), "Expected the form to be an object!");
    if (!form.isObject())
    {
        m_errors.emplace("Invalid wxCrafter file -- top level window is not a JSON object.");
        return;
    }

    auto& value = form["m_type"];
    if (!value.isNumeric())
    {
        m_errors.emplace("Invalid wxCrafter file -- top level window is missing a numeric m_type key to indicate what type "
                         "of window it is.");
        return;
    }

    auto gen_name = gen_unknown;
    gen_name = GetGenName(value);
    if (gen_name == gen_unknown)
    {
        m_errors.emplace("Unrecognized window type!");
        return;
    }

    auto new_node = g_NodeCreator.CreateNode(gen_name, m_project.get());
    m_project->Adopt(new_node);
    try
    {
        if (auto& children = form["m_children"]; children.isArray())
        {
            for (Json::Value::ArrayIndex idx = 0; idx < children.size(); ++idx)
            {
                if (!children[idx].isObject())
                {
                    m_errors.emplace(ttlib::cstr() << "Invalid wxCrafter file -- child of " << map_GenNames.at(gen_name)
                                                   << " is not a JSON object.");
                    continue;
                }

                ProcessChild(new_node.get(), children[idx]);
            }
        }
    }

    catch (const std::exception& e)
    {
        FAIL_MSG(e.what())
        MSG_ERROR(e.what());
        wxMessageBox(ttlib::cstr("Internal error: ") << e.what(), "Import wxCrafter project");
        return;
    }
}

void WxCrafter::ProcessChild(Node* parent, const Json::Value object)
{
    auto& value = object["m_type"];
    if (!value.isNumeric())
    {
        m_errors.emplace("Invalid wxCrafter file -- child is missing a numeric m_type key to indicate what type "
                         "of child it is.");
        return;
    }

    auto gen_name = gen_unknown;
    gen_name = GetGenName(value);
    if (gen_name == gen_unknown)
    {
        m_errors.emplace("Unrecognized child type!");
        return;
    }

    auto new_node = g_NodeCreator.CreateNode(gen_name, parent);
    if (!new_node)
    {
        m_errors.emplace(ttlib::cstr() << map_GenNames.at(gen_name) << " cannot be a child of " << parent->DeclName());
        return;
    }
    parent->Adopt(new_node);

    if (auto& array = object["m_sizerFlags"]; array.isArray())
        ProcessSizerFlags(new_node.get(), array);
    if (auto& array = object["m_properties"]; array.isArray())
        ProcessProperties(new_node.get(), array);

    if (auto& children = object["m_children"]; children.isArray())
    {
        for (Json::Value::ArrayIndex idx = 0; idx < children.size(); ++idx)
        {
            if (!children[idx].isObject())
            {
                m_errors.emplace(ttlib::cstr() << "Invalid wxCrafter file -- child of " << map_GenNames.at(gen_name)
                                               << " is not a JSON object.");
                continue;
            }

            ProcessChild(new_node.get(), children[idx]);
        }
    }
}

void WxCrafter::ProcessSizerFlags(Node* node, const Json::Value sizer_flags)
{
    try
    {
        std::set<std::string> all_flags;
        for (Json::Value::ArrayIndex idx = 0; idx < sizer_flags.size(); ++idx)
        {
            all_flags.insert(sizer_flags[idx].asCString());
        }

        // If the node has porp_alignment, then it will also have prop_borders and prop_flags
        if (node->HasProp(prop_alignment))
        {
            if (all_flags.find("wxEXPAND") != all_flags.end())
            {
                node->prop_set_value(prop_flags, "wxEXPAND");
            }
            else
            {
                auto alignment = node->prop_as_raw_ptr(prop_alignment);

                if (all_flags.find("wxALIGN_CENTER") != all_flags.end())
                {
                    if (alignment->size())
                        *alignment << '|';
                    *alignment << "wxALIGN_CENTER";
                }
                else if (all_flags.find("wxALIGN_CENTER_HORIZONTAL") != all_flags.end())
                {
                    if (alignment->size())
                        *alignment << '|';
                    *alignment << "wxALIGN_CENTER_HORIZONTAL";
                }
                else if (all_flags.find("wxALIGN_CENTER_VERTICAL") != all_flags.end())
                {
                    if (alignment->size())
                        *alignment << '|';
                    *alignment << "wxALIGN_CENTER_VERTICAL";
                }

                if (all_flags.find("wxALIGN_RIGHT") != all_flags.end())
                {
                    if (!alignment->contains("wxALIGN_CENTER"))
                    {
                        if (alignment->size())
                            *alignment << '|';
                        *alignment << "wxALIGN_RIGHT";
                    }
                }
                else if (all_flags.find("wxALIGN_LEFT") != all_flags.end())
                {
                    if (!alignment->contains("wxALIGN_CENTER"))
                    {
                        if (alignment->size())
                            *alignment << '|';
                        *alignment << "wxALIGN_LEFT";
                    }
                }
                else if (all_flags.find("wxALIGN_TOP") != all_flags.end())
                {
                    if (!alignment->contains("wxALIGN_CENTER"))
                    {
                        if (alignment->size())
                            *alignment << '|';
                        *alignment << "wxALIGN_TOP";
                    }
                }
                else if (all_flags.find("wxALIGN_BOTTOM") != all_flags.end())
                {
                    if (!alignment->contains("wxALIGN_CENTER"))
                    {
                        if (alignment->size())
                            *alignment << '|';
                        *alignment << "wxALIGN_BOTTOM";
                    }
                }
            }
        }

        if (node->HasProp(prop_border))
        {
            if (all_flags.find("wxALL") != all_flags.end())
            {
                node->prop_set_value(prop_border, "wxALL");
            }
            else
            {
                auto border_ptr = node->prop_as_raw_ptr(prop_border);
                border_ptr->clear();

                if (all_flags.find("wxLEFT") != all_flags.end())
                {
                    if (border_ptr->size())
                        *border_ptr << ',';
                    *border_ptr << "wxLEFT";
                }
                if (all_flags.find("wxRIGHT") != all_flags.end())
                {
                    if (border_ptr->size())
                        *border_ptr << ',';
                    *border_ptr << "wxRIGHT";
                }
                if (all_flags.find("wxTOP") != all_flags.end())
                {
                    if (border_ptr->size())
                        *border_ptr << ',';
                    *border_ptr << "wxTOP";
                }
                if (all_flags.find("wxBOTTOM") != all_flags.end())
                {
                    if (border_ptr->size())
                        *border_ptr << ',';
                    *border_ptr << "wxBOTTOM";
                }
            }
        }
    }
    catch (const std::exception& e)
    {
#if defined(_DEBUG)
        FAIL_MSG(e.what())
        MSG_ERROR(e.what());
#else
        wxMessageBox(ttlib::cstr("Internal error: ") << e.what(), "Import wxCrafter project");
#endif
    }
}

void WxCrafter::ProcessProperties(Node* node, const Json::Value properties)
{
    try
    {
        for (Json::Value::ArrayIndex idx = 0; idx < properties.size(); ++idx)
        {
            const auto& value = properties[idx];
            ttlib::cstr name;
            if (value["m_label"].isString())
            {
                name = value["m_label"].asCString();
                if (name.back() == ':')
                    name.pop_back();
                name.MakeLower();
                auto prop_name = FindProp(name);
                if (prop_name == prop_unknown)
                {
                    if (name.is_sameas("minimum size"))
                        prop_name = prop_min_size;
                    else if (name.is_sameas("name"))
                        prop_name = (node->IsForm() ? prop_class_name : prop_var_name);
                    else if (name.is_sameas("bg colour"))
                        prop_name = prop_background_colour;
                    else if (name.is_sameas("fg colour"))
                        prop_name = prop_foreground_colour;
                    else if (name.is_sameas("class name"))
                        prop_name = prop_derived_class;
                    else if (name.is_sameas("class name"))
                        prop_name = prop_derived_class;
                    else if (name.is_sameas("include file"))
                        prop_name = prop_derived_header;
                    else if (name.is_sameas("enable window persistency"))
                        prop_name = prop_persist;
                    else if (name.is_sameas("inherited class"))
                        prop_name = prop_class_name;
                    else if (name.is_sameas("file"))
                        prop_name = prop_base_file;

                    else if (name.is_sameas("centre"))
                    {
                        if (value["m_selection"].isNumeric())
                        {
                            switch (value["m_selection"].asInt())
                            {
                                case 0:
                                    node->prop_set_value(prop_center, "no");
                                    break;

                                case 1:
                                    node->prop_set_value(prop_center, "wxBOTH");
                                    break;

                                case 2:
                                    node->prop_set_value(prop_center, "wxVERTICAL");
                                    break;

                                case 3:
                                    node->prop_set_value(prop_center, "wxHORIZONTAL");
                                    break;

                                default:
                                    break;
                            }
                        }
                        continue;
                    }

                    else if (name.is_sameas("virtual folder"))
                        continue;  // this doesn't apply to wxUiEditor
                }

                if (prop_name == prop_background_colour || prop_name == prop_foreground_colour)
                {
                    if (auto& prop_value = value["colour"]; !prop_value.isNull())
                    {
                        ttlib::cstr color = prop_value.asString();
                        if (color.contains("Default"))
                            continue;
                        else if (color.at(0) == '(')
                        {
                            color.erase(0, 1);
                            color.pop_back();
                            node->prop_set_value(prop_name, color);
                        }
                    }
                }
                else if (prop_name == prop_id)
                {
                    if (auto& prop_value = value["m_winid"]; !prop_value.isNull())
                    {
                        node->prop_set_value(prop_name, prop_value.asString());
                    }
                }

                else if (prop_name != prop_unknown)
                {
                    if (auto& prop_value = value["m_value"]; !prop_value.isNull())
                    {
                        if (prop_value.isBool())
                            node->prop_set_value(prop_name, prop_value.asBool());
                        else
                            node->prop_set_value(prop_name, prop_value.asString());
                    }
                }
            }
        }
    }
    catch (const std::exception& e)
    {
#if defined(_DEBUG)
        FAIL_MSG(e.what())
        MSG_ERROR(e.what());
#else
        wxMessageBox(ttlib::cstr("Internal error: ") << e.what(), "Import wxCrafter project");
#endif
    }
}

bool WxCrafter::GetBoolValue(const Json::Value& value, ttlib::cview name, bool def_return)
{
    if (auto& result = value[name.c_str()]; !result.isNull())
    {
        return result.asBool();
    }
    else
    {
        return def_return;
    }
}

GenEnum::GenName WxCrafter::GetGenName(const Json::Value& value)
{
    if (auto result = g_map_id_generator.find(value.asInt()); result != g_map_id_generator.end())
        return result->second;
    else
        return gen_unknown;
}
