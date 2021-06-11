/////////////////////////////////////////////////////////////////////////////
// Purpose:   Convert one or more Windows Resource files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dbg_winres_dlg_base.h"

class wxFileHistory;

class DbgWinResDlg : public DbgWinResBase
{
public:
    DbgWinResDlg(wxWindow* parent = nullptr);

    auto& GetFilename() { return m_filename; }

protected:
    // Handlers for DbgWinResBase events
    void OnInit(wxInitDialogEvent& WXUNUSED(event)) override;
    void OnSelectFolder(wxCommandEvent& WXUNUSED(event)) override;
    void OnFolderBtn(wxCommandEvent& WXUNUSED(event)) override;
    void OnAffirmative(wxCommandEvent& WXUNUSED(event)) override;

private:
    wxFileHistory m_history;
    ttString m_filename;
};
