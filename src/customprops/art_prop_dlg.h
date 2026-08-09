///////////////////////////////////////////////////////////////////////////////
// Purpose:   Art Property Dialog for image property
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [08-09-2026]

#pragma once

#include "../wxui/art_prop_dlg_gen.h"

#include "img_props.h"

class ArtPropertyDlg : public ArtBrowserDialog
{
public:
    ArtPropertyDlg() {}  // If you use this constructor, you must call Create(parent)
    ArtPropertyDlg(wxWindow* parent) { Create(parent); }
    ArtPropertyDlg(wxWindow* parent, const ImageProperties& img_props);

    wxString GetResults();

protected:
    void OnInit(wxInitDialogEvent& event) override;
    void OnChooseClient(wxCommandEvent& event) override;
    void OnSelectItem(wxListEvent& event) override;

    void ChangeClient();

private:
    wxString m_client;
    wxue::string m_org_id;
    const char* m_id = nullptr;
};
