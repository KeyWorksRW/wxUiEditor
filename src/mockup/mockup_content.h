/////////////////////////////////////////////////////////////////////////////
// Purpose:   Mockup of a form's contents
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <unordered_map>

#include <wx/panel.h>  // Base header for wxPanel
#include <wx/pen.h>    // Base header for wxPen
#include <wx/sizer.h>  // provide wxSizer class for layout

#include "node_classes.h"  // Forward defintions of Node classes

class MockupParent;
class MockupWizard;

class MockupContent : public wxPanel
{
public:
    MockupContent(wxWindow* parent, MockupParent* mockupParent);

    void CreateAllGenerators();

    // Call this to switch to the correct page in a wizard, book, or ribbon
    void OnNodeSelected(Node* node);

    void RemoveNodes();

    Node* getNode(wxObject* wxobject);
    wxObject* Get_wxObject(Node* node);

    void SelectNode(wxObject* wxobject);

    // Set all of the inherited wxWindow interface properties
    //
    // convert_win is used by ConvertDialogToPixels
    static void SetWindowProperties(Node* node, wxWindow* window, wxWindow* convert_win);

protected:
    // The opposite of wxWindowBase::DoSetWindowVariant, this restores a windows variant to wxWINDOW_VARIANT_NORMAL
    void ResetWindowVariant();
    void MockupSetWindowVariant(wxWindowVariant variant);
    void CreateChildren(Node*, wxWindow* parent, wxObject* parentNode, wxBoxSizer* parent_sizer = nullptr);

private:
    MockupParent* m_mockupParent;
    wxBoxSizer* m_parent_sizer { nullptr };

    wxWindowVariant m_variant { wxWINDOW_VARIANT_NORMAL };

    // wxObject and Node are always paired, but we need to quickly lookup the pair given either a wxObject, or a
    // Node. We store two maps to maximize speed of either lookup.

    std::unordered_map<wxObject*, Node*> m_obj_node_pair;
    std::unordered_map<Node*, wxObject*> m_node_obj_pair;

    MockupWizard* m_wizard { nullptr };
};
