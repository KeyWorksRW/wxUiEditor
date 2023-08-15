/////////////////////////////////////////////////////////////////////////////
// Purpose:   Top-level MockUp Parent window
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/scrolwin.h>  // wxScrolledWindow, wxScrolledControl and wxScrollHelper

#include "node_classes.h"  // Forward defintions of Node classes

class CustomEvent;
class MockupTitleBar;
class MockupContent;
class wxStaticText;
class MainFrame;

class MockupParent : public wxScrolled<wxPanel>
{
public:
    MockupParent(wxWindow* parent, MainFrame* frame);

    Node* getSelectedForm() { return m_form; }
    bool IsShowingHidden() { return m_ShowHiddenControls; }
    bool IsMagnified() { return m_IsMagnifyWindow; }
    void ShowHiddenControls(bool show);
    void MagnifyWindow(bool show);
    void CreateContent();

    // Given a component object, locate it's Node and select it in the Navigation pane
    void SelectNode(wxObject* wxobject);
    void ClearIgnoreSelection() { m_isIgnoreSelection = false; }

    Node* getNode(wxObject* wxobject);
    wxObject* Get_wxObject(Node* node);

    MockupContent* GetMockupContent() { return m_panelContent; }

    wxObject* getChild(wxObject* wxobject, size_t childIndex);
    wxObject* GetParentNode(wxObject* wxobject);

protected:
    void OnNodeSelected(CustomEvent& event);
    void OnNodeDeleted(CustomEvent& event);
    void OnReCreateContent(CustomEvent&);
    void OnNodePropModified(CustomEvent&);

private:
    wxPanel* m_panelTitleBar;
    MockupContent* m_panelContent;

    wxStaticText* m_text_title;

    Node* m_form;

    // This is the panel that emulates the form, and hosts the title bar and content panels.
    wxPanel* m_MockupWindow;
    wxSize m_size_magnified;

    bool m_AreNodesCreated { false };
    bool m_IsMagnifyWindow { false };
    bool m_ShowHiddenControls { false };
    bool m_isIgnoreSelection { false };
    bool m_isPropertyChanging { false };
};
