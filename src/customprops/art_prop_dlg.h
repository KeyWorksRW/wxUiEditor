/////////////////////////////////////////////////////////////////////////////
// Purpose:   Art Property Dialog for image property
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>  // wxArtProvider class

#include "../ui/artpropdlg_base.h"

#include "img_props.h"  // ImageProperties -- Handles property grid image properties

class ArtBrowserDialog : public ArtPropertyDlgBase
{
public:
    ArtBrowserDialog(wxWindow* parent, const ImageProperties& img_props);

    wxString GetResults();

protected:
    void OnChooseClient(wxCommandEvent& WXUNUSED(event)) override;
    void OnSelectItem(wxListEvent& WXUNUSED(event)) override;

    void ChangeClient();

private:
    wxString m_client;
    const char* m_id;
};
