/////////////////////////////////////////////////////////////////////////////
// Purpose:   Panel to display original imported file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/fdrepdlg.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>  // wxScrolledWindow, wxScrolledControl and wxScrollHelper

#include "wxue_namespace/wxue_string.h"       // wxue::string
#include "wxue_namespace/wxue_view_vector.h"  // wxue::ViewVector

class MainFrame;
class wxStyledTextCtrl;
class Node;

class ImportPanel : public wxScrolled<wxPanel>
{
public:
    ImportPanel(wxWindow* parent);

    // Clears scintilla and internal buffer, removes read-only flag in scintilla
    void Clear();

    void SetImportFile(const wxue::string& file, int lexer = 5 /* wxSTC_LEX_XML */);
    const wxue::string& GetImportFile() const { return m_import_file; }

    void OnNodeSelected(Node* node);

    wxStyledTextCtrl* GetTextCtrl() { return m_scintilla; };

protected:
    void OnFind(wxFindDialogEvent& event);

private:
    wxue::ViewVector m_view;
    wxStyledTextCtrl* m_scintilla;
    wxue::string m_import_file;
    int m_lexer;
};
