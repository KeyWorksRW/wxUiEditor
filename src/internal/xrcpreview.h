/////////////////////////////////////////////////////////////////////////////
// Purpose:   Test XRC
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "xrcpreview_base.h"

class XrcPreview : public XrcPreviewBase
{
public:
    XrcPreview();  // If you use this constructor, you must call Create(parent)
    XrcPreview(wxWindow* parent);

protected:
    void OnExport(wxCommandEvent& event) override;
    // Handlers for XrcPreviewBase events
    void OnInit(wxInitDialogEvent& event) override;
    void OnXrcCopy(wxCommandEvent& event) override;
    void OnCreate(wxCommandEvent& WXUNUSED(event)) override;
    void OnPreview(wxCommandEvent& WXUNUSED(event)) override;
};
