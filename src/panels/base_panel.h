/////////////////////////////////////////////////////////////////////////////
// Purpose:   Code generation panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/panel.h>  // Base header for wxPanel

#include "node_classes.h"  // Forward defintions of Node classes

class CodeDisplay;
class CustomEvent;
class MainFrame;
class PanelCodeWriter;

class wxAuiNotebook;
class wxFindDialogEvent;
class wxStyledTextCtrl;

class BasePanel : public wxPanel
{
public:
    BasePanel(wxWindow* parent, MainFrame* frame, bool GenerateDerivedCode);
    ~BasePanel() override;

    void GenerateBaseClass();

    void OnFind(wxFindDialogEvent& event);
    void FindItemName(const wxString& name);

protected:
    void OnNodeSelected(CustomEvent& event);

private:
    CodeDisplay* m_cppPanel;
    CodeDisplay* m_hPanel;
    wxAuiNotebook* m_notebook;
    Node* m_cur_form { nullptr };

    std::unique_ptr<PanelCodeWriter> m_hdr_display;
    std::unique_ptr<PanelCodeWriter> m_src_display;

    bool m_GenerateDerivedCode;
};
