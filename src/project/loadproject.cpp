/////////////////////////////////////////////////////////////////////////////
// Purpose:   Load wxUiEditor project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <ttcwd.h>       // cwd -- Class for storing and optionally restoring the current directory
#include <ttmultistr.h>  // multistr -- Breaks a single string into multiple strings

#include "../nodes/node_creator.h"  // NodeCreator class
#include "mainapp.h"                // App -- Main application class
#include "mainframe.h"              // MainFrame -- Main window frame
#include "node.h"                   // Node class
#include "pjtsettings.h"            // ProjectSettings -- Hold data for currently loaded project
#include "uifuncs.h"                // Miscellaneous functions for displaying UI

#include "../ui/newproject.h"  // NewProjectDlg -- Dialog to create a new project
#include "import_formblder.h"  // FormBuilder -- Import a wxFormBuider project
#include "import_winres.h"     // WinResource -- Parse a Windows resource file
#include "import_wxsmith.h"    // WxSmith -- Import a wxSmith file
#include "oldproject.h"        // Load older version of wxUiEditor project

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
        appMsgBox(_ttc(strIdCantOpen) << file.wx_str() << "\n\n" << result.description(), _tt(strIdTitleLoadProject));
        return false;
    }

    auto root = doc.first_child();
    if (!ttlib::is_sameas(root.name(), "wxUiEditorData", tt::CASE::either))
    {
        appMsgBox(file.wx_str() + ttlib::cstr(" is not a wxUiEditor XML file"), _tt(strIdTitleLoadProject));
        return false;
    }

    NodeSharedPtr project;

    auto version = root.attribute("data_version").as_int((curWxuiMajorVer * 10) + curWxuiMinorVer);

    if (version > (curWxuiMajorVer * 10) + curWxuiMinorVer)
    {
        if (wxMessageBox("wxUiEditor does not recognize this version of the data file.\n"
                         "You may be able to load the file, but if you then save it you could lose data.\n\n"
                         "Do you want to try to open it anyway?",
                         "Unrecognized Version", wxYES_NO) != wxYES)
            return false;
    }

    else if (version < (curWxuiMajorVer * 10) + curWxuiMinorVer)
    {
        if (!root.child("object") && !root.child("node"))
        {
            appMsgBox(ttlib::cstr() << "The data file " << file.wx_str() << " is invalid and cannot be opened.");
            return false;
        }

        project = ::LoadOldProject(doc);
        m_isProject_updated = true;
    }

    else
    {
        if (!root.child("node"))
        {
            appMsgBox(ttlib::cstr() << "The data file " << file.wx_str() << " is invalid and cannot be opened.");
            return false;
        }

        project = LoadProject(doc);
    }

    if (!project)
    {
        ASSERT_MSG(project, ttlib::cstr() << "Failed trying to load " << file.wx_str());

        appMsgBox(ttlib::cstr() << "The project file " << file.wx_str() << " is invalid and cannot be opened.");
        return false;
    }

    m_project = project;

    m_pjtSettings->SetProjectFile(file);
    m_pjtSettings->SetProjectPath(file);

    wxGetFrame().FireProjectLoadedEvent();

    if (m_isProject_updated)
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
        appMsgBox(_tt("This wxUiEditor project file is invalid and cannot be loaded."), _tt(strIdTitleLoadProject));
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

NodeSharedPtr NodeCreator::CreateNode(pugi::xml_node& xml_obj, Node* parent)
{
    auto class_name = xml_obj.attribute("class").as_cview();
    if (class_name.empty())
        return NodeSharedPtr();

    //// Temporary until all projects converted
    if (class_name.is_sameas("wxListCtrl"))
        class_name = "wxListView";
    else if (class_name.is_sameas("Dialog"))
        class_name = "wxDialog";
    else if (class_name.is_sameas("Frame"))
        class_name = "wxFrame";
    else if (class_name.is_sameas("Wizard"))
        class_name = "wxWizard";
    else if (class_name.is_sameas("WizardPageSimple"))
        class_name = "wxWizardPageSimple";
    else if (class_name.is_sameas("Panel"))
        class_name = "PanelForm";

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

        ///////////////////////////////////// Begin slightly old project conversion ////////////////////////////////////////

        // Convert old name property into class_name and var_name
        if (iter.cname().is_sameas("name"))
        {
            if (new_node->IsForm())
            {
                if (auto prop = new_node->get_prop_ptr("class_name"); prop)
                {
                    prop->set_value(iter.value());
                    continue;
                }
            }
            else
            {
                if (auto prop = new_node->get_prop_ptr("var_name"); prop)
                {
                    prop->set_value(iter.value());
                    continue;
                }
            }
        }

        // Convert old orient name to orientationr
        else if (iter.cname().is_sameas("orient"))
        {
            if (auto prop = new_node->get_prop_ptr(txt_orientation); prop)
            {
                prop->set_value(iter.value());
                continue;
            }
        }

        // Convert old access name to class_access
        else if (iter.cname().is_sameas("access"))
        {
            if (auto prop = new_node->get_prop_ptr(txt_class_access); prop)
            {
                prop->set_value(iter.value());
                continue;
            }
        }

        // Convert old derived_name to derived_class_name
        else if (iter.cname().is_sameas("derived_name"))
        {
            if (auto prop = new_node->get_prop_ptr(txt_derived_class_name); prop)
            {
                prop->set_value(iter.value());
                continue;
            }
        }

        // Convert old hdr_preamble to  to derived_class_name
        else if (iter.cname().is_sameas("hdr_preamble"))
        {
            if (auto prop = new_node->get_prop_ptr(txt_base_hdr_includes); prop)
            {
                prop->set_value(iter.value());
                continue;
            }
        }

        // Convert old HDR; style bitmap to new image style
        else if (iter.cname().is_sameas("bitmap") && iter.cvalue().contains("HDR;"))
        {
            if (auto prop = new_node->get_prop_ptr("bitmap"); prop)
            {
                ttlib::cstr new_value = iter.value();
                new_value.Replace("HDR;", "XPM;");
                new_value.Replace("; [", "; ; [");

                prop->set_value(new_value);
                continue;
            }
        }

        // Convert old value property names
        else if (iter.cname().is_sameas("value"))
        {
            if (class_name.is_sameas("wxFilePickerCtrl") || class_name.is_sameas("wxDirPickerCtrl"))
            {
                if (auto prop = new_node->get_prop_ptr("initial_path"); prop)
                {
                    prop->set_value(iter.value());
                    continue;
                }
            }
            else if (class_name.is_sameas("wxFontPickerCtrl"))
            {
                if (auto prop = new_node->get_prop_ptr("initial_font"); prop)
                {
                    prop->set_value(iter.value());
                    continue;
                }
            }
        }

        /////////////////////////////////// End slightly old project conversion //////////////////////////////////////

        if (auto prop = new_node->get_prop_ptr(iter.name()); prop)
        {
            if (prop->type() == type_bool)
                prop->set_value(iter.as_bool());
            else
                prop->set_value(iter.value());
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
                // We get here if a property is specified that we don't recognize. While we can continue to load
                // just fine, if the user attempts to save the project than the property will be lost.

                // TODO: [KeyWorks - 06-03-2020] We need to store a list of unrecognized properties and display them to
                // the user all at once after the project is completely loaded. We also need to flag the project file as
                // unsaveable (only SaveAs can be used. See issue #69).

                MSG_WARNING(ttlib::cstr("Unrecognized property: ") << iter.name() << " in class: " << class_name);

                appMsgBox(ttlib::cstr().Format(
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
        parent->AddChild(new_node);
        new_node->SetParent(parent->GetSharedPtr());
    }

    for (auto child = xml_obj.child("node"); child; child = child.next_sibling("node"))
    {
        CreateNode(child, new_node.get());
    }

    return new_node;
}

bool App::ImportProject(ttString& file)
{
    if (file.has_extension(".fbp"))
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

    return false;
}

bool App::Import(ImportInterface& import, ttString& file, bool append)
{
    if (import.Import(file))
    {
        auto& doc = import.GetDocument();
        auto root = doc.first_child();
        auto project = root.child("node");
        if (!project || !project.attribute("class").as_cview().is_sameas("Project"))
        {
            ASSERT_MSG(project, ttlib::cstr() << "Failed trying to load converted xml document: " << file.wx_str());

            // TODO: [KeyWorks - 10-23-2020] Need to let the user know
            return false;
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

        wxGetFrame().SetImportedFlag();
        wxGetFrame().FireProjectLoadedEvent();
        wxGetFrame().SetModified();

        return true;
    }

    return false;
}

bool App::NewProject()
{
    if (m_frame->IsModified() && m_frame && !m_frame->SaveWarning())
        return false;

    NewProjectDlg dlg(m_frame);
    if (dlg.ShowModal() != wxID_OK)
        return false;

    delete m_pjtSettings;
    m_pjtSettings = new ProjectSettings;
    ttString file;
    file.assignCwd();
    file.append_filename("MyNewProject");
    m_pjtSettings->SetProjectFile(file);
    m_pjtSettings->SetProjectPath(file);

    m_project = g_NodeCreator.CreateNode("Project", nullptr);

    auto& file_list = dlg.GetFileList();
    if (file_list.size())
    {
        for (auto& iter: file_list)
        {
            try
            {
                if (iter.has_extension(".fbp"))
                {
                    FormBuilder fb;
                    Import(fb, iter, true);
                }
                else if (iter.has_extension(".wxs") || iter.has_extension(".xrc"))
                {
                    WxSmith smith;
                    Import(smith, iter, true);
                }
                else if (iter.has_extension(".rc") || iter.has_extension(".dlg"))
                {
                    WinResource winres;
                    Import(winres, iter, true);
                }
            }
            catch (const std::exception& /* e */)
            {
                // silently continue with the next project file
            }
        }
        m_frame->SetImportedFlag();
    }

    wxGetFrame().FireProjectLoadedEvent();
    if (m_project->GetChildCount())
        wxGetFrame().SetModified();
    return true;
}

void App::ImportWinRes(const ttlib::cstr& rc_file, std::vector<ttlib::cstr>& dialogs)
{
    WinResource winres;
    if (winres.ImportRc(rc_file, dialogs))
    {
        winres.InsertDialogs(dialogs);
        wxGetFrame().FireProjectUpdatedEvent();
        wxGetFrame().SetModified();
    }
}

void App::ImportFormBuilder(wxArrayString& files)
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
                appMsgBox(ttlib::cstr("The project file ") << files[pos].wx_str() << " is invalid and cannot be opened.",
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
