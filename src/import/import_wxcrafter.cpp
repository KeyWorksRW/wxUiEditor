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

#define RAPIDJSON_HAS_STDSTRING 1
#define RAPIDJSON_ASSERT(x)     ASSERT(x)

#include "rapidjson/rapidjson.h"

#include "import_wxcrafter.h"  // This will include rapidjson/document.h

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
        return ttlib::is_sameas(value.GetString(), str);
    }

    // Converts a m_type numeric id into the equivalent gen_ value. Returns gen_unknown if
    // there is no equivalent.
    GenEnum::GenName GetGenName(const Value& value);

    // Convert a colour value into a string that can be stored in a colour property
    ttlib::cstr ConvertColour(const rapidjson::Value& colour);

    // If object contains m_selection(int) and m_options(array), this will return a pointer
    // to the string in the array
    std::string_view GetSelectedString(const rapidjson::Value& object);

    // If the array contains strings, this will collect them into a vector.
    std::vector<std::string> GetStringVector(const rapidjson::Value& array);

    static const Value empty_value;
}  // namespace rapidjson

using namespace rapidjson;

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "base_generator.h"  // BaseGenerator -- Base widget generator class
#include "mainapp.h"         // App -- Main application class
#include "mainframe.h"       // Main window frame
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "pjtsettings.h"     // ProjectSettings -- Hold data for currently loaded project
#include "utils.h"           // Utility functions that work with properties

extern std::map<int, GenEnum::GenName> g_map_id_generator;
extern std::map<std::string, GenEnum::PropName> g_map_crafter_props;

WxCrafter::WxCrafter() {}

bool WxCrafter::Import(const ttString& filename, bool write_doc)
{
    std::ifstream input(filename.wx_str(), std::ifstream::binary);
    if (!input.is_open())
    {
        wxMessageBox(wxString() << "Cannot open " << filename, "Import wxCrafter project");
        return false;
    }
    std::string buffer(std::istreambuf_iterator<char>(input), {});
    input.close();

    Document document;
    if (document.Parse(buffer).HasParseError())
    {
        wxMessageBox(wxString() << filename << " is not a valid wxCrafter file", "Import wxCrafter project");
        return false;
    }
    if (!document.IsObject())
    {
        wxMessageBox(wxString() << filename << " is not a valid wxCrafter file", "Import wxCrafter project");
        return false;
    }

    m_project = g_NodeCreator.CreateNode(gen_Project, nullptr);

    try
    {
        if (auto& metadata = FindValue(document, "metadata"); metadata.IsObject())
        {
            if (auto& result = FindValue(metadata, "m_useEnum"); result.IsBool())
                m_generate_ids = result.GetBool();

            if (auto& include_files = FindValue(metadata, "m_includeFiles"); include_files.IsArray())
            {
                auto preamble_ptr = m_project->prop_as_raw_ptr(prop_src_preamble);
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
                m_project->prop_set_value(prop_internationalize, internationalize.GetBool());
            if (auto& out_file = FindValue(metadata, "m_outputFileName"); out_file.IsString())
                m_output_name = out_file.GetString();
            if (auto& out_dir = FindValue(metadata, "m_generatedFilesDir"); out_dir.IsString())
                m_project->prop_set_value(prop_base_directory, out_dir.GetString());
        }

        if (auto& windows = FindValue(document, "windows"); windows.IsArray())
        {
            for (auto& iter: windows.GetArray())
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
        wxMessageBox(ttlib::cstr("Internal error: ") << e.what(), "Import wxCrafter project");
        wxMessageBox(wxString("This wxCrafter project file is invalid and cannot be loaded: ") << filename,
                     "Import wxCrafter project");
        return false;
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

    auto gen_name = gen_unknown;
    gen_name = GetGenName(value);
    if (gen_name == gen_unknown)
    {
        MSG_ERROR(ttlib::cstr("Unrecognized window type: ") << value.GetInt())
        m_errors.emplace("Unrecognized window type!");
        return;
    }

    auto new_node = g_NodeCreator.CreateNode(gen_name, m_project.get());
    m_project->Adopt(new_node);

    if (!m_is_output_name_used && m_output_name.size())
    {
        new_node->prop_set_value(prop_base_file, m_output_name);
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
                m_errors.emplace(ttlib::cstr() << "Invalid wxCrafter file -- child of " << map_GenNames.at(gen_name)
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

    auto gen_name = gen_unknown;
    gen_name = GetGenName(value);
    if (gen_name == gen_unknown)
    {
        MSG_ERROR(ttlib::cstr("Unrecognized child type: ") << value.GetInt());
        // m_errors.emplace("Unrecognized child type!");
        return;
    }

    if (gen_name == gen_wxCheckBox)
    {
        if (auto& array = FindValue(object, "m_styles"); array.IsArray())
        {
            for (auto& iter: array.GetArray())
            {
                if (ttlib::is_sameas(iter.GetString(), "wxCHK_3STATE"))
                {
                    gen_name = gen_Check3State;
                    break;
                }
            }
        }
    }

    auto new_node = g_NodeCreator.CreateNode(gen_name, parent);
    if (!new_node)
    {
        m_errors.emplace(ttlib::cstr() << map_GenNames.at(gen_name) << " cannot be a child of " << parent->DeclName());
        return;
    }
    parent->Adopt(new_node);

    if (auto& proportion = FindValue(object, "proportion"); proportion.IsInt() && proportion.GetInt() > 0)
        new_node->prop_set_value(prop_proportion, proportion.GetInt());
    if (auto& border = FindValue(object, "border"); border.IsInt() && border.GetInt() != 5)
        new_node->prop_set_value(prop_border_size, border.GetInt());

    if (auto& gbSpan = FindValue(object, "gbSpan"); gbSpan.IsString() && !IsSame(gbSpan, "1,1"))
    {
        ttlib::cview positions = gbSpan.GetString();
        new_node->prop_set_value(prop_rowspan, positions.atoi());
        positions.moveto_nondigit();
        positions.moveto_digit();
        new_node->prop_set_value(prop_colspan, positions.atoi());
    }
    if (auto& gbPosition = FindValue(object, "gbPosition"); gbPosition.IsString() && !IsSame(gbPosition, "0,0"))
    {
        ttlib::cview positions = gbPosition.GetString();
        new_node->prop_set_value(prop_row, positions.atoi());
        positions.moveto_nondigit();
        positions.moveto_digit();
        new_node->prop_set_value(prop_column, positions.atoi());
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
        if (gen_name == gen_wxStdDialogButtonSizer)
        {
            ProcessStdBtnChildren(new_node.get(), children);
            if (new_node->prop_as_string(prop_alignment).size())
            {
                new_node->prop_set_value(prop_static_line, false);
            }
        }
        else
        {
            for (auto& child: children.GetArray())
            {
                /* code */
                if (!child.IsObject())
                {
                    m_errors.emplace(ttlib::cstr() << "Invalid wxCrafter file -- child of " << map_GenNames.at(gen_name)
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
                ttlib::sview id = GetSelectedString(object);
                if (id.size())
                {
                    // If there is at least one valid id, then clear all of the default settings
                    if (!is_default_cleared)
                    {
                        is_default_cleared = true;
                        node->prop_set_value(prop_OK, false);
                        node->prop_set_value(prop_Cancel, false);
                        node->prop_set_value(prop_default_button, "");
                    }

                    if (id.is_sameas("wxID_OK"))
                    {
                        node->prop_set_value(prop_OK, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->prop_set_value(prop_default_button, "OK");
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                ttlib::cstr function = handler.GetString();
                                function.erase_from('(');
                                node->GetEvent("OKButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id.is_sameas("wxID_YES"))
                    {
                        node->prop_set_value(prop_Yes, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->prop_set_value(prop_default_button, "Yes");
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                ttlib::cstr function = handler.GetString();
                                function.erase_from('(');
                                node->GetEvent("YesButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id.is_sameas("wxID_SAVE"))
                    {
                        node->prop_set_value(prop_Save, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->prop_set_value(prop_default_button, "Save");
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                ttlib::cstr function = handler.GetString();
                                function.erase_from('(');
                                node->GetEvent("SaveButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id.is_sameas("wxID_CLOSE"))
                    {
                        node->prop_set_value(prop_Close, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->prop_set_value(prop_default_button, "Close");
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                ttlib::cstr function = handler.GetString();
                                function.erase_from('(');
                                node->GetEvent("CloseButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id.is_sameas("wxID_CANCEL"))
                    {
                        node->prop_set_value(prop_Cancel, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->prop_set_value(prop_default_button, "Cancel");
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                ttlib::cstr function = handler.GetString();
                                function.erase_from('(');
                                node->GetEvent("CancelButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id.is_sameas("wxID_NO"))
                    {
                        node->prop_set_value(prop_No, true);
                        if (!FindObject("m_label", "Default Button", properties).IsNull())
                            node->prop_set_value(prop_default_button, "No");
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                ttlib::cstr function = handler.GetString();
                                function.erase_from('(');
                                node->GetEvent("NoButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id.is_sameas("wxID_APPLY"))
                    {
                        node->prop_set_value(prop_Apply, true);
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                ttlib::cstr function = handler.GetString();
                                function.erase_from('(');
                                node->GetEvent("ApplyButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id.is_sameas("wxID_HELP"))
                    {
                        node->prop_set_value(prop_Help, true);
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                ttlib::cstr function = handler.GetString();
                                function.erase_from('(');
                                node->GetEvent("HelpButtonClicked")->set_value(function);
                            }
                        }
                    }
                    else if (id.is_sameas("wxID_CONTEXT_HELP"))
                    {
                        node->prop_set_value(prop_ContextHelp, true);
                        if (auto& name = FindValue(iter, "m_events"); name.IsArray() && name.Size())
                        {
                            if (auto& handler = FindValue(name[0], "m_functionNameAndSignature"); handler.IsString())
                            {
                                ttlib::cstr function = handler.GetString();
                                function.erase_from('(');
                                node->GetEvent("ContextHelpButtonClicked")->set_value(function);
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

    auto style = node->get_prop_ptr(prop_style);
    if (style)
        style->set_value("");
    auto win_style = node->get_prop_ptr(prop_window_style);
    if (win_style)
        win_style->set_value("");

    for (auto& iter: array.GetArray())
    {
        auto style_bit = iter.GetString();
        if (style)
        {
            bool bit_found { false };
            for (auto& option: style->GetPropDeclaration()->GetOptions())
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
        }

        if (win_style)
        {
            for (auto& option: win_style->GetPropDeclaration()->GetOptions())
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
                if (auto node_event = node->GetEvent(name.GetString()); node_event)
                {
                    if (auto& handler = event["m_functionNameAndSignature"]; handler.IsString())
                    {
                        ttlib::cview function = handler.GetString();
                        if (auto pos = function.find('('); ttlib::is_found(pos))
                            function.remove_prefix(pos + 1);
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
    if (node->HasProp(prop_alignment))
    {
        if (all_items.find("wxEXPAND") != all_items.end())
        {
            node->prop_set_value(prop_flags, "wxEXPAND");
        }
        else
        {
            auto alignment = node->prop_as_raw_ptr(prop_alignment);

            if (all_items.find("wxALIGN_CENTER") != all_items.end())
            {
                if (alignment->size())
                    *alignment << '|';
                *alignment << "wxALIGN_CENTER";
            }
            else if (all_items.find("wxALIGN_CENTER_HORIZONTAL") != all_items.end())
            {
                if (alignment->size())
                    *alignment << '|';
                *alignment << "wxALIGN_CENTER_HORIZONTAL";
            }
            else if (all_items.find("wxALIGN_CENTER_VERTICAL") != all_items.end())
            {
                if (alignment->size())
                    *alignment << '|';
                *alignment << "wxALIGN_CENTER_VERTICAL";
            }

            if (all_items.find("wxALIGN_RIGHT") != all_items.end())
            {
                if (!alignment->contains("wxALIGN_CENTER"))
                {
                    if (alignment->size())
                        *alignment << '|';
                    *alignment << "wxALIGN_RIGHT";
                }
            }
            else if (all_items.find("wxALIGN_LEFT") != all_items.end())
            {
                if (!alignment->contains("wxALIGN_CENTER"))
                {
                    if (alignment->size())
                        *alignment << '|';
                    *alignment << "wxALIGN_LEFT";
                }
            }
            else if (all_items.find("wxALIGN_TOP") != all_items.end())
            {
                if (!alignment->contains("wxALIGN_CENTER"))
                {
                    if (alignment->size())
                        *alignment << '|';
                    *alignment << "wxALIGN_TOP";
                }
            }
            else if (all_items.find("wxALIGN_BOTTOM") != all_items.end())
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
        if (all_items.find("wxALL") != all_items.end())
        {
            node->prop_set_value(prop_border, "wxALL");
        }
        else
        {
            auto border_ptr = node->prop_as_raw_ptr(prop_border);
            border_ptr->clear();

            if (all_items.find("wxLEFT") != all_items.end())
            {
                if (border_ptr->size())
                    *border_ptr << ',';
                *border_ptr << "wxLEFT";
            }
            if (all_items.find("wxRIGHT") != all_items.end())
            {
                if (border_ptr->size())
                    *border_ptr << ',';
                *border_ptr << "wxRIGHT";
            }
            if (all_items.find("wxTOP") != all_items.end())
            {
                if (border_ptr->size())
                    *border_ptr << ',';
                *border_ptr << "wxTOP";
            }
            if (all_items.find("wxBOTTOM") != all_items.end())
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
        ttlib::cstr name;
        if (value["m_label"].IsString())
        {
            name = value["m_label"].GetString();
            if (name.back() == ':')
                name.pop_back();
            name.MakeLower();
            auto prop_name = FindProp(name);
            if (prop_name == prop_unknown)
            {
                if (auto result = g_map_crafter_props.find(name); result != g_map_crafter_props.end())
                {
                    prop_name = result->second;
                }
                else
                {
                    if (name.is_sameas("name"))
                    {
                        prop_name = (node->IsForm() ? prop_class_name : prop_var_name);
                    }
                    else if (node->isGen(gen_wxStyledTextCtrl) && ProcessedScintillaProperty(node, iter))
                    {
                        continue;
                    }

                    else if (name.is_sameas("centre"))
                    {
                        if (value["m_selection"].IsNumber())
                        {
                            switch (value["m_selection"].GetInt())
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
                    else if (name.is_sameas("construct the dropdown menu"))
                    {
                        if (node->isGen(gen_tool))
                        {
                            if (auto& tool_kind = FindValue(value, "m_value"); tool_kind.IsBool())
                            {
                                if (tool_kind.GetBool())
                                    node->prop_set_value(prop_kind, "wxITEM_DROPDOWN");
                            }
                        }
                    }
                    else if (name.is_sameas("gradient start"))
                    {
                        if (auto& colour = FindValue(value, "colour"); colour.IsString())
                        {
                            node->prop_set_value(prop_start_colour, ConvertColour(colour));
                            continue;
                        }
                    }
                    else if (name.is_sameas("gradient end"))
                    {
                        if (auto& colour = FindValue(value, "colour"); colour.IsString())
                        {
                            node->prop_set_value(prop_end_colour, ConvertColour(colour));
                            continue;
                        }
                    }
                    else if (name.is_sameas("bitmap file"))
                    {
                        ProcessBitmapPropety(node, value);
                        continue;
                    }

                    else if (name.is_sameprefix("bitmap file ("))
                    {
                        continue;  // These are different icon sizes
                    }
                    else if (name.is_sameprefix("bitmap file ("))
                    {
                        continue;  // These are different icon sizes
                    }
                    else if (name.is_sameas("auto complete directories") || name.is_sameas("auto complete files"))
                    {
                        // [KeyWorks - 12-23-2021]
                        // These are only valid on Windows -- using them means the app will not work correctly on other
                        // platforms. Since the user can add them to their derived class, or even in a lambda OnInit event
                        // handler, I don't see a reason to support them.
                        continue;
                    }
                    else if (name.is_sameas("disabled-bitmap file"))
                    {
                        // Currently we don't support this.
                        continue;
                    }
                    else if (name.is_sameas("focused"))
                    {
                        // Currently we don't support this.
                        continue;
                    }
                    else if (name.is_sameas("virtual folder"))
                        continue;  // this doesn't apply to wxUiEditor
                    else
                    {
                        MSG_WARNING(ttlib::cstr("Unknown property: \"") << value["m_label"].GetString() << '"');
                        continue;
                    }
                }
            }

            if (prop_name == prop_background_colour || prop_name == prop_foreground_colour)
            {
                if (auto& colour = FindValue(value, "colour"); colour.IsString())
                {
                    node->prop_set_value(prop_name, ConvertColour(colour));
                }
            }
            else if (prop_name == prop_id)
            {
                if (auto& setting = FindValue(value, "m_winid"); setting.IsString())
                {
                    node->prop_set_value(prop_name, setting.GetString());
                }
            }
            else if (prop_name == prop_selection && !FindValue(value, "m_value").IsNull())
            {
                auto& setting = FindValue(value, "m_value");
                // This is a bug in version 2.9 of wxCrafter -- the value should be an int, not a string. We add the GetInt()
                // variant in case they ever fix it.
                if (setting.IsString())
                    node->prop_set_value(prop_selection_int, FindValue(value, "m_value").GetString());
                else if (setting.IsString())
                    node->prop_set_value(prop_selection_int, FindValue(value, "m_value").GetInt());
            }
            else if (prop_name == prop_orientation)
            {
                if (auto& setting = value["m_selection"]; setting.IsInt())
                {
                    node->prop_set_value(prop_orientation, setting.GetInt() == 0 ? "wxVERTICAL" : "wxHORIZONTAL");
                }
            }
            else if (prop_name == prop_value)
            {
                if (auto& setting = FindValue(value, "m_value"); !setting.IsNull())
                {
                    if (node->isGen(gen_wxSpinCtrl))
                        node->prop_set_value(prop_initial, setting.GetString());
                    else if (node->isGen(gen_wxFilePickerCtrl))
                        node->prop_set_value(prop_initial_path, setting.GetString());
                    else if (node->isGen(gen_wxGauge))
                        node->prop_set_value(prop_position, setting.GetString());
                    else if (node->HasProp(prop_value))
                    {
                        node->prop_set_value(prop_name, setting.GetString());
                    }
                    else
                    {
                        MSG_ERROR(ttlib::cstr("Json sets value, but ")
                                  << map_GenNames[node->gen_name()] << " doesn't support that property!");
                    }
                }
            }
            else if (prop_name == prop_contents)
            {
                if (auto& setting = FindValue(value, "m_value"); setting.IsString())
                {
                    ttlib::multistr contents(setting.GetString(), ';');
                    auto str_ptr = node->get_prop_ptr(prop_contents)->as_raw_ptr();
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
            }
            else if (prop_name == prop_stc_lexer)
            {
                ProcessedScintillaProperty(node, iter);
            }
            else if (prop_name != prop_unknown)
            {
                if (auto& prop_value = FindValue(value, "m_value"); !prop_value.IsNull())
                {
                    if (prop_value.IsBool())
                        node->prop_set_value(prop_name, prop_value.GetBool());
                    else
                    {
                        ttlib::cview val = prop_value.GetString();
                        if (val.is_sameas("-1,-1") &&
                            (prop_name == prop_size || prop_name == prop_min_size || prop_name == prop_pos))
                        {
                            continue;  // Don't set if it is a default value
                        }
                        else if (prop_name == prop_message)
                        {
                            auto escape_removal = ConvertEscapeSlashes(val);
                            node->prop_set_value(prop_name, escape_removal);
                        }
                        else
                        {
                            node->prop_set_value(prop_name, val);
                        }
                    }
                }
            }
        }
    }
}

void WxCrafter::ProcessBitmapPropety(Node* node, const Value& object)
{
    if (ttlib::cview path = object["m_path"].GetString(); path.size())
    {
        ttlib::cstr bitmap;
        if (path.is_sameprefix("wxART"))
        {
            ttlib::multiview parts(path, ',');
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
            if (node->HasProp(prop_bitmap))
                node->prop_set_value(prop_bitmap, bitmap);
        }
        else if (IsSame(object["m_label"], "Disabled-Bitmap File"))
        {
            if (node->HasProp(prop_disabled_bmp))
                node->prop_set_value(prop_disabled_bmp, bitmap);
        }
    }
}

bool WxCrafter::ProcessedScintillaProperty(Node* node, const Value& object)
{
    // wxCrafter hard-codes margin numbers. line:0, symbol:2, separator:3, fold:4,

    ttlib::cstr name = object["m_label"].GetString();
    name.MakeLower();
    if (name == "fold margin")
    {
        if (object["m_value"].GetBool())
        {
            node->prop_set_value(prop_fold_margin, "4");
            node->prop_set_value(prop_fold_width, "16");
        }
        return true;
    }
    else if (name == "line number margin")
    {
        if (object["m_value"].GetBool())
        {
            node->prop_set_value(prop_line_margin, "0");
            node->prop_set_value(prop_line_digits, "5");
        }
        return true;
    }
    else if (name == "separator margin")
    {
        if (object["m_value"].GetBool())
        {
            node->prop_set_value(prop_separator_margin, "3");
            node->prop_set_value(prop_separator_width, 1);
        }
        return true;
    }
    else if (name == "symbol margin")
    {
        if (object["m_value"].GetBool())
        {
            node->prop_set_value(prop_symbol_margin, "2");
            node->prop_set_value(prop_symbol_mouse_sensitive, true);
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
                    node->prop_set_value(prop_stc_wrap_mode, "word");
                    break;

                case 2:
                    node->prop_set_value(prop_stc_wrap_mode, "character");
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
                    node->prop_set_value(prop_indentation_guides, "real");
                    break;

                case 2:
                    node->prop_set_value(prop_indentation_guides, "forward");
                    break;

                case 3:
                    node->prop_set_value(prop_indentation_guides, "both");
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
                    node->prop_set_value(prop_eol_mode, "\r\n (CR/LF)");
                    break;

                case 1:
                    node->prop_set_value(prop_indentation_guides, "\r (CR)");
                    break;

                case 2:
                default:
                    node->prop_set_value(prop_indentation_guides, "\n (LF)");
                    break;
            }
        }
        return true;
    }
    else if (name == "display eol markers")
    {
        if (object["m_value"].GetBool())
        {
            node->prop_set_value(prop_view_eol, true);
        }
        return true;
    }
    else if (name == "lexer")
    {
        if (object["m_selection"].IsNumber())
        {
            auto items = GetStringVector(object["m_options"]);
            auto index = object["m_selection"].GetInt();
            if (index < items.size())
            {
                node->prop_set_value(prop_stc_lexer, items[index].data() + (sizeof("wxSTC_LEX_") - 1));
            }
        }
        return true;
    }
    else if (name.contains("keywords set"))
    {
        // We don't currently support keyword sets since fully supporting them would require processing every possible Lexer
        // to figure out what constants to use.
        return true;
    }
    return false;
}

GenEnum::GenName rapidjson::GetGenName(const Value& value)
{
    ASSERT(value.IsInt())
    if (auto result = g_map_id_generator.find(value.GetInt()); result != g_map_id_generator.end())
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
                if (ttlib::is_sameas(pair.GetString(), value))
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

ttlib::cstr rapidjson::ConvertColour(const rapidjson::Value& colour)
{
    ttlib::cstr result;
    if (colour.IsString() && !ttlib::is_sameprefix(colour.GetString(), "Default"))
    {
        if (colour.GetString()[0] == '(')
        {
            result = colour.GetString() + 1;
            result.pop_back();
        }
        else if (auto colour_pair = g_sys_colour_pair.find(colour.GetString()); colour_pair != g_sys_colour_pair.end())
            result = colour_pair->second;
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
    return nullptr;
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
