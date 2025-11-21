/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid class for node properties and events
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: PropGridPanel is the central property editing panel in wxUiEditor's main window.
// It manages two wxPropertyGridManager instances: one for node properties and one for event
// handlers. The class dynamically creates property editors based on Node and NodeProperty
// definitions from the node system, organizing them into categories by functionality. It handles
// property validation, change notifications via CustomEvent (EVT_NodePropChange), and maintains
// property state including expansion states and enabled/disabled conditions. The implementation is
// split across multiple files: propgrid_panel.cpp (construction/creation), propgrid_events.cpp
// (event handlers), propgrid_modify.cpp (property modification), and propgrid_create.cpp (property
// creation). Custom property editors in src/customprops/ are instantiated by CreatePGProperty() for
// specialized editing of fonts, images, code, colors, and other complex property types.

#pragma once

#include <map>
#include <set>

#include <wx/aui/auibook.h>       // wxaui: wx advanced user interface - notebook
#include <wx/propgrid/manager.h>  // wxPropertyGridManager

#include "gen_enums.h"  // Enumerations for generators

using namespace GenEnum;

using PropNameSet = std::set<PropName>;
using EventSet = std::set<std::string>;

class CustomEvent;
class MainFrame;
class Node;
class NodeEvent;
class NodeProperty;
class wxAuiNotebook;

class PropGridPanel : public wxPanel
{
public:
    PropGridPanel(wxWindow* parent, MainFrame* frame);

    auto IsEventPageShowing() -> bool;

    void RestoreDescBoxHeight();
    void SaveDescBoxHeight();

    // Prevents creation of node's properties until UnLock() is called.
    void Lock() { m_locked = true; }

    // Allows creation of node properties
    void UnLock() { m_locked = false; }

    // Creates properties for currently selected node.
    void Create();

    void OnNodePropChange(CustomEvent& event);

protected:
    auto GetPropHelp(NodeProperty* prop) const -> tt_string;
    auto GetCategoryDisplayName(const wxString& original) -> wxString;

    // Called to determine if a property should be displayed or not
    auto IsPropAllowed(Node* node, NodeProperty* prop) -> bool;

    // Called after a property has been changed. Used to display info-bar notification if
    // needed.
    void OnPostPropChange(CustomEvent& event);

    void CreatePropCategory(tt_string_view name, Node* node, NodeDeclaration* obj_info,
                            PropNameSet& prop_set);
    void CreateEventCategory(tt_string_view name, Node* node, NodeDeclaration* obj_info,
                             EventSet& event_set);
    void CreateLayoutCategory(Node* node);

    void AddEvents(tt_string_view name, Node* node, NodeCategory& category, EventSet& event_set);
    void AddProperties(tt_string_view name, Node* node, NodeCategory& category,
                       PropNameSet& prop_set, bool is_child_cat = false);

    void ReplaceDerivedName(const tt_string& formName, NodeProperty* propType);
    void ReplaceDerivedFile(const tt_string& formName, NodeProperty* propType);

    // Called when the class_name for a form changes. If the preferred code language output
    // file is empty, then create a suggested file name based on the class name.
    void CheckOutputFile(const tt_string& formName, Node* node);

    auto CreatePGProperty(NodeProperty* prop) -> wxPGProperty*;

    // Called after a property has been modified, this checks to see if various property
    // items need to be enabled or disabled based on the current value of the changed
    // property.
    void ChangeEnableState(NodeProperty* changed_prop);

    void ReselectItem();

    // void ModifyProperty(NodeProperty* prop, tt_string_view str);
    void ModifyProperty(NodeProperty* prop, const wxString& str);

    void ModifyBitlistProperty(NodeProperty* node_prop, wxPGProperty* grid_prop);
    void ModifyBoolProperty(NodeProperty* node_prop, wxPGProperty* grid_prop);
    void ModifyEmbeddedProperty(NodeProperty* node_prop, wxPGProperty* grid_prop);
    void ModifyFileProperty(NodeProperty* node_prop, wxPGProperty* grid_prop);
    void ModifyOptionsProperty(NodeProperty* node_prop, wxPGProperty* grid_prop);

    static auto GetBitlistValue(const wxString& strVal, wxPGChoices& bit_flags) -> int;

    void AllowIdChange(wxPropertyGridEvent& event, NodeProperty* prop, Node* node);

    // Directory and file path validation and modification
    void AllowDirectoryChange(wxPropertyGridEvent& event, NodeProperty* prop, Node* node);
    void AllowFileChange(wxPropertyGridEvent& event, NodeProperty* prop, Node* node);
    void OnPathChanged(wxPropertyGridEvent& event, NodeProperty* prop, Node* node);
    void ChangeDerivedDirectory(tt_string& path);
    void ChangeBaseDirectory(tt_string& path);

    // Event handlers

    void OnEventGridChanged(wxPropertyGridEvent& event);
    void OnEventGridExpand(wxPropertyGridEvent& event);
    void OnPropertyGridChanged(wxPropertyGridEvent& event);
    void OnPropertyGridChanging(wxPropertyGridEvent& event);
    void OnPropertyGridExpand(wxPropertyGridEvent& event);
    void OnPropertyGridItemSelected(wxPropertyGridEvent& event);
    void OnReCreateGrid(wxCommandEvent& event);
    void OnAuiNotebookPageChanged(wxAuiNotebookEvent& event);

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

    GenLang m_preferred_lang { GEN_LANG_CPLUSPLUS };

    bool m_isPropChangeSuspended { false };

    bool m_locked { false };
};
