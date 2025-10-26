/////////////////////////////////////////////////////////////////////////////
// Purpose:   Load wxUiEditor project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/stc/stc.h>   // A wxWidgets implementation of Scintilla.
#include <wx/utils.h>     // Miscellaneous utilities

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "gen_enums.h"        // Enumerations for generators
#include "image_handler.h"    // ProjectImage class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator class
#include "preferences.h"      // Prefs -- Set/Get wxUiEditor preferences
#include "project_handler.h"  // ProjectHandler class

#include "../pugixml/pugixml.hpp"  // pugixml parser

using namespace GenEnum;

#include "../import/import_dialogblocks.h"  // DialogBlocks -- Import a DialogBlocks project
#include "../import/import_formblder.h"     // FormBuilder -- Import a wxFormBuider project
#include "../import/import_wxcrafter.h"     // WxCrafter -- Import a wxCrafter project
#include "../import/import_wxglade.h"       // WxGlade -- Import a wxGlade file
#include "../import/import_wxsmith.h"       // WxSmith -- Import a wxSmith file
#include "../winres/import_winres.h"        // WinResource -- Parse a Windows resource file
#include "gen_images_list.h"
#include "import_dlg.h"    // ImportDlg -- Dialog to create a new project
#include "node_gridbag.h"  // GridBag -- Create and modify a node containing a wxGridBagSizer

#include "../ui/code_preference_dlg.h"  // CodePreferenceDlg -- Dialog to set code generation preference

#include "../internal/import_panel.h"  // ImportPanel -- Panel to display original imported file

using namespace GenEnum;

bool ProjectHandler::LoadProject(const tt_string& file, bool allow_ui)
{
    pugi::xml_document doc;
    auto result = doc.load_file_string(file);
    if (!result)
    {
        tt_string msg;
        msg << "Parsing error in file: " << file;
        msg << "\nError description: " << result.description();
        msg << "\nError line: " << result.line;
        msg << "\nError column: " << result.column;
#if defined(_DEBUG)
        FAIL_MSG(msg);
#endif
        if (allow_ui)
        {
            wxMessageDialog(wxGetMainFrame()->getWindow(), msg, "Parsing Error",
                            wxOK | wxICON_ERROR)
                .ShowModal();
        }
        return false;
    }

    auto root = doc.first_child();
    if (!tt::is_sameas(root.name(), "wxUiEditorData", tt::CASE::either))
    {
        if (allow_ui)
        {
            wxMessageBox(wxString() << file << " is not a wxUiEditor XML file", "Load Project");
        }
        return false;
    }

    NodeSharedPtr project;

    m_ProjectVersion = root.attribute("data_version").as_int(curSupportedVer);
    m_OriginalProjectVersion = m_ProjectVersion;

    if (m_ProjectVersion > curSupportedVer)
    {
        if (allow_ui)
        {
            if (wxMessageBox("wxUiEditor does not recognize this version of the data file.\n"
                             "You may be able to load the file, but if you then save it you could "
                             "lose data.\n\n"
                             "Do you want to try to open it anyway?",
                             "Unrecognized Version", wxYES_NO) != wxYES)
            {
                return false;
            }
            else
            {
                project = LoadProject(doc, allow_ui);
            }
        }
        else
        {
            return false;
        }
    }

    else if (m_ProjectVersion < minRequiredVer)
    {
        if (!root.child("object") && !root.child("node"))
        {
            if (allow_ui)
            {
                wxMessageBox(wxString()
                             << "The data file " << file << " is invalid and cannot be opened.");
            }
            return false;
        }
        else if (m_ProjectVersion < 11)
        {
            if (allow_ui)
            {
                if (wxMessageBox(
                        tt_string()
                            << "Project version " << m_ProjectVersion / 10 << '.'
                            << m_ProjectVersion % 10
                            << " is not supported.\n\nDo you want to attempt to load it anyway?",
                        "Unsupported Project Version", wxYES_NO) == wxNO)
                {
                    return false;
                }
            }
            else
            {
                return false;
            }

            project = LoadProject(doc, allow_ui);
        }
    }
    else
    {
        if (!root.child("node"))
        {
            if (allow_ui)
            {
                wxMessageBox(wxString()
                             << "The data file " << file << " is invalid and cannot be opened.");
            }
            return false;
        }

        project = LoadProject(doc, allow_ui);
    }

    if (!project)
    {
        ASSERT_MSG(project, tt_string() << "Failed trying to load " << file);

        if (allow_ui)
        {
            wxMessageBox(wxString()
                         << "The project file " << file << " is invalid and cannot be opened.");
        }
        return false;
    }

    if (wxGetApp().isTestingMenuEnabled())
    {
        // This should NOT be necessary if all alignment in the project file has been set correctly.
        // However, it it has not been set correctly, this will correct it and issue a MSG_WARNING
        // about what got fixed.
        RecursiveNodeCheck(project.get());
    }

    // Calling this will also initialize the ImageHandler class
    Project.Initialize(project);
    Project.set_ProjectFile(file);
    ProjectImages.CollectBundles();

    // Imported projects start with an older version so that they pass through the old project
    // fixups.
    if (m_ProjectVersion == ImportProjectVersion)
    {
        m_ProjectVersion = minRequiredVer;
    }

    // We need to ensure any Images List is sorted (in case it's an old project or the user
    // hand-edited the project file)
    img_list::UpdateImagesList(m_ProjectVersion);

    if (allow_ui)
    {
        wxGetFrame().setImportedFlag(false);
        wxGetFrame().FireProjectLoadedEvent();
        if (wxGetApp().isTestingMenuEnabled())
            wxGetFrame().getImportPanel()->SetImportFile(file);

        if (m_isProject_updated || m_ProjectVersion < minRequiredVer)
            wxGetFrame().setModified();
    }
    return true;
}

NodeSharedPtr ProjectHandler::LoadProject(pugi::xml_document& doc, bool allow_ui)
{
    NodeSharedPtr project;
    try
    {
        auto root = doc.first_child();
        if (!root)
        {
            FAIL_MSG("Project does not have a root child.");
            throw std::runtime_error("Invalid project file");
        }

        auto node = root.child("node");
        if (!node)
        {
            FAIL_MSG("Project does not have a \"node\" node.");
            throw std::runtime_error("Invalid project file");
        }
        project = NodeCreation.CreateProjectNode(&node, allow_ui);
    }
    catch (const std::exception& err)
    {
        if (allow_ui)
        {
            MSG_ERROR(err.what());
            wxMessageBox("This wxUiEditor project file is invalid and cannot be loaded.",
                         "Load Project");
        }
    }

    return project;
}

// clang-format off
static const auto lstStdButtonEvents = {

    "OKButtonClicked",
    "YesButtonClicked",
    "SaveButtonClicked",
    "ApplyButtonClicked",
    "NoButtonClicked",
    "CancelButtonClicked",
    "CloseButtonClicked",
    "HelpButtonClicked",
    "ContextHelpButtonClicked",

    "OKButton",
    "YesButton",
    "SaveButton",
    "ApplyButton",
    "NoButton",
    "CancelButton",
    "CloseButton",
    "HelpButton",
    "ContextHelpButton",

};
// clang-format on

#include "utils.h"  // for old style art indices

NodeSharedPtr NodeCreator::CreateNodeFromXml(pugi::xml_node& xml_obj, Node* parent,
                                             bool check_for_duplicates, bool allow_ui)
{
    auto class_name = xml_obj.attribute("class").as_str();
    if (class_name.empty())
        return NodeSharedPtr();

    // This should never be the case, but let's switch it in the off chance it slips through
    if (class_name == "wxListCtrl")
        class_name = "wxListView";

    auto new_node = CreateNode(class_name, parent).first;
    if (!new_node)
    {
        FAIL_MSG(tt_string() << "Invalid project file: could not create " << class_name);
        throw std::runtime_error("Invalid project file");
    }

    for (auto& iter: xml_obj.attributes())
    {
        if (iter.name() == "class")
            continue;

        if (iter.name().starts_with("wxEVT_"))
        {
            if (auto event = new_node->get_Event(iter.name()); event)
            {
                event->set_value(iter.value());
            }
            continue;
        }

        NodeProperty* prop = nullptr;
        if (auto find_prop = rmap_PropNames.find(iter.name()); find_prop != rmap_PropNames.end())
        {
            prop = new_node->get_PropPtr(find_prop->second);

            if (prop)
            {
                auto convert_quoted_array = [&]()
                {
                    // Convert old style wxCheckListBox contents in quotes to new style separated by
                    // semicolons
                    std::vector<tt_string> items;
                    auto view = iter.as_sview().view_substr(0, '"', '"');
                    while (view.size() > 0)
                    {
                        items.emplace_back(view);
                        view = tt::stepover(view.data() + view.size());
                        view = view.view_substr(0, '"', '"');
                    }

                    tt_string value;
                    for (auto& item: items)
                    {
                        if (value.size())
                            value << ';';
                        value << item;
                    }

                    prop->set_value(value);
                    // Conversion from quoted items to semicolon separated items was introduced
                    // in 1.1.1 (project version 18)
                    if (Project.get_ProjectVersion() < 18)
                    {
                        Project.ForceProjectVersion(18);
                    }
                };

                // If there is a mainframe window, then convert dialog units to pixels since
                // starting with wxUiEditor 21 (1.3.0) all positions and sizes are scaled
                // automatically using FromDIP().
                if (Project.get_OriginalProjectVersion() < 21 && allow_ui &&
                    (prop->type() == type_wxSize || prop->type() == type_wxPoint) &&
                    tt::contains(iter.value(), 'd', tt::CASE::either))
                {
                    auto convertToWxSize = [](std::string_view value) -> wxSize
                    {
                        wxSize result { -1, -1 };
                        if (value.size())
                        {
                            tt_view_vector tokens(value, ',');
                            if (tokens.size())
                            {
                                if (tokens[0].size())
                                    result.x = tokens[0].atoi();

                                if (tokens.size() > 1 && tokens[1].size())
                                    result.y = tokens[1].atoi();
                            }
                        }
                        return result;
                    };

                    if (allow_ui)
                    {
                        auto pixel_value = wxGetMainFrame()->getWindow()->ConvertDialogToPixels(
                            convertToWxSize(iter.value()));
                        prop->set_value(pixel_value);
                    }
                    Project.ForceProjectVersion(21);
                    continue;
                }

                // wxUiEditor 1.2.0 mistakenly added both prop_hidden and prop_hide_children.
                // 1.2.1 removes the duplicate prop_hide_children, so this sets prop_hidden to
                // true if prop_hide_children is true.
                else if (prop->get_name() == prop_hide_children &&
                         new_node->is_Gen(gen_wxStaticBoxSizer) && iter.as_bool())
                {
                    new_node->set_value(prop_hidden, true);
                    prop->set_value(false);
                    continue;
                }

                if (prop->type() == type_bool)
                {
                    prop->set_value(iter.as_bool());

                    // wxGTK is the only OS that uses a native wxAnimationCtrl, so setting generic
                    // has no actual effect on other platforms. On wxGTK, you can't just switch to
                    // wxGenericAnimationCtrl, you have to also retrieve wxAnimation from
                    // wxGenericAnimationCtrl -- if you don't, the app will crash. Since this is
                    // only needed to display .ANI files on wxGTK, we remove the generic flag.
                    if (prop->get_name() == prop_use_generic &&
                        new_node->is_Gen(gen_wxAnimationCtrl))
                    {
                        prop->set_value(false);
                    }
                }
                else if (prop->get_name() == prop_contents &&
                         Project.get_OriginalProjectVersion() < 18)
                {
                    if (new_node->is_Gen(gen_wxCheckListBox) && iter.as_sview().size() &&
                        iter.as_sview()[0] == '"')
                    {
                        convert_quoted_array();
                    }
                    else
                    {
                        prop->set_value(iter.as_sview());
                    }
                }
                else if (prop->type() == type_stringlist_semi &&
                         Project.get_OriginalProjectVersion() < 18)
                {
                    if (iter.as_sview().size() && iter.as_sview()[0] == '"')
                    {
                        convert_quoted_array();
                    }
                    else
                    {
                        prop->set_value(iter.as_sview());
                    }
                }

                // Imported projects will be set as version ImportProjectVersion to get the fixups
                // of constant to friendly name, and bit flag conflict resolution.

                else if (Project.get_ProjectVersion() <= ImportProjectVersion)
                {
                    switch (prop->type())
                    {
                        case type_editoption:
                        case type_option:
                            {
                                bool found = false;
                                for (auto& friendly_pair: g_friend_constant)
                                {
                                    if (tt::is_sameas(friendly_pair.second, iter.value()))
                                    {
                                        prop->set_value(friendly_pair.first.substr(
                                            friendly_pair.first.find('_') + 1));
                                        found = true;
                                        break;
                                    }
                                }
                                if (!found)
                                {
                                    prop->set_value(iter.value());
                                }
                            }
                            break;

                        case type_bitlist:
                            {
                                tt_string_vector mstr(iter.value(), '|', tt::TRIM::both);
                                bool found = false;
                                tt_string new_value;
                                for (auto& bit_value: mstr)
                                {
                                    for (auto& friendly_pair: g_friend_constant)
                                    {
                                        if (tt::is_sameas(friendly_pair.second, bit_value))
                                        {
                                            if (new_value.size())
                                            {
                                                new_value << '|';
                                            }

                                            new_value << friendly_pair.first.substr(
                                                friendly_pair.first.find('_') + 1);
                                            found = true;
                                            break;
                                        }
                                    }
                                    if (!found)
                                    {
                                        break;
                                    }
                                }
                                if (found)
                                {
                                    prop->set_value(new_value);
                                }
                                else
                                {
                                    prop->set_value(iter.value());
                                }

                                if (auto gen = new_node->get_Generator(); gen)
                                {
                                    gen->VerifyProperty(prop);
                                }
                            }
                            break;

                        case type_image:
                            {
                                tt_string_vector parts(iter.value(), ';', tt::TRIM::both);
                                if (parts.size() < 3)
                                {
                                    prop->set_value(iter.value());
                                }
                                else
                                {
                                    parts[1].backslashestoforward();
                                    tt_string description(parts[0]);
                                    description << ';' << parts[1];
                                    if (parts[0].starts_with("SVG"))
                                        description << ';' << parts[2];
                                    prop->set_value(description);
                                }
                            }

                        default:
                            prop->set_value(iter.value());
                    }
                }
                else
                {
                    prop->set_value(iter.value());
                }
            }
            else  // property was not found
            {
                // In version 1.3.0, the wxWindow property derived class property names have been
                // replaced with subclass names.
                if (find_prop->second == prop_derived_class)
                {
                    new_node->set_value(prop_subclass, iter.value());
                    Project.set_ProjectUpdated();
                    Project.ForceProjectVersion(curSupportedVer);
                }
                else if (find_prop->second == prop_derived_header)
                {
                    new_node->set_value(prop_subclass_header, iter.value());
                    Project.set_ProjectUpdated();
                    Project.ForceProjectVersion(curSupportedVer);
                }
                else if (find_prop->second == prop_derived_params)
                {
                    new_node->set_value(prop_subclass_params, iter.value());
                    Project.set_ProjectUpdated();
                    Project.ForceProjectVersion(curSupportedVer);
                }

                else if (find_prop->second == prop_base_hdr_includes)
                {
                    new_node->set_value(prop_header_preamble, iter.value());
                    Project.set_ProjectUpdated();
                    Project.ForceProjectVersion(curSupportedVer);
                }
                else if (find_prop->second == prop_base_src_includes)
                {
                    new_node->set_value(prop_source_preamble, iter.value());
                    Project.set_ProjectUpdated();
                    Project.ForceProjectVersion(curSupportedVer);
                }
            }
        }
        else
        {
            bool is_event = false;
            for (auto& iterStdBtns: lstStdButtonEvents)
            {
                if (tt::is_sameas(iter.name(), iterStdBtns))
                {
                    if (auto event = new_node->get_Event(iter.name()); event)
                    {
                        event->set_value(iter.value());
                    }
                    is_event = true;
                    break;
                }
            }

            if (is_event)
                continue;

            if (auto value = iter.value(); value.size())
            {
                {
                    // REVIEW: [KeyWorks - 11-30-2021] This code block deals with changes to the 1.2
                    // project format prior to it being released in beta. Once we make a full
                    // release, we should be able to safely remove all of this.

                    if (tt::is_sameas(iter.name(), "converted_art"))
                    {
                        // Just ignore it
                        continue;
                    }
                    else if (tt::is_sameas(iter.name(), "original_art"))
                    {
                        new_node->set_value(prop_art_directory, value);
                        continue;
                    }
                    else if (tt::is_sameas(iter.name(), "virtual_events"))
                    {
                        new_node->set_value(prop_use_derived_class, value);
                        continue;
                    }
                    else if (tt::is_sameas(iter.name(), "choices") ||
                             tt::is_sameas(iter.name(), "strings"))
                    {
                        new_node->set_value(prop_contents, value);
                        continue;
                    }
                }

                // We get here if a property is specified that we don't recognize. While we can
                // continue to load just fine, if the user attempts to save the project than the
                // property will be lost.

                // TODO: [KeyWorks - 06-03-2020] We need to store a list of unrecognized properties
                // and display them to the user all at once after the project is completely loaded.
                // We also need to flag the project file as unsaveable (only SaveAs can be used. See
                // https://github.com/KeyWorksRW/wxUiEditor/issues/385 ).

                if (allow_ui)
                {
                    MSG_WARNING(tt_string("Unrecognized property: ")
                                << iter.name() << " in class: " << class_name);

                    tt_string prop_name = iter.name();
                    tt_string prop_value = iter.value();
                    wxMessageBox(wxString().Format(
                        "The property named \"%s\" of class \"%s\" is not supported by this "
                        "version of wxUiEditor.\n\n"
                        "If your project file was just converted from an older version, then the "
                        "conversion was not "
                        "complete. Otherwise, this project is from a newer version of "
                        "wxUiEditor.\n\n"
                        "The property's value is: %s\n\n"
                        "If you save this project, YOU WILL LOSE DATA",
                        prop_name.c_str(), class_name.c_str(), prop_value.c_str()));
                }
            }
        }
    }

    if (parent)
    {
        // Order is important -- don't call GetProject() if check_for_duplicates is false
        // because there may not be a project yet.
        if (check_for_duplicates && parent == Project.get_ProjectNode())
            Project.FixupDuplicatedNode(new_node.get());
        parent->AdoptChild(new_node);
    }

    for (auto child = xml_obj.child("node"); child; child = child.next_sibling("node"))
    {
        CreateNodeFromXml(child, new_node.get(), false, allow_ui);
    }

    if (new_node->is_Gen(gen_wxGridBagSizer))
    {
        GridBag::GridBagSort(new_node.get());
    }

    return new_node;
}

NodeSharedPtr NodeCreator::CreateProjectNode(pugi::xml_node* xml_obj, bool allow_ui)
{
    auto node_decl = m_a_declarations[gen_Project];
    auto new_node = std::make_shared<Node>(node_decl);

    // Calling GetBaseClassCount() is expensive, so do it once and store the result
    auto node_info_base_count = m_a_declarations[gen_Project]->GetBaseClassCount();

    size_t base = 0;
    for (auto class_info = node_decl; class_info; class_info = node_decl->GetBaseClass(base++))
    {
        for (size_t index = 0; index < class_info->get_PropertyCount(); ++index)
        {
            auto prop_declaration = class_info->get_PropDeclaration(index);

            // Set the default value, either from the property info, or an override from this class
            auto defaultValue = prop_declaration->getDefaultValue();
            if (base > 0)
            {
                auto result = node_decl->GetOverRideDefValue(prop_declaration->get_name());
                if (result)
                    defaultValue = result.value();
            }

            auto prop = new_node->AddNodeProperty(prop_declaration);
            prop->set_value(defaultValue);
        }

        for (size_t index = 0; index < class_info->get_EventCount(); ++index)
        {
            new_node->AddNodeEvent(class_info->get_EventInfo(index));
        }

        if (base >= node_info_base_count)
            break;
    }

    if (!xml_obj)
        return new_node;

    for (auto& iter: xml_obj->attributes())
    {
        if (iter.name() == "class")
            continue;

        NodeProperty* prop = nullptr;
        if (auto find_prop = rmap_PropNames.find(iter.name()); find_prop != rmap_PropNames.end())
        {
            prop = new_node->get_PropPtr(find_prop->second);

            if (prop)
            {
                if (prop->type() == type_bool)
                {
                    prop->set_value(iter.as_bool());
                }
                else if (prop->type() == type_stringlist_semi &&
                         Project.get_OriginalProjectVersion() < 18)
                {
                    auto view = iter.as_sview();
                    if (view.size() > 0 && view[0] == '"')
                    {
                        std::vector<tt_string> items;
                        view = view.view_substr(0, '"', '"');
                        while (view.size() > 0)
                        {
                            items.emplace_back(view);
                            view = tt::stepover(view.data() + view.size());
                            view = view.view_substr(0, '"', '"');
                        }

                        tt_string value;
                        for (auto& item: items)
                        {
                            if (value.size())
                                value << ';';
                            value << item;
                        }
                        prop->set_value(value);
                    }
                    else
                    {
                        prop->set_value(iter.value());
                    }
                }
                else
                {
                    prop->set_value(iter.value());
                }
            }
        }
    }

    for (auto child = xml_obj->child("node"); child; child = child.next_sibling("node"))
    {
        CreateNodeFromXml(child, new_node.get(), false, allow_ui);
    }

    if (new_node->is_Gen(gen_wxGridBagSizer))
    {
        GridBag::GridBagSort(new_node.get());
    }

    return new_node;
}

#include <frozen/map.h>

namespace
{
    enum class ImportFileType : std::uint8_t
    {
        wxcp,
        fbp,
        rc_dlg,
        wxs_xrc,
        wxg,
        pjd,
        unknown
    };

    constexpr auto import_file_types =
        frozen::make_map<std::string_view, ImportFileType>({ { "wxcp", ImportFileType::wxcp },
                                                             { "fbp", ImportFileType::fbp },
                                                             { "rc", ImportFileType::rc_dlg },
                                                             { "dlg", ImportFileType::rc_dlg },
                                                             { "wxs", ImportFileType::wxs_xrc },
                                                             { "xrc", ImportFileType::wxs_xrc },
                                                             { "wxg", ImportFileType::wxg },
                                                             { "pjd", ImportFileType::pjd } });

    [[nodiscard]] auto GetImportFileType(std::string_view ext) -> ImportFileType
    {
        if (ext.empty())
        {
            return ImportFileType::unknown;
        }

        const auto* result = import_file_types.find(ext);
        return (result != import_file_types.end()) ? result->second : ImportFileType::unknown;
    }

    [[nodiscard]] auto GetLexerType(ImportFileType type) -> int
    {
        switch (type)
        {
            case ImportFileType::wxcp:
                return wxSTC_LEX_JSON;
            case ImportFileType::fbp:
            case ImportFileType::wxs_xrc:
            case ImportFileType::wxg:
            case ImportFileType::pjd:
                return wxSTC_LEX_XML;
            case ImportFileType::rc_dlg:
                return wxSTC_LEX_CPP;
            default:
                return wxSTC_LEX_XML;
        }
    }
}  // namespace

[[nodiscard]] auto ProjectHandler::ImportProject(std::string_view file, bool allow_ui) -> bool
{
    // Importers will change the file extension, so make a copy here
    auto import_file = wxFileName(wxString(file));
    auto file_type = GetImportFileType(import_file.GetExt().ToStdString());

    tt_string import_path(import_file.GetFullPath().ToStdString());
    bool result = false;

    switch (file_type)
    {
        case ImportFileType::wxcp:
            {
                WxCrafter crafter;
                result = Import(crafter, import_path);
                break;
            }
        case ImportFileType::fbp:
            {
                FormBuilder formbuilder;
                result = Import(formbuilder, import_path);
                break;
            }
        case ImportFileType::rc_dlg:
            {
                WinResource winres;
                result = Import(winres, import_path);
                break;
            }
        case ImportFileType::wxs_xrc:
            {
                WxSmith smith;
                result = Import(smith, import_path);
                break;
            }
        case ImportFileType::wxg:
            {
                WxGlade glade;
                result = Import(glade, import_path);
                break;
            }
        case ImportFileType::pjd:
            {
                DialogBlocks dialogblocks;
                result = Import(dialogblocks, import_path);
                break;
            }
        case ImportFileType::unknown:
            return false;
    }

    if (result && allow_ui && wxGetApp().isTestingMenuEnabled())
    {
        wxGetFrame().getImportPanel()->SetImportFile(file, GetLexerType(file_type));
    }

    return result;
}

bool ProjectHandler::Import(ImportXML& import, tt_string& file, bool append, bool allow_ui)
{
    m_ProjectVersion = ImportProjectVersion;
    if (import.Import(file))
    {
        if (allow_ui && wxGetApp().isTestingMenuEnabled())
        {
            tt_string full_path(file);
            full_path.make_absolute();
            wxGetFrame().GetAppendImportHistory()->AddFileToHistory(full_path.make_wxString());
        }

        // By having the importer create an XML document, we can pass it through
        // NodeCreation.CreateNodeFromXml() which will fix bitflag conflicts, convert wxWidgets
        // constants to friendly names, and handle old-project style conversions.

        auto& doc = import.GetDocument();
        auto root = doc.first_child();
        auto project = root.child("node");
        if (!project || project.attribute("class").as_view() != "Project")
        {
            ASSERT_MSG(project, tt_string()
                                    << "Failed trying to load converted xml document: " << file);

            // TODO: [KeyWorks - 10-23-2020] Need to let the user know
            return false;
        }

        if (append && m_project_node->get_ChildCount())
        {
            auto form = project.child("node");
            while (form)
            {
                NodeCreation.CreateNodeFromXml(form, m_project_node.get(), false, allow_ui);
                form = form.next_sibling("node");
            }

            return true;
        }

        auto project_node = NodeCreation.CreateProjectNode(&project);

        auto SetLangFilenames = [&]()
        {
            for (const auto& iter: project_node->get_ChildNodePtrs())
            {
                // If importing from wxGlade, then either a combined file will be set, or the
                // individual file for the language will be already set.
                if (iter->HasValue(prop_base_file) &&
                    project_node->as_string(prop_code_preference) != "C++")
                {
                    if (project_node->as_string(prop_code_preference) == "Python" &&
                        !iter->HasValue(prop_python_file))
                    {
                        iter->set_value(prop_python_file, iter->as_string(prop_base_file));
                    }
                    else if (project_node->as_string(prop_code_preference) == "Ruby" &&
                             !iter->HasValue(prop_ruby_file))
                    {
                        iter->set_value(prop_ruby_file, iter->as_string(prop_base_file));
                    }
                    else if (project_node->as_string(prop_code_preference) == "XRC" &&
                             !iter->HasValue(prop_xrc_file))
                    {
                        iter->set_value(prop_xrc_file, iter->as_string(prop_base_file));
                        // XRC files can be combined into a single file
                        if (!project_node->HasValue(prop_combined_xrc_file))
                            project_node->set_value(prop_combined_xrc_file,
                                                    iter->as_string(prop_base_file));
                    }
                }
            }

            if (project_node->get_ChildCount() > 1 &&
                project_node->as_string(prop_code_preference) != "XRC")
            {
                wxMessageBox("Each form must have a unique base filename when generating Python or "
                             "C++ code.\nCurrently, "
                             "only one form has a unique filename. You will need to add names to "
                             "the other forms before "
                             "generating code for them.",
                             "Code Import Change", wxOK | wxICON_WARNING);
            }
        };

        if (auto language = import.GetLanguage(); language != GEN_LANG_NONE)
        {
            if (language & GEN_LANG_CPLUSPLUS)
                project_node->set_value(prop_code_preference, "C++");
            else if (language & GEN_LANG_PERL)  // wxGlade can generate Perl
                project_node->set_value(prop_code_preference, "Perl");
            else if (language & GEN_LANG_PYTHON)
                project_node->set_value(prop_code_preference, "Python");
            else if (language & GEN_LANG_XRC)
                project_node->set_value(prop_code_preference, "XRC");

            // None of the other designers generate code for wxRuby3 or wxHaskell

            SetLangFilenames();
        }

        if (allow_ui && import.GetLanguage() == GEN_LANG_NONE)
        {
            CodePreferenceDlg dlg(wxGetMainFrame());
            if (dlg.ShowModal() == wxID_OK)
            {
                if (dlg.is_gen_python())
                {
                    project_node->set_value(prop_code_preference, "Python");
                }
                else if (dlg.is_gen_python())
                {
                    project_node->set_value(prop_code_preference, "Ruby");
                }
                else if (dlg.is_gen_perl())
                {
                    project_node->set_value(prop_code_preference, "Perl");
                }
                else if (dlg.is_gen_xrc())
                {
                    project_node->set_value(prop_code_preference, "XRC");
                }
                else  // default to C++
                {
                    project_node->set_value(prop_code_preference, "C++");
                }
                SetLangFilenames();
            }
        }

        FinalImportCheck(project_node.get());
        // Calling this will also initialize the ProjectImage class
        Project.Initialize(project_node, allow_ui);
        file.replace_extension(std::string(PROJECT_FILE_EXTENSION));
        Project.set_ProjectFile(file);
        ProjectImages.CollectBundles();

#if defined(_DEBUG)
        // If the file has been created once before, then for the first form, copy the old classname
        // and base filename to the re-converted first form.

        if (m_project_node->get_ChildCount() && file.file_exists())
        {
            doc.reset();
            auto result = doc.load_file_string(file);
            if (!result)
            {
    #if defined(_DEBUG)
                wxMessageDialog(wxGetMainFrame()->getWindow(), result.detailed_msg, "Parsing Error",
                                wxOK | wxICON_ERROR)
                    .ShowModal();
    #else
                if (allow_ui)
                {
                    wxMessageDialog(wxGetMainFrame()->getWindow(), result.detailed_msg,
                                    "Parsing Error", wxOK | wxICON_ERROR)
                        .ShowModal();
                }
    #endif  // _DEBUG
            }
            else
            {
                if (auto old_project = LoadProject(doc, allow_ui);
                    old_project && old_project->get_ChildCount())
                {
                    auto old_form = old_project->get_Child(0);
                    auto new_form = m_project_node->get_Child(0);
                    new_form->set_value(prop_class_name, old_form->as_string(prop_class_name));
                    new_form->set_value(prop_base_file, old_form->as_string(prop_base_file));
                }
            }
        }
#endif  // _DEBUG
        if (allow_ui)
        {
            wxGetFrame().setImportedFlag(true);
            wxGetFrame().FireProjectLoadedEvent();
            wxGetFrame().setModified();
        }

        return true;
    }

    return false;
}

bool ProjectHandler::NewProject(bool create_empty, bool allow_ui)
{
    if (allow_ui && wxGetFrame().isModified() && wxGetMainFrame() && !wxGetFrame().SaveWarning())
        return false;

    if (create_empty)
    {
        auto project = NodeCreation.CreateProjectNode(nullptr);

        tt_string file;
        file.assignCwd();
        file.append_filename(txtEmptyProject);

        if (allow_ui)
        {
            CodePreferenceDlg dlg(wxGetMainFrame());
            if (dlg.ShowModal() == wxID_OK)
            {
                tt_string generate_languages = project->as_string(prop_generate_languages);
                bool generated_changed = false;
                if (dlg.is_gen_python())
                {
                    project->set_value(prop_code_preference, "Python");
                    if (!generate_languages.contains("Python", tt::CASE::either))
                    {
                        if (generate_languages.size())
                            generate_languages << '|';
                        generate_languages << "Python";
                        generated_changed = true;
                    }
                }
                else if (dlg.is_gen_ruby())
                {
                    project->set_value(prop_code_preference, "Ruby");
                    if (!generate_languages.contains("Ruby", tt::CASE::either))
                    {
                        if (generate_languages.size())
                            generate_languages << '|';
                        generate_languages << "Ruby";
                        generated_changed = true;
                    }
                }
                else if (dlg.is_gen_perl())
                {
                    project->set_value(prop_code_preference, "Perl");
                    if (!generate_languages.contains("Perl", tt::CASE::either))
                    {
                        if (generate_languages.size())
                            generate_languages << '|';
                        generate_languages << "Perl";
                        generated_changed = true;
                    }
                }
                else if (dlg.is_gen_xrc())
                {
                    project->set_value(prop_code_preference, "XRC");
                    if (!generate_languages.contains("XRC", tt::CASE::either))
                    {
                        if (generate_languages.size())
                            generate_languages << '|';
                        generate_languages << "XRC";
                        generated_changed = true;
                    }
                }
                else  // default to C++
                {
                    project->set_value(prop_code_preference, "C++");
                }

                if (generated_changed)
                {
                    project->set_value(prop_generate_languages, generate_languages);
                }
            }
        }

        FinalImportCheck(project.get());
        // Calling this will also initialize the ProjectImage class
        Project.Initialize(project);
        file.replace_extension(std::string(PROJECT_FILE_EXTENSION));
        Project.set_ProjectFile(file);

        if (allow_ui)
        {
            wxGetFrame().FireProjectLoadedEvent();
        }
        return true;
    }

    if (!allow_ui)
        return false;

    ImportDlg dlg(wxGetMainFrame());
    if (dlg.ShowModal() != wxID_OK)
        return false;

    auto project = NodeCreation.CreateProjectNode(nullptr);

    tt_string file;
    tt_cwd starting_cwd;
    file.assignCwd();
    file.append_filename("MyImportedProject");

    FinalImportCheck(project.get());
    // Calling this will also initialize the ProjectImage class
    Project.Initialize(project);
    file.replace_extension(std::string(PROJECT_FILE_EXTENSION));
    Project.set_ProjectFile(file);

    tt_string imported_from;

    auto& file_list = dlg.GetFileList();
    if (file_list.size())
    {
        for (auto& iter: file_list)
        {
            try
            {
                // Importers will change the file extension, so make a copy here
                tt_string import_file = iter;

                if (iter.has_extension(".wxcp"))
                {
                    WxCrafter crafter;
                    Import(crafter, iter, true);
                    if (wxGetApp().isTestingMenuEnabled())
                        wxGetFrame().getImportPanel()->SetImportFile(import_file, wxSTC_LEX_JSON);
                }
                else if (iter.has_extension(".fbp"))
                {
                    FormBuilder fb;
                    Import(fb, iter, true);
                    if (wxGetApp().isTestingMenuEnabled())
                        wxGetFrame().getImportPanel()->SetImportFile(import_file, wxSTC_LEX_XML);
                }
                else if (iter.has_extension(".wxs") || iter.has_extension(".xrc"))
                {
                    WxSmith smith;
                    Import(smith, iter, true);
                    if (wxGetApp().isTestingMenuEnabled())
                        wxGetFrame().getImportPanel()->SetImportFile(import_file, wxSTC_LEX_XML);
                }
                else if (iter.has_extension(".wxg"))
                {
                    WxGlade glade;
                    Import(glade, iter, true);
                    if (wxGetApp().isTestingMenuEnabled())
                        wxGetFrame().getImportPanel()->SetImportFile(import_file, wxSTC_LEX_XML);
                }
                else if (iter.has_extension(".rc") || iter.has_extension(".dlg"))
                {
                    WinResource winres;
                    Import(winres, iter, true);
                    if (wxGetApp().isTestingMenuEnabled())
                        wxGetFrame().getImportPanel()->SetImportFile(import_file, wxSTC_LEX_CPP);
                }
                else if (iter.has_extension(".pjd"))
                {
                    DialogBlocks db;
                    Import(db, iter, true);
                    if (wxGetApp().isTestingMenuEnabled())
                        wxGetFrame().getImportPanel()->SetImportFile(import_file, wxSTC_LEX_XML);
                }

                if (imported_from.size())
                    imported_from << "@@";
                imported_from << "// Imported from " << iter;
            }
            catch (const std::exception& /* e */)
            {
                // silently continue with the next project file
            }
        }

        if (imported_from.size())
        {
            tt_string preamble = m_project_node->as_string(prop_src_preamble);
            if (preamble.size())
                preamble << "@@@@";
            preamble << imported_from;
            m_project_node->set_value(prop_src_preamble, preamble);
        }

        wxFileName path(file_list[0]);
        if (path.IsOk())
        {
            path.SetExt("wxui");
            path.MakeAbsolute();
            set_ProjectPath(&path);
        }
        wxGetFrame().setImportedFlag();
    }
    ProjectImages.CollectBundles();

    wxGetFrame().FireProjectLoadedEvent();
    if (m_project_node->get_ChildCount())
        wxGetFrame().setModified();
    return true;
}

void ProjectHandler::AppendWinRes(const tt_string& rc_file, std::vector<tt_string>& dialogs)
{
    WinResource winres;
    if (winres.ImportRc(rc_file, dialogs))
    {
        const auto& project = winres.GetProjectPtr();
        for (const auto& child: project->get_ChildNodePtrs())
        {
            auto new_node = NodeCreation.MakeCopy(child);
            Project.FixupDuplicatedNode(new_node.get());
            m_project_node->AdoptChild(new_node);
        }
        if (m_allow_ui)
        {
            wxGetFrame().FireProjectUpdatedEvent();
            wxGetFrame().setModified();
        }
    }
}

void ProjectHandler::AppendCrafter(wxArrayString& files)
{
    for (const auto& file: files)
    {
        WxCrafter crafter;

        if (crafter.Import(file))
        {
            auto& doc = crafter.GetDocument();
            auto root = doc.first_child();
            auto project = root.child("node");
            if (!project || project.attribute("class").as_cstr() != "Project")
            {
                if (m_allow_ui)
                {
                    wxMessageBox(wxString("The project file ")
                                     << file << " is invalid and cannot be opened.",
                                 "Import wxCrafter project");
                }
                return;
            }

            auto cur_sel = wxGetFrame().getSelectedNode();
            if (!cur_sel)
                cur_sel = m_project_node.get();
            else
            {
                if (!cur_sel->is_Gen(gen_Project) && !cur_sel->is_Gen(gen_folder))
                {
                    cur_sel = cur_sel->get_Folder();
                    if (!cur_sel)
                        cur_sel = m_project_node.get();
                }
            }

            auto form = project.child("node");
            while (form)
            {
                if (auto new_node = NodeCreation.CreateNodeFromXml(form, cur_sel, true, m_allow_ui);
                    new_node)
                {
                    FinalImportCheck(new_node.get(), false);
                }
                form = form.next_sibling("node");
            }
        }
    }
    if (m_allow_ui)
    {
        wxGetFrame().FireProjectUpdatedEvent();
        wxGetFrame().setModified();
    }
}

void ProjectHandler::AppendFormBuilder(wxArrayString& files)
{
    for (auto& file: files)
    {
        FormBuilder fb;

        if (fb.Import(file))
        {
            auto& doc = fb.GetDocument();
            auto root = doc.first_child();
            auto project = root.child("node");
            if (!project || project.attribute("class").as_cstr() != "Project")
            {
                if (m_allow_ui)
                {
                    wxMessageBox(wxString("The project file ")
                                     << file << " is invalid and cannot be opened.",
                                 "Import wxFormBuilder project");
                }
                return;
            }

            auto cur_sel = wxGetFrame().getSelectedNode();
            if (!cur_sel)
                cur_sel = m_project_node.get();
            else
            {
                if (!cur_sel->is_Gen(gen_Project) && !cur_sel->is_Gen(gen_folder))
                {
                    cur_sel = cur_sel->get_Folder();
                    if (!cur_sel)
                        cur_sel = m_project_node.get();
                }
            }

            auto form = project.child("node");
            while (form)
            {
                if (auto new_node = NodeCreation.CreateNodeFromXml(form, cur_sel, true, m_allow_ui);
                    new_node)
                {
                    FinalImportCheck(new_node.get(), false);
                }
                form = form.next_sibling("node");
            }
        }
    }
    if (m_allow_ui)
    {
        wxGetFrame().FireProjectUpdatedEvent();
        wxGetFrame().setModified();
    }
}

void ProjectHandler::AppendDialogBlocks(wxArrayString& files)
{
    for (auto& file: files)
    {
        DialogBlocks db;

        if (db.Import(file))
        {
            auto& doc = db.GetDocument();
            auto root = doc.first_child();
            auto project = root.child("node");
            if (!project || project.attribute("class").as_cstr() != "Project")
            {
                if (m_allow_ui)
                {
                    wxMessageBox(wxString("The project file ")
                                     << file << " is invalid and cannot be opened.",
                                 "Import wxFormBuilder project");
                }
                return;
            }

            auto cur_sel = wxGetFrame().getSelectedNode();
            if (!cur_sel)
                cur_sel = m_project_node.get();
            else
            {
                if (!cur_sel->is_Gen(gen_Project) && !cur_sel->is_Gen(gen_folder))
                {
                    cur_sel = cur_sel->get_Folder();
                    if (!cur_sel)
                        cur_sel = m_project_node.get();
                }
            }

            auto form = project.child("node");
            while (form)
            {
                if (auto new_node = NodeCreation.CreateNodeFromXml(form, cur_sel, true, m_allow_ui);
                    new_node)
                {
                    FinalImportCheck(new_node.get(), false);
                }
                form = form.next_sibling("node");
            }
        }
    }
    if (m_allow_ui)
    {
        wxGetFrame().FireProjectUpdatedEvent();
        wxGetFrame().setModified();
    }
}

void ProjectHandler::AppendGlade(wxArrayString& files)
{
    for (auto& file: files)
    {
        WxGlade glade;

        if (glade.Import(file))
        {
            auto& doc = glade.GetDocument();
            auto root = doc.first_child();
            auto project = root.child("node");
            if (!project || project.attribute("class").as_cstr() != "Project")
            {
                if (m_allow_ui)
                {
                    wxMessageBox(wxString("The project file ")
                                     << file << " is invalid and cannot be opened.",
                                 "Import wxGlade project");
                }
                return;
            }

            auto cur_sel = wxGetFrame().getSelectedNode();
            if (!cur_sel)
                cur_sel = m_project_node.get();
            else
            {
                if (!cur_sel->is_Gen(gen_Project) && !cur_sel->is_Gen(gen_folder))
                {
                    cur_sel = cur_sel->get_Folder();
                    if (!cur_sel)
                        cur_sel = m_project_node.get();
                }
            }

            auto form = project.child("node");
            while (form)
            {
                if (auto new_node = NodeCreation.CreateNodeFromXml(form, cur_sel, true, m_allow_ui);
                    new_node)
                {
                    FinalImportCheck(new_node.get(), false);
                }
                form = form.next_sibling("node");
            }
        }
    }
    if (m_allow_ui)
    {
        wxGetFrame().FireProjectUpdatedEvent();
        wxGetFrame().setModified();
    }
}

void ProjectHandler::AppendSmith(wxArrayString& files)
{
    for (auto& file: files)
    {
        WxSmith smith;

        if (smith.Import(file))
        {
            auto& doc = smith.GetDocument();
            auto root = doc.first_child();
            auto project = root.child("node");
            if (!project || project.attribute("class").as_cstr() != "Project")
            {
                if (m_allow_ui)
                {
                    wxMessageBox(wxString("The project file ")
                                     << file << " is invalid and cannot be opened.",
                                 "Import wxSmith project");
                }
                return;
            }

            auto cur_sel = wxGetFrame().getSelectedNode();
            if (!cur_sel)
                cur_sel = m_project_node.get();
            else
            {
                if (!cur_sel->is_Gen(gen_Project) && !cur_sel->is_Gen(gen_folder))
                {
                    cur_sel = cur_sel->get_Folder();
                    if (!cur_sel)
                        cur_sel = m_project_node.get();
                }
            }

            auto form = project.child("node");
            while (form)
            {
                if (auto new_node = NodeCreation.CreateNodeFromXml(form, cur_sel, true, m_allow_ui);
                    new_node)
                {
                    FinalImportCheck(new_node.get(), false);
                }
                form = form.next_sibling("node");
            }
        }
    }
    if (m_allow_ui)
    {
        wxGetFrame().FireProjectUpdatedEvent();
        wxGetFrame().setModified();
    }
}

void ProjectHandler::AppendXRC(wxArrayString& files)
{
    for (auto& file: files)
    {
        // wxSmith files are a superset of XRC files, so we use the wxSmith class to process both
        WxSmith smith;

        if (smith.Import(file))
        {
            auto& doc = smith.GetDocument();
            auto root = doc.first_child();
            auto project = root.child("node");
            if (!project || project.attribute("class").as_cstr() != "Project")
            {
                if (m_allow_ui)
                {
                    wxMessageBox(wxString("The project file ")
                                     << file << " is invalid and cannot be opened.",
                                 "Import XRC project");
                }
                return;
            }

            auto cur_sel = wxGetFrame().getSelectedNode();
            if (!cur_sel)
                cur_sel = m_project_node.get();
            else
            {
                if (!cur_sel->is_Gen(gen_Project) && !cur_sel->is_Gen(gen_folder))
                {
                    cur_sel = cur_sel->get_Folder();
                    if (!cur_sel)
                        cur_sel = m_project_node.get();
                }
            }

            auto form = project.child("node");
            while (form)
            {
                if (auto new_node = NodeCreation.CreateNodeFromXml(form, cur_sel, true, m_allow_ui);
                    new_node)
                {
                    FinalImportCheck(new_node.get(), false);
                }
                form = form.next_sibling("node");
            }
        }
    }
    if (m_allow_ui)
    {
        wxGetFrame().FireProjectUpdatedEvent();
        wxGetFrame().setModified();
    }
}

void ProjectHandler::RecursiveNodeCheck(Node* node)
{
    if (auto prop_ptr = node->get_PropPtr(prop_alignment); prop_ptr && prop_ptr->as_string().size())
    {
        if (auto parent = node->get_Parent(); parent && parent->is_Sizer())
        {
            tt_string old_value = prop_ptr->as_string();
            if (parent->as_string(prop_orientation).contains("wxVERTICAL"))
            {
                // You can't set vertical alignment flags if the parent sizer is vertical
                prop_ptr->get_value().Replace("wxALIGN_TOP", "");
                prop_ptr->get_value().Replace("wxALIGN_BOTTOM", "");
                prop_ptr->get_value().Replace("wxALIGN_CENTER_VERTICAL", "");
            }
            else if (node->as_string(prop_flags).contains("wxEXPAND"))
            {
                // You can't set vertical alignment flags in a horizontal sizer if wxEXPAND is set
                prop_ptr->get_value().Replace("wxALIGN_TOP", "");
                prop_ptr->get_value().Replace("wxALIGN_BOTTOM", "");
                prop_ptr->get_value().Replace("wxALIGN_CENTER_VERTICAL", "");
                prop_ptr->get_value().Replace("wxALIGN_CENTER", "");
            }

            if (parent->as_string(prop_orientation).contains("wxHORIZONTAL"))
            {
                // You can't set horizontal alignment flags if the parent sizer is horizontal
                prop_ptr->get_value().Replace("wxALIGN_LEFT", "");
                prop_ptr->get_value().Replace("wxALIGN_RIGHT", "");
                prop_ptr->get_value().Replace("wxALIGN_CENTER_HORIZONTAL", "");
            }
            else if (node->as_string(prop_flags).contains("wxEXPAND"))
            {
                // You can't set horizontal alignment flags in a vertical sizer if wxEXPAND is set
                prop_ptr->get_value().Replace("wxALIGN_LEFT", "");
                prop_ptr->get_value().Replace("wxALIGN_RIGHT", "");
                prop_ptr->get_value().Replace("wxALIGN_CENTER_HORIZONTAL", "");
                prop_ptr->get_value().Replace("wxALIGN_CENTER", "");
            }
            if (wxGetApp().isTestingMenuEnabled())
            {
                if (old_value != prop_ptr->as_string())
                {
                    tt_string msg;
                    if (prop_ptr->as_string().empty())
                    {
                        msg = "Alignment flags for " + node->as_string(prop_var_name) + " in " +
                              parent->as_string(prop_var_name) + " changed from " + old_value +
                              " to no flags";
                    }
                    else
                    {
                        msg = "Alignment flags for " + node->as_string(prop_var_name) + " in " +
                              parent->as_string(prop_var_name) + " changed from " + old_value +
                              " to " + prop_ptr->as_string();
                    }
                    MSG_INFO(msg);

                    m_isProject_updated = true;
                }
            }
        }
    }

    if (node->is_Gen(gen_wxFlexGridSizer) || node->is_Gen(gen_wxGridSizer))
    {
        // Don't set prop_rows if prop_cols is set. This lets wxWidgets determine the number of
        // rows rather than relying on the user to always figure it out (or for our code
        // generation to always figure it out).
        if (node->as_int(prop_rows) > 0 && node->as_int(prop_cols) > 0)
        {
            // REVIEW: [Randalphwa - 08-29-2023] Need to check if it is a performance hit to make
            // the sizer figure this out. We could set it whenver we generate the code for it.
            node->set_value(prop_rows, 0);
            m_isProject_updated = true;
            MSG_INFO(tt_string("Removed row setting from ")
                     << node->as_string(prop_var_name) << " since cols is set");
        }
    }

    for (auto& iter: node->get_ChildNodePtrs())
    {
        RecursiveNodeCheck(iter.get());
    }
}

// A lot of designers create projects that can result in assertion warnings when their
// generated code is run under a Debug build of wxWidgets. While the generated UI usually works
// fine, it would be better to generate the correct code in the first place. That means fixing
// up conflicts between styles and other properties that either conflict or were not set
// properlyy by the designer.

void ProjectHandler::FinalImportCheck(Node* parent, bool set_line_length)
{
    if (set_line_length && parent->is_Gen(gen_Project))
    {
        parent->set_value(prop_cpp_line_length, UserPrefs.get_CppLineLength());
        parent->set_value(prop_python_line_length, UserPrefs.get_PythonLineLength());
        parent->set_value(prop_ruby_line_length, UserPrefs.get_RubyLineLength());
        if (!parent->HasValue(prop_wxWidgets_version))
        {
            parent->set_value(prop_wxWidgets_version, "3.1.0");
        }
    }

    RecursiveNodeCheck(parent);
}
