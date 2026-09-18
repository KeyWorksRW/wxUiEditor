/////////////////////////////////////////////////////////////////////////////
// Purpose:   Test XRC
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "xrc_list_dlg.h"     // XrcListDlg (derived class)
#include "mainframe.h"        // MainFrame -- Main window frame
#include "project_handler.h"  // ProjectHandler class

XrcListDlg::XrcListDlg() {}

XrcListDlg::XrcListDlg(wxWindow* parent)
{
    Create(parent);
}

void XrcListDlg::OnInit(wxInitDialogEvent& /* event unused */)
{
    std::vector<Node*> forms;
    Project.CollectForms(forms);
    auto idx_cur_sel = 0;
    for (auto& form: forms)
    {
        // THis list should be synchronized with the list of forms in previews.cpp
        // (MainFrame::OnPreviewXrc)
        switch (form->get_GenName())
        {
            case gen_wxDialog:
            case gen_PanelForm:
            case gen_wxFrame:
            case gen_wxWizard:
            case gen_MenuBar:
            case gen_RibbonBar:
            case gen_ToolBar:
                {
                    auto index = m_listbox->Append(form->as_string(prop_class_name),
                                                   static_cast<void*>(form));
                    if (m_form == wxGetMainFrame()->getSelectedNode())
                    {
                        idx_cur_sel = index;
                    }
                }
                break;

            default:
                continue;  // Skip any other types of forms
        }
    }

    if (m_listbox->GetCount())
    {
        m_listbox->SetSelection(idx_cur_sel);
    }
}

void XrcListDlg::OnOK(wxCommandEvent& /* event unused */)
{
    auto idx = m_listbox->GetSelection();
    if (idx == wxNOT_FOUND)
    {
        EndModal(wxID_CANCEL);
        return;
    }

    m_form = static_cast<Node*>(m_listbox->GetClientData(idx));
    EndModal(wxID_OK);
}
