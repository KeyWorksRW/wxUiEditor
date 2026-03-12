/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main window frame Update() functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/wupdlock.h>     // wxWindowUpdateLocker prevents window redrawing

#include "wxue_namespace/wxue_string.h"  // wxue::string

#include "mainframe.h"

#include "preferences.h"      // Preferences -- Stores user preferences
#include "project_handler.h"  // ProjectHandler class

#include "panels/base_panel.h"  // BasePanel -- C++ panel
#include "panels/doc_view.h"    // Panel for displaying docs in wxWebView

#include "internal/import_panel.h"  // ImportPanel -- Panel to display original imported file

auto MainFrame::UpdateFrame() -> void
{
    wxue::string filename;
    if (UserPrefs.is_FullPathTitle())
    {
        filename = Project.get_ProjectFile();
    }
    else
    {
        filename = Project.get_ProjectFile().filename();
    }

    if (filename.empty())
    {
        filename = "untitled";
    }
    filename.remove_extension();

    if (m_isProject_modified)
    {
        filename.insert(0, "*");
    }
    SetTitle(filename.wx());

    wxString menu_text = "Undo";
    if (m_undo_stack.IsUndoAvailable())
    {
        if (m_undo_stack.GetUndoString().size())
        {
            menu_text << ' ' << m_undo_stack.GetUndoString();
        }
    }
    menu_text << "\tCtrl+Z";
    m_menuEdit->FindChildItem(wxID_UNDO)->SetItemLabel(menu_text);

    menu_text = "Redo";
    if (m_undo_stack.IsRedoAvailable())
    {
        if (m_undo_stack.GetRedoString().size())
        {
            menu_text << ' ' << m_undo_stack.GetRedoString();
        }
    }
    menu_text << "\tCtrl+Y";
    m_menuEdit->FindChildItem(wxID_REDO)->SetItemLabel(menu_text);

    bool isMockup = (m_notebook->GetPageText(m_notebook->GetSelection()) == "Mock Up");
    m_menuEdit->Enable(wxID_FIND, !isMockup);

    UpdateMoveMenu();
    UpdateLayoutTools();
    UpdateWakaTime();
}

auto MainFrame::UpdateLanguagePanels() -> void
{
    wxWindowUpdateLocker freeze(this);

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

    const auto languages = Project.get_GenerateLanguages();

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

    manage_panel(GEN_LANG_CPLUSPLUS, m_cppPanel, "C++");
    manage_panel(GEN_LANG_PYTHON, m_pythonPanel, "Python");
    manage_panel(GEN_LANG_FORTRAN, m_fortranPanel, "Fortran");
    manage_panel(GEN_LANG_GO, m_goPanel, "Go");
    manage_panel(GEN_LANG_JULIA, m_juliaPanel, "Julia");
    manage_panel(GEN_LANG_LUAJIT, m_luajitPanel, "LuaJIT");
    manage_panel(GEN_LANG_PERL, m_perlPanel, "Perl");
    manage_panel(GEN_LANG_RUBY, m_rubyPanel, "Ruby");
    manage_panel(GEN_LANG_RUST, m_rustPanel, "Rust");
    manage_panel(GEN_LANG_XRC, m_xrcPanel, "XRC");

    // Ensure the preferred language panel sits at notebook position 1.
    struct LangInfo
    {
        GenLang flag;
        BasePanel* panel;
        const char* label;
    };
    const auto preferred = Project.get_CodePreference();
    const std::array lang_info = {
        LangInfo { GEN_LANG_CPLUSPLUS, m_cppPanel, "C++" },
        LangInfo { GEN_LANG_PYTHON, m_pythonPanel, "Python" },
        LangInfo { GEN_LANG_FORTRAN, m_fortranPanel, "Fortran" },
        LangInfo { GEN_LANG_GO, m_goPanel, "Go" },
        LangInfo { GEN_LANG_JULIA, m_juliaPanel, "Julia" },
        LangInfo { GEN_LANG_LUAJIT, m_luajitPanel, "LuaJIT" },
        LangInfo { GEN_LANG_PERL, m_perlPanel, "Perl" },
        LangInfo { GEN_LANG_RUBY, m_rubyPanel, "Ruby" },
        LangInfo { GEN_LANG_RUST, m_rustPanel, "Rust" },
        LangInfo { GEN_LANG_XRC, m_xrcPanel, "XRC" },
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

auto MainFrame::UpdateLayoutTools() -> void
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

auto MainFrame::UpdateMoveMenu() -> void
{
    auto* node = m_selected_node.get();
    Node* parent = nullptr;
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

    m_menuEdit->Enable(id_MoveUp, MoveNode(node, MoveDirection::Up, true));
    m_menuEdit->Enable(id_MoveDown, MoveNode(node, MoveDirection::Down, true));
    m_menuEdit->Enable(id_MoveLeft, MoveNode(node, MoveDirection::Left, true));
    m_menuEdit->Enable(id_MoveRight, MoveNode(node, MoveDirection::Right, true));
}

auto MainFrame::UpdateStatusWidths() -> void
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
        1, (m_MainSashPosition + m_SecondarySashPosition - 16), -1
    };
    SetStatusWidths(static_cast<int>(widths.size()), widths.data());
}

auto MainFrame::UpdateWakaTime([[maybe_unused]] bool FileSavedEvent) -> void
{
    if (m_wakatime && UserPrefs.is_WakaTimeEnabled())
    {
        m_wakatime->SendHeartbeat(FileSavedEvent);
    }
}
