/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to display if wxUiEditor is launched with no arguments
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ui/startupdlg_base.h"

class StartupDlg : public StartupDlgBase
{
public:
    StartupDlg();  // If you use this constructor, you must call Create(parent)
    StartupDlg(wxWindow* parent);

    enum : size_t
    {
        START_MRU,
        START_CONVERT,
        START_OPEN,
        START_EMPTY,
    };

    auto GetCommandType() const { return m_cmdType; }
    ttString& GetProjectFile() { return m_value; }

protected:
    // Handlers for StartupDlgBase events
    void OnInit(wxInitDialogEvent& event) override;
    void OnImport(wxHyperlinkEvent& WXUNUSED(event)) override;
    void OnOpen(wxHyperlinkEvent& WXUNUSED(event)) override;
    void OnNew(wxHyperlinkEvent& WXUNUSED(event)) override;
    void OnHyperlink(wxHyperlinkEvent& event);

    ttString m_value;
    size_t m_cmdType { START_EMPTY };
};
