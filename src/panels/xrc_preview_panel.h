///////////////////////////////////////////////////////////////////////////////
// Purpose:   Live XRC preview page for the XRC panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
///////////////////////////////////////////////////////////////////////////////
// CR: [11-09-2026]

#pragma once

#include <wx/panel.h>  // Base header for wxPanel

class wxBoxSizer;
class Node;

// Displays the currently selected form as a live XRC preview. Forms that cannot be embedded in a
// wxPanel (wxFrame, wxWizard, MenuBar, RibbonBar, ToolBar) are described instead, with a button
// that opens them in a top-level window.
class XrcPreviewPanel : public wxPanel
{
public:
    explicit XrcPreviewPanel(wxWindow* parent);

    // Rebuilds the entire contents. form_node may be nullptr.
    void ShowForm(Node* form_node);

private:
    void ClearContent();

    // form_node non-null => add the "Open in Window" button
    void ShowMessage(const wxString& message, Node* form_node);

    wxBoxSizer* m_sizer { nullptr };
};
