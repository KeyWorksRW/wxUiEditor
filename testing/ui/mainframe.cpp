/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main window
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "mainframe.h"  // auto-generated: ui/mainframe_base.h and ui/mainframe_base.cpp

#include "commonctrls.h"   // CommonCtrls -- Common controls dialog
#include "dlgmultitest.h"  // DlgMultiTest -- Multiple Tests dialog
#include "other_ctrls.h"   // OtherCtrlsDlg
#include "ribbondlg.h"     // RibbonDlg
#include "wizard.h"        // Wizard

#include "ui/choicebook_base.h"
#include "ui/listbook_base.h"
#include "ui/notebook_base.h"
#include "ui/toolbook_base.h"
#include "ui/treebook_base.h"

MainFrame::MainFrame(wxWindow* parent) : MainFrameBase(parent) {}

void MainFrame::OnMultiTestDialog(wxCommandEvent& WXUNUSED(event))
{
    DlgMultiTest dlg(this);
    dlg.ShowModal();
}

void MainFrame::OnCommonDialog(wxCommandEvent& WXUNUSED(event))
{
    CommonCtrls dlg(this);
    dlg.ShowModal();
}

void MainFrame::OnOtherCtrls(wxCommandEvent& WXUNUSED(event))
{
    OtherCtrlsDlg dlg(this);
    dlg.ShowModal();
}

void MainFrame::OnRibbonDialog(wxCommandEvent& WXUNUSED(event))
{
    RibbonDlg dlg(this);
    dlg.ShowModal();
}

void MainFrame::OnWizard(wxCommandEvent& WXUNUSED(event))
{
    Wizard wizard(this);
    wizard.Run();
}

void MainFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true forces the frame to close
    Close(true);
}

void MainFrame::OnChoicebook(wxCommandEvent& WXUNUSED(event))
{
    ChoiceBookBase dlg(this);
    dlg.ShowModal();
}

void MainFrame::OnNotebook(wxCommandEvent& WXUNUSED(event))
{
    NotebookBase dlg(this);
    dlg.ShowModal();
}

void MainFrame::OnListbook(wxCommandEvent& WXUNUSED(event))
{
    ListbookBase dlg(this);
    dlg.ShowModal();
}

void MainFrame::OnTreebook(wxCommandEvent& WXUNUSED(event))
{
    TreebookBase dlg(this);
    dlg.ShowModal();
}

void MainFrame::OnToolbook(wxCommandEvent& WXUNUSED(event))
{
    ToolbookBase dlg(this);
    dlg.ShowModal();
}
