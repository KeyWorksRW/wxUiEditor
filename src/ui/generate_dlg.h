/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for generating XRC file(s)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../wxui/generatedlg_base.h"  // auto-generated: ../wxui/generate_dlg_base.h and ../wxui/generate_dlg_base.cpp

class GenerateDlg : public GenerateDlgBase
{
public:
    GenerateDlg();  // If you use this constructor, you must call Create(parent)
    GenerateDlg(wxWindow* parent);

protected:
    void OnInit(wxInitDialogEvent& event) override;
};
