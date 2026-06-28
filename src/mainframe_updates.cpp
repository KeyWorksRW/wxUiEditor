/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main window frame Update() functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-27-2026]

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/wupdlock.h>     // wxWindowUpdateLocker prevents window redrawing

#include "wxue_namespace/wxue_string.h"  // wxue::string

#include "mainframe.h"

#include "preferences.h"      // Preferences -- Stores user preferences
#include "project_handler.h"  // ProjectHandler class

#include "panels/base_panel.h"  // BasePanel -- C++ panel
#include "panels/doc_view.h"    // Panel for displaying docs in wxWebView

#include "internal/import_panel.h"  // ImportPanel -- Panel to display original imported file

void MainFrame::UpdateFrame()
{
    wxue::string filename;

    if (Project.is_NewProject())
    {
        // For new projects, only show the path (no filename since there isn't one)
        const wxue::string project_path = Project.get_ProjectPath();
        if (!project_path.empty())
        {
            filename = "call Save As to save project (";
            filename += project_path;
            filename += ")";
        }
    }
    else
    {
        filename = Project.get_ProjectFile().filename();

        if (filename.empty())
        {
            filename = "untitled";
        }
        filename.remove_extension();

        // Append full path in parentheses
        const wxue::string project_path = Project.get_ProjectFile();
        if (!project_path.empty())
        {
            filename += "  (";
            filename += project_path;
            filename += ")";
        }
    }

    if (m_isProject_modified)
    {
        filename.insert(0, "*");
    }

    SetTitle(filename.wx());

    wxString menu_text = "Undo";
    if (m_undo_stack.IsUndoAvailable())
    {
        if (!m_undo_stack.GetUndoString().empty())
        {
            menu_text << ' ' << m_undo_stack.GetUndoString();
        }
    }
    menu_text << "\tCtrl+Z";
    if (auto* item = m_menuEdit->FindChildItem(wxID_UNDO); item)
    {
        item->SetItemLabel(menu_text);
    }

    menu_text = "Redo";
    if (m_undo_stack.IsRedoAvailable())
    {
        if (!m_undo_stack.GetRedoString().empty())
        {
            menu_text << ' ' << m_undo_stack.GetRedoString();
        }
    }
    menu_text << "\tCtrl+Y";
    if (auto* item = m_menuEdit->FindChildItem(wxID_REDO); item)
    {
        item->SetItemLabel(menu_text);
    }

    const bool isMockup = (m_notebook->GetPageText(m_notebook->GetSelection()) == "Mock Up");
    m_menuEdit->Enable(wxID_FIND, !isMockup);

    UpdateMoveMenu();
    UpdateLayoutTools();
    UpdateWakaTime();
}

void MainFrame::UpdateLanguagePanels()
{
    const wxWindowUpdateLocker freeze(this);

    // Temporarily remove end panels so language panels can be added in display order,
    // then restored at the end.
    if (m_importPanel)
    {
        m_notebook->RemovePage(m_notebook->GetPageIndex(m_importPanel));
    }
    if (m_docviewPanel)
    {
        m_notebook->RemovePage(m_notebook->GetPageIndex(m_docviewPanel));
    }

    const GenLang languages = Project.get_GenerateLanguages();

    // For each language: create the panel if newly enabled, or destroy it if disabled.
    auto manage_panel = [&](GenLang flag, BasePanel*& panel, const char* label)
    {
        if ((languages & flag) && !panel)
        {
            panel = new BasePanel(m_notebook, this, flag);
            m_notebook->AddPage(panel, label, false, wxWithImages::NO_IMAGE);
        }
        else if (!(languages & flag) && panel)
        {
            m_notebook->DeletePage(m_notebook->GetPageIndex(panel));
            panel = nullptr;
        }
    };

    manage_panel(GenLang::cplusplus, m_cppPanel, "C++");
    manage_panel(GenLang::python, m_pythonPanel, "Python");
    manage_panel(GenLang::ruby, m_rubyPanel, "Ruby");
    manage_panel(GenLang::fortran, m_fortranPanel, "Fortran");
    manage_panel(GenLang::go, m_goPanel, "GO");
    manage_panel(GenLang::julia, m_juliaPanel, "Julia");
    manage_panel(GenLang::luajit, m_luajitPanel, "LuaJIT");
    manage_panel(GenLang::typescript, m_typescriptPanel, "TypeScript");
    manage_panel(GenLang::xrc, m_xrcPanel, "XRC");

    // Ensure the preferred language panel sits at notebook position 1.
    struct LangInfo
    {
        GenLang flag;
        BasePanel* panel;
        const char* label;
    };
    const GenLang preferred = Project.get_CodePreference();
    const std::array lang_info = {
        LangInfo { .flag = GenLang::cplusplus, .panel = m_cppPanel, .label = "C++" },
        LangInfo { .flag = GenLang::python, .panel = m_pythonPanel, .label = "Python" },
        LangInfo { .flag = GenLang::ruby, .panel = m_rubyPanel, .label = "Ruby" },
        LangInfo { .flag = GenLang::fortran, .panel = m_fortranPanel, .label = "Fortran" },
        LangInfo { .flag = GenLang::go, .panel = m_goPanel, .label = "GO" },
        LangInfo { .flag = GenLang::julia, .panel = m_juliaPanel, .label = "Julia" },
        LangInfo { .flag = GenLang::luajit, .panel = m_luajitPanel, .label = "LuaJIT" },
        LangInfo { .flag = GenLang::typescript, .panel = m_typescriptPanel, .label = "TypeScript" },
        LangInfo { .flag = GenLang::xrc, .panel = m_xrcPanel, .label = "XRC" },
    };
    for (const auto& [flag, panel, label]: lang_info)
    {
        if (flag == preferred && panel)
        {
            const int pos = m_notebook->GetPageIndex(panel);
            if (pos != 1)
            {
                m_notebook->RemovePage(pos);
                m_notebook->InsertPage(1, panel, label, false, wxWithImages::NO_IMAGE);
            }
            break;
        }
    }

    // Restore end panels in fixed order: Import, Docs.
    if (m_importPanel)
    {
        m_notebook->AddPage(m_importPanel, "Import", false, wxWithImages::NO_IMAGE);
    }
    if (m_docviewPanel)
    {
        m_notebook->AddPage(m_docviewPanel, "Docs", false, wxWithImages::NO_IMAGE);
    }
}

void MainFrame::UpdateLayoutTools()
{
    int option = -1;
    int border = 0;
    int flag = 0;
    int orient = 0;

    const bool gotLayoutSettings = GetLayoutSettings(&flag, &option, &border, &orient);

    const bool enableHorizontalTools = (orient != wxHORIZONTAL) && gotLayoutSettings;
    m_menuEdit->Enable(id_AlignLeft, enableHorizontalTools);
    m_toolbar->EnableTool(id_AlignLeft, enableHorizontalTools);
    m_menuEdit->Check(id_AlignLeft, ((flag & (wxALIGN_RIGHT | wxALIGN_CENTER_HORIZONTAL)) == 0) &&
                                        enableHorizontalTools);
    m_toolbar->ToggleTool(id_AlignLeft,
                          ((flag & (wxALIGN_RIGHT | wxALIGN_CENTER_HORIZONTAL)) == 0) &&
                              enableHorizontalTools);

    m_menuEdit->Enable(id_AlignCenterHorizontal, enableHorizontalTools);
    m_toolbar->EnableTool(id_AlignCenterHorizontal, enableHorizontalTools);
    m_menuEdit->Check(id_AlignCenterHorizontal,
                      (flag & wxALIGN_CENTER_HORIZONTAL) && enableHorizontalTools);
    m_toolbar->ToggleTool(id_AlignCenterHorizontal,
                          (flag & wxALIGN_CENTER_HORIZONTAL) && enableHorizontalTools);

    m_menuEdit->Enable(id_AlignRight, enableHorizontalTools);
    m_toolbar->EnableTool(id_AlignRight, enableHorizontalTools);
    m_menuEdit->Check(id_AlignRight, (flag & wxALIGN_RIGHT) && enableHorizontalTools);
    m_toolbar->ToggleTool(id_AlignRight, (flag & wxALIGN_RIGHT) && enableHorizontalTools);

    const bool enableVerticalTools = (orient != wxVERTICAL) && gotLayoutSettings;
    m_menuEdit->Enable(id_AlignTop, enableVerticalTools);
    m_toolbar->EnableTool(id_AlignTop, enableVerticalTools);
    m_menuEdit->Check(id_AlignTop, ((flag & (wxALIGN_BOTTOM | wxALIGN_CENTER_VERTICAL)) == 0) &&
                                       enableVerticalTools);
    m_toolbar->ToggleTool(id_AlignTop, ((flag & (wxALIGN_BOTTOM | wxALIGN_CENTER_VERTICAL)) == 0) &&
                                           enableVerticalTools);

    m_menuEdit->Enable(id_AlignCenterVertical, enableVerticalTools);
    m_toolbar->EnableTool(id_AlignCenterVertical, enableVerticalTools);
    m_menuEdit->Check(id_AlignCenterVertical,
                      (flag & wxALIGN_CENTER_VERTICAL) && enableVerticalTools);
    m_toolbar->ToggleTool(id_AlignCenterVertical,
                          (flag & wxALIGN_CENTER_VERTICAL) && enableVerticalTools);

    m_menuEdit->Enable(id_AlignBottom, enableVerticalTools);
    m_toolbar->EnableTool(id_AlignBottom, enableVerticalTools);
    m_menuEdit->Check(id_AlignBottom, (flag & wxALIGN_BOTTOM) && enableVerticalTools);
    m_toolbar->ToggleTool(id_AlignBottom, (flag & wxALIGN_BOTTOM) && enableVerticalTools);

    m_menuEdit->Enable(id_BorderLeft, gotLayoutSettings);
    m_menuEdit->Enable(id_BorderRight, gotLayoutSettings);
    m_menuEdit->Enable(id_BorderTop, gotLayoutSettings);
    m_menuEdit->Enable(id_BorderBottom, gotLayoutSettings);

    m_toolbar->EnableTool(id_BorderLeft, gotLayoutSettings);
    m_toolbar->EnableTool(id_BorderRight, gotLayoutSettings);
    m_toolbar->EnableTool(id_BorderTop, gotLayoutSettings);
    m_toolbar->EnableTool(id_BorderBottom, gotLayoutSettings);

    m_menuEdit->Check(id_BorderTop, ((flag & wxTOP) != 0) && gotLayoutSettings);
    m_menuEdit->Check(id_BorderRight, ((flag & wxRIGHT) != 0) && gotLayoutSettings);
    m_menuEdit->Check(id_BorderLeft, ((flag & wxLEFT) != 0) && gotLayoutSettings);
    m_menuEdit->Check(id_BorderBottom, ((flag & wxBOTTOM) != 0) && gotLayoutSettings);

    m_toolbar->ToggleTool(id_BorderTop, ((flag & wxTOP) != 0) && gotLayoutSettings);
    m_toolbar->ToggleTool(id_BorderRight, ((flag & wxRIGHT) != 0) && gotLayoutSettings);
    m_toolbar->ToggleTool(id_BorderLeft, ((flag & wxLEFT) != 0) && gotLayoutSettings);
    m_toolbar->ToggleTool(id_BorderBottom, ((flag & wxBOTTOM) != 0) && gotLayoutSettings);

    m_menuEdit->Enable(id_Expand, gotLayoutSettings);
    m_toolbar->EnableTool(id_Expand, gotLayoutSettings);
    m_menuEdit->Check(id_Expand, ((flag & wxEXPAND) != 0) && gotLayoutSettings);
    m_toolbar->ToggleTool(id_Expand, ((flag & wxEXPAND) != 0) && gotLayoutSettings);
}

void MainFrame::UpdateMoveMenu()
{
    Node* node = m_selected_node.get();
    const Node* parent = nullptr;
    if (node)
    {
        parent = node->get_Parent();
    }
    if (!node || !parent)
    {
        m_menuEdit->Enable(id_MoveUp, false);
        m_menuEdit->Enable(id_MoveDown, false);
        m_menuEdit->Enable(id_MoveLeft, false);
        m_menuEdit->Enable(id_MoveRight, false);
        return;
    }

    m_menuEdit->Enable(id_MoveUp, node->MoveNode(MoveDirection::Up, true));
    m_menuEdit->Enable(id_MoveDown, node->MoveNode(MoveDirection::Down, true));
    m_menuEdit->Enable(id_MoveLeft, node->MoveNode(MoveDirection::Left, true));
    m_menuEdit->Enable(id_MoveRight, node->MoveNode(MoveDirection::Right, true));
}

void MainFrame::UpdateStatusWidths()
{
    if (m_MainSplitter)
    {
        m_MainSashPosition = m_MainSplitter->GetSashPosition();
    }
    if (m_SecondarySplitter)
    {
        m_SecondarySashPosition = m_SecondarySplitter->GetSashPosition();
    }

    const std::array<int, STATUS_PANELS> widths = {
        1, (m_MainSashPosition + m_SecondarySashPosition - STATUS_SASH_INSET), -1
    };
    SetStatusWidths(static_cast<int>(widths.size()), widths.data());
}

void MainFrame::UpdateWakaTime([[maybe_unused]] bool FileSavedEvent)
{
    if (m_wakatime && UserPrefs.is_WakaTimeEnabled())
    {
        m_wakatime->SendHeartbeat(FileSavedEvent);
    }
}
