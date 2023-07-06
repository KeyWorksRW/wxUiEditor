/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to Globally edit Custom IDs
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "global_ids_dlg_base.h"  // Generated header

#include "mainframe.h"        // MainFrame -- Main application window
#include "project_handler.h"  // ProjectHandler class

void MainFrame::OnEditCustomIds(wxCommandEvent& WXUNUSED(event))
{
    GlobalCustomIDS dlg(this);
    dlg.ShowModal();
}

void GlobalCustomIDS::OnInit(wxInitDialogEvent& event)
{
    event.Skip();  // transfer all validator data to their windows and update UI
}

void GlobalCustomIDS::OnSelectFolders(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnSelectFolders
}

void GlobalCustomIDS::OnSelectForms(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnSelectForms
}

void GlobalCustomIDS::OnSelectAllFolders(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnSelectAllFolders
}

void GlobalCustomIDS::OnSelectNoFolders(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnSelectNoFolders
}

void GlobalCustomIDS::OnSelectAllForms(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnSelectAllForms
}

void GlobalCustomIDS::OnSelectNoForms(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnSelectNoForms
}

void GlobalCustomIDS::OnRemovePrefix(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnRemovePrefix
}

void GlobalCustomIDS::OnRemoveSuffix(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnRemoveSuffix
}

void GlobalCustomIDS::OnAddPrefix(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnAddPrefix
}

void GlobalCustomIDS::OnAddSuffix(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnAddSuffix
}

void GlobalCustomIDS::OnCommit(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnCommit
}
