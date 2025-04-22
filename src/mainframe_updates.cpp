/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main window frame Update() functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/aui/auibook.h>  // wxaui: wx advanced user interface - notebook
#include <wx/wupdlock.h>     // wxWindowUpdateLocker prevents window redrawing

#include "mainframe.h"

#include "preferences.h"      // Preferences -- Stores user preferences
#include "project_handler.h"  // ProjectHandler class

#include "panels/base_panel.h"  // BasePanel -- C++ panel
#include "panels/doc_view.h"    // Panel for displaying docs in wxWebView

#include "internal/import_panel.h"  // ImportPanel -- Panel to display original imported file

void MainFrame::UpdateFrame()
{
    tt_string filename;
    if (UserPrefs.is_FullPathTitle())
        filename = Project.getProjectFile();
    else
        filename = Project.getProjectFile().filename();

    if (filename.empty())
    {
        filename = "untitled";
    }
    filename.remove_extension();

    if (m_isProject_modified)
    {
        filename.insert(0, "*");
    }
    SetTitle(filename.make_wxString());

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

void MainFrame::UpdateLanguagePanels()
{
    wxWindowUpdateLocker freeze(this);

    // Temporarily remove XRC and DocView panels which are at the end. This allows us to simply add
    // Language panels in order, then restore the XRC and DocView panels after all language panels
    // have been added.

    if (m_imnportPanel)
    {
        m_notebook->RemovePage(m_notebook->GetPageIndex(m_imnportPanel));
    }

    m_notebook->RemovePage(m_notebook->GetPageIndex(m_xrcPanel));
    if (m_docviewPanel)
        m_notebook->RemovePage(m_notebook->GetPageIndex(m_docviewPanel));

    auto languages = Project.getGenerateLanguages();
    if (languages & GEN_LANG_CPLUSPLUS && !m_cppPanel)
    {
        m_cppPanel = new BasePanel(m_notebook, this, GEN_LANG_CPLUSPLUS);
        if (Project.getCodePreference() == GEN_LANG_CPLUSPLUS)
        {
            m_notebook->InsertPage(1, m_cppPanel, "C++", false, wxWithImages::NO_IMAGE);
        }
        else
        {
            m_notebook->AddPage(m_cppPanel, "C++", false, wxWithImages::NO_IMAGE);
        }
    }
    else if (!(languages & GEN_LANG_CPLUSPLUS) && m_cppPanel)
    {
        m_notebook->DeletePage(m_notebook->GetPageIndex(m_cppPanel));
        m_cppPanel = nullptr;
    }

    if (languages & GEN_LANG_PERL && !m_perlPanel)
    {
        m_perlPanel = new BasePanel(m_notebook, this, GEN_LANG_PERL);
        if (Project.getCodePreference() == GEN_LANG_PERL)
        {
            m_notebook->InsertPage(1, m_perlPanel, "Perl", false, wxWithImages::NO_IMAGE);
        }
        else
        {
            m_notebook->AddPage(m_perlPanel, "Perl", false, wxWithImages::NO_IMAGE);
        }
    }
    else if (!(languages & GEN_LANG_PERL) && m_perlPanel)
    {
        m_notebook->DeletePage(m_notebook->GetPageIndex(m_perlPanel));
        m_perlPanel = nullptr;
    }

    if (languages & GEN_LANG_PYTHON && !m_pythonPanel)
    {
        m_pythonPanel = new BasePanel(m_notebook, this, GEN_LANG_PYTHON);
        if (Project.getCodePreference() == GEN_LANG_PYTHON)
        {
            m_notebook->InsertPage(1, m_pythonPanel, "Python", false, wxWithImages::NO_IMAGE);
        }
        else
        {
            m_notebook->AddPage(m_pythonPanel, "Python", false, wxWithImages::NO_IMAGE);
        }
    }
    else if (!(languages & GEN_LANG_PYTHON) && m_pythonPanel)
    {
        m_notebook->DeletePage(m_notebook->GetPageIndex(m_pythonPanel));
        m_pythonPanel = nullptr;
    }

    if (languages & GEN_LANG_RUBY && !m_rubyPanel)
    {
        m_rubyPanel = new BasePanel(m_notebook, this, GEN_LANG_RUBY);
        if (Project.getCodePreference() == GEN_LANG_RUBY)
        {
            m_notebook->InsertPage(1, m_rubyPanel, "Ruby", false, wxWithImages::NO_IMAGE);
        }
        else
        {
            m_notebook->AddPage(m_rubyPanel, "Ruby", false, wxWithImages::NO_IMAGE);
        }
    }
    else if (!(languages & GEN_LANG_RUBY) && m_rubyPanel)
    {
        m_notebook->DeletePage(m_notebook->GetPageIndex(m_rubyPanel));
        m_rubyPanel = nullptr;
    }

    if (languages & GEN_LANG_RUST && !m_rustPanel)
    {
        m_rustPanel = new BasePanel(m_notebook, this, GEN_LANG_RUST);
        if (Project.getCodePreference() == GEN_LANG_RUST)
        {
            m_notebook->InsertPage(1, m_rustPanel, "Rust", false, wxWithImages::NO_IMAGE);
        }
        else
        {
            m_notebook->AddPage(m_rustPanel, "Rust", false, wxWithImages::NO_IMAGE);
        }
    }
    else if (!(languages & GEN_LANG_RUST) && m_rustPanel)
    {
        m_notebook->DeletePage(m_notebook->GetPageIndex(m_rustPanel));
        m_rustPanel = nullptr;
    }

#if GENERATE_NEW_LANG_CODE
    if (languages & GEN_LANG_FORTRAN && !m_fortranPanel)
    {
        m_fortranPanel = new BasePanel(m_notebook, this, GEN_LANG_FORTRAN);
        if (Project.getCodePreference() == GEN_LANG_FORTRAN)
        {
            m_notebook->InsertPage(1, m_fortranPanel, "Fortran", false, wxWithImages::NO_IMAGE);
        }
        else
        {
            m_notebook->AddPage(m_fortranPanel, "Fortran", false, wxWithImages::NO_IMAGE);
        }
    }
    else if (!(languages & GEN_LANG_FORTRAN) && m_fortranPanel)
    {
        m_notebook->DeletePage(m_notebook->GetPageIndex(m_fortranPanel));
        m_fortranPanel = nullptr;
    }

    if (languages & GEN_LANG_HASKELL && !m_haskellPanel)
    {
        m_haskellPanel = new BasePanel(m_notebook, this, GEN_LANG_HASKELL);
        if (Project.getCodePreference() == GEN_LANG_HASKELL)
        {
            m_notebook->InsertPage(1, m_haskellPanel, "Haskell", false, wxWithImages::NO_IMAGE);
        }
        else
        {
            m_notebook->AddPage(m_haskellPanel, "Haskell", false, wxWithImages::NO_IMAGE);
        }
    }
    else if (!(languages & GEN_LANG_HASKELL) && m_haskellPanel)
    {
        m_notebook->DeletePage(m_notebook->GetPageIndex(m_haskellPanel));
        m_haskellPanel = nullptr;
    }

    if (languages & GEN_LANG_LUA && !m_luaPanel)
    {
        m_luaPanel = new BasePanel(m_notebook, this, GEN_LANG_LUA);
        if (Project.getCodePreference() == GEN_LANG_LUA)
        {
            m_notebook->InsertPage(1, m_luaPanel, "Lua", false, wxWithImages::NO_IMAGE);
        }
        else
        {
            m_notebook->AddPage(m_luaPanel, "Lua", false, wxWithImages::NO_IMAGE);
        }
    }
    else if (!(languages & GEN_LANG_LUA) && m_luaPanel)
    {
        m_notebook->DeletePage(m_notebook->GetPageIndex(m_luaPanel));
        m_luaPanel = nullptr;
    }
#endif  // GENERATE_NEW_LANG_CODE

    int position;
    switch (Project.getCodePreference())
    {
        case GEN_LANG_CPLUSPLUS:
            ASSERT(m_cppPanel);
            position = m_notebook->GetPageIndex(m_cppPanel);
            if (position != 1)
            {
                m_notebook->RemovePage(position);
                m_notebook->InsertPage(1, m_cppPanel, "C++", false, wxWithImages::NO_IMAGE);
            }
            break;

        case GEN_LANG_PERL:
            ASSERT(m_perlPanel);
            position = m_notebook->GetPageIndex(m_perlPanel);
            if (position != 1)
            {
                m_notebook->RemovePage(position);
                m_notebook->InsertPage(1, m_perlPanel, "Perl", false, wxWithImages::NO_IMAGE);
            }
            break;

        case GEN_LANG_PYTHON:
            ASSERT(m_pythonPanel);
            position = m_notebook->GetPageIndex(m_pythonPanel);
            if (position != 1)
            {
                m_notebook->RemovePage(position);
                m_notebook->InsertPage(1, m_pythonPanel, "Python", false, wxWithImages::NO_IMAGE);
            }
            break;

        case GEN_LANG_RUBY:
            ASSERT(m_rubyPanel);
            position = m_notebook->GetPageIndex(m_rubyPanel);
            if (position != 1)
            {
                m_notebook->RemovePage(position);
                m_notebook->InsertPage(1, m_rubyPanel, "Ruby", false, wxWithImages::NO_IMAGE);
            }
            break;

        case GEN_LANG_RUST:
            ASSERT(m_rustPanel);
            position = m_notebook->GetPageIndex(m_rustPanel);
            if (position != 1)
            {
                m_notebook->RemovePage(position);
                m_notebook->InsertPage(1, m_rustPanel, "Rust", false, wxWithImages::NO_IMAGE);
            }
            break;

#if GENERATE_NEW_LANG_CODE
        case GEN_LANG_FORTRAN:
            ASSERT(m_fortranPanel);
            position = m_notebook->GetPageIndex(m_fortranPanel);
            if (position != 1)
            {
                m_notebook->RemovePage(position);
                m_notebook->InsertPage(1, m_fortranPanel, "Fortran", false, wxWithImages::NO_IMAGE);
            }
            break;

        case GEN_LANG_HASKELL:
            ASSERT(m_haskellPanel);
            position = m_notebook->GetPageIndex(m_haskellPanel);
            if (position != 1)
            {
                m_notebook->RemovePage(position);
                m_notebook->InsertPage(1, m_haskellPanel, "Haskell", false, wxWithImages::NO_IMAGE);
            }
            break;

        case GEN_LANG_LUA:
            ASSERT(m_luaPanel);
            position = m_notebook->GetPageIndex(m_luaPanel);
            if (position != 1)
            {
                m_notebook->RemovePage(position);
                m_notebook->InsertPage(1, m_luaPanel, "Lua", false, wxWithImages::NO_IMAGE);
            }
            break;
#endif  // GENERATE_NEW_LANG_CODE

        case GEN_LANG_XRC:
            ASSERT(m_xrcPanel);
            position = m_notebook->GetPageIndex(m_xrcPanel);
            if (position != 1)
            {
                m_notebook->RemovePage(position);
                m_notebook->InsertPage(1, m_xrcPanel, "XRC", false, wxWithImages::NO_IMAGE);
            }
            break;

        default:
            break;
    }

    // Now add back the XRC and DocView panels at the end.
    if (m_imnportPanel)
    {
        m_notebook->AddPage(m_imnportPanel, "Import", false, wxWithImages::NO_IMAGE);
    }

    if (Project.getCodePreference() != GEN_LANG_XRC)
        m_notebook->AddPage(m_xrcPanel, "XRC", false, wxWithImages::NO_IMAGE);
    if (m_docviewPanel)
        m_notebook->AddPage(m_docviewPanel, "Docs", false, wxWithImages::NO_IMAGE);
}

void MainFrame::UpdateLayoutTools()
{
    int option = -1;
    int border = 0;
    int flag = 0;
    int orient = 0;

    bool gotLayoutSettings = GetLayoutSettings(&flag, &option, &border, &orient);

    bool enableHorizontalTools = (orient != wxHORIZONTAL) && gotLayoutSettings;
    m_menuEdit->Enable(id_AlignLeft, enableHorizontalTools);
    m_toolbar->EnableTool(id_AlignLeft, enableHorizontalTools);
    m_menuEdit->Check(id_AlignLeft, ((flag & (wxALIGN_RIGHT | wxALIGN_CENTER_HORIZONTAL)) == 0) && enableHorizontalTools);
    m_toolbar->ToggleTool(id_AlignLeft,
                          ((flag & (wxALIGN_RIGHT | wxALIGN_CENTER_HORIZONTAL)) == 0) && enableHorizontalTools);

    m_menuEdit->Enable(id_AlignCenterHorizontal, enableHorizontalTools);
    m_toolbar->EnableTool(id_AlignCenterHorizontal, enableHorizontalTools);
    m_menuEdit->Check(id_AlignCenterHorizontal, (flag & wxALIGN_CENTER_HORIZONTAL) && enableHorizontalTools);
    m_toolbar->ToggleTool(id_AlignCenterHorizontal, (flag & wxALIGN_CENTER_HORIZONTAL) && enableHorizontalTools);

    m_menuEdit->Enable(id_AlignRight, enableHorizontalTools);
    m_toolbar->EnableTool(id_AlignRight, enableHorizontalTools);
    m_menuEdit->Check(id_AlignRight, (flag & wxALIGN_RIGHT) && enableHorizontalTools);
    m_toolbar->ToggleTool(id_AlignRight, (flag & wxALIGN_RIGHT) && enableHorizontalTools);

    bool enableVerticalTools = (orient != wxVERTICAL) && gotLayoutSettings;
    m_menuEdit->Enable(id_AlignTop, enableVerticalTools);
    m_toolbar->EnableTool(id_AlignTop, enableVerticalTools);
    m_menuEdit->Check(id_AlignTop, ((flag & (wxALIGN_BOTTOM | wxALIGN_CENTER_VERTICAL)) == 0) && enableVerticalTools);
    m_toolbar->ToggleTool(id_AlignTop, ((flag & (wxALIGN_BOTTOM | wxALIGN_CENTER_VERTICAL)) == 0) && enableVerticalTools);

    m_menuEdit->Enable(id_AlignCenterVertical, enableVerticalTools);
    m_toolbar->EnableTool(id_AlignCenterVertical, enableVerticalTools);
    m_menuEdit->Check(id_AlignCenterVertical, (flag & wxALIGN_CENTER_VERTICAL) && enableVerticalTools);
    m_toolbar->ToggleTool(id_AlignCenterVertical, (flag & wxALIGN_CENTER_VERTICAL) && enableVerticalTools);

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
    auto node = m_selected_node.get();
    Node* parent = nullptr;
    if (node)
        parent = node->getParent();
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

    int widths[STATUS_PANELS] = { 1, (m_MainSashPosition + m_SecondarySashPosition - 16), -1 };
    SetStatusWidths(sizeof(widths) / sizeof(int), widths);
}

void MainFrame::UpdateWakaTime(bool FileSavedEvent)
{
    if (m_wakatime && UserPrefs.is_WakaTimeEnabled())
    {
        m_wakatime->SendHeartbeat(FileSavedEvent);
    }
}
