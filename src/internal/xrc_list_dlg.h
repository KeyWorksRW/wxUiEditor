/////////////////////////////////////////////////////////////////////////////
// Purpose:   Test XRC
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "generated/xrc_list_dlg_base.h"

class XrcListDlg : public XrcListDlgBase
{
public:
    XrcListDlg();  // If you use this constructor, you must call Create(parent)
    XrcListDlg(wxWindow* parent);

    Node* get_form() { return m_form; }

protected:
    // Handlers for XrcListDlgBase events
    void OnInit(wxInitDialogEvent& event) override;
    void OnOK(wxCommandEvent& event) override;

private:
    Node* m_form { nullptr };
};
