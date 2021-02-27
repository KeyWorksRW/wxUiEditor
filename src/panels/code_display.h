/////////////////////////////////////////////////////////////////////////////
// Purpose:   Display code in scintilla control
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/panel.h>

class wxStyledTextCtrl;

class wxFindDialogEvent;

class CodeDisplay : public wxPanel
{
public:
    CodeDisplay(wxWindow* parent, int id);

    void FindItemName(const wxString& name);

    wxStyledTextCtrl* GetTextCtrl() { return m_code; };

    void OnFind(wxFindDialogEvent& event);

private:
    wxStyledTextCtrl* m_code;
};
