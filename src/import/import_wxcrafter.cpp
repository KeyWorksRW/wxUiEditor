/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxCrafter project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iostream>
#include <set>

#include <frozen/map.h>

#include <wx/mstream.h>  // Memory stream classes

#define RAPIDJSON_HAS_STDSTRING 1
#define RAPIDJSON_ASSERT(x)     ASSERT(x)

#include "rapidjson/rapidjson.h"

#include "import_wxcrafter.h"  // This will include rapidjson/document.h

#include "base_generator.h"  // BaseGenerator -- Base widget generator class
#include "dlg_msgs.h"        // wxMessageDialog dialogs
#include "font_prop.h"       // FontProperty class
#include "gen_enums.h"       // Enumerations for generators
#include "mainframe.h"       // Main window frame
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "utils.h"           // Utility functions that work with properties

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
        return tt::is_sameas(value.GetString(), str);
    }

    // Converts a m_type numeric id into the equivalent gen_ value. Returns gen_unknown if
    // there is no equivalent.
    GenEnum::GenName GetGenName(const Value& value);

    // Convert a colour value into a string that can be stored in a colour property
    tt_string ConvertColour(const rapidjson::Value& colour);

    // If object contains m_selection(int) and m_options(array), this will return a pointer
    // to the string in the array
    std::string_view GetSelectedString(const rapidjson::Value& object);

    // If the array contains strings, this will collect them into a vector.
    std::vector<std::string> GetStringVector(const rapidjson::Value& array);

    static const Value empty_value;
}  // namespace rapidjson

using namespace rapidjson;

WxCrafter::WxCrafter() {}

bool WxCrafter::Import(const tt_string& filename, bool write_doc)
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

    m_project = NodeCreation.createNode(gen_Project, nullptr).first;

    try
    {
        if (auto& metadata = FindValue(document, "metadata"); metadata.IsObject())
        {
            if (auto& result = FindValue(metadata, "m_useEnum"); result.IsBool())
            {
                // wxCrafter is project wide, wxUiEditor is per-form.
                m_generate_ids = result.GetBool();
            }

            if (auto& include_files = FindValue(metadata, "m_includeFiles"); include_files.IsArray())
            {
                auto preamble_ptr = m_project->getPropValuePtr(prop_src_preamble);
                for (auto& iter: include_files.GetArray())
                {
                    if (iter.IsString())
                    {
                        if (preamble_ptr->size())
                            *preamble_ptr << "@@";
                        *preamble_ptr << "#include \"" << iter.GetString() << "\"";
                    }
                }
            }

            if (auto& internationalize = FindValue(metadata, "m_useUnderscoreMacro"); internationalize.IsBool())
                m_project->set_value(prop_internationalize, internationalize.GetBool());
            if (auto& out_file = FindValue(metadata, "m_outputFileName"); out_file.IsString())
                m_output_name = out_file.GetString();
            if (auto& out_dir = FindValue(metadata, "m_generatedFilesDir"); out_dir.IsString())
                m_project->set_value(prop_base_directory, out_dir.GetString());
        }

        if (auto& windows = FindValue(document, "windows"); windows.IsArray())
        {
            for (auto& iter: windows.GetArray())
            {
                ProcessForm(iter);
            }

            if (write_doc)
                m_project->createDoc(m_docOut);
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
        tt_string errMsg("Not everything in the wxCrafter project could be converted:\n\n");
        MSG_ERROR(tt_string() << "------  " << m_importProjectFile.filename() << "------");
        for (auto& iter: m_errors)
        {
            MSG_ERROR(iter);
            errMsg << iter << '\n';
        }
        wxMessageDialog dlg(nullptr, errMsg, "Import wxCrafter project", wxICON_WARNING | wxOK);
        dlg.ShowModal();
    }

    return true;
}

void WxCrafter::ProcessForm(const Value& form)
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
        m_errors.emplace("Invalid wxCrafter file -- top level window is missing a numeric m_type key to indicate what type "
                         "of window it is.");
        return;
    }

    auto getGenName = gen_unknown;
    getGenName = GetGenName(value);
    if (getGenName == gen_unknown)
    {
        MSG_ERROR(tt_string("Unrecognized window type: ") << value.GetInt());
        m_errors.emplace("Unrecognized window type!");
        return;
    }

    auto new_node = NodeCreation.createNode(getGenName, m_project.get()).first;
    m_project->adoptChild(new_node);

    if (!m_generate_ids)
    {
        new_node->set_value(prop_generate_ids, false);
    }

    if (!m_is_output_name_used && m_output_name.size())
    {
        new_node->set_value(prop_base_file, m_output_name);
        m_is_output_name_used = true;
    }

    if (auto& array = form["m_properties"]; array.IsArray())
        ProcessProperties(new_node.get(), array);
    if (auto& array = form["m_styles"]; array.IsArray())
        ProcessStyles(new_node.get(), array);
    if (auto& array = form["m_events"]; array.IsArray())
        ProcessEvents(new_node.get(), array);

    if (auto& children = form["m_children"]; children.IsArray())
    {
        for (auto& child: children.GetArray())
        {
            if (!child.IsObject())
            {
                m_errors.emplace(tt_string() << "Invalid wxCrafter file -- child of " << map_GenNames.at(getGenName)
                                             << " is not a JSON object.");
                continue;
            }

            ProcessChild(new_node.get(), child);
        }
    }
}

void WxCrafter::ProcessChild(Node* parent, const Value& object)
{
    auto& value = object["m_type"];
    if (!value.IsNumber())
    {
        m_errors.emplace("Invalid wxCrafter file -- child is missing a numeric m_type key to indicate what type "
                         "of child it is.");
        return;
    }

    auto getGenName = gen_unknown;
    getGenName = GetGenName(value);
    if (getGenName == gen_unknown)
    {
        if (value.GetInt() == 4414)
        {
            // This is a column header for a wxListCtrl
            if (parent->isGen(gen_wxListView))
            {
                tt_string cur_headers = parent->as_string(prop_column_labels);
                if (auto& properties = object["m_properties"]; properties.IsArray())
                {
                    for (auto& iter: properties.GetArray())
                    {
                        if (iter.IsObject())
                        {
                            // TODO: [KeyWorks - 01-10-2022] A width can also be specified -- wxUE doesn't currently support
                            // that, but when it does, it should be processed here as well.
                            if (auto& label_type = FindValue(iter, "m_label");
                                label_type.IsString() && tt::is_sameas(label_type.GetString(), "Name:"))
                            {
                                if (auto& label = FindValue(iter, "m_value"); label.IsString())
                                {
                                    if (cur_headers.size())
                                        cur_headers << ' ';
                                    cur_headers << '"' << label.GetString() << '"';
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
            // wxCrafter puts a bogus window between collapsible pane and the actual content. We simply grab it's children
            // and apply it directly (which is also the way wxCrafter generates the code for this).
            //
            // WARNING! wxCrafter allows multiple "panes", but only the last one is actually used. We use the first "pane"
            // rather than the last "pane".

            if (auto& children = FindValue(object, "m_children"); children.IsArray())
            {
                if (getGenName == gen_wxStdDialogButtonSizer)
                {
                    ProcessStdBtnChildren(parent, children);
                    if (parent->as_string(prop_alignment).size())
                    {
                        parent->set_value(prop_static_line, false);
                    }
                }
                else
                {
                    for (auto& child: children.GetArray())
                    {
                        /* code */
                        if (!child.IsObject())
                        {
                            m_errors.emplace(tt_string() << "Invalid wxCrafter file -- child of "
                                                         << map_GenNames.at(getGenName) << " is not a JSON object.");
                            continue;
                        }

                        ProcessChild(parent, child);
                    }
                }
            }
            return;
        }

        MSG_ERROR(tt_string("Unrecognized child type: ") << value.GetInt());
        // m_errors.emplace("Unrecognized child type!");
        return;
    }

    if (getGenName == gen_wxCheckBox)
    {
        if (auto& array = FindValue(object, "m_styles"); array.IsArray())
        {
            for (auto& iter: array.GetArray())
            {
                if (tt::is_sameas(iter.GetString(), "wxCHK_3STATE"))
                {
                    getGenName = gen_Check3State;
                    break;
                }
            }
        }
    }

    if (getGenName == gen_tool && parent->isGen(gen_wxAuiToolBar))
    {
        getGenName = gen_auitool;
    }

    auto new_node = NodeCreation.createNode(getGenName, parent).first;
    if (!new_node)
    {
        m_errors.emplace(tt_string() << map_GenNames.at(getGenName) << " cannot be a child of " << parent->declName());
        return;
    }
    parent->adoptChild(new_node);
    if (value == 4404)  // Originally, this was a wxBitmapButton
    {
        new_node->set_value(prop_style, "wxBU_EXACTFIT");
        new_node->set_value(prop_label, "");
    }

    if (auto& proportion = FindValue(object, "proportion"); proportion.IsInt() && proportion.GetInt() > 0)
        new_node->set_value(prop_proportion, proportion.GetInt());
    if (auto& border = FindValue(object, "border"); border.IsInt() && border.GetInt() != 5)
        new_node->set_value(prop_border_size, border.GetInt());

    if (auto& gbSpan = FindValue(object, "gbSpan"); gbSpan.IsString() && !IsSame(gbSpan, "1,1"))
    {
        tt_string_view positions = gbSpan.GetString();
        new_node->set_value(prop_rowspan, positions.atoi());
        positions.moveto_nondigit();
        positions.moveto_digit();
        new_node->set_value(prop_colspan, positions.atoi());
    }
    if (auto& gbPosition = FindValue(object, "gbPosition"); gbPosition.IsString() && !IsSame(gbPosition, "0,0"))
    {
        tt_string_view positions = gbPosition.GetString();
        new_node->set_value(prop_row, positions.atoi());
        positions.moveto_nondigit();
        positions.moveto_digit();
        new_node->set_value(prop_column, positions.atoi());
    }

    if (auto& array = FindValue(object, "m_sizerFlags"); array.IsArray())
        ProcessSizerFlags(new_node.get(), array);
    if (auto& array = FindValue(object, "m_properties"); array.IsArray())
        ProcessProperties(new_node.get(), array);
    if (auto& array = FindValue(object, "m_styles"); array.IsArray())
        ProcessStyles(new_node.get(), array);
    if (auto& array = FindValue(object, "m_events"); array.IsArray())
        ProcessEvents(new_node.get(), array);

    if (auto& children = FindValue(object, "m_children"); children.IsArray())
    {
        if (getGenName == gen_wxPropertyGridManager)
        {
            // We always supply one page even if it's empty so that wxPropertyGridManager::Clear() will work correctly
            getGenName = gen_propGridPage;
            auto child_node = NodeCreation.createNode(getGenName, new_node.get()).first;
            if (!child_node)
            {
                m_errors.emplace(tt_string()
                                 << map_GenNames.at(getGenName) << " cannot be a child of " << new_node->declName());
                return;
            }
            new_node->adoptChild(child_node);
            new_node = child_node;
        }

        if (getGenName == gen_wxStdDialogButtonSizer)
        {
            ProcessStdBtnChildren(new_node.get(), children);
            if (new_node->as_string(prop_alignment).size())
            {
                new_node->set_value(prop_static_line, false);
            }
        }
        else
        {
            for (auto& child: children.GetArray())
            {
                /* code */
                if (!child.IsObject())
                {
                    m_errors.emplace(tt_string() << "Invalid wxCrafter file -- child of " << map_GenNames.at(getGenName)
                                                 << " is not a JSON object.");
                    continue;
                }

                ProcessChild(new_node.get(), child);
            }
        }
    }
}

void WxCrafter::ProcessStdBtnChildren(Node* node, const Value& array)
{
    bool is_default_cleared { false };
    for (auto& iter: array.GetArray())
    {
        if (auto& properties = FindValue(iter, "m_properties"); properties.IsArray())
        {
            if (auto& object = FindObject("m_label", "ID:", properties); !object.IsNull())
            {
                tt_string_view id = GetSelectedString(object);
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

                    if (id.is_sameas("wxID_OK"))
                    {
                        node->set_value(prop_OK, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->set_value(prop_default_button, "OK");
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                tt_string function = handler.GetString();
                                function.erase_from('(');
                                node->getEvent("OKButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id.is_sameas("wxID_YES"))
                    {
                        node->set_value(prop_Yes, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->set_value(prop_default_button, "Yes");
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                tt_string function = handler.GetString();
                                function.erase_from('(');
                                node->getEvent("YesButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id.is_sameas("wxID_SAVE"))
                    {
                        node->set_value(prop_Save, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->set_value(prop_default_button, "Save");
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                tt_string function = handler.GetString();
                                function.erase_from('(');
                                node->getEvent("SaveButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id.is_sameas("wxID_CLOSE"))
                    {
                        node->set_value(prop_Close, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->set_value(prop_default_button, "Close");
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                tt_string function = handler.GetString();
                                function.erase_from('(');
                                node->getEvent("CloseButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id.is_sameas("wxID_CANCEL"))
                    {
                        node->set_value(prop_Cancel, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->set_value(prop_default_button, "Cancel");
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                tt_string function = handler.GetString();
                                function.erase_from('(');
                                node->getEvent("CancelButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id.is_sameas("wxID_NO"))
                    {
                        node->set_value(prop_No, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->set_value(prop_default_button, "No");
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                tt_string function = handler.GetString();
                                function.erase_from('(');
                                node->getEvent("NoButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id.is_sameas("wxID_APPLY"))
                    {
                        node->set_value(prop_Apply, true);
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                tt_string function = handler.GetString();
                                function.erase_from('(');
                                node->getEvent("ApplyButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id.is_sameas("wxID_HELP"))
                    {
                        node->set_value(prop_Help, true);
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                tt_string function = handler.GetString();
                                function.erase_from('(');
                                node->getEvent("HelpButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id.is_sameas("wxID_CONTEXT_HELP"))
                    {
                        node->set_value(prop_ContextHelp, true);
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                tt_string function = handler.GetString();
                                function.erase_from('(');
                                node->getEvent("ContextHelpButtonClicked")->set_value(function);
                            }
                        }
                    }
                }
            }
        }
    }
}

void WxCrafter::ProcessStyles(Node* node, const Value& array)
{
    // Caution: any of these property options could be a null ptr

    auto style = node->getPropPtr(prop_style);

    if (style)
    {
        // Some styles will have been specifically set when other properties were processed, so
        // we need to be careful about clearing all styles.
        if (!node->isGen(gen_wxButton) && !node->isGen(gen_wxTextCtrl))
        {
            style->set_value("");
        }
    }
    auto win_style = node->getPropPtr(prop_window_style);
    if (win_style)
        win_style->set_value("");

    for (auto& iter: array.GetArray())
    {
        auto style_bit = iter.GetString();
        if (style)
        {
            bool bit_found { false };
            for (auto& option: style->getPropDeclaration()->getOptions())
            {
                if (option.name == style_bit)
                {
                    if (style->get_value().size())
                        style->get_value() << '|';
                    style->get_value() << style_bit;
                    bit_found = true;
                    break;
                }
            }
            if (bit_found)
                continue;
            else if (node->isGen(gen_wxRadioBox))
            {
                if (tt::is_sameas(style_bit, "wxRA_SPECIFY_ROWS"))
                {
                    node->set_value(prop_style, "rows");
                }
                continue;
            }
        }

        if (win_style)
        {
            for (auto& option: win_style->getPropDeclaration()->getOptions())
            {
                if (option.name == style_bit)
                {
                    if (win_style->get_value().size())
                        win_style->get_value() << '|';
                    win_style->get_value() << style_bit;
                    break;
                }
            }
        }
    }
}

void WxCrafter::ProcessEvents(Node* node, const Value& array)
{
    for (auto& iter: array.GetArray())
    {
        if (auto& event = iter; event.IsObject())
        {
            if (auto& name = event["m_eventName"]; name.IsString())
            {
                auto node_event = node->getEvent(GetCorrectEventName(name.GetString()));
                if (!node_event)
                {
                    tt_string modified_name(name.GetString());
                    modified_name.Replace("_COMMAND", "");
                    node_event = node->getEvent(GetCorrectEventName(modified_name));
                    if (!node_event)
                    {
                        auto pos = modified_name.find_last_of('_');
                        if (tt::is_found(pos))
                        {
                            modified_name.erase(pos);
                            node_event = node->getEvent(GetCorrectEventName(modified_name));
                        }
                    }
                }

                if (node_event)
                {
                    if (auto& handler = event["m_functionNameAndSignature"]; handler.IsString())
                    {
                        tt_string function = handler.GetString();
                        function.erase_from('(');
                        node_event->set_value(function);
                    }
                }
            }
        }
    }
}

void WxCrafter::ProcessSizerFlags(Node* node, const Value& array)
{
    std::set<std::string> all_items;
    for (auto& iter: array.GetArray())
    {
        all_items.insert(std::move(iter.GetString()));
    }

    // If the node has porp_alignment, then it will also have prop_borders and prop_flags
    if (node->hasProp(prop_alignment))
    {
        if (all_items.contains("wxEXPAND"))
        {
            node->set_value(prop_flags, "wxEXPAND");
        }
        else
        {
            auto alignment = node->getPropValuePtr(prop_alignment);

            if (all_items.contains("wxALIGN_CENTER"))
            {
                if (alignment->size())
                    *alignment << '|';
                *alignment << "wxALIGN_CENTER";
            }
            else if (all_items.contains("wxALIGN_CENTER_HORIZONTAL"))
            {
                if (alignment->size())
                    *alignment << '|';
                *alignment << "wxALIGN_CENTER_HORIZONTAL";
            }
            else if (all_items.contains("wxALIGN_CENTER_VERTICAL"))
            {
                if (alignment->size())
                    *alignment << '|';
                *alignment << "wxALIGN_CENTER_VERTICAL";
            }

            if (all_items.contains("wxALIGN_RIGHT"))
            {
                if (!alignment->contains("wxALIGN_CENTER"))
                {
                    if (alignment->size())
                        *alignment << '|';
                    *alignment << "wxALIGN_RIGHT";
                }
            }
            else if (all_items.contains("wxALIGN_LEFT"))
            {
                if (!alignment->contains("wxALIGN_CENTER"))
                {
                    if (alignment->size())
                        *alignment << '|';
                    *alignment << "wxALIGN_LEFT";
                }
            }
            else if (all_items.contains("wxALIGN_TOP"))
            {
                if (!alignment->contains("wxALIGN_CENTER"))
                {
                    if (alignment->size())
                        *alignment << '|';
                    *alignment << "wxALIGN_TOP";
                }
            }
            else if (all_items.contains("wxALIGN_BOTTOM"))
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

    if (node->hasProp(prop_border))
    {
        if (all_items.contains("wxALL"))
        {
            node->set_value(prop_border, "wxALL");
        }
        else
        {
            auto border_ptr = node->getPropValuePtr(prop_border);
            border_ptr->clear();

            if (all_items.contains("wxLEFT"))
            {
                if (border_ptr->size())
                    *border_ptr << ',';
                *border_ptr << "wxLEFT";
            }
            if (all_items.contains("wxRIGHT"))
            {
                if (border_ptr->size())
                    *border_ptr << ',';
                *border_ptr << "wxRIGHT";
            }
            if (all_items.contains("wxTOP"))
            {
                if (border_ptr->size())
                    *border_ptr << ',';
                *border_ptr << "wxTOP";
            }
            if (all_items.contains("wxBOTTOM"))
            {
                if (border_ptr->size())
                    *border_ptr << ',';
                *border_ptr << "wxBOTTOM";
            }
        }
    }
}

void WxCrafter::ProcessProperties(Node* node, const Value& array)
{
    for (auto& iter: array.GetArray())
    {
        const auto& value = iter;
        tt_string name;
        if (value["m_label"].IsString())
        {
            name = value["m_label"].GetString();
            if (name.back() == ':')
                name.pop_back();
            name.MakeLower();
            auto prop_name = FindProp(name);
            if (prop_name == prop_unknown)
            {
                prop_name = UnknownProperty(node, value, name);
                if (prop_name == prop_processed)
                    continue;
            }

            KnownProperty(node, value, prop_name);
        }
    }
}

GenEnum::PropName WxCrafter::UnknownProperty(Node* node, const Value& value, tt_string& name)
{
    GenEnum::PropName prop_name = prop_unknown;

    if (auto result = map_crafter_props.find(name); result != map_crafter_props.end())
    {
        prop_name = result->second;
    }
    else
    {
        if (name.is_sameas("name"))
        {
            if (node->isGen(gen_TreeListCtrlColumn))
            {
                prop_name = prop_label;
            }
            else
            {
                prop_name = (node->isForm() ? prop_class_name : prop_var_name);
            }
        }
        else if (node->isGen(gen_wxStyledTextCtrl) && ProcessScintillaProperty(node, value))
        {
            return prop_processed;
        }

        else if (name.is_sameas("centre"))
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
        else if (name.is_sameas("show effect"))
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
                        for (auto& friendly_pair: g_friend_constant)
                        {
                            if (tt::is_sameas(friendly_pair.second, list_effects[index]))
                            {
                                node->set_value(prop_show_effect,
                                                friendly_pair.first.c_str() + friendly_pair.first.find('_') + 1);
                                node->set_value(prop_hide_effect,
                                                friendly_pair.first.c_str() + friendly_pair.first.find('_') + 1);
                                break;
                            }
                        }
                    }
                }
            }
            return prop_processed;
        }

        else if (name.is_sameas("construct the dropdown menu"))
        {
            if (node->isGen(gen_tool) || node->isGen(gen_auitool))
            {
                // wxCrafter seems to always set this to true, ignoring the value for Kind.
                return prop_processed;
            }
        }
        else if (name.is_sameas("gradient start"))
        {
            if (auto& colour = FindValue(value, "colour"); colour.IsString())
            {
                node->set_value(prop_start_colour, ConvertColour(colour));
                return prop_processed;
            }
        }
        else if (name.is_sameas("combobox choices"))
        {
            if (auto& choices = FindValue(value, "m_value"); choices.IsString())
            {
                tt_view_vector mview(choices.GetString(), "\\n");
                tt_string contents;
                for (auto& choice: mview)
                {
                    if (choice.size())
                    {
                        if (contents.size())
                            contents << ' ';
                        contents << '"' << choice << '"';
                    }
                }
                node->set_value(prop_contents, contents);
                return prop_processed;
            }
        }
        else if (name.is_sameas("gradient end"))
        {
            if (auto& colour = FindValue(value, "colour"); colour.IsString())
            {
                node->set_value(prop_end_colour, ConvertColour(colour));
                return prop_processed;
            }
        }
        else if (name.is_sameas("bitmap file"))
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
        else if (name.is_sameas("auto complete directories") || name.is_sameas("auto complete files"))
        {
            // [KeyWorks - 12-23-2021]
            // These are only valid on Windows -- using them means the app will not work correctly on other
            // platforms. Since the user can add them to their derived class, or even in a lambda OnInit event
            // handler, I don't see a reason to support them.
            return prop_processed;
        }
        else if (name.is_sameas("disabled-bitmap file"))
        {
            // Currently we don't support this.
            return prop_processed;
        }
        else if (name.is_sameas("focused"))
        {
            if (auto& setting = FindValue(value, "m_value"); setting.IsBool())
            {
                node->set_value(prop_focus, setting.GetBool());
            }
            return prop_processed;
        }
        else if (name.is_sameas("selected") && node->isGen(gen_BookPage))
        {
            if (auto& setting = FindValue(value, "m_value"); setting.IsBool())
            {
                node->set_value(prop_select, setting.GetBool());
            }
            return prop_processed;
        }
        else if (name.is_sameas("virtual folder"))
            return prop_processed;  // this doesn't apply to wxUiEditor
        else if (name.is_sameas("null page"))
            return prop_processed;  // unused
        else if (name.is_sameas("enable spell checking") && node->hasProp(prop_spellcheck))
        {
            node->set_value(prop_spellcheck, "enabled");
            tt_string style = node->as_string(prop_style);
            if (style.size() && !style.contains("wxTE_RICH2"))
                style << "|wxTE_RICH2";
            else
                style << "wxTE_RICH2";
            node->set_value(prop_style, style);
        }
        else if (name.is_sameas("keep as a class member"))
        {
            if (node->as_string(prop_class_access) == "none")
                node->set_value(prop_class_access, "protected:");
        }
        else if (name.is_sameas("Start the timer"))
        {
            if (auto& setting = FindValue(value, "m_value"); setting.IsBool())
            {
                node->set_value(prop_auto_start, setting.GetBool());
            }
            return prop_processed;
        }
        else if (name.is_sameas("One Shot Timer"))
        {
            if (auto& setting = FindValue(value, "m_value"); setting.IsBool())
            {
                node->set_value(prop_auto_start, setting.GetBool());
            }
            return prop_processed;
        }
        else
        {
            if (!node->isGen(gen_propGridItem))
            {
                // wxCrafter outputs a boatload of empty fields for property grid items
                MSG_WARNING(tt_string("Unknown property: \"") << value["m_label"].GetString() << '"');
            }
            return prop_processed;
        }
    }

    return prop_name;
}

void WxCrafter::KnownProperty(Node* node, const Value& value, GenEnum::PropName prop_name)
{
    if (node->isGen(gen_wxPopupWindow))
    {
        if (prop_name == prop_size || prop_name == prop_minimum_size || prop_name == prop_title)
            return;  // wxCrafter writes these, but doesn't use them (wxUiEditor does support size and minimum_size)
    }

    if (node->isGen(gen_ribbonTool) || node->isGen(gen_ribbonSeparator) || node->isGen(gen_ribbonButton))
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
        else if (prop_name == prop_size || prop_name == prop_minimum_size || prop_name == prop_var_name)
        {
            return;
        }
    }

    if (prop_name == prop_background_colour || prop_name == prop_foreground_colour || prop_name == prop_normal_color ||
        prop_name == prop_visited_color || prop_name == prop_hover_color)
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
        // This is a bug in version 2.9 of wxCrafter -- the value should be an int, not a string. We add the GetInt()
        // variant in case they ever fix it.
        if (setting.IsString())
        {
            tt_string result = setting.GetString();
            if (tt::is_digit(result[0]))
            {
                if (node->hasProp(prop_selection_int))
                    node->set_value(prop_selection_int, result.atoi());
                else if (node->hasProp(prop_selection))
                    node->set_value(prop_selection, result.atoi());
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
            node->set_value(prop_orientation, setting.GetInt() == 0 ? "wxVERTICAL" : "wxHORIZONTAL");
        }
    }
    else if (prop_name == prop_value)
    {
        ValueProperty(node, value);
    }

    else if (prop_name == prop_var_name &&
             (node->isGen(gen_spacer) || node->isGen(gen_toolSeparator) || node->isGen(gen_ribbonGalleryItem)))
    {
        return;
    }
    else if (prop_name == prop_size && node->isGen(gen_spacer))
    {
        tt_view_vector mview(FindValue(value, "m_value").GetString(), ',');
        if (mview.size() > 1)
        {
            node->set_value(prop_width, mview[0].atoi());
            node->set_value(prop_height, mview[1].atoi());
        }
    }
    else if (prop_name == prop_size && node->isGen(gen_wxStdDialogButtonSizer))
    {
        return;  // wxCrafter doesn't use this either, but does generate it
    }

    else if (prop_name == prop_contents)
    {
        if (auto& setting = FindValue(value, "m_value"); setting.IsString())
        {
            if (node->hasProp(prop_contents))
            {
                tt_string_vector contents(setting.GetString(), ';');
                auto str_ptr = node->getPropPtr(prop_contents)->as_raw_ptr();
                str_ptr->clear();  // remove any default string
                for (auto& item: contents)
                {
                    if (item.size())
                    {
                        if (str_ptr->size())
                            *str_ptr << ' ';
                        *str_ptr << '"' << item << '"';
                    }
                }
            }
            else
            {
                // typically this is a wxPGProperty -- we don't support strings for this
            }
        }
    }
    else if (prop_name == prop_stc_lexer)
    {
        ProcessScintillaProperty(node, value);
    }
    else if (prop_name == prop_font)
    {
        ProcessFont(node, value);
    }
    else if (prop_name == prop_statusbar && node->isGen(gen_ribbonButton))
    {
        if (auto& prop_value = FindValue(value, "m_value"); prop_value.IsString())
        {
            node->set_value(prop_help, prop_value.GetString());
        }
    }

    else if (prop_name == prop_kind && (node->isGen(gen_tool) || node->isGen(gen_auitool)))
    {
        tt_string_view tool_kind = GetSelectedString(value);
        if (tool_kind.is_sameas("checkable"))
            node->set_value(prop_kind, "wxITEM_CHECK");
        else if (tool_kind.is_sameas("radio"))
            node->set_value(prop_kind, "wxITEM_RADIO");
        else if (tool_kind.is_sameas("dropdown"))
            node->set_value(prop_kind, "wxITEM_DROPDOWN");
        else
            node->set_value(prop_kind, "wxITEM_NORMAL");
    }
    else if (prop_name == prop_kind && node->isGen(gen_ribbonTool))
    {
        node->set_value(prop_kind, GetSelectedString(value));
    }

    else if (prop_name != prop_unknown)
    {
        if (auto& prop_value = FindValue(value, "m_value"); !prop_value.IsNull())
        {
            if (!node->hasProp(prop_name))
            {
                if (prop_name == prop_min && node->hasProp(prop_minValue))
                    prop_name = prop_minValue;
                else if (prop_name == prop_checked && node->hasProp(prop_pressed))
                    prop_name = prop_pressed;
                else if (prop_name == prop_max && node->hasProp(prop_maxValue))
                    prop_name = prop_maxValue;
                else if (prop_name == prop_tooltip && node->isGen(gen_propGridItem))
                    prop_name = prop_help;
                else if (wxGetApp().isTestingMenuEnabled())
                {
                    if ((prop_value.IsString() && prop_value.GetStringLength()) ||
                        (prop_value.IsBool() && prop_value.GetBool()))
                    {
                        MSG_INFO(tt_string() << node->declName() << " doesn't have a property called "
                                             << GenEnum::map_PropNames[prop_name]);
                    }
                }
            }
            if (prop_value.IsBool())
                node->set_value(prop_name, prop_value.GetBool());
            else if (prop_value.IsInt())
                node->set_value(prop_name, prop_value.GetInt());
            else
            {
                tt_string_view val = prop_value.GetString();
                if (val.is_sameas("-1,-1") &&
                    (prop_name == prop_size || prop_name == prop_min_size || prop_name == prop_pos))
                {
                    return;  // Don't set if it is a default value
                }
                else if (prop_name == prop_message)
                {
                    auto escape_removal = ConvertEscapeSlashes(val);
                    node->set_value(prop_name, escape_removal);
                }
                else
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

void WxCrafter::ValueProperty(Node* node, const Value& value)
{
    if (auto& setting = FindValue(value, "m_value"); !setting.IsNull())
    {
        if (node->isGen(gen_wxSpinCtrl) || node->isGen(gen_wxSpinButton))
        {
            node->set_value(prop_initial, setting.GetString());
        }
        else if (node->isGen(gen_wxFilePickerCtrl))
        {
            node->set_value(prop_initial_path, setting.GetString());
        }
        else if (node->isGen(gen_wxDirPickerCtrl))
        {
            node->set_value(prop_initial_path, setting.GetString());
        }
        else if (node->isGen(gen_wxFontPickerCtrl))
        {
            ProcessFont(node, value);
        }
        else if (node->isGen(gen_wxGauge))
        {
            node->set_value(prop_position, setting.GetString());
        }
        else if (node->isGen(gen_wxComboBox))
        {
            node->set_value(prop_selection_string, setting.GetString());
        }
        else if (node->isGen(gen_wxCheckBox) || node->isGen(gen_wxRadioButton))
        {
            node->set_value(prop_checked, setting.GetBool());
        }
        else if (node->isGen(gen_wxScrollBar))
        {
            node->set_value(prop_position, setting.GetString());
        }
        else if (node->isGen(gen_Check3State))
        {
            if (setting.GetBool())
                node->set_value(prop_initial_state, "wxCHK_CHECKED");
        }
        else if (node->isGen(gen_wxSlider))
        {
            node->set_value(prop_position, setting.GetString());
        }
        else if (node->hasProp(prop_value))
        {
            node->set_value(prop_value, setting.GetString());
        }
        else
        {
            MSG_ERROR(tt_string("Json sets value, but ")
                      << map_GenNames[node->getGenName()] << " doesn't support that property!");
        }
    }
    else if (auto& colour = FindValue(value, "colour"); !colour.IsNull())
    {
        node->set_value(prop_colour, ConvertColour(colour));
    }
}

void WxCrafter::ProcessBitmapPropety(Node* node, const Value& object)
{
    if (tt_string_view path = object["m_path"].GetString(); path.size())
    {
        tt_string bitmap;
        if (path.starts_with("wxART"))
        {
            tt_view_vector parts(path, ',');
            if (parts.size() > 1)
            {
                bitmap << "Art;" << parts[0] << '|' << parts[1] << ";[-1,-1]";
            }
        }
        else
        {
            bitmap << "Embed;" << path << ";[-1,-1]";
        }

        if (IsSame(object["m_label"], "Bitmap File:"))
        {
            if (node->isGen(gen_wxAnimationCtrl))
            {
                node->set_value(prop_animation, bitmap);
            }
            else if (node->hasProp(prop_bitmap))
            {
                node->set_value(prop_bitmap, bitmap);
            }
        }
        else if (IsSame(object["m_label"], "Disabled-Bitmap File"))
        {
            if (node->hasProp(prop_disabled_bmp))
                node->set_value(prop_disabled_bmp, bitmap);
        }
    }
}

// For system fonts, wxCrafter doesn't support size or weigth -- you are limited to italic, bold, and underlined.
// Note that wxCrafter supports ALL of the system fonts, not just wxSYS_DEFAULT_GUI_FONT.
//
// For custom fonts, wxCrafter uses the system font picker, but ignores some of the results. wxCrafter supports integer point
// size, italic, bold, underlines, family and facename.

bool WxCrafter::ProcessFont(Node* node, const Value& object)
{
    if (object.HasMember("m_value"))
    {
        tt_string crafter_str = object["m_value"].GetString();
        if (crafter_str.empty())
            return true;

        FontProperty font_info;
        if (crafter_str.contains("italic"))
            font_info.Italic(wxFONTFLAG_ITALIC);
        if (crafter_str.contains("bold"))
            font_info.Bold(wxFONTFLAG_ITALIC);
        if (crafter_str.contains("underlined"))
            font_info.Underlined();

        if (!crafter_str.starts_with("wxSYS_DEFAULT_GUI_FONT"))
        {
            font_info.setDefGuiFont(false);
            font_info.FaceName("");
            tt_view_vector mstr(crafter_str, ',', tt::TRIM::left);

            if (mstr[0].is_sameas("wxSYS_OEM_FIXED_FONT") || mstr[0].is_sameas("wxSYS_ANSI_FIXED_FONT"))
                font_info.Family(wxFONTFAMILY_TELETYPE);

            if (tt::is_digit(mstr[0][0]))
            {
                font_info.PointSize(mstr[0].atoi());

                if (mstr.size() > 3 && mstr[3] != "default")
                    font_info.Family(font_family_pairs.GetValue(mstr[3]));
                if (mstr.size() > 4 && mstr[4] == "1")
                    font_info.Underlined();
                if (mstr.size() > 5)
                    font_info.FaceName(mstr[5].make_wxString());
            }
        }

        if (node->isGen(gen_wxFontPickerCtrl))
            node->set_value(prop_initial_font, font_info.as_string());
        else
            node->set_value(prop_font, font_info.as_string());
    }
    return true;
}

bool WxCrafter::ProcessScintillaProperty(Node* node, const Value& object)
{
    // wxCrafter hard-codes margin numbers. line:0, symbol:2, separator:3, fold:4,

    tt_string name = object["m_label"].GetString();
    name.MakeLower();
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
    else if (name.contains("keywords set"))
    {
        // We don't currently support keyword sets since fully supporting them would require processing every possible
        // Lexer to figure out what constants to use.
        return true;
    }
    return false;
}

GenEnum::GenName rapidjson::GetGenName(const Value& value)
{
    ASSERT(value.IsInt())
    if (auto result = map_id_generator.find(value.GetInt()); result != map_id_generator.end())
        return result->second;
    else
        return gen_unknown;
}

const Value& rapidjson::FindObject(const char* key, std::string_view value, const rapidjson::Value& array)
{
    ASSERT(array.IsArray())
    for (auto& iter: array.GetArray())
    {
        if (iter.IsObject())
        {
            if (auto& pair = iter[key]; pair.IsString())
            {
                if (tt::is_sameas(pair.GetString(), value))
                    return iter;
            }
        }
    }

    return empty_value;
}

const Value& rapidjson::FindValue(const rapidjson::Value& object, const char* key)
{
    if (auto result = object.FindMember(key); result != object.MemberEnd())
        return result->value;
    else
        return empty_value;
}

tt_string rapidjson::ConvertColour(const rapidjson::Value& colour)
{
    tt_string result;
    if (colour.IsString())
    {
        tt_string_view clr_string = colour.GetString();
        if (!clr_string.starts_with("Default"))
        {
            if (clr_string[0] == '(')
            {
                result = clr_string.substr(1);
                result.pop_back();
            }
            else if (colour.GetString()[0] == '#')
            {
                wxColour clr(clr_string.make_wxString());
                result << clr.Red() << ',' << clr.Green() << ',' << clr.Blue();
            }
            else if (clr_string.starts_with("wx"))
            {
                result = clr_string;
            }
            else
            {
                if (auto colour_pair = map_sys_colour_pair.find(clr_string); colour_pair != map_sys_colour_pair.end())
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
    for (auto& iter: array.GetArray())
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
