/////////////////////////////////////////////////////////////////////////////
// Purpose:   Load wxUiEditor project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "ttcwd.h"       // cwd -- Class for storing and optionally restoring the current directory
#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "../nodes/node_creator.h"  // NodeCreator class
#include "base_generator.h"         // BaseGenerator -- Base widget generator class
#include "gen_enums.h"              // Enumerations for generators
#include "mainapp.h"                // App -- Main application class
#include "mainframe.h"              // MainFrame -- Main window frame
#include "node.h"                   // Node class
#include "pjtsettings.h"            // ProjectSettings -- Hold data for currently loaded project

using namespace GenEnum;

#include "../import/import_formblder.h"  // FormBuilder -- Import a wxFormBuider project
#include "../import/import_wxcrafter.h"  // WxCrafter -- Import a wxCrafter project
#include "../import/import_wxglade.h"    // WxGlade -- Import a wxGlade file
#include "../import/import_wxsmith.h"    // WxSmith -- Import a wxSmith file
#include "../ui/import_dlg.h"            // ImportDlg -- Dialog to create a new project
#include "../winres/import_winres.h"     // WinResource -- Parse a Windows resource file
#include "node_gridbag.h"                // GridBag -- Create and modify a node containing a wxGridBagSizer

using namespace GenEnum;

bool App::LoadProject(const ttString& file)
{
    delete m_pjtSettings;
    m_pjtSettings = new ProjectSettings;

    m_pjtSettings->SetProjectFile(file);

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

    NodeSharedPtr project;

    m_ProjectVersion = root.attribute("data_version").as_int((curWxuiMajorVer * 10) + curWxuiMinorVer);

    if (m_ProjectVersion > curCombinedVer)
    {
        if (wxMessageBox("wxUiEditor does not recognize this version of the data file.\n"
                         "You may be able to load the file, but if you then save it you could lose data.\n\n"
                         "Do you want to try to open it anyway?",
                         "Unrecognized Version", wxYES_NO) != wxYES)
            return false;
    }

    else if (m_ProjectVersion < curCombinedVer)
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
        project = LoadProject(doc);
    }

    else
    {
        if (!root.child("node"))
        {
            wxMessageBox(wxString() << "The data file " << file << " is invalid and cannot be opened.");
            return false;
        }

        project = LoadProject(doc);
    }

    if (!project)
    {
        ASSERT_MSG(project, ttlib::cstr() << "Failed trying to load " << file.wx_str());

        wxMessageBox(wxString() << "The project file " << file << " is invalid and cannot be opened.");
        return false;
    }

    m_project = project;

    m_pjtSettings->SetProjectFile(file);
    m_pjtSettings->SetProjectPath(file);

    // Start a thread to collect all of the embedded images
    m_pjtSettings->ParseEmbeddedImages();

    // Imported projects should be set to version 13 to so that they pass through the old project fixups.
    if (m_ProjectVersion == 13)
    {
        m_ProjectVersion = curCombinedVer;
    }

    wxGetFrame().SetImportedFlag(false);
    wxGetFrame().FireProjectLoadedEvent();

    if (m_isProject_updated || m_ProjectVersion < curCombinedVer)
        wxGetFrame().SetModified();

    return true;
}

NodeSharedPtr App::LoadProject(pugi::xml_document& doc)
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
        project = g_NodeCreator.CreateNode(node);
    }
    catch (const std::exception& DBG_PARAM(e))
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
    auto class_name = xml_obj.attribute("class").as_cview();
    if (class_name.empty())
        return NodeSharedPtr();

    // This should never be the case, but let's switch it in the off chance it slips through
    if (class_name.is_sameas("wxListCtrl"))
        class_name = "wxListView";

    auto new_node = CreateNode(class_name, parent);
    if (!new_node)
    {
        FAIL_MSG(ttlib::cstr() << "Invalid project file: could not create " << class_name);
        throw std::runtime_error("Invalid project file");
    }

    for (auto& iter: xml_obj.attributes())
    {
        if (iter.cname().is_sameas("class"))
            continue;

        if (iter.cname().is_sameprefix("wxEVT_"))
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

                // Imported projects should be set as version 13 to get the fixups of constant to friendly name, and bit flag
                // conflict resolution.

                else if (wxGetApp().GetProjectVersion() < 14)
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

            if (auto value = iter.value(); *value)
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
                    "The property named \"%s\" of class \"%s\" is not supported by this version of wxUiEditor.\n"
                    "If your project file was just converted from an older version, then the conversion was not "
                    "complete.\n"
                    "Otherwise, this project is from a newer version of wxUiEditor.\n\n"
                    "The property's value is: %s\n"
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

bool App::ImportProject(ttString& file)
{
    if (file.has_extension(".wxcp"))
    {
        WxCrafter crafter;
        return Import(crafter, file);
    }
    else if (file.has_extension(".fbp"))
    {
        FormBuilder fb;
        return Import(fb, file);
    }
    else if (file.has_extension(".rc") || file.has_extension(".dlg"))
    {
        WinResource winres;
        return Import(winres, file);
    }
    else if (file.has_extension(".wxs") || file.has_extension(".xrc"))
    {
        WxSmith smith;
        return Import(smith, file);
    }
    else if (file.has_extension(".wxg"))
    {
        WxGlade glade;
        return Import(glade, file);
    }

    return false;
}

bool App::Import(ImportXML& import, ttString& file, bool append)
{
    if (import.Import(file))
    {
#if defined(_DEBUG)
        ttString full_path(file);
        full_path.make_absolute();
        wxGetFrame().GetAppendImportHistory()->AddFileToHistory(full_path);
#endif  // _DEBUG

        // By having the importer create an XML document, we can pass it through g_NodeCreator.CreateNode() which will
        // fix bitflag conflicts, convert wxWidgets constants to friendly names, and handle old-project style conversions.

        auto& doc = import.GetDocument();
        auto root = doc.first_child();
        auto project = root.child("node");
        if (!project || !project.attribute("class").as_cview().is_sameas("Project"))
        {
            ASSERT_MSG(project, ttlib::cstr() << "Failed trying to load converted xml document: " << file.wx_str());

            // TODO: [KeyWorks - 10-23-2020] Need to let the user know
            return false;
        }

        if (m_pjtSettings && m_pjtSettings->GetProjectFile().filename() == "MyImportedProject")
        {
            m_pjtSettings->getProjectFile().remove_filename();
            m_pjtSettings->getProjectFile().append_filename(file.filename().wx_str());
            m_pjtSettings->getProjectFile().remove_extension();
        }

        if (append)
        {
            auto form = project.child("node");
            while (form)
            {
                g_NodeCreator.CreateNode(form, m_project.get());
                form = form.next_sibling("node");
            }

            return true;
        }

        delete m_pjtSettings;
        m_pjtSettings = new ProjectSettings;

        m_project = g_NodeCreator.CreateNode(project);

        file.remove_extension();
        m_pjtSettings->SetProjectFile(file);
        m_pjtSettings->SetProjectPath(file);

        // Start a thread to collect all of the embedded images
        m_pjtSettings->ParseEmbeddedImages();

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
        delete m_pjtSettings;
        m_pjtSettings = new ProjectSettings;
        ttString file;
        file.assignCwd();
        file.append_filename("MyNewProject");
        m_pjtSettings->SetProjectFile(file);
        m_pjtSettings->SetProjectPath(file);

        m_project = g_NodeCreator.CreateNode(gen_Project, nullptr);

        wxGetFrame().FireProjectLoadedEvent();
        return true;
    }

    ImportDlg dlg(m_frame);
    if (dlg.ShowModal() != wxID_OK)
        return false;

    delete m_pjtSettings;
    m_pjtSettings = new ProjectSettings;
    ttString file;
    file.assignCwd();
    file.append_filename("MyImportedProject");
    m_pjtSettings->SetProjectFile(file);
    m_pjtSettings->SetProjectPath(file);

    m_project = g_NodeCreator.CreateNode(gen_Project, nullptr);

    ttlib::cstr imported_from;

    auto& file_list = dlg.GetFileList();
    if (file_list.size())
    {
        for (auto& iter: file_list)
        {
            try
            {
                if (iter.has_extension(".wxcp"))
                {
                    WxCrafter crafter;
                    Import(crafter, iter, true);
                }
                else if (iter.has_extension(".fbp"))
                {
                    FormBuilder fb;
                    Import(fb, iter, true);
                }
                else if (iter.has_extension(".wxs") || iter.has_extension(".xrc"))
                {
                    WxSmith smith;
                    Import(smith, iter, true);
                }
                else if (iter.has_extension(".wxg"))
                {
                    WxGlade glade;
                    Import(glade, iter, true);
                }
                else if (iter.has_extension(".rc") || iter.has_extension(".dlg"))
                {
                    WinResource winres;
                    Import(winres, iter, true);
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
            m_pjtSettings->SetProjectFile(path);
            path.remove_filename();
            m_pjtSettings->SetProjectPath(path);
        }
        m_frame->SetImportedFlag();
    }

    // Start a thread to collect all of the embedded images
    m_pjtSettings->ParseEmbeddedImages();

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
        auto project = winres.GetProjectPtr();
        for (size_t idx_child = 0; idx_child < project->GetChildCount(); ++idx_child)
        {
            auto new_node = g_NodeCreator.MakeCopy(project->GetChildPtr(idx_child));
            m_project->Adopt(new_node);
        }

        wxGetFrame().FireProjectUpdatedEvent();
        wxGetFrame().SetModified();
    }
}

void App::AppendCrafter(wxArrayString& files)
{
    for (size_t pos = 0; pos < files.size(); ++pos)
    {
        WxCrafter crafter;

        if (crafter.Import(files[pos]))
        {
            auto& doc = crafter.GetDocument();
            auto root = doc.first_child();
            auto project = root.child("node");
            if (!project || project.attribute("class").as_cstr() != "Project")
            {
                wxMessageBox(wxString("The project file ") << files[pos] << " is invalid and cannot be opened.",
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
    for (size_t pos = 0; pos < files.size(); ++pos)
    {
        FormBuilder fb;

        if (fb.Import(files[pos]))
        {
            auto& doc = fb.GetDocument();
            auto root = doc.first_child();
            auto project = root.child("node");
            if (!project || project.attribute("class").as_cstr() != "Project")
            {
                wxMessageBox(wxString("The project file ") << files[pos] << " is invalid and cannot be opened.",
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
    for (size_t pos = 0; pos < files.size(); ++pos)
    {
        WxGlade glade;

        if (glade.Import(files[pos]))
        {
            auto& doc = glade.GetDocument();
            auto root = doc.first_child();
            auto project = root.child("node");
            if (!project || project.attribute("class").as_cstr() != "Project")
            {
                wxMessageBox(wxString("The project file ") << files[pos] << " is invalid and cannot be opened.",
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
    for (size_t pos = 0; pos < files.size(); ++pos)
    {
        WxSmith smith;

        if (smith.Import(files[pos]))
        {
            auto& doc = smith.GetDocument();
            auto root = doc.first_child();
            auto project = root.child("node");
            if (!project || project.attribute("class").as_cstr() != "Project")
            {
                wxMessageBox(wxString("The project file ") << files[pos] << " is invalid and cannot be opened.",
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
    for (size_t pos = 0; pos < files.size(); ++pos)
    {
        // wxSmith files are a superset of XRC files, so we use the wxSmith class to process both
        WxSmith smith;

        if (smith.Import(files[pos]))
        {
            auto& doc = smith.GetDocument();
            auto root = doc.first_child();
            auto project = root.child("node");
            if (!project || project.attribute("class").as_cstr() != "Project")
            {
                wxMessageBox(wxString("The project file ") << files[pos] << " is invalid and cannot be opened.",
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

const ttlib::cstr& App::getProjectFileName()
{
    if (!m_pjtSettings)
        m_pjtSettings = new ProjectSettings;

    return m_pjtSettings->getProjectFile();
}

const ttlib::cstr& App::getProjectPath()
{
    if (!m_pjtSettings)
        m_pjtSettings = new ProjectSettings;

    return m_pjtSettings->getProjectPath();
}

ttString App::GetProjectFileName()
{
    if (!m_pjtSettings)
        m_pjtSettings = new ProjectSettings;

    return m_pjtSettings->GetProjectFile();
}

ttString App::GetProjectPath()
{
    if (!m_pjtSettings)
        m_pjtSettings = new ProjectSettings;

    return m_pjtSettings->GetProjectPath();
}
