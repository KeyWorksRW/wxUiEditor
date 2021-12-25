/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid class for node properties and events
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <set>

#include <wx/propgrid/manager.h>  // wxPropertyGridManager

#include "../nodes/node_classes.h"  // Forward defintions of Node classes
#include "cstm_propman.h"           // CustomPropertyManager -- Derived wxPropertyGrid class
#include "gen_enums.h"              // Enumerations for generators

using namespace GenEnum;

using PropNameSet = std::set<PropName>;
using EventSet = std::set<std::string>;

class CustomEvent;
class MainFrame;
class wxAuiNotebook;

class PropGridPanel : public wxPanel
{
public:
    PropGridPanel(wxWindow* parent, MainFrame* frame);

    void RestoreDescBoxHeight();
    void SaveDescBoxHeight();

    // Returns true if a validation failure message has already been displayed to the user
    bool WasFailureHandled() { return m_failure_handled; }

    // Prevents creation of node's properties until UnLock() is called.
    void Lock() { m_locked = true; }

    // Allows creation of node properties
    void UnLock() { m_locked = false; }

    // Creates properties for currently selected node.
    void Create();

protected:
    wxString GetCategoryDisplayName(const wxString& original);

    // Called to determine if a property should be displayed or not
    bool IsPropAllowed(Node* node, NodeProperty* prop);

    void CreatePropCategory(ttlib::cview name, Node* node, NodeDeclaration* obj_info, PropNameSet& prop_set);
    void CreateEventCategory(ttlib::cview name, Node* node, NodeDeclaration* obj_info, EventSet& event_set);
    void CreateLayoutCategory(Node* node);

    void AddEvents(ttlib::cview name, Node* node, NodeCategory& category, EventSet& event_set);
    void AddProperties(ttlib::cview name, Node* node, NodeCategory& category, PropNameSet& prop_set,
                       bool is_child_cat = false);

    void ReplaceDrvName(const wxString& formName, NodeProperty* propType);
    void ReplaceBaseFile(const wxString& formName, NodeProperty* propType);
    void ReplaceDrvFile(const wxString& formName, NodeProperty* propType);

    wxPGProperty* GetProperty(NodeProperty* prop);

    // Called after a property has been modified, this checks to see if various property
    // items need to be enabled or disabled based on the current value of the changed
    // property.
    void ChangeEnableState(NodeProperty* changed_prop);

    void ReselectItem();

    void ModifyProperty(NodeProperty* prop, const wxString& str);
    void modifyProperty(NodeProperty* prop, ttlib::cview str);

    int GetBitlistValue(const wxString& strVal, wxPGChoices& bit_flags);

    // The VerifyChange...() functions are called when a property is changing. The function is used to verify that the change
    // is valid, and if not, the user is warned and the wxEVT_PG_CHANGING event is vetoed.

    void VerifyChangeFile(wxPropertyGridEvent& event, NodeProperty* prop, Node* node);

    // Event handlers

    void OnChildFocus(wxChildFocusEvent& event);
    void OnEventGridChanged(wxPropertyGridEvent& event);
    void OnEventGridExpand(wxPropertyGridEvent& event);
    void OnPropertyGridChanged(wxPropertyGridEvent& event);
    void OnPropertyGridChanging(wxPropertyGridEvent& event);
    void OnPropertyGridExpand(wxPropertyGridEvent& event);
    void OnPropertyGridItemSelected(wxPropertyGridEvent& event);
    void OnNodePropChange(CustomEvent& event);
    void OnReCreateGrid(wxCommandEvent& event);

private:
    std::map<wxPGProperty*, NodeProperty*> m_property_map;
    std::map<wxPGProperty*, NodeEvent*> m_event_map;

    Node* m_currentSel { nullptr };

    wxString m_selected_prop_name;
    wxString m_selected_event_name;
    wxString m_pageName;

    wxAuiNotebook* m_notebook_parent;

    wxPropertyGridManager* m_prop_grid;
    wxPropertyGridManager* m_event_grid;

    std::map<std::string, bool> m_expansion_map;

    // List of wxID_ strings
    wxArrayString m_astr_wx_ids;

    // Class decorations
    wxArrayString m_astr_wx_decorations;

    bool m_isPropChangeSuspended { false };

    // Set to true if a VerifyChangeFile() function already disaplayed a message to the user.
    bool m_failure_handled { false };

    bool m_locked { false };
};
