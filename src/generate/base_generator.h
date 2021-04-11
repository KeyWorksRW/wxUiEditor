/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base widget generator class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <optional>
#include <set>
#include <string>

#include "node_classes.h"  // Forward defintions of Node classes

class wxObject;
class MockupParent;
class wxWindow;
class wxMouseEvent;

namespace pugi
{
    class xml_node;
}

using OptionalIncludes = std::optional<std::vector<std::string>>;

class BaseGenerator
{
public:
    BaseGenerator() {}
    virtual ~BaseGenerator() {}

    MockupParent* GetMockup();

    virtual wxObject* Create(Node* /*node*/, wxObject* /*parent*/) { return nullptr; }

    // Called after all children have been created
    virtual void AfterCreation(wxObject* /*wxobject*/, wxWindow* /*wxparent*/) {}

    // Returns true if the Mockup window needs to be refreshed
    virtual bool OnSelected(Node* /*node*/) { return false; }

    virtual std::optional<ttlib::cstr> GenConstruction(Node*) { return {}; }
    virtual std::optional<ttlib::cstr> GenCode(const std::string& /*command*/, Node*) { return {}; }
    virtual std::optional<ttlib::cstr> GenEvents(NodeEvent*, const std::string&) { return {}; }
    virtual std::optional<ttlib::cstr> GenSettings(Node*, size_t&) { return {}; }

    virtual bool GetIncludes(Node*, std::set<std::string>&, std::set<std::string>&) { return false; };
    virtual OptionalIncludes GetEventIncludes(Node*) { return {}; }

    // Return true if the widget was changed which will resize and repaint the Mockup window
    virtual bool OnPropertyChange(wxObject*, Node*, NodeProperty*) { return false; }

    virtual NodeSharedPtr CreateSmithNode(pugi::xml_node&, Node*) { return NodeSharedPtr(); };
    virtual NodeSharedPtr CreateXrcNode(pugi::xml_node&, Node*) { return NodeSharedPtr(); };

    // Bind wxEVT_LEFT_DOWN to this so that clicking on the widget will select it in the navigation panel
    void OnLeftClick(wxMouseEvent& event);
};
