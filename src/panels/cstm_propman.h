/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxPropertyGridManager class with an HTML description box
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [11-07-2026]

// In order to use a custom wxPropertyGrid, you have to create a class derived from
// wxPropertyGridManager and override the CreatePropertyGrid method.

#pragma once

#include <wx/propgrid/manager.h>  // wxPropertyGridManager

#include "cstm_propgrid.h"  // CustomPropertyGrid -- Derived wxPropertyGrid class

class wxHtmlWindow;

class CustomPropertyManager : public wxPropertyGridManager
{
public:
    CustomPropertyManager() : wxPropertyGridManager() {}

    // Must be called after Create() has built the manager. Creates the wxHtmlWindow that overlays
    // the description box content area and binds the handlers that keep it in sync.
    void InitializeHtmlDescription();

    // Displays markdown_text (converted to HTML) in the description box. Empty text hides the HTML
    // window and leaves only the caption visible.
    void ShowDescription(const wxString& caption, const wxString& markdown_text);

    void ClearDescription();

    // Repositions the HTML window over the description box content area. Call this after anything
    // that could have moved or resized the description box.
    void SyncDescriptionWindow();

protected:
    wxPropertyGrid* CreatePropertyGrid() const { return new CustomPropertyGrid(); };

    void OnPropertyGridSelect(wxPropertyGridEvent& event);

    void OnDescriptionBoxSize(wxSizeEvent& event);
    void OnDescriptionBoxMotion(wxMouseEvent& event);
    void OnDescriptionBoxLeftUp(wxMouseEvent& event);

private:
    wxHtmlWindow* m_html_content { nullptr };

    // False when the last ShowDescription() call had no text -- SyncDescriptionWindow() must not
    // re-show the HTML window then, or it would leave the previous page visible.
    bool m_has_description { false };
};
