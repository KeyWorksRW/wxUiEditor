/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid class for node properties and events
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include <wx/propgrid/manager.h>  // wxPropertyGridManager

#include "../nodes/node_classes.h"  // Forward defintions of Node classes

using EventMap = std::map<std::string, NodeEvent*>;
using PropertyMap = std::map<std::string, NodeProperty*>;

class CustomEvent;
class MainFrame;
class wxAuiNotebook;

class PropGridPanel : public wxPanel
{
public:
    PropGridPanel(wxWindow* parent, MainFrame* frame);

    void RestoreDescBoxHeight();
    void SaveDescBoxHeight();

protected:
    wxString GetCategoryDisplayName(const wxString& original);

    // Called to determine if a property should be displayed or not
    bool IsPropAllowed(Node* node, NodeProperty* prop);

    void CreatePropCategory(const ttlib::cstr& name, Node* node, NodeDeclaration* obj_info, PropertyMap& itemMap);
    void CreateEventCategory(const ttlib::cstr& name, Node* node, NodeDeclaration* obj_info, EventMap& itemMap);
    void CreateLayoutCategory(Node* node, PropertyMap& itemMap);

    void AddEvents(const ttlib::cstr& name, Node* node, NodeCategory& category, EventMap& map);
    void AddProperties(const ttlib::cstr& name, Node* node, NodeCategory& category, PropertyMap& map);

    void ReplaceDrvName(const wxString& formName, NodeProperty* propType);
    void ReplaceBaseFile(const wxString& formName, NodeProperty* propType);
    void ReplaceDrvFile(const wxString& formName, NodeProperty* propType);

    wxPGProperty* GetProperty(NodeProperty* prop);

    void Create();

    void ReselectItem();

    void ModifyProperty(NodeProperty* prop, const wxString& str);
    void modifyProperty(NodeProperty* prop, ttlib::cview str);

    int GetBitlistValue(const wxString& strVal, wxPGChoices& bit_flags);

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

    bool m_isPropChangeSuspended { false };
};
