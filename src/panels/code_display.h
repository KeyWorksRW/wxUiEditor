/////////////////////////////////////////////////////////////////////////////
// Purpose:   Display code in scintilla control
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/panel.h>

#include "../ui/codedisplay_base.h"

class wxFindDialogEvent;

class CodeDisplay : public CodeDisplayBase
{
public:
    CodeDisplay(wxWindow* parent);

    void FindItemName(const wxString& name);

    wxStyledTextCtrl* GetTextCtrl() { return m_scintilla; };

protected:
    void OnFind(wxFindDialogEvent& event);
};
