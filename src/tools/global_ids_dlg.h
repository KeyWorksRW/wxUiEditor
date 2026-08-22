/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to Globally edit Custom IDs
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [08-21-2026]

#pragma once

#include "global_ids_dlg_base.h"

class GlobalCustomIDS : public GlobalCustomIDSBase
{
public:
    GlobalCustomIDS();  // If you use this constructor, you must call Create(parent)
    GlobalCustomIDS(wxWindow* parent);

    // Hides GlobalCustomIDSBase::Create() -- while the base class is not virtual, this
    // lets us add the wxEVT_CLOSE_WINDOW binding that the generated base omits (the
    // generated base only binds wxEVT_BUTTON for wID_CLOSE, but the dialog's sizer
    // creates a wID_CANCEL button and the title-bar X sends wxEVT_CLOSE_WINDOW).
    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY,
                const wxString& title = "Globally Add Prefix/Suffix to Custom IDs",
                const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
                long flags = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxDialogNameStr);

protected:
    // Handlers for GlobalCustomIDSBase events
    void OnClose(wxCommandEvent& event) override;
    void OnCloseWindow(wxCloseEvent& event);
    void OnCommit(wxCommandEvent& event) override;
    void OnInit(wxInitDialogEvent& event) override;
    void OnSelectAllFolders(wxCommandEvent& event) override;
    void OnSelectAllForms(wxCommandEvent& event) override;
    void OnSelectFolders(wxCommandEvent& event) override;
    void OnSelectForms(wxCommandEvent& event) override;
    void OnSelectNoFolders(wxCommandEvent& event) override;
    void OnSelectNoForms(wxCommandEvent& event) override;
    void OnUpdate(wxCommandEvent& event) override;

private:
    void CommitIfNotCommitted();
    bool m_committed { true };
};
