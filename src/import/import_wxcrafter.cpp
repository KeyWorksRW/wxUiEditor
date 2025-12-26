/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxCrafter project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <set>

#include <frozen/map.h>

#include <wx/mstream.h>  // Memory stream classes

#define RAPIDJSON_HAS_STDSTRING 1
#define RAPIDJSON_ASSERT(x)     ASSERT(x)

#include "import_wxcrafter.h"  // This will include rapidjson/document.h

#include "base_generator.h"    // BaseGenerator -- Base widget generator class
#include "dlg_msgs.h"          // wxMessageDialog dialogs
#include "font_prop.h"         // FontProperty class
#include "gen_enums.h"         // Enumerations for generators
#include "mainapp.h"           // MainApp class
#include "node.h"              // Node class
#include "node_creator.h"      // NodeCreator class
#include "utils.h"             // Utility functions that work with properties
#include "wxue_view_vector.h"  // wxue::ViewVector

#include "import_crafter_maps.cpp"  // Map of wxCrafter properties to wxUiEditor properties

namespace rapidjson
{
    // If object contains the specified key, this returns the Value. Otherwise, it returns
    // the address of Value::empty_value.
    const Value& FindValue(const Value& object, const char* key);

    // If array contains objects, then this can find an object containing both the key and
    // value specified.
    const Value& FindObject(const char* key, std::string_view value, const Value& array);

    inline bool IsSame(const rapidjson::Value& value, std::string_view str)
    {
        ASSERT(value.IsString())
        return wxue::is_sameas(value.GetString(), str);
    }

    // Converts a m_type numeric id into the equivalent gen_ value. Returns gen_unknown if
    // there is no equivalent.
    GenEnum::GenName GetGenName(const Value& value);

    // Convert a colour value into a string that can be stored in a colour property
    std::string ConvertColour(const rapidjson::Value& colour);

    // If object contains m_selection(int) and m_options(array), this will return a pointer
    // to the string in the array
    std::string_view GetSelectedString(const rapidjson::Value& object);

    // If the array contains strings, this will collect them into a vector.
    std::vector<std::string> GetStringVector(const rapidjson::Value& array);

    static const Value empty_value;
}  // namespace rapidjson

using namespace rapidjson;

WxCrafter::WxCrafter() {}

bool WxCrafter::Import(const std::string& filename, bool write_doc)
{
    std::ifstream input(filename, std::ifstream::binary);
    if (!input.is_open())
    {
        std::string msg("Unable to open\n    \"" + filename + "\"");
        wxMessageDialog dlg(nullptr, msg, "Import wxCrafter project", wxICON_ERROR | wxOK);
        dlg.ShowModal();
        return false;
    }
    std::string buffer(std::istreambuf_iterator<char>(input), {});
    input.close();

    Document document;
    if (document.Parse(buffer).HasParseError())
    {
        dlgInvalidProject(filename, "wxCrafter", "Import wxCrafter project");
        return false;
    }
    if (!document.IsObject())
    {
        dlgInvalidProject(filename, "wxCrafter", "Import wxCrafter project");
        return false;
    }

    m_project = NodeCreation.CreateNode(gen_Project, nullptr).first;

    try
    {
        if (auto& metadata = FindValue(document, "metadata"); metadata.IsObject())
        {
            if (auto& result = FindValue(metadata, "m_useEnum"); result.IsBool())
            {
                // wxCrafter is project wide, wxUiEditor is per-form.
                m_generate_ids = result.GetBool();
            }

            if (auto& include_files = FindValue(metadata, "m_includeFiles");
                include_files.IsArray())
            {
                auto preamble_ptr = m_project->get_PropValuePtr(prop_src_preamble);
                for (const auto& iter: include_files.GetArray())
                {
                    if (iter.IsString())
                    {
                        if (preamble_ptr->size())
                            *preamble_ptr << "@@";
                        *preamble_ptr << "#include \"" << iter.GetString() << "\"";
                    }
                }
            }

            if (auto& internationalize = FindValue(metadata, "m_useUnderscoreMacro");
                internationalize.IsBool())
                m_project->set_value(prop_internationalize, internationalize.GetBool());
            if (auto& out_file = FindValue(metadata, "m_outputFileName"); out_file.IsString())
                m_output_name = out_file.GetString();
            if (auto& out_dir = FindValue(metadata, "m_generatedFilesDir"); out_dir.IsString())
                m_project->set_value(prop_base_directory, out_dir.GetString());
        }

        if (auto& windows = FindValue(document, "windows"); windows.IsArray())
        {
            for (const auto& iter: windows.GetArray())
            {
                ProcessForm(iter);
            }

            if (write_doc)
                m_project->CreateDoc(m_docOut);
        }
    }
    catch (const std::exception& e)
    {
        FAIL_MSG(e.what())
        MSG_ERROR(e.what());
        dlgImportError(e, filename, "Import wxCrafter Project");
        return false;
    }

    if (m_errors.size())
    {
        std::string errMsg("Not everything in the wxCrafter project could be converted:\n\n");
        MSG_ERROR(std::format("------  {}------",
                              std::filesystem::path(m_importProjectFile).filename().string()));
        for (const auto& iter: m_errors)
        {
            MSG_ERROR(iter);
            errMsg += iter + '\n';
        }
        wxMessageDialog dlg(nullptr, errMsg, "Import wxCrafter project", wxICON_WARNING | wxOK);
        dlg.ShowModal();
    }

    return true;
}

auto WxCrafter::ProcessForm(const Value& form) -> void
{
    ASSERT_MSG(form.IsObject(), "Expected the form to be an object!");
    if (!form.IsObject())
    {
        m_errors.emplace("Invalid wxCrafter file -- top level window is not a JSON object.");
        return;
    }

    auto& value = form["m_type"];
    if (!value.IsNumber())
    {
        m_errors.emplace("Invalid wxCrafter file -- top level window is missing a numeric m_type "
                         "key to indicate what type "
                         "of window it is.");
        return;
    }

    auto get_GenName = gen_unknown;
    get_GenName = GetGenName(value);
    if (get_GenName == gen_unknown)
    {
        MSG_ERROR(std::format("Unrecognized window type: {}", value.GetInt()));
        m_errors.emplace("Unrecognized window type!");
        return;
    }

    auto new_node = NodeCreation.CreateNode(get_GenName, m_project.get()).first;
    m_project->AdoptChild(new_node);

    if (!m_generate_ids)
    {
        new_node->set_value(prop_generate_ids, false);
    }

    if (!m_is_output_name_used && m_output_name.size())
    {
        new_node->set_value(prop_base_file, m_output_name);
        m_is_output_name_used = true;
    }

    if (const auto& array = form["m_properties"]; array.IsArray())
    {
        ProcessProperties(new_node.get(), array);
    }
    if (const auto& array = form["m_styles"]; array.IsArray())
    {
        ProcessStyles(new_node.get(), array);
    }
    if (const auto& array = form["m_events"]; array.IsArray())
    {
        ProcessEvents(new_node.get(), array);
    }

    if (const auto& children = form["m_children"]; children.IsArray())
    {
        for (const auto& child: children.GetArray())
        {
            if (!child.IsObject())
            {
                m_errors.emplace(
                    std::format("Invalid wxCrafter file -- child of {} is not a JSON object.",
                                map_GenNames.at(get_GenName)));
                continue;
            }

            ProcessChild(new_node.get(), child);
        }
    }
}

auto WxCrafter::ProcessChild(Node* parent, const Value& object) -> void
{
    const auto& value = object["m_type"];
    if (!value.IsNumber())
    {
        m_errors.emplace(
            "Invalid wxCrafter file -- child is missing a numeric m_type key to indicate what type "
            "of child it is.");
        return;
    }

    auto get_GenName = GetGenName(value);
    if (get_GenName == gen_unknown)
    {
        if (value.GetInt() == 4414)
        {
            // This is a column header for a wxListCtrl
            if (parent->is_Gen(gen_wxListView))
            {
                std::string cur_headers = parent->as_string(prop_column_labels);
                if (const auto& properties = object["m_properties"]; properties.IsArray())
                {
                    for (auto& iter: properties.GetArray())
                    {
                        if (iter.IsObject())
                        {
                            // TODO: [KeyWorks - 01-10-2022] A width can also be specified -- wxUE
                            // doesn't currently support that, but when it does, it should be
                            // processed here as well.
                            if (const auto& label_type = FindValue(iter, "m_label");
                                label_type.IsString() &&
                                wxue::is_sameas(label_type.GetString(), "Name:"))
                            {
                                if (const auto& label = FindValue(iter, "m_value");
                                    label.IsString())
                                {
                                    if (cur_headers.size())
                                    {
                                        cur_headers += ' ';
                                    }
                                    cur_headers += '"' + std::string(label.GetString()) + '"';
                                    parent->set_value(prop_column_labels, cur_headers);
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (value.GetInt() == 4476)
        {
            // wxCrafter puts a bogus window between collapsible pane and the actual content. We
            // simply grab it's children and apply it directly (which is also the way wxCrafter
            // generates the code for this).
            //
            // WARNING! wxCrafter allows multiple "panes", but only the last one is actually used.
            // We use the first "pane" rather than the last "pane".

            if (const auto& children = FindValue(object, "m_children"); children.IsArray())
            {
                if (get_GenName == gen_wxStdDialogButtonSizer)
                {
                    ProcessStdBtnChildren(parent, children);
                    if (parent->as_string(prop_alignment).size())
                    {
                        parent->set_value(prop_static_line, false);
                    }
                }
                else
                {
                    for (const auto& child: children.GetArray())
                    {
                        if (!child.IsObject())
                        {
                            m_errors.emplace(std::format(
                                "Invalid wxCrafter file -- child of {} is not a JSON object.",
                                map_GenNames.at(get_GenName)));
                            continue;
                        }

                        ProcessChild(parent, child);
                    }
                }
            }
            return;
        }

        MSG_ERROR(std::format("Unrecognized child type: {}", value.GetInt()));
        // m_errors.emplace("Unrecognized child type!");
        return;
    }

    if (get_GenName == gen_wxCheckBox)
    {
        if (const auto& array = FindValue(object, "m_styles"); array.IsArray())
        {
            for (const auto& iter: array.GetArray())
            {
                if (wxue::is_sameas(iter.GetString(), "wxCHK_3STATE"))
                {
                    get_GenName = gen_Check3State;
                    break;
                }
            }
        }
    }

    if (get_GenName == gen_tool && parent->is_Gen(gen_wxAuiToolBar))
    {
        get_GenName = gen_auitool;
    }

    auto new_node = NodeCreation.CreateNode(get_GenName, parent).first;
    if (!new_node)
    {
        m_errors.emplace(std::format("{} cannot be a child of {}", map_GenNames.at(get_GenName),
                                     parent->get_DeclName()));
        return;
    }
    parent->AdoptChild(new_node);
    if (value == 4404)  // Originally, this was a wxBitmapButton
    {
        new_node->set_value(prop_style, "wxBU_EXACTFIT");
        new_node->set_value(prop_label, "");
    }

    if (const auto& proportion = FindValue(object, "proportion");
        proportion.IsInt() && proportion.GetInt() > 0)
    {
        new_node->set_value(prop_proportion, proportion.GetInt());
    }
    if (const auto& border = FindValue(object, "border"); border.IsInt() && border.GetInt() != 5)
    {
        new_node->set_value(prop_border_size, border.GetInt());
    }

    if (const auto& gbSpan = FindValue(object, "gbSpan");
        gbSpan.IsString() && !IsSame(gbSpan, "1,1"))
    {
        std::string_view positions = gbSpan.GetString();
        new_node->set_value(prop_rowspan, wxue::atoi(positions));
        positions = positions.substr(positions.find_first_not_of("0123456789"));
        positions = positions.substr(positions.find_first_of("0123456789"));
        new_node->set_value(prop_colspan, wxue::atoi(positions));
    }
    if (const auto& gbPosition = FindValue(object, "gbPosition");
        gbPosition.IsString() && !IsSame(gbPosition, "0,0"))
    {
        std::string_view positions = gbPosition.GetString();
        new_node->set_value(prop_row, wxue::atoi(positions));
        positions = positions.substr(positions.find_first_not_of("0123456789"));
        positions = positions.substr(positions.find_first_of("0123456789"));
        new_node->set_value(prop_column, wxue::atoi(positions));
    }

    if (const auto& array = FindValue(object, "m_sizerFlags"); array.IsArray())
    {
        ProcessSizerFlags(new_node.get(), array);
    }
    if (const auto& array = FindValue(object, "m_properties"); array.IsArray())
    {
        ProcessProperties(new_node.get(), array);
    }
    if (const auto& array = FindValue(object, "m_styles"); array.IsArray())
    {
        ProcessStyles(new_node.get(), array);
    }
    if (const auto& array = FindValue(object, "m_events"); array.IsArray())
    {
        ProcessEvents(new_node.get(), array);
    }

    if (const auto& children = FindValue(object, "m_children"); children.IsArray())
    {
        if (get_GenName == gen_wxPropertyGridManager)
        {
            // We always supply one page even if it's empty so that wxPropertyGridManager::Clear()
            // will work correctly
            get_GenName = gen_propGridPage;
            auto child_node = NodeCreation.CreateNode(get_GenName, new_node.get()).first;
            if (!child_node)
            {
                m_errors.emplace(std::format("{} cannot be a child of {}",
                                             map_GenNames.at(get_GenName),
                                             new_node->get_DeclName()));
                return;
            }
            new_node->AdoptChild(child_node);
            new_node = child_node;
        }

        if (get_GenName == gen_wxStdDialogButtonSizer)
        {
            ProcessStdBtnChildren(new_node.get(), children);
            if (new_node->as_string(prop_alignment).size())
            {
                new_node->set_value(prop_static_line, false);
            }
        }
        else
        {
            for (const auto& child: children.GetArray())
            {
                if (!child.IsObject())
                {
                    m_errors.emplace(
                        std::format("Invalid wxCrafter file -- child of {} is not a JSON object.",
                                    map_GenNames.at(get_GenName)));
                    continue;
                }

                ProcessChild(new_node.get(), child);
            }
        }
    }
}

auto WxCrafter::ProcessStdBtnChildren(Node* node, const Value& array) -> void
{
    bool is_default_cleared { false };
    for (const auto& iter: array.GetArray())
    {
        if (const auto& properties = FindValue(iter, "m_properties"); properties.IsArray())
        {
            if (const auto& object = FindObject("m_label", "ID:", properties); !object.IsNull())
            {
                std::string_view id = GetSelectedString(object);
                if (id.size())
                {
                    // If there is at least one valid id, then clear all of the default settings
                    if (!is_default_cleared)
                    {
                        is_default_cleared = true;
                        node->set_value(prop_OK, false);
                        node->set_value(prop_Cancel, false);
                        node->set_value(prop_default_button, "");
                    }

                    if (id == "wxID_OK")
                    {
                        node->set_value(prop_OK, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->set_value(prop_default_button, "OK");
                        if (const auto& name = FindValue(iter, "m_events");
                            name.IsArray() && name.Size())
                        {
                            if (const auto& handler =
                                    FindValue(name[0], "m_functionNameAndSignature");
                                handler.IsString())
                            {
                                std::string function = handler.GetString();
                                function.erase(function.find('('));
                                node->get_Event("OKButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id == "wxID_YES")
                    {
                        node->set_value(prop_Yes, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->set_value(prop_default_button, "Yes");
                        if (const auto& name = FindValue(iter, "m_events");
                            name.IsArray() && name.Size())
                        {
                            if (const auto& handler =
                                    FindValue(name[0], "m_functionNameAndSignature");
                                handler.IsString())
                            {
                                std::string function = handler.GetString();
                                function.erase(function.find('('));
                                node->get_Event("YesButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id == "wxID_SAVE")
                    {
                        node->set_value(prop_Save, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->set_value(prop_default_button, "Save");
                        if (const auto& name = FindValue(iter, "m_events");
                            name.IsArray() && name.Size())
                        {
                            if (const auto& handler =
                                    FindValue(name[0], "m_functionNameAndSignature");
                                handler.IsString())
                            {
                                std::string function = handler.GetString();
                                function.erase(function.find('('));
                                node->get_Event("SaveButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id == "wxID_CLOSE")
                    {
                        node->set_value(prop_Close, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->set_value(prop_default_button, "Close");
                        if (const auto& name = FindValue(iter, "m_events");
                            name.IsArray() && name.Size())
                        {
                            if (const auto& handler =
                                    FindValue(name[0], "m_functionNameAndSignature");
                                handler.IsString())
                            {
                                std::string function = handler.GetString();
                                function.erase(function.find('('));
                                node->get_Event("CloseButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id == "wxID_CANCEL")
                    {
                        node->set_value(prop_Cancel, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->set_value(prop_default_button, "Cancel");
                        if (const auto& name = FindValue(iter, "m_events");
                            name.IsArray() && name.Size())
                        {
                            if (const auto& handler =
                                    FindValue(name[0], "m_functionNameAndSignature");
                                handler.IsString())
                            {
                                std::string function = handler.GetString();
                                function.erase(function.find('('));
                                node->get_Event("CancelButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id == "wxID_NO")
                    {
                        node->set_value(prop_No, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->set_value(prop_default_button, "No");
                        if (const auto& name = FindValue(iter, "m_events");
                            name.IsArray() && name.Size())
                        {
                            if (const auto& handler =
                                    FindValue(name[0], "m_functionNameAndSignature");
                                handler.IsString())
                            {
                                std::string function = handler.GetString();
                                function.erase(function.find('('));
                                node->get_Event("NoButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id == "wxID_APPLY")
                    {
                        node->set_value(prop_Apply, true);
                        if (const auto& name = FindValue(iter, "m_events");
                            name.IsArray() && name.Size())
                        {
                            if (const auto& handler =
                                    FindValue(name[0], "m_functionNameAndSignature");
                                handler.IsString())
                            {
                                std::string function = handler.GetString();
                                function.erase(function.find('('));
                                node->get_Event("ApplyButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id == "wxID_HELP")
                    {
                        node->set_value(prop_Help, true);
                        if (const auto& name = FindValue(iter, "m_events");
                            name.IsArray() && name.Size())
                        {
                            if (const auto& handler =
                                    FindValue(name[0], "m_functionNameAndSignature");
                                handler.IsString())
                            {
                                std::string function = handler.GetString();
                                function.erase(function.find('('));
                                node->get_Event("HelpButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id == "wxID_CONTEXT_HELP")
                    {
                        node->set_value(prop_ContextHelp, true);
                        if (const auto& name = FindValue(iter, "m_events");
                            name.IsArray() && name.Size())
                        {
                            if (const auto& handler =
                                    FindValue(name[0], "m_functionNameAndSignature");
                                handler.IsString())
                            {
                                std::string function = handler.GetString();
                                function.erase(function.find('('));
                                node->get_Event("ContextHelpButtonClicked")->set_value(function);
                            }
                        }
                    }
                }
            }
        }
    }
}

auto WxCrafter::ProcessStyles(Node* node, const Value& array) -> void
{
    // Caution: any of these property options could be a null ptr

    auto* style = node->get_PropPtr(prop_style);

    if (style)
    {
        // Some styles will have been specifically set when other properties were processed, so
        // we need to be careful about clearing all styles.
        if (!node->is_Gen(gen_wxButton) && !node->is_Gen(gen_wxTextCtrl))
        {
            style->set_value("");
        }
    }
    auto* win_style = node->get_PropPtr(prop_window_style);
    if (win_style)
    {
        win_style->set_value("");
    }

    for (const auto& iter: array.GetArray())
    {
        const auto* style_bit = iter.GetString();
        if (style)
        {
            bool bit_found { false };
            for (auto& option: style->get_PropDeclaration()->getOptions())
            {
                if (option.name == style_bit)
                {
                    if (style->get_value().size())
                    {
                        style->get_value() << '|';
                    }
                    style->get_value() << style_bit;
                    bit_found = true;
                    break;
                }
            }
            if (bit_found)
            {
                continue;
            }
            if (node->is_Gen(gen_wxRadioBox))
            {
                if (wxue::is_sameas(style_bit, "wxRA_SPECIFY_ROWS"))
                {
                    node->set_value(prop_style, "rows");
                }
                continue;
            }
        }

        if (win_style)
        {
            for (auto& option: win_style->get_PropDeclaration()->getOptions())
            {
                if (option.name == style_bit)
                {
                    if (win_style->get_value().size())
                    {
                        win_style->get_value() << '|';
                    }
                    win_style->get_value() << style_bit;
                    break;
                }
            }
        }
    }
}

auto WxCrafter::ProcessEvents(Node* node, const Value& array) -> void
{
    for (const auto& iter: array.GetArray())
    {
        if (const auto& event = iter; event.IsObject())
        {
            if (const auto& name = event["m_eventName"]; name.IsString())
            {
                auto* node_event = node->get_Event(GetCorrectEventName(name.GetString()));
                if (!node_event)
                {
                    std::string modified_name(name.GetString());
                    if (auto pos = modified_name.find("_COMMAND"); pos != std::string::npos)
                    {
                        modified_name.erase(pos, 8);  // length of "_COMMAND"
                    }
                    node_event = node->get_Event(GetCorrectEventName(modified_name));
                    if (!node_event)
                    {
                        auto pos = modified_name.find_last_of('_');
                        if (wxue::is_found(pos))
                        {
                            modified_name.erase(pos);
                            node_event = node->get_Event(GetCorrectEventName(modified_name));
                        }
                    }
                }

                if (node_event)
                {
                    if (auto& handler = event["m_functionNameAndSignature"]; handler.IsString())
                    {
                        std::string function = handler.GetString();
                        function.erase(function.find('('));
                        node_event->set_value(function);
                    }
                }
            }
        }
    }
}

auto WxCrafter::ProcessSizerFlags(Node* node, const Value& array) -> void
{
    std::set<std::string> all_items = {};
    for (const auto& iter: array.GetArray())
    {
        all_items.insert(std::move(iter.GetString()));
    }

    // If the node has porp_alignment, then it will also have prop_borders and prop_flags
    if (node->HasProp(prop_alignment))
    {
        if (all_items.contains("wxEXPAND"))
        {
            node->set_value(prop_flags, "wxEXPAND");
        }
        else
        {
            auto alignment = node->get_PropValuePtr(prop_alignment);

            if (all_items.contains("wxALIGN_CENTER"))
            {
                if (alignment->size())
                {
                    *alignment << '|';
                }
                *alignment << "wxALIGN_CENTER";
            }
            else if (all_items.contains("wxALIGN_CENTER_HORIZONTAL"))
            {
                if (alignment->size())
                {
                    *alignment << '|';
                }
                *alignment << "wxALIGN_CENTER_HORIZONTAL";
            }
            else if (all_items.contains("wxALIGN_CENTER_VERTICAL"))
            {
                if (alignment->size())
                {
                    *alignment << '|';
                }
                *alignment << "wxALIGN_CENTER_VERTICAL";
            }

            if (all_items.contains("wxALIGN_RIGHT"))
            {
                if (!alignment->contains("wxALIGN_CENTER"))
                {
                    if (alignment->size())
                    {
                        *alignment << '|';
                    }
                    *alignment << "wxALIGN_RIGHT";
                }
            }
            else if (all_items.contains("wxALIGN_LEFT"))
            {
                if (!alignment->contains("wxALIGN_CENTER"))
                {
                    if (alignment->size())
                    {
                        *alignment << '|';
                    }
                    *alignment << "wxALIGN_LEFT";
                }
            }
            else if (all_items.contains("wxALIGN_TOP"))
            {
                if (!alignment->contains("wxALIGN_CENTER"))
                {
                    if (alignment->size())
                    {
                        *alignment << '|';
                    }
                    *alignment << "wxALIGN_TOP";
                }
            }
            else if (all_items.contains("wxALIGN_BOTTOM"))
            {
                if (!alignment->contains("wxALIGN_CENTER"))
                {
                    if (alignment->size())
                    {
                        *alignment << '|';
                    }
                    *alignment << "wxALIGN_BOTTOM";
                }
            }
        }
    }

    if (node->HasProp(prop_border))
    {
        if (all_items.contains("wxALL"))
        {
            node->set_value(prop_border, "wxALL");
        }
        else
        {
            auto border_ptr = node->get_PropValuePtr(prop_border);
            border_ptr->clear();

            if (all_items.contains("wxLEFT"))
            {
                if (border_ptr->size())
                {
                    *border_ptr << ',';
                }
                *border_ptr << "wxLEFT";
            }
            if (all_items.contains("wxRIGHT"))
            {
                if (border_ptr->size())
                {
                    *border_ptr << ',';
                }
                *border_ptr << "wxRIGHT";
            }
            if (all_items.contains("wxTOP"))
            {
                if (border_ptr->size())
                {
                    *border_ptr << ',';
                }
                *border_ptr << "wxTOP";
            }
            if (all_items.contains("wxBOTTOM"))
            {
                if (border_ptr->size())
                {
                    *border_ptr << ',';
                }
                *border_ptr << "wxBOTTOM";
            }
        }
    }
}

auto WxCrafter::ProcessProperties(Node* node, const Value& array) -> void
{
    for (const auto& iter: array.GetArray())
    {
        const auto& value = iter;
        std::string name = "";
        if (value["m_label"].IsString())
        {
            name = value["m_label"].GetString();
            if (name.back() == ':')
            {
                name.pop_back();
            }
            std::ranges::transform(name, name.begin(),
                                   [](unsigned char c)
                                   {
                                       return std::tolower(c);
                                   });
            auto prop_name = FindProp(name);
            if (prop_name == prop_unknown)
            {
                prop_name = UnknownProperty(node, value, name);
                if (prop_name == prop_processed)
                {
                    continue;
                }
            }

            KnownProperty(node, value, prop_name);
        }
    }
}

GenEnum::PropName WxCrafter::UnknownProperty(Node* node, const Value& value, std::string& name)
{
    GenEnum::PropName prop_name = prop_unknown;

    if (auto result = map_crafter_props.find(name); result != map_crafter_props.end())
    {
        prop_name = result->second;
    }
    else
    {
        if (wxue::is_sameas(name, "name"))
        {
            if (node->is_Gen(gen_TreeListCtrlColumn))
            {
                prop_name = prop_label;
            }
            else
            {
                prop_name = (node->is_Form() ? prop_class_name : prop_var_name);
            }
        }
        else if (node->is_Gen(gen_wxStyledTextCtrl) && ProcessScintillaProperty(node, value))
        {
            return prop_processed;
        }

        else if (wxue::is_sameas(name, "centre"))
        {
            if (value["m_selection"].IsNumber())
            {
                switch (value["m_selection"].GetInt())
                {
                    case 0:
                        node->set_value(prop_center, "no");
                        break;

                    case 1:
                        node->set_value(prop_center, "wxBOTH");
                        break;

                    case 2:
                        node->set_value(prop_center, "wxVERTICAL");
                        break;

                    case 3:
                        node->set_value(prop_center, "wxHORIZONTAL");
                        break;

                    default:
                        break;
                }
            }
            return prop_processed;
        }
        else if (wxue::is_sameas(name, "show effect"))
        {
            int index = 0;
            if (value["m_selection"].IsNumber())
            {
                index = value["m_selection"].GetInt();
                if (index > 0)
                {
                    auto list_effects = GetStringVector(FindValue(value, "m_options"));
                    if ((to_size_t) index < list_effects.size())
                    {
                        for (const auto& friendly_pair: g_friend_constant)
                        {
                            if (wxue::is_sameas(friendly_pair.second, list_effects[index]))
                            {
                                node->set_value(prop_show_effect,
                                                std::string_view(friendly_pair.first)
                                                    .substr(friendly_pair.first.find('_') + 1));
                                node->set_value(prop_hide_effect,
                                                std::string_view(friendly_pair.first)
                                                    .substr(friendly_pair.first.find('_') + 1));
                                break;
                            }
                        }
                    }
                }
            }
            return prop_processed;
        }

        else if (wxue::is_sameas(name, "construct the dropdown menu"))
        {
            if (node->is_Gen(gen_tool) || node->is_Gen(gen_auitool))
            {
                // wxCrafter seems to always set this to true, ignoring the value for Kind.
                return prop_processed;
            }
        }
        else if (wxue::is_sameas(name, "gradient start"))
        {
            if (auto& colour = FindValue(value, "colour"); colour.IsString())
            {
                node->set_value(prop_start_colour, ConvertColour(colour));
                return prop_processed;
            }
        }
        else if (wxue::is_sameas(name, "combobox choices"))
        {
            if (auto& choices = FindValue(value, "m_value"); choices.IsString())
            {
                wxue::ViewVector mview(choices.GetString(), "\\n");
                std::string contents;
                for (auto& choice: mview)
                {
                    if (choice.size())
                    {
                        if (contents.size())
                            contents += ' ';
                        contents += '"';
                        contents += choice;
                        contents += '"';
                    }
                }
                node->set_value(prop_contents, contents);
                return prop_processed;
            }
        }
        else if (wxue::is_sameas(name, "gradient end"))
        {
            if (auto& colour = FindValue(value, "colour"); colour.IsString())
            {
                node->set_value(prop_end_colour, ConvertColour(colour));
                return prop_processed;
            }
        }
        else if (wxue::is_sameas(name, "bitmap file"))
        {
            ProcessBitmapPropety(node, value);
            return prop_processed;
        }

        else if (name.starts_with("bitmap file ("))
        {
            return prop_processed;  // These are different icon sizes
        }
        else if (name.starts_with("bitmap file ("))
        {
            return prop_processed;  // These are different icon sizes
        }
        else if (wxue::is_sameas(name, "auto complete directories") ||
                 wxue::is_sameas(name, "auto complete files"))
        {
            // [KeyWorks - 12-23-2021]
            // These are only valid on Windows -- using them means the app will not work correctly
            // on other platforms. Since the user can add them to their derived class, or even in a
            // lambda OnInit event handler, I don't see a reason to support them.
            return prop_processed;
        }
        else if (wxue::is_sameas(name, "disabled-bitmap file"))
        {
            // Currently we don't support this.
            return prop_processed;
        }
        else if (wxue::is_sameas(name, "focused"))
        {
            if (auto& setting = FindValue(value, "m_value"); setting.IsBool())
            {
                node->set_value(prop_focus, setting.GetBool());
            }
            return prop_processed;
        }
        else if (wxue::is_sameas(name, "selected") && node->is_Gen(gen_BookPage))
        {
            if (auto& setting = FindValue(value, "m_value"); setting.IsBool())
            {
                node->set_value(prop_select, setting.GetBool());
            }
            return prop_processed;
        }
        else if (wxue::is_sameas(name, "virtual folder"))
            return prop_processed;  // this doesn't apply to wxUiEditor
        else if (wxue::is_sameas(name, "null page"))
            return prop_processed;  // unused
        else if (wxue::is_sameas(name, "enable spell checking") && node->HasProp(prop_spellcheck))
        {
            node->set_value(prop_spellcheck, "enabled");
            std::string style = node->as_string(prop_style);
            if (style.size() && style.find("wxTE_RICH2") == std::string::npos)
                style += "|wxTE_RICH2";
            else
                style += "wxTE_RICH2";
            node->set_value(prop_style, style);
        }
        else if (wxue::is_sameas(name, "keep as a class member"))
        {
            if (node->as_string(prop_class_access) == "none")
                node->set_value(prop_class_access, "protected:");
        }
        else if (wxue::is_sameas(name, "Start the timer"))
        {
            if (auto& setting = FindValue(value, "m_value"); setting.IsBool())
            {
                node->set_value(prop_auto_start, setting.GetBool());
            }
            return prop_processed;
        }
        else if (wxue::is_sameas(name, "One Shot Timer"))
        {
            if (auto& setting = FindValue(value, "m_value"); setting.IsBool())
            {
                node->set_value(prop_auto_start, setting.GetBool());
            }
            return prop_processed;
        }
        else
        {
            if (!node->is_Gen(gen_propGridItem))
            {
                // wxCrafter outputs a boatload of empty fields for property grid items
                MSG_WARNING(std::format("Unknown property: \"{}\"", value["m_label"].GetString()));
            }
            return prop_processed;
        }
    }

    return prop_name;
}

auto WxCrafter::KnownProperty(Node* node, const Value& value, GenEnum::PropName prop_name) -> void
{
    if (node->is_Gen(gen_wxPopupWindow))
    {
        if (prop_name == prop_size || prop_name == prop_minimum_size || prop_name == prop_title)
            return;  // wxCrafter writes these, but doesn't use them (wxUiEditor does support size
                     // and minimum_size)
    }

    if (node->is_Gen(gen_ribbonTool) || node->is_Gen(gen_ribbonSeparator) ||
        node->is_Gen(gen_ribbonButton))
    {
        if (prop_name == prop_kind)
        {
            node->set_value(prop_kind, GetSelectedString(value));
            return;
        }
        else if (prop_name == prop_statusbar)
        {
            if (auto& prop_value = FindValue(value, "m_value"); prop_value.IsString())
            {
                node->set_value(prop_help, prop_value.GetString());
            }
            return;
        }
        else if (prop_name == prop_size || prop_name == prop_minimum_size ||
                 prop_name == prop_var_name)
        {
            return;
        }
    }

    if (prop_name == prop_background_colour || prop_name == prop_foreground_colour ||
        prop_name == prop_normal_color || prop_name == prop_visited_color ||
        prop_name == prop_hover_color)
    {
        if (auto& colour = FindValue(value, "colour"); colour.IsString())
        {
            node->set_value(prop_name, ConvertColour(colour));
            // Convert old style into #RRGGBB
            node->set_value(prop_name, node->as_wxColour(prop_name).GetAsString(wxC2S_HTML_SYNTAX));
        }
    }
    else if (prop_name == prop_id)
    {
        if (auto& setting = FindValue(value, "m_winid"); setting.IsString())
        {
            node->set_value(prop_name, setting.GetString());
        }
    }
    else if (prop_name == prop_selection && !FindValue(value, "m_value").IsNull())
    {
        auto& setting = FindValue(value, "m_value");
        // This is a bug in version 2.9 of wxCrafter -- the value should be an int, not a string. We
        // add the GetInt() variant in case they ever fix it.
        if (setting.IsString())
        {
            std::string result = setting.GetString();
            if (wxue::is_digit(result[0]))
            {
                if (node->HasProp(prop_selection_int))
                    node->set_value(prop_selection_int, wxue::atoi(result));
                else if (node->HasProp(prop_selection))
                    node->set_value(prop_selection, wxue::atoi(result));
            }
            else
            {
                node->set_value(prop_selection_string, FindValue(value, "m_value").GetString());
            }
        }
        else if (setting.IsString())
            node->set_value(prop_selection_int, FindValue(value, "m_value").GetInt());
    }
    else if (prop_name == prop_orientation)
    {
        if (auto& setting = value["m_selection"]; setting.IsInt())
        {
            node->set_value(prop_orientation,
                            setting.GetInt() == 0 ? "wxVERTICAL" : "wxHORIZONTAL");
        }
    }
    else if (prop_name == prop_value)
    {
        ValueProperty(node, value);
    }

    if (prop_name == prop_var_name &&
        (node->is_Gen(gen_spacer) || node->is_Gen(gen_toolSeparator) ||
         node->is_Gen(gen_ribbonGalleryItem)))
    {
        return;
    }
    if (prop_name == prop_size && node->is_Gen(gen_spacer))
    {
        wxue::ViewVector mview(FindValue(value, "m_value").GetString(), ',');
        if (mview.size() > 1)
        {
            node->set_value(prop_width, wxue::atoi(mview[0]));
            node->set_value(prop_height, wxue::atoi(mview[1]));
        }
    }
    if (prop_name == prop_size && node->is_Gen(gen_wxStdDialogButtonSizer))
    {
        return;  // wxCrafter doesn't use this either, but does generate it
    }

    if (prop_name == prop_contents)
    {
        if (auto& setting = FindValue(value, "m_value"); setting.IsString())
        {
            if (node->HasProp(prop_contents))
            {
                wxue::StringVector contents(std::string_view(setting.GetString()), ';');
                auto str_ptr = node->get_PropPtr(prop_contents)->as_raw_ptr();
                str_ptr->clear();  // remove any default string
                for (const auto& item: contents)
                {
                    if (item.size())
                    {
                        if (str_ptr->size())
                            *str_ptr << ' ';
                        *str_ptr << '"' << item.ToStdString() << '"';
                    }
                }
            }
            // typically this is a wxPGProperty -- we don't support strings for this
        }
    }
    if (prop_name == prop_stc_lexer)
    {
        ProcessScintillaProperty(node, value);
    }
    else if (prop_name == prop_font)
    {
        ProcessFont(node, value);
    }
    else if (prop_name == prop_statusbar && node->is_Gen(gen_ribbonButton))
    {
        if (auto& prop_value = FindValue(value, "m_value"); prop_value.IsString())
        {
            node->set_value(prop_help, prop_value.GetString());
        }
    }

    if (prop_name == prop_kind && (node->is_Gen(gen_tool) || node->is_Gen(gen_auitool)))
    {
        std::string_view tool_kind = GetSelectedString(value);
        if (wxue::is_sameas(tool_kind, "checkable"))
        {
            node->set_value(prop_kind, "wxITEM_CHECK");
        }
        else if (wxue::is_sameas(tool_kind, "radio"))
        {
            node->set_value(prop_kind, "wxITEM_RADIO");
        }
        else if (wxue::is_sameas(tool_kind, "dropdown"))
        {
            node->set_value(prop_kind, "wxITEM_DROPDOWN");
        }
        else
        {
            node->set_value(prop_kind, "wxITEM_NORMAL");
        }
    }
    if (prop_name == prop_kind && node->is_Gen(gen_ribbonTool))
    {
        node->set_value(prop_kind, GetSelectedString(value));
    }

    if (prop_name != prop_unknown)
    {
        if (auto& prop_value = FindValue(value, "m_value"); !prop_value.IsNull())
        {
            if (!node->HasProp(prop_name))
            {
                if (prop_name == prop_min && node->HasProp(prop_minValue))
                {
                    prop_name = prop_minValue;
                }
                else if (prop_name == prop_checked && node->HasProp(prop_pressed))
                {
                    prop_name = prop_pressed;
                }
                else if (prop_name == prop_max && node->HasProp(prop_maxValue))
                {
                    prop_name = prop_maxValue;
                }
                else if (prop_name == prop_tooltip && node->is_Gen(gen_propGridItem))
                {
                    prop_name = prop_help;
                }
                else if (wxGetApp().isTestingMenuEnabled())
                {
                    if ((prop_value.IsString() && prop_value.GetStringLength()) ||
                        (prop_value.IsBool() && prop_value.GetBool()))
                    {
                        MSG_INFO(std::format("{} doesn't have a property called {}",
                                             node->get_DeclName(),
                                             GenEnum::map_PropNames.at(prop_name)));
                    }
                }
            }
            if (prop_value.IsBool())
            {
                node->set_value(prop_name, prop_value.GetBool());
            }
            else if (prop_value.IsInt())
            {
                node->set_value(prop_name, prop_value.GetInt());
            }
            else
            {
                std::string_view val = prop_value.GetString();
                if (val == "-1,-1" &&
                    (prop_name == prop_size || prop_name == prop_min_size || prop_name == prop_pos))
                {
                    return;  // Don't set if it is a default value
                }
                if (prop_name == prop_message)
                {
                    auto escape_removal = ConvertEscapeSlashes(val);
                    node->set_value(prop_name, escape_removal);
                }
                if (val.size())
                {
                    if (val.size())
                    {
                        node->set_value(prop_name, val);
                    }
                }
            }
        }
    }
}

auto WxCrafter::ValueProperty(Node* node, const Value& value) -> void
{
    if (auto& setting = FindValue(value, "m_value"); !setting.IsNull())
    {
        if (node->is_Gen(gen_wxSpinCtrl) || node->is_Gen(gen_wxSpinButton))
        {
            node->set_value(prop_initial, setting.GetString());
        }
        else if (node->is_Gen(gen_wxFilePickerCtrl))
        {
            node->set_value(prop_initial_path, setting.GetString());
        }
        else if (node->is_Gen(gen_wxDirPickerCtrl))
        {
            node->set_value(prop_initial_path, setting.GetString());
        }
        else if (node->is_Gen(gen_wxFontPickerCtrl))
        {
            ProcessFont(node, value);
        }
        else if (node->is_Gen(gen_wxGauge))
        {
            node->set_value(prop_position, setting.GetString());
        }
        else if (node->is_Gen(gen_wxComboBox))
        {
            node->set_value(prop_selection_string, setting.GetString());
        }
        else if (node->is_Gen(gen_wxCheckBox) || node->is_Gen(gen_wxRadioButton))
        {
            node->set_value(prop_checked, setting.GetBool());
        }
        else if (node->is_Gen(gen_wxScrollBar))
        {
            node->set_value(prop_position, setting.GetString());
        }
        else if (node->is_Gen(gen_Check3State))
        {
            if (setting.GetBool())
            {
                node->set_value(prop_initial_state, "wxCHK_CHECKED");
            }
        }
        else if (node->is_Gen(gen_wxSlider))
        {
            node->set_value(prop_position, setting.GetString());
        }
        else if (node->HasProp(prop_value))
        {
            node->set_value(prop_value, setting.GetString());
        }
        else
        {
            MSG_ERROR(std::format("Json sets value, but {} doesn't support that property!",
                                  map_GenNames.at(node->get_GenName())));
        }
    }
    else if (auto& colour = FindValue(value, "colour"); !colour.IsNull())
    {
        node->set_value(prop_colour, ConvertColour(colour));
    }
}

auto WxCrafter::ProcessBitmapPropety(Node* node, const Value& object) -> void
{
    if (std::string_view path = object["m_path"].GetString(); path.size())
    {
        std::string bitmap;
        if (path.starts_with("wxART"))
        {
            wxue::ViewVector parts(path, ',');
            if (parts.size() > 1)
            {
                bitmap = std::format("Art;{}|{};[-1,-1]", parts[0], parts[1]);
            }
        }
        else
        {
            bitmap = std::format("Embed;{};[-1,-1]", path);
        }

        if (IsSame(object["m_label"], "Bitmap File:"))
        {
            if (node->is_Gen(gen_wxAnimationCtrl))
            {
                node->set_value(prop_animation, bitmap);
            }
            else if (node->HasProp(prop_bitmap))
            {
                node->set_value(prop_bitmap, bitmap);
            }
        }
        else if (IsSame(object["m_label"], "Disabled-Bitmap File"))
        {
            if (node->HasProp(prop_disabled_bmp))
                node->set_value(prop_disabled_bmp, bitmap);
        }
    }
}

// For system fonts, wxCrafter doesn't support size or weigth -- you are limited to italic, bold,
// and underlined. Note that wxCrafter supports ALL of the system fonts, not just
// wxSYS_DEFAULT_GUI_FONT.
//
// For custom fonts, wxCrafter uses the system font picker, but ignores some of the results.
// wxCrafter supports integer point size, italic, bold, underlines, family and facename.

bool WxCrafter::ProcessFont(Node* node, const Value& object)
{
    if (object.HasMember("m_value"))
    {
        std::string crafter_str = object["m_value"].GetString();
        if (crafter_str.empty())
            return true;

        FontProperty font_info;
        if (crafter_str.find("italic") != std::string::npos)
            font_info.Italic(wxFONTFLAG_ITALIC);
        if (crafter_str.find("bold") != std::string::npos)
            font_info.Bold(wxFONTFLAG_ITALIC);
        if (crafter_str.find("underlined") != std::string::npos)
            font_info.Underlined();

        if (!crafter_str.starts_with("wxSYS_DEFAULT_GUI_FONT"))
        {
            font_info.setDefGuiFont(false);
            font_info.FaceName("");
            wxue::ViewVector mstr(crafter_str, ',', wxue::TRIM::left);

            if (mstr[0] == "wxSYS_OEM_FIXED_FONT" || mstr[0] == "wxSYS_ANSI_FIXED_FONT")
                font_info.Family(wxFONTFAMILY_TELETYPE);

            if (wxue::is_digit(mstr[0][0]))
            {
                font_info.PointSize(wxue::atoi(mstr[0]));

                if (mstr.size() > 3 && mstr[3] != "default")
                    font_info.Family(font_family_pairs.GetValue(mstr[3]));
                if (mstr.size() > 4 && mstr[4] == "1")
                    font_info.Underlined();
                if (mstr.size() > 5)
                    font_info.FaceName(wxString::FromUTF8(mstr[5].data(), mstr[5].size()));
            }
        }

        if (node->is_Gen(gen_wxFontPickerCtrl))
            node->set_value(prop_initial_font, font_info.as_string());
        else
            node->set_value(prop_font, font_info.as_string());
    }
    return true;
}

bool WxCrafter::ProcessScintillaProperty(Node* node, const Value& object)
{
    // wxCrafter hard-codes margin numbers. line:0, symbol:2, separator:3, fold:4,

    std::string name = object["m_label"].GetString();
    std::ranges::transform(name, name.begin(),
                           [](unsigned char c)
                           {
                               return std::tolower(c);
                           });
    if (name == "fold margin")
    {
        if (object["m_value"].GetBool())
        {
            node->set_value(prop_fold_margin, "4");
            node->set_value(prop_fold_width, "16");
        }
        return true;
    }
    else if (name == "line number margin")
    {
        if (object["m_value"].GetBool())
        {
            node->set_value(prop_line_margin, "0");
            node->set_value(prop_line_digits, "5");
        }
        return true;
    }
    else if (name == "separator margin")
    {
        if (object["m_value"].GetBool())
        {
            node->set_value(prop_separator_margin, "3");
            node->set_value(prop_separator_width, 1);
        }
        return true;
    }
    else if (name == "symbol margin")
    {
        if (object["m_value"].GetBool())
        {
            node->set_value(prop_symbol_margin, "2");
            node->set_value(prop_symbol_mouse_sensitive, true);
        }
        return true;
    }
    else if (name == "wrap text")
    {
        if (object["m_selection"].IsNumber())
        {
            switch (object["m_selection"].GetInt())
            {
                case 1:
                    node->set_value(prop_stc_wrap_mode, "word");
                    break;

                case 2:
                    node->set_value(prop_stc_wrap_mode, "character");
                    break;
            }
        }
        return true;
    }
    else if (name == "indentation guides")
    {
        if (object["m_selection"].IsNumber())
        {
            switch (object["m_selection"].GetInt())
            {
                case 1:
                    node->set_value(prop_indentation_guides, "real");
                    break;

                case 2:
                    node->set_value(prop_indentation_guides, "forward");
                    break;

                case 3:
                    node->set_value(prop_indentation_guides, "both");
                    break;
            }
        }
        return true;
    }
    else if (name == "eol mode")
    {
        if (object["m_selection"].IsNumber())
        {
            switch (object["m_selection"].GetInt())
            {
                case 0:
                    node->set_value(prop_eol_mode, "\r\n (CR/LF)");
                    break;

                case 1:
                    node->set_value(prop_indentation_guides, "\r (CR)");
                    break;

                case 2:
                default:
                    node->set_value(prop_indentation_guides, "\n (LF)");
                    break;
            }
        }
        return true;
    }
    else if (name == "display eol markers")
    {
        if (object["m_value"].GetBool())
        {
            node->set_value(prop_view_eol, true);
        }
        return true;
    }
    else if (name == "lexer")
    {
        if (object["m_selection"].IsNumber())
        {
            auto items = GetStringVector(object["m_options"]);
            size_t index = object["m_selection"].GetInt();
            if (index < items.size())
            {
                node->set_value(prop_stc_lexer, items[index].data() + (sizeof("wxSTC_LEX_") - 1));
            }
        }
        return true;
    }
    else if (name.find("keywords set") != std::string::npos)
    {
        // We don't currently support keyword sets since fully supporting them would require
        // processing every possible Lexer to figure out what constants to use.
        return true;
    }
    return false;
}

GenEnum::GenName rapidjson::GetGenName(const Value& value)
{
    ASSERT(value.IsInt())
    if (auto result = map_id_generator.find(value.GetInt()); result != map_id_generator.end())
    {
        return result->second;
    }
    return gen_unknown;
}

const Value& rapidjson::FindObject(const char* key, std::string_view value,
                                   const rapidjson::Value& array)
{
    ASSERT(array.IsArray())
    for (const auto& iter: array.GetArray())
    {
        if (iter.IsObject())
        {
            if (auto& pair = iter[key]; pair.IsString())
            {
                if (wxue::is_sameas(pair.GetString(), value))
                    return iter;
            }
        }
    }

    return empty_value;
}

const Value& rapidjson::FindValue(const rapidjson::Value& object, const char* key)
{
    if (auto result = object.FindMember(key); result != object.MemberEnd())
    {
        return result->value;
    }
    return empty_value;
}

std::string rapidjson::ConvertColour(const rapidjson::Value& colour)
{
    std::string result = "";
    if (colour.IsString())
    {
        std::string_view clr_string = colour.GetString();
        if (!clr_string.starts_with("Default"))
        {
            if (clr_string[0] == '(')
            {
                result = clr_string.substr(1);
                result.pop_back();
            }
            else if (colour.GetString()[0] == '#')
            {
                wxColour clr(wxString::FromUTF8(clr_string.data(), clr_string.size()));
                result = std::format("{},{},{}", clr.Red(), clr.Green(), clr.Blue());
            }
            else if (clr_string.starts_with("wx"))
            {
                result = clr_string;
            }
            else
            {
                if (auto colour_pair = map_sys_colour_pair.find(clr_string);
                    colour_pair != map_sys_colour_pair.end())
                {
                    result = colour_pair->second;
                }
            }
        }
    }
    return result;
}

std::string_view rapidjson::GetSelectedString(const rapidjson::Value& object)
{
    if (auto& sel_value = FindValue(object, "m_selection"); sel_value.IsInt())
    {
        auto sel = sel_value.GetUint();
        if (auto& array = FindValue(object, "m_options"); array.IsArray() && array.Size() > sel)
        {
            return array[sel].GetString();
        }
    }
    return {};
}

std::vector<std::string> rapidjson::GetStringVector(const rapidjson::Value& array)
{
    ASSERT(array.IsArray())

    std::vector<std::string> items;
    for (const auto& iter: array.GetArray())
    {
        if (iter.IsString())
        {
            items.emplace_back(iter.GetString());
        }
        else
        {
            // This is so that a "m_selection" name will correctly index.
            items.emplace_back("");
        }
    }

    return items;
}
