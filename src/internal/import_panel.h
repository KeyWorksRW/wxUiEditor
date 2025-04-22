/////////////////////////////////////////////////////////////////////////////
// Purpose:   Panel to display original imported file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/fdrepdlg.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>  // wxScrolledWindow, wxScrolledControl and wxScrollHelper

#include "tt_view_vector.h"  // tt_view_vector -- Class for reading and writing line-oriented strings/files

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

    wxStyledTextCtrl* GetTextCtrl() { return m_scintilla; };

protected:
    void OnFind(wxFindDialogEvent& event);

private:
    tt_view_vector m_view;
    wxStyledTextCtrl* m_scintilla;
    tt_string m_import_file;
    int m_lexer;
};
