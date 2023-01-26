/////////////////////////////////////////////////////////////////////////////
// Purpose:   Panel to display original imported file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/panel.h>
#include <wx/scrolwin.h>  // wxScrolledWindow, wxScrolledControl and wxScrollHelper

class MainFrame;
class wxStyledTextCtrl;
class Node;

class ImportPanel : public wxScrolled<wxPanel>
{
public:
    ImportPanel(wxWindow* parent);

    // Clears scintilla and internal buffer, removes read-only flag in scintilla
    void Clear();

    void SetImportFile(const tt_string& file, int lexer = 5 /* wxSTC_LEX_XML */);

    void OnNodeSelected(Node* node);

protected:
    void OnFind(wxFindDialogEvent& event);

private:
    tt_view_vector m_view;
    wxStyledTextCtrl* m_scintilla;
    int m_lexer;
};
