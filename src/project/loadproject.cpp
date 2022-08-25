/////////////////////////////////////////////////////////////////////////////
// Purpose:   Load wxUiEditor project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/stc/stc.h>  // A wxWidgets implementation of Scintilla.
#include <wx/utils.h>    // Miscellaneous utilities

#include "../nodes/node_creator.h"  // NodeCreator class
#include "base_generator.h"         // BaseGenerator -- Base widget generator class
#include "gen_enums.h"              // Enumerations for generators
#include "mainapp.h"                // App -- Main application class
#include "mainframe.h"              // MainFrame -- Main window frame
#include "node.h"                   // Node class
#include "project_class.h"          // Project class

using namespace GenEnum;

#include "../import/import_formblder.h"  // FormBuilder -- Import a wxFormBuider project
#include "../import/import_wxcrafter.h"  // WxCrafter -- Import a wxCrafter project
#include "../import/import_wxglade.h"    // WxGlade -- Import a wxGlade file
#include "../import/import_wxsmith.h"    // WxSmith -- Import a wxSmith file
#include "../winres/import_winres.h"     // WinResource -- Parse a Windows resource file
#include "import_dlg.h"                  // ImportDlg -- Dialog to create a new project
#include "node_gridbag.h"                // GridBag -- Create and modify a node containing a wxGridBagSizer

#if defined(INTERNAL_TESTING)
    #include "../internal/import_panel.h"  // ImportPanel -- Panel to display original imported file
#endif

using namespace GenEnum;

bool App::LoadProject(const ttString& file)
{
    pugi::xml_document doc;
    auto result = doc.load_file(file.wx_str());
    if (!result)
    {
        ASSERT_MSG(result, ttlib::cstr() << "pugi failed trying to load " << file.wx_str());
        wxMessageBox(wxString("Cannot open ") << file << "\n\n" << result.description(), "Load Project");
        return false;
    }

    auto root = doc.first_child();
    if (!ttlib::is_sameas(root.name(), "wxUiEditorData", tt::CASE::either))
    {
        wxMessageBox(wxString() << file << " is not a wxUiEditor XML file", "Load Project");
        return false;
    }

    ProjectSharedPtr project;

    m_ProjectVersion = root.attribute("data_version").as_int(curSupportedVer);

    if (m_ProjectVersion > curSupportedVer)
    {
        if (wxMessageBox("wxUiEditor does not recognize this version of the data file.\n"
                         "You may be able to load the file, but if you then save it you could lose data.\n\n"
                         "Do you want to try to open it anyway?",
                         "Unrecognized Version", wxYES_NO) != wxYES)
        {
            return false;
        }
        else
        {
            wxBusyCursor wait;
            project = LoadProject(doc);
        }
    }

    else if (m_ProjectVersion < minRequiredVer)
    {
        if (!root.child("object") && !root.child("node"))
        {
            wxMessageBox(wxString() << "The data file " << file << " is invalid and cannot be opened.");
            return false;
        }
        else if (m_ProjectVersion < 11)
        {
            if (wxMessageBox(ttlib::cstr() << "Project version " << m_ProjectVersion / 10 << '.' << m_ProjectVersion % 10
                                           << " is not supported.\n\nDo you want to attempt to load it anyway?",
                             "Unsupported Project Version", wxYES_NO) == wxNO)
            {
                return false;
            }
        }

        wxBusyCursor wait;
        project = LoadProject(doc);
    }

    else
    {
        if (!root.child("node"))
        {
            wxMessageBox(wxString() << "The data file " << file << " is invalid and cannot be opened.");
            return false;
        }

        wxBusyCursor wait;
        project = LoadProject(doc);
    }

    if (!project)
    {
        ASSERT_MSG(project, ttlib::cstr() << "Failed trying to load " << file.wx_str());

        wxMessageBox(wxString() << "The project file " << file << " is invalid and cannot be opened.");
        return false;
    }

    m_project = project;
    m_project->SetProjectFile(file);
    m_project->SetProjectPath(file);

    {
        wxBusyCursor wait;

        ttSaveCwd cwd;
        m_project->GetProjectPath().ChangeDir();
        m_project->CollectBundles();
    }

    // Imported projects start with an older version so that they pass through the old project fixups.
    if (m_ProjectVersion == ImportProjectVersion)
    {
        m_ProjectVersion = minRequiredVer;
    }

    wxGetFrame().SetImportedFlag(false);
    wxGetFrame().FireProjectLoadedEvent();

    if (m_isProject_updated || m_ProjectVersion < minRequiredVer)
        wxGetFrame().SetModified();

    return true;
}

ProjectSharedPtr App::LoadProject(pugi::xml_document& doc)
{
    ProjectSharedPtr project;
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
        project = g_NodeCreator.CreateProjectClass(&node);
    }
    catch (const std::exception& TESTING_PARAM(e))
    {
        MSG_ERROR(e.what());
        wxMessageBox("This wxUiEditor project file is invalid and cannot be loaded.", "Load Project");
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
    "HelpButtonClicked",
    "ContextHelpButtonClicked",

    "OKButton",
    "YesButton",
    "SaveButton",
    "ApplyButton",
    "NoButton",
    "CancelButton",
    "HelpButton",
    "ContextHelpButton",

};
// clang-format on

#include "utils.h"  // for old style art indices

NodeSharedPtr NodeCreator::CreateNode(pugi::xml_node& xml_obj, Node* parent)
{
    auto class_name = xml_obj.attribute("class").as_std_str();
    if (class_name.empty())
        return NodeSharedPtr();

    // This should never be the case, but let's switch it in the off chance it slips through
    if (class_name == "wxListCtrl")
        class_name = "wxListView";

    auto new_node = CreateNode(class_name, parent);
    if (!new_node)
    {
        FAIL_MSG(ttlib::cstr() << "Invalid project file: could not create " << class_name);
        throw std::runtime_error("Invalid project file");
    }

    for (auto& iter: xml_obj.attributes())
    {
        if (iter.name() == "class")
            continue;

        if (iter.name().starts_with("wxEVT_"))
        {
            if (auto event = new_node->GetEvent(iter.name()); event)
            {
                event->set_value(iter.value());
            }
            continue;
        }

        NodeProperty* prop = nullptr;
        if (auto find_prop = rmap_PropNames.find(iter.name()); find_prop != rmap_PropNames.end())
        {
            prop = new_node->get_prop_ptr(find_prop->second);

            if (prop)
            {
                if (prop->type() == type_bool)
                {
                    prop->set_value(iter.as_bool());
                }

                // Imported projects will be set as version ImportProjectVersion to get the fixups of constant to friendly
                // name, and bit flag conflict resolution.

                else if (wxGetApp().GetProjectVersion() <= ImportProjectVersion)
                {
                    switch (prop->type())
                    {
                        case type_editoption:
                        case type_option:
                            {
                                bool found = false;
                                for (auto& friendly_pair: g_friend_constant)
                                {
                                    if (ttlib::is_sameas(friendly_pair.second, iter.value()))
                                    {
                                        prop->set_value(friendly_pair.first.c_str() + friendly_pair.first.find('_') + 1);
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
                                ttlib::multistr mstr(iter.value(), '|', tt::TRIM::both);
                                bool found = false;
                                ttlib::cstr new_value;
                                for (auto& bit_value: mstr)
                                {
                                    for (auto& friendly_pair: g_friend_constant)
                                    {
                                        if (ttlib::is_sameas(friendly_pair.second, bit_value))
                                        {
                                            if (new_value.size())
                                            {
                                                new_value << '|';
                                            }

                                            new_value << (friendly_pair.first.c_str() + friendly_pair.first.find('_') + 1);
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

                                if (auto gen = new_node->GetGenerator(); gen)
                                {
                                    gen->VerifyProperty(prop);
                                }
                            }
                            break;

                        case type_image:
                            {
                                ttlib::multistr parts(iter.value(), ';', tt::TRIM::both);
                                if (parts.size() < 3)
                                {
                                    prop->set_value(iter.value());
                                }
                                else
                                {
                                    parts[1].backslashestoforward();
                                    ttlib::cstr description(parts[0]);
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
        }
        else
        {
            bool is_event = false;
            for (auto& iterStdBtns: lstStdButtonEvents)
            {
                if (ttlib::is_sameas(iter.name(), iterStdBtns))
                {
                    if (auto event = new_node->GetEvent(iter.name()); event)
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
                    // REVIEW: [KeyWorks - 11-30-2021] This code block deals with changes to the 1.2 project format prior to
                    // it being released in beta. Once we make a full release, we should be able to safely remove all of
                    // this.

                    if (ttlib::is_sameas(iter.name(), "converted_art"))
                    {
                        // Just ignore it
                        continue;
                    }
                    else if (ttlib::is_sameas(iter.name(), "original_art"))
                    {
                        new_node->prop_set_value(prop_art_directory, value);
                        continue;
                    }
                    else if (ttlib::is_sameas(iter.name(), "virtual_events"))
                    {
                        new_node->prop_set_value(prop_use_derived_class, value);
                        continue;
                    }
                    else if (ttlib::is_sameas(iter.name(), "choices") || ttlib::is_sameas(iter.name(), "strings"))
                    {
                        new_node->prop_set_value(prop_contents, value);
                        continue;
                    }
                }

                // We get here if a property is specified that we don't recognize. While we can continue to load
                // just fine, if the user attempts to save the project than the property will be lost.

                // TODO: [KeyWorks - 06-03-2020] We need to store a list of unrecognized properties and display them to
                // the user all at once after the project is completely loaded. We also need to flag the project file as
                // unsaveable (only SaveAs can be used. See https://github.com/KeyWorksRW/wxUiEditor/issues/385 ).

                MSG_WARNING(ttlib::cstr("Unrecognized property: ") << iter.name() << " in class: " << class_name);

                wxMessageBox(ttlib::cstr().Format(
                    "The property named \"%v\" of class \"%s\" is not supported by this version of wxUiEditor.\n\n"
                    "If your project file was just converted from an older version, then the conversion was not "
                    "complete. Otherwise, this project is from a newer version of wxUiEditor.\n\n"
                    "The property's value is: %v\n\n"
                    "If you save this project, YOU WILL LOSE DATA",
                    iter.name(), class_name.c_str(), value));
            }
        }
    }

    if (parent)
    {
        parent->Adopt(new_node);
    }

    for (auto child = xml_obj.child("node"); child; child = child.next_sibling("node"))
    {
        CreateNode(child, new_node.get());
    }

    if (new_node->isGen(gen_wxGridBagSizer))
    {
        GridBag::GridBagSort(new_node.get());
    }

    return new_node;
}

ProjectSharedPtr NodeCreator::CreateProjectClass(pugi::xml_node* xml_obj)
{
    auto node_decl = m_a_declarations[gen_Project];
    auto new_node = std::make_shared<Project>(node_decl);

    // Calling GetBaseClassCount() is expensive, so do it once and store the result
    auto node_info_base_count = m_a_declarations[gen_Project]->GetBaseClassCount();

    size_t base = 0;
    for (auto class_info = node_decl; class_info; class_info = node_decl->GetBaseClass(base++))
    {
        for (size_t index = 0; index < class_info->GetPropertyCount(); ++index)
        {
            auto prop_declaration = class_info->GetPropDeclaration(index);

            // Set the default value, either from the property info, or an override from this class
            auto defaultValue = prop_declaration->GetDefaultValue();
            if (base > 0)
            {
                auto result = node_decl->GetOverRideDefValue(prop_declaration->get_name());
                if (result)
                    defaultValue = result.value();
            }

            auto prop = new_node->AddNodeProperty(prop_declaration);
            prop->set_value(defaultValue);
        }

        for (size_t index = 0; index < class_info->GetEventCount(); ++index)
        {
            new_node->AddNodeEvent(class_info->GetEventInfo(index));
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
            prop = new_node->get_prop_ptr(find_prop->second);

            if (prop)
            {
                if (prop->type() == type_bool)
                {
                    prop->set_value(iter.as_bool());
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
        CreateNode(child, new_node.get());
    }

    if (new_node->isGen(gen_wxGridBagSizer))
    {
        GridBag::GridBagSort(new_node.get());
    }

    return new_node;
}

bool App::ImportProject(ttString& file)
{
#if defined(INTERNAL_TESTING)
    // Importers will change the file extension, so make a copy here
    auto import_file = file.sub_cstr();
#endif
    bool result = false;
    if (file.has_extension(".wxcp"))
    {
        WxCrafter crafter;
        result = Import(crafter, file);
#if defined(INTERNAL_TESTING)
        if (result)
            wxGetFrame().GetImportPanel()->SetImportFile(import_file, wxSTC_LEX_JSON);
#endif
    }
    else if (file.has_extension(".fbp"))
    {
        FormBuilder fb;
        result = Import(fb, file);
#if defined(INTERNAL_TESTING)
        if (result)
            wxGetFrame().GetImportPanel()->SetImportFile(import_file, wxSTC_LEX_XML);
#endif
    }
    else if (file.has_extension(".rc") || file.has_extension(".dlg"))
    {
        WinResource winres;
        result = Import(winres, file);
#if defined(INTERNAL_TESTING)
        if (result)
            wxGetFrame().GetImportPanel()->SetImportFile(import_file, wxSTC_LEX_CPP);
#endif
    }
    else if (file.has_extension(".wxs") || file.has_extension(".xrc"))
    {
        WxSmith smith;
        result = Import(smith, file);
#if defined(INTERNAL_TESTING)
        if (result)
            wxGetFrame().GetImportPanel()->SetImportFile(import_file, wxSTC_LEX_XML);
#endif
    }
    else if (file.has_extension(".wxg"))
    {
        WxGlade glade;
        result = Import(glade, file);
#if defined(INTERNAL_TESTING)
        if (result)
            wxGetFrame().GetImportPanel()->SetImportFile(import_file, wxSTC_LEX_XML);
#endif
    }

    return result;
}

bool App::Import(ImportXML& import, ttString& file, bool append)
{
    m_ProjectVersion = ImportProjectVersion;
    if (import.Import(file))
    {
#if defined(_DEBUG) || defined(INTERNAL_TESTING)
        ttString full_path(file);
        full_path.make_absolute();
        wxGetFrame().GetAppendImportHistory()->AddFileToHistory(full_path);
#endif  // _DEBUG

        // By having the importer create an XML document, we can pass it through g_NodeCreator.CreateNode() which will
        // fix bitflag conflicts, convert wxWidgets constants to friendly names, and handle old-project style conversions.

        auto& doc = import.GetDocument();
        auto root = doc.first_child();
        auto project = root.child("node");
        if (!project || project.attribute("class").as_string() != "Project")
        {
            ASSERT_MSG(project, ttlib::cstr() << "Failed trying to load converted xml document: " << file.wx_str());

            // TODO: [KeyWorks - 10-23-2020] Need to let the user know
            return false;
        }

        if (append && m_project->GetChildCount())
        {
            auto form = project.child("node");
            while (form)
            {
                g_NodeCreator.CreateNode(form, m_project.get());
                form = form.next_sibling("node");
            }

            return true;
        }

        m_project = g_NodeCreator.CreateProjectClass(&project);

        file.remove_extension();
        m_project->SetProjectFile(file);
        m_project->SetProjectPath(file);

        {
            wxBusyCursor wait;

            ttSaveCwd cwd;
            m_project->GetProjectPath().ChangeDir();
            m_project->CollectBundles();
        }

#if defined(_DEBUG)
        // If the file has been created once before, then for the first form, copy the old classname and base filename to the
        // re-converted first form.

        file.replace_extension(".wxui");
        if (m_project->GetChildCount() && file.file_exists())
        {
            doc.reset();
            auto result = doc.load_file(file.wx_str());
            if (!result)
            {
                ASSERT_MSG(result, ttlib::cstr() << "pugi failed trying to load " << file.wx_str());
                wxMessageBox(wxString("Cannot open ") << file << "\n\n" << result.description(), "Load Project");
            }
            else
            {
                if (auto old_project = LoadProject(doc); old_project && old_project->GetChildCount())
                {
                    auto old_form = old_project->GetChild(0);
                    auto new_form = m_project->GetChild(0);
                    new_form->prop_set_value(prop_class_name, old_form->prop_as_string(prop_class_name));
                    new_form->prop_set_value(prop_base_file, old_form->prop_as_string(prop_base_file));
                }
            }
        }
#endif  // _DEBUG

        wxGetFrame().SetImportedFlag(true);
        wxGetFrame().FireProjectLoadedEvent();
        wxGetFrame().SetModified();

        return true;
    }

    return false;
}

bool App::NewProject(bool create_empty)
{
    if (m_frame->IsModified() && m_frame && !m_frame->SaveWarning())
        return false;

    if (create_empty)
    {
        m_project = g_NodeCreator.CreateProjectClass(nullptr);

        ttString file;
        file.assignCwd();
        file.append_filename(txtEmptyProject);
        m_project->SetProjectFile(file);
        m_project->SetProjectPath(file);

        wxGetFrame().FireProjectLoadedEvent();
        return true;
    }

    ImportDlg dlg(m_frame);
    if (dlg.ShowModal() != wxID_OK)
        return false;

    m_project = g_NodeCreator.CreateProjectClass(nullptr);

    ttString file;
    file.assignCwd();
    file.append_filename("MyImportedProject");
    m_project->SetProjectFile(file);
    m_project->SetProjectPath(file);

    ttlib::cstr imported_from;

    auto& file_list = dlg.GetFileList();
    if (file_list.size())
    {
        for (auto& iter: file_list)
        {
            try
            {
#if defined(INTERNAL_TESTING)
                // Importers will change the file extension, so make a copy here
                auto import_file = iter.sub_cstr();
#endif

                if (iter.has_extension(".wxcp"))
                {
                    WxCrafter crafter;
                    Import(crafter, iter, true);
#if defined(INTERNAL_TESTING)
                    wxGetFrame().GetImportPanel()->SetImportFile(import_file, wxSTC_LEX_JSON);
#endif
                }
                else if (iter.has_extension(".fbp"))
                {
                    FormBuilder fb;
                    Import(fb, iter, true);
#if defined(INTERNAL_TESTING)
                    wxGetFrame().GetImportPanel()->SetImportFile(import_file, wxSTC_LEX_XML);
#endif
                }
                else if (iter.has_extension(".wxs") || iter.has_extension(".xrc"))
                {
                    WxSmith smith;
                    Import(smith, iter, true);
#if defined(INTERNAL_TESTING)
                    wxGetFrame().GetImportPanel()->SetImportFile(import_file, wxSTC_LEX_XML);
#endif
                }
                else if (iter.has_extension(".wxg"))
                {
                    WxGlade glade;
                    Import(glade, iter, true);
#if defined(INTERNAL_TESTING)
                    wxGetFrame().GetImportPanel()->SetImportFile(import_file, wxSTC_LEX_XML);
#endif
                }
                else if (iter.has_extension(".rc") || iter.has_extension(".dlg"))
                {
                    WinResource winres;
                    Import(winres, iter, true);
#if defined(INTERNAL_TESTING)
                    wxGetFrame().GetImportPanel()->SetImportFile(import_file, wxSTC_LEX_CPP);
#endif
                }

                if (imported_from.size())
                    imported_from << "@@";
                imported_from << "// Imported from " << iter.wx_str();
            }
            catch (const std::exception& /* e */)
            {
                // silently continue with the next project file
            }
        }

        if (imported_from.size())
        {
            ttlib::cstr preamble = m_project->prop_as_string(prop_src_preamble);
            if (preamble.size())
                preamble << "@@@@";
            preamble << imported_from;
            m_project->prop_set_value(prop_src_preamble, preamble);
        }

        // Set the current working directory to the first file imported.
        ttString path(file_list[0]);
        if (path.size())
        {
            path.replace_extension_wx(".wxui");
            path.make_absolute();
            path.backslashestoforward();
            m_project->SetProjectFile(path);
            path.remove_filename();
            m_project->SetProjectPath(path);
        }
        m_frame->SetImportedFlag();
    }

    {
        wxBusyCursor wait;

        ttSaveCwd cwd;
        m_project->GetProjectPath().ChangeDir();
        m_project->CollectBundles();
    }

    wxGetFrame().FireProjectLoadedEvent();
    if (m_project->GetChildCount())
        wxGetFrame().SetModified();
    return true;
}

void App::AppendWinRes(const ttlib::cstr& rc_file, std::vector<ttlib::cstr>& dialogs)
{
    WinResource winres;
    if (winres.ImportRc(rc_file, dialogs))
    {
        const auto& project = winres.GetProjectPtr();
        for (const auto& child: project->GetChildNodePtrs())
        {
            auto new_node = g_NodeCreator.MakeCopy(child);
            m_project->Adopt(new_node);
        }

        wxGetFrame().FireProjectUpdatedEvent();
        wxGetFrame().SetModified();
    }
}

void App::AppendCrafter(wxArrayString& files)
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
                wxMessageBox(wxString("The project file ") << file << " is invalid and cannot be opened.",
                             "Import wxCrafter project");
                return;
            }

            auto form = project.child("node");
            while (form)
            {
                g_NodeCreator.CreateNode(form, m_project.get());
                form = form.next_sibling("node");
            }
        }
    }
    wxGetFrame().FireProjectUpdatedEvent();
    wxGetFrame().SetModified();
}

void App::AppendFormBuilder(wxArrayString& files)
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
                wxMessageBox(wxString("The project file ") << file << " is invalid and cannot be opened.",
                             "Import wxFormBuilder project");
                return;
            }

            auto form = project.child("node");
            while (form)
            {
                g_NodeCreator.CreateNode(form, m_project.get());
                form = form.next_sibling("node");
            }
        }
    }
    wxGetFrame().FireProjectUpdatedEvent();
    wxGetFrame().SetModified();
}

void App::AppendGlade(wxArrayString& files)
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
                wxMessageBox(wxString("The project file ") << file << " is invalid and cannot be opened.",
                             "Import wxGlade project");
                return;
            }

            auto form = project.child("node");
            while (form)
            {
                g_NodeCreator.CreateNode(form, m_project.get());
                form = form.next_sibling("node");
            }
        }
    }
    wxGetFrame().FireProjectUpdatedEvent();
    wxGetFrame().SetModified();
}

void App::AppendSmith(wxArrayString& files)
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
                wxMessageBox(wxString("The project file ") << file << " is invalid and cannot be opened.",
                             "Import wxSmith project");
                return;
            }

            auto form = project.child("node");
            while (form)
            {
                g_NodeCreator.CreateNode(form, m_project.get());
                form = form.next_sibling("node");
            }
        }
    }
    wxGetFrame().FireProjectUpdatedEvent();
    wxGetFrame().SetModified();
}

void App::AppendXRC(wxArrayString& files)
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
                wxMessageBox(wxString("The project file ") << file << " is invalid and cannot be opened.",
                             "Import XRC project");
                return;
            }

            auto form = project.child("node");
            while (form)
            {
                g_NodeCreator.CreateNode(form, m_project.get());
                form = form.next_sibling("node");
            }
        }
    }
    wxGetFrame().FireProjectUpdatedEvent();
    wxGetFrame().SetModified();
}
