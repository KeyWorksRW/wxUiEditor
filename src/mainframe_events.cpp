/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main window frame event handlers
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <format>

#include <wx/aboutdlg.h>     // declaration of wxAboutDialog class
#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/clipbrd.h>      // wxClipboad class and clipboard functions
#include <wx/config.h>       // wxConfig base header
#include <wx/filedlg.h>      // wxFileDialog base header
#include <wx/infobar.h>      // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/stc/stc.h>      // Scintilla

#include "wxue_namespace/wxue_string.h"  // wxue::string

#include "mainframe.h"

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "cstm_event.h"       // CustomEvent -- Custom Event class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties
#include "version.h"          // Version numbers and other constants

#include "preferences_dlg.h"  // PreferencesDlg -- Dialog for setting user preferences
#include "ui/startup_dlg.h"  // StartupDlg -- Dialog to display if wxUE is launched with no arguments

#include "panels/base_panel.h"      // BasePanel -- C++ panel
#include "panels/doc_view.h"        // Panel for displaying docs in wxWebView
#include "panels/nav_panel.h"       // NavigationPanel -- Node tree class
#include "panels/propgrid_panel.h"  // PropGridPanel -- Node inspector class

#include "mockup/mockup_parent.h"  // MockupParent -- Top-level MockUp Parent window

#include "internal/import_panel.h"  // ImportPanel -- Panel to display original imported file
#include "internal/msg_logging.h"   // MsgLogging -- Message logging class

#include "pugixml.hpp"

// The following is taken from wxui/ui_images.h
namespace wxue_img
{
    wxBitmapBundle bundle_debug_logo_svg(int width, int height);
    wxBitmapBundle bundle_wxUiEditor_svg(int width, int height);
}  // namespace wxue_img

auto MainFrame::OnAbout(wxCommandEvent&) -> void
{
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName(txtVersion);

    // Use trailing spaces to make the dialog width a bit wider
    wxue::string description;
    description
        << "wxUiEditor is a designer for wxWidgets\ngenerating C++, Python, Ruby and XRC code.\n";

    if (wxGetApp().isTestingMenuEnabled())
    {
        description << "\n" << Project.get_ProjectFile() << "  \n";
        description << "Original Project version: " << Project.get_OriginalProjectVersion() << "\n";
        description << "wxUiEditor Project version: " << curSupportedVer << "\n";
    }

    aboutInfo.SetDescription(description);
    aboutInfo.SetWebSite("https://github.com/KeyWorksRW/wxUiEditor");

    aboutInfo.SetCopyright(txtCopyRight);

    // Save this for when/if other people regularly contribute to the project
    // aboutInfo.AddDeveloper("Randalphwa");

#if defined(_DEBUG)
    aboutInfo.SetIcon(wxue_img::bundle_debug_logo_svg(64, 64).GetIconFor(this));
#else
    aboutInfo.SetIcon(wxue_img::bundle_wxUiEditor_svg(64, 64).GetIconFor(this));
#endif  // _DEBUG

    wxAboutBox(aboutInfo);
}

auto MainFrame::OnAppendCrafter(wxCommandEvent& /* event unused */) -> void
{
    wxue::SaveCwd cwd(wxue::restore_cwd);
    wxFileDialog dlg(this, "Open or Import Project", cwd.get_SavedCwd(), wxEmptyString,
                     "wxCrafter Project File (*.wxcp)|*.wxcp||",
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dlg.GetPaths(files);
        Project.AppendCrafter(files);
    }
}

auto MainFrame::OnAppendDialogBlocks(wxCommandEvent& /* event unused */) -> void
{
    wxue::SaveCwd cwd(wxue::restore_cwd);
    wxFileDialog dlg(this, "Open or Import Project", cwd.get_SavedCwd(), wxEmptyString,
                     "DialogBlocks Project File (*.pjd)|*.pjd||",
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dlg.GetPaths(files);
        Project.AppendDialogBlocks(files);
    }
}

auto MainFrame::OnAppendFormBuilder(wxCommandEvent& /* event unused */) -> void
{
    wxue::SaveCwd cwd(wxue::restore_cwd);
    wxFileDialog dlg(this, "Open or Import Project", cwd.get_SavedCwd(), wxEmptyString,
                     "wxFormBuilder Project File (*.fbp)|*.fbp||",
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dlg.GetPaths(files);
        Project.AppendFormBuilder(files);
    }
}

auto MainFrame::OnAppendGlade(wxCommandEvent& /* event unused */) -> void
{
    wxue::SaveCwd cwd(wxue::restore_cwd);
    wxFileDialog dlg(this, "Open or Import Project", cwd.get_SavedCwd(), wxEmptyString,
                     "wxGlade Project File (*.wxg)|*.wxg||",
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dlg.GetPaths(files);
        Project.AppendGlade(files);
    }
}

auto MainFrame::OnAppendSmith(wxCommandEvent& /* event unused */) -> void
{
    wxue::SaveCwd cwd(wxue::restore_cwd);
    wxFileDialog dlg(this, "Open or Import Project", cwd.get_SavedCwd(), wxEmptyString,
                     "wxSmith File (*.wxs)|*.wxs||",
                     wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dlg.GetPaths(files);
        Project.AppendSmith(files);
    }
}

auto MainFrame::OnAppendXRC(wxCommandEvent& /* event unused */) -> void
{
    wxue::SaveCwd cwd(wxue::restore_cwd);
    wxFileDialog dlg(this, "Open or Import Project", cwd.get_SavedCwd(), wxEmptyString,
                     "XRC File (*.xrc)|*.xrc||", wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxArrayString files;
        dlg.GetPaths(files);
        Project.AppendXRC(files);
    }
}

auto MainFrame::OnAuiNotebookPageChanged(wxAuiNotebookEvent& /* event unused */) -> void
{
    UpdateFrame();
    if (auto* page = m_notebook->GetCurrentPage(); page)
    {
        if (page == m_mockupPanel)
        {
            m_mockupPanel->CreateContent();
        }
#if wxUSE_WEBVIEW
        else if (page == m_docviewPanel)
        {
            m_docviewPanel->ActivatePage();
        }
#endif
        else if (page != m_importPanel)
        {
            dynamic_cast<BasePanel*>(page)->GenerateBaseClass();
        }
    }
}

auto MainFrame::OnBrowseDocs(wxCommandEvent& /* event unused */) -> void
{
    wxString url;
    url = (Project.get_LangVersion(GEN_LANG_CPLUSPLUS) < 30300) ?
              "https://docs.wxwidgets.org/3.2.8" :
              "https://docs.wxwidgets.org/latest";

    if (m_selected_node)
    {
        if (auto* generator = m_selected_node->get_Generator(); generator)
        {
            auto file = generator->GetHelpURL(m_selected_node.get());
            if (file.size())
            {
                url += "/class";
                if (file.starts_with("group"))
                {
                    url.RemoveLast(sizeof("class") - 1);
                }
                url << file;
                wxLaunchDefaultBrowser(url);
                return;
            }
        }
    }
    wxLaunchDefaultBrowser(url);
}

auto MainFrame::OnBrowsePython(wxCommandEvent& /* event unused */) -> void
{
    if (m_selected_node)
    {
        if (auto* generator = m_selected_node->get_Generator(); generator)
        {
            auto file = generator->GetPythonURL(m_selected_node.get());
            if (file.size())
            {
                wxString url("https://docs.wxpython.org/");
                url << file;
                wxLaunchDefaultBrowser(url);
                return;
            }
        }
    }
    wxLaunchDefaultBrowser("https://docs.wxpython.org/index.html");
}

auto MainFrame::OnBrowseRuby(wxCommandEvent& /* event unused */) -> void
{
    if (m_selected_node)
    {
        if (auto* generator = m_selected_node->get_Generator(); generator)
        {
            auto file = generator->GetRubyURL(m_selected_node.get());
            if (file.size())
            {
                wxString url("https://mcorino.github.io/wxRuby3/");
                url << file;
                wxLaunchDefaultBrowser(url);
                return;
            }
        }
    }
    wxLaunchDefaultBrowser("https://mcorino.github.io/wxRuby3/");
}

auto MainFrame::OnChangeAlignment(wxCommandEvent& event) -> void
{
    int align = 0;
    bool vertical = (event.GetId() == id_AlignTop || event.GetId() == id_AlignBottom ||
                     event.GetId() == id_AlignCenterVertical);

    switch (event.GetId())  // NOLINT (cppcheck-suppress)
    {
        case id_AlignRight:
            align = wxALIGN_RIGHT;

            break;

        case id_AlignCenterHorizontal:
            align = wxALIGN_CENTER_HORIZONTAL;

            break;

        case id_AlignBottom:
            align = wxALIGN_BOTTOM;

            break;

        case id_AlignCenterVertical:
            align = wxALIGN_CENTER_VERTICAL;

            break;
    }

    ChangeAlignment(m_selected_node.get(), align, vertical);

    UpdateLayoutTools();
}

auto MainFrame::OnChangeBorder(wxCommandEvent& event) -> void
{
    int border = 0;

    switch (event.GetId())
    {
        case id_BorderLeft:
            border = wxLEFT;
            break;

        case id_BorderRight:
            border = wxRIGHT;
            break;

        case id_BorderTop:
            border = wxTOP;
            break;

        case id_BorderBottom:
            border = wxBOTTOM;
            break;

        default:
            border = 0;
            break;
    }

    ToggleBorderFlag(m_selected_node.get(), border);

    UpdateLayoutTools();
}

auto MainFrame::OnClose(wxCloseEvent& event) -> void
{
    if (!SaveWarning())
    {
        return;
    }

    wxGetApp().setMainFrameClosing();

    auto* config = wxConfig::Get();
#if defined(_DEBUG)
    config->SetPath("/debug_mainframe");
#else
    config->SetPath("/mainframe");
#endif

    bool isIconized = IsIconized();
    bool isMaximized = IsMaximized();

    if (!isMaximized)
    {
        config->Write("PosX", isIconized ? -1 : GetPosition().x);
        config->Write("PosY", isIconized ? -1 : GetPosition().y);
        config->Write("SizeW", isIconized ? -1 : GetSize().GetWidth());
        config->Write("SizeH", isIconized ? -1 : GetSize().GetHeight());
    }

    config->Write("IsMaximized", isMaximized);
    config->Write("IsIconized", isIconized);

    config->SetPath(txt_main_window_config);
    m_FileHistory.Save(*config);
    m_property_panel->SaveDescBoxHeight();

    if (wxGetApp().isTestingMenuEnabled())
    {
        config->SetPath("/debug_history");
        m_ImportHistory.Save(*config);
        config->SetPath("/");

        if (g_pMsgLogging)
        {
            g_pMsgLogging->CloseLogger();
        }
    }
    event.Skip();
}

auto MainFrame::OnCopy(wxCommandEvent& /* event unused */) -> void
{
    if (auto win = wxWindow::FindFocus(); win && win->IsKindOf(wxCLASSINFO(wxStyledTextCtrl)))
    {
        wxStaticCast(win, wxStyledTextCtrl)->Copy();
        return;
    }
    if (m_selected_node)
    {
        CopyNode(m_selected_node.get());
        UpdateFrame();
    }
}

auto MainFrame::OnCut(wxCommandEvent& /* event unused */) -> void
{
    if (auto* win = wxWindow::FindFocus(); win && win->IsKindOf(wxCLASSINFO(wxStyledTextCtrl)))
    {
        // This is a read-only control, so we don't allow Cut
        return;
    }
    RemoveNode(m_selected_node.get(), true);
    UpdateFrame();
}

auto MainFrame::OnDelete(wxCommandEvent& /* event unused */) -> void
{
    ASSERT(m_selected_node);
    RemoveNode(m_selected_node.get(), false);
    UpdateFrame();
}

auto MainFrame::OnDifferentProject(wxCommandEvent& /* event unused */) -> void
{
    if (!SaveWarning())
    {
        return;
    }

    (void) DsisplayStartupDlg(m_nav_panel);
}

auto MainFrame::OnDuplicate(wxCommandEvent& /* event unused */) -> void
{
    ASSERT(m_selected_node);
    DuplicateNode(m_selected_node.get());
}

auto MainFrame::OnFind(wxFindDialogEvent& event) -> void
{
    if (auto* page = m_notebook->GetCurrentPage(); page)
    {
        event.StopPropagation();
        event.SetClientData(m_findDialog);
        page->GetEventHandler()->ProcessEvent(event);
    }
}

auto MainFrame::OnFindClose(wxFindDialogEvent&) -> void
{
    m_findDialog->Destroy();
    m_findDialog = nullptr;
}

auto MainFrame::OnFindDialog(wxCommandEvent&) -> void
{
    if (!m_findDialog)
    {
        if (auto* page = m_notebook->GetCurrentPage(); page)
        {
            if (wxGetApp().isTestingMenuEnabled() && page == m_importPanel)
            {
                m_findData.SetFindString(m_importPanel->GetTextCtrl()->GetSelectedText());
            }
            else if (page != m_mockupPanel && page != m_docviewPanel)
            {
                m_findData.SetFindString(dynamic_cast<BasePanel*>(page)->GetSelectedText());
            }
        }
        m_findDialog = new wxFindReplaceDialog(this, &m_findData, "Find");
        m_findDialog->Centre(wxCENTRE_ON_SCREEN | wxBOTH);
    }
    m_findDialog->Show(true);
}

auto MainFrame::OnImportProject(wxCommandEvent&) -> void
{
    if (!SaveWarning())
    {
        return;
    }

    if (g_pMsgLogging)
    {
        g_pMsgLogging->Clear();
    }

    Project.NewProject();
}

auto MainFrame::OnImportRecent(wxCommandEvent& event) -> void
{
    wxue::string file =
        m_ImportHistory.GetHistoryFile(event.GetId() - (START_IMPORT_FILE_IDS)).utf8_string();

    if (!SaveWarning())
    {
        return;
    }

    if (file.file_exists())
    {
        g_pMsgLogging->Clear();
        Project.ImportProject(file);
    }
    else if (wxMessageBox(wxString::Format("The project file '%s' doesn't exist.\n\nWould you "
                                           "like to remove it from the recent files list?",
                                           file.c_str()),
                          "Open recent project", wxICON_WARNING | wxYES_NO) == wxYES)
    {
        m_ImportHistory.RemoveFileFromHistory(event.GetId() - wxID_FILE1);
    }
}

auto MainFrame::OnNewProject(wxCommandEvent& /* event unused */) -> void
{
    if (!SaveWarning())
    {
        return;
    }

    // true means create an empty project
    Project.NewProject(true);
}

auto MainFrame::OnNodeSelected(CustomEvent& event) -> void
{
    // This event is normally only fired if the current selection has changed. We dismiss any
    // previous infobar message, and check to see if the current selection has any kind of issue
    // that we should warn the user about.
    m_info_bar->Dismiss();

    auto* evt_flags = event.getNode();

    if (evt_flags->is_Gen(gen_wxToolBar))
    {
        if (evt_flags->get_Parent()->is_Sizer())
        {
            auto* grandparent = evt_flags->get_Parent()->get_Parent();
            if (grandparent->is_Type(type_frame_form) ||
                grandparent->is_Gen(gen_wxAuiMDIChildFrame))
            {
                GetPropInfoBar()->ShowMessage("For the toolbar to be owned by the frame window, it "
                                              "should be placed directly under the frame, "
                                              "not under a sizer. Use Cut and Paste to move it.",
                                              wxICON_INFORMATION);
            }
        }
    }

    if (wxGetApp().isTestingMenuEnabled())
    {
        g_pMsgLogging->OnNodeSelected();
        m_importPanel->OnNodeSelected(evt_flags);
    }

    UpdateFrame();
}

auto MainFrame::OnOpenProject(wxCommandEvent&) -> void
{
    if (!SaveWarning())
    {
        return;
    }

    auto path = ShowOpenProjectDialog(this);
    if (!path.IsEmpty())
    {
        wxue::string filename = path.utf8_string();
        // The ".wxue" extension is only used for testing -- all normal projects should have a .wxui
        // extension
        if (filename.extension().is_sameas(PROJECT_FILE_EXTENSION, wxue::CASE::either) ||
            filename.extension().is_sameas(PROJECT_LEGACY_FILE_EXTENSION, wxue::CASE::either))
        {
            Project.LoadProject(filename);
        }
        else
        {
            Project.ImportProject(filename);
        }
    }
}

auto MainFrame::OnOpenRecentProject(wxCommandEvent& event) -> void
{
    if (!SaveWarning())
    {
        return;
    }
    wxue::string file = m_FileHistory.GetHistoryFile(event.GetId() - wxID_FILE1).utf8_string();

    if (file.file_exists())
    {
        Project.LoadProject(file);
    }
    else if (wxMessageBox(wxString::Format("The project file '%s' doesn't exist.\n\nWould you "
                                           "like to remove it from the recent files list?",
                                           file.c_str()),
                          "Open recent project", wxICON_WARNING | wxYES_NO) == wxYES)
    {
        m_FileHistory.RemoveFileFromHistory(event.GetId() - wxID_FILE1);
    }
}

auto MainFrame::OnPaste(wxCommandEvent& /* event unused */) -> void
{
    if (auto* win = wxWindow::FindFocus(); win && win->IsKindOf(wxCLASSINFO(wxStyledTextCtrl)))
    {
        // This is a read-only control, so we don't allow Paste
        return;  // we don't allow pasting into the code generation windows which are marked as
                 // read-only
    }
    if (m_selected_node)
    {
        PasteNode(m_selected_node.get());
        UpdateFrame();
    }
}

auto MainFrame::OnPreferencesDlg(wxCommandEvent& /* event unused */) -> void
{
    PreferencesDlg dlg(this);
    dlg.ShowModal();
}

auto MainFrame::OnProjectLoaded() -> void
{
    UpdateFrame();
}

auto MainFrame::OnReloadProject(wxCommandEvent& /* event unused */) -> void
{
    if (wxMessageBox(
            wxString() << "This will lose any changes you have made since the last save.\n\n"
                          "Are you sure you want to reload the project?",
            "Reload Project", wxICON_WARNING | wxYES_NO) == wxYES)
    {
        Project.LoadProject(Project.get_ProjectFile());
    }
}

auto MainFrame::OnSaveAsProject(wxCommandEvent& /* event unused */) -> void
{
    wxFileName filename(*Project.get_wxFileName());
    if (!filename.IsOk())
    {
        filename.Assign("MyProject");
    }

    // The ".wxue" extension is only used for testing -- all normal projects should have a .wxui
    // extension
    wxFileDialog dialog(
        this, "Save Project As", wxFileName::GetCwd(), filename.GetFullName(),
        wxString(std::format("wxUiEditor Project File (*{})|{};{}",
                     PROJECT_FILE_EXTENSION, PROJECT_FILE_EXTENSION,
                     PROJECT_LEGACY_FILE_EXTENSION))
                               .c_str()),
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() == wxID_OK)
    {
        filename = dialog.GetPath();
        // Note that under Windows, any extension the user added will be followed with a .wxui
        // extension
        auto ext = filename.GetExt();
        if (ext.empty())
        {
            filename.SetExt("wxui");
        }

        // Don't allow the user to walk over existing project file types that are probably
        // associated with another designer tool

        else if (ext.CmpNoCase("fbp") == 0)
        {
            wxMessageBox("You cannot save the project as a wxFormBuilder project file",
                         "Save Project As");
            return;
        }
        else if (ext.CmpNoCase("fjd") == 0)
        {
            wxMessageBox("You cannot save the project as a DialogBlocks project file",
                         "Save Project As");
            return;
        }
        else if (ext.CmpNoCase("wxg") == 0)
        {
            wxMessageBox("You cannot save the project as a wxGlade file", "Save Project As");
            return;
        }
        else if (ext.CmpNoCase("wxs") == 0)
        {
            wxMessageBox("You cannot save the project as a wxSmith file", "Save Project As");
            return;
        }
        else if (ext.CmpNoCase("xrc") == 0)
        {
            wxMessageBox("You cannot save the project as a XRC file", "Save Project As");
            return;
        }
        else if (ext.CmpNoCase("rc") == 0 || ext.CmpNoCase("dlg") == 0)
        {
            wxMessageBox("You cannot save the project as a Windows Resource file",
                         "Save Project As");
            return;
        }

        pugi::xml_document doc;
        Project.get_ProjectNode()->CreateDoc(doc);
        if (doc.save_file(filename.GetFullPath().utf8_string(), "  ",
                          pugi::format_indent_attributes))
        {
            m_isProject_modified = false;
            m_isImported = false;
            m_FileHistory.AddFileToHistory(filename.GetFullPath());
            Project.set_ProjectPath(&filename);
            ProjectSaved();
            FireProjectLoadedEvent();
        }
        else
        {
            wxMessageBox(wxString("Unable to save the project: ") << filename.GetFullPath(),
                         "Save Project As");
        }
    };
}

auto MainFrame::OnSaveProject(wxCommandEvent& event) -> void
{
    if (m_isImported || Project.get_ProjectFile().empty() ||
        Project.get_ProjectFile().filename().is_sameas(txtEmptyProject))
    {
        OnSaveAsProject(event);
    }
    else
    {
        if (Project.get_OriginalProjectVersion() != Project.get_ProjectVersion())
        {
            if (wxMessageBox("A project saved with this version of wxUiEditor is not compatible "
                             "with older versions of "
                             "wxUiEditor.\n"
                             "Continue with save?",
                             "Save Project", wxYES_NO) == wxNO)
            {
                return;
            }
            Project.UpdateOriginalProjectVersion();  // Don't ask again
        }
        pugi::xml_document doc;
        Project.get_ProjectNode()->CreateDoc(doc);
        if (doc.save_file(Project.get_ProjectFile(), "  ", pugi::format_indent_attributes))
        {
            m_isProject_modified = false;
            ProjectSaved();
        }
        else
        {
            wxMessageBox(wxString("Unable to save the project: ") << Project.get_ProjectFile(),
                         "Save Project");
        }
    }
}

auto MainFrame::OnToggleExpandLayout(wxCommandEvent& /* event unused */) -> void
{
    if (!m_selected_node || !m_selected_node->get_Parent() ||
        !m_selected_node->get_Parent()->is_Sizer())
    {
        return;
    }

    auto* propFlag = m_selected_node->get_PropPtr(prop_flags);

    if (!propFlag)
    {
        return;
    }

    auto currentValue = propFlag->as_string();
    auto wasExpanded = isPropFlagSet("wxEXPAND", currentValue);
    auto value = (wasExpanded ? ClearPropFlag("wxEXPAND", currentValue) :
                                SetPropFlag("wxEXPAND", currentValue));

    if (!wasExpanded)
    {
        auto* alignment = m_selected_node->get_PropPtr(prop_alignment);
        if (alignment && alignment->as_string().size())
        {
            // All alignment flags are invalid if wxEXPAND is set
            ModifyProperty(alignment, "");
        }
    }

    ModifyProperty(propFlag, value);
}

auto MainFrame::OnUpdateBrowseDocs(wxUpdateUIEvent& event) -> void
{
    if (m_selected_node)
    {
        if (auto* generator = m_selected_node->get_Generator(); generator)
        {
            wxString label = generator->GetHelpText(m_selected_node.get());
            if (label.empty())
            {
                label << "wxWidgets";
            }
            label << " Documentation";
            event.SetText(label);
            return;
        }
    }

    event.SetText("wxWidgets Documentation");
}

auto MainFrame::OnUpdateBrowsePython(wxUpdateUIEvent& event) -> void
{
    if (m_selected_node)
    {
        if (auto* generator = m_selected_node->get_Generator(); generator)
        {
            wxString label = generator->GetPythonHelpText(m_selected_node.get());
            if (label.empty())
            {
                label << "wxPython";
            }
            label << " Documentation";
            event.SetText(label);
            return;
        }
    }

    event.SetText("wxPython Documentation");
}

auto MainFrame::OnUpdateBrowseRuby(wxUpdateUIEvent& event) -> void
{
    if (m_selected_node)
    {
        if (auto* generator = m_selected_node->get_Generator(); generator)
        {
            wxString label = generator->GetRubyHelpText(m_selected_node.get());
            if (label.empty())
            {
                label << "wxRuby";
            }
            label << " Documentation";
            event.SetText(label);
            return;
        }
    }

    event.SetText("wxRuby Documentation");
}
