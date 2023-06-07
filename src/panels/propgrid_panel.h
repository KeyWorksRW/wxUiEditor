/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid class for node properties and events
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <set>

#include <wx/aui/auibook.h>       // wxaui: wx advanced user interface - notebook
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

    bool IsEventPageShowing();

    void RestoreDescBoxHeight();
    void SaveDescBoxHeight();

    // Prevents creation of node's properties until UnLock() is called.
    void Lock() { m_locked = true; }

    // Allows creation of node properties
    void UnLock() { m_locked = false; }

    // Creates properties for currently selected node.
    void Create();

protected:
    wxString GetPropHelp(NodeProperty* prop);
    wxString GetCategoryDisplayName(const wxString& original);

    // Called to determine if a property should be displayed or not
    bool IsPropAllowed(Node* node, NodeProperty* prop);

    // Called after a property has been changed. Used to display info-bar notification if
    // needed.
    void OnPostPropChange(CustomEvent& event);

    void CreatePropCategory(tt_string_view name, Node* node, NodeDeclaration* obj_info, PropNameSet& prop_set);
    void CreateEventCategory(tt_string_view name, Node* node, NodeDeclaration* obj_info, EventSet& event_set);
    void CreateLayoutCategory(Node* node);

    void AddEvents(tt_string_view name, Node* node, NodeCategory& category, EventSet& event_set);
    void AddProperties(tt_string_view name, Node* node, NodeCategory& category, PropNameSet& prop_set,
                       bool is_child_cat = false);

    void ReplaceDerivedName(const wxString& formName, NodeProperty* propType);
    void ReplaceBaseFile(const wxString& formName, NodeProperty* propType);
    void ReplaceDerivedFile(const wxString& formName, NodeProperty* propType);

    wxPGProperty* CreatePGProperty(NodeProperty* prop);

    // Called after a property has been modified, this checks to see if various property
    // items need to be enabled or disabled based on the current value of the changed
    // property.
    void ChangeEnableState(NodeProperty* changed_prop);

    void ReselectItem();

    void ModifyProperty(NodeProperty* prop, const wxString& str);
    void modifyProperty(NodeProperty* prop, tt_string_view str);

    void ModifyBitlistProperty(NodeProperty* node_prop, wxPGProperty* grid_prop);
    void ModifyBoolProperty(NodeProperty* node_prop, wxPGProperty* grid_prop);
    void ModifyEmbeddedProperty(NodeProperty* node_prop, wxPGProperty* grid_prop);
    void ModifyFileProperty(NodeProperty* node_prop, wxPGProperty* grid_prop);
    void ModifyOptionsProperty(NodeProperty* node_prop, wxPGProperty* grid_prop);

    int GetBitlistValue(const wxString& strVal, wxPGChoices& bit_flags);

    // Event handlers

    void OnEventGridChanged(wxPropertyGridEvent& event);
    void OnEventGridExpand(wxPropertyGridEvent& event);
    void OnPropertyGridChanged(wxPropertyGridEvent& event);
    void OnPropertyGridChanging(wxPropertyGridEvent& event);
    void OnPropertyGridExpand(wxPropertyGridEvent& event);
    void OnPropertyGridItemSelected(wxPropertyGridEvent& event);
    void OnReCreateGrid(wxCommandEvent& event);
    void OnAuiNotebookPageChanged(wxAuiNotebookEvent& event);

public:
    void OnNodePropChange(CustomEvent& event);

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

    bool m_locked { false };
};
