/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base widget generator class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <optional>
#include <set>
#include <string>

#include "node_classes.h"  // Forward defintions of Node classes

class BaseCodeGenerator;
class MockupParent;
class NavPopupMenu;
class WriteCode;
class wxMouseEvent;
class wxObject;
class wxPropertyGridEvent;
class wxPropertyGridManager;
class wxWindow;

class Code;

namespace pugi
{
    class xml_node;
}

using OptionalIncludes = std::optional<std::vector<std::string>>;

#define ADD_ITEM_PROP(name_prop, name_child)                                  \
    if (node->HasValue(name_prop))                                            \
    {                                                                         \
        item.append_child(name_child).text().set(node->as_string(name_prop)); \
    }

#define ADD_ITEM_BOOL(name_prop, name_child)           \
    if (node->prop_as_bool(name_prop))                 \
    {                                                  \
        item.append_child(name_child).text().set("1"); \
    }

#define ADD_ITEM_COMMENT(text) item.append_child(pugi::node_comment).set_value(text);

// This is the interface class that all generators derive from.
class BaseGenerator
{
public:
    enum
    {
        xrc_not_supported = 0,
        xrc_form_not_supported,
        xrc_sizer_item_created,
        xrc_updated
    };

    BaseGenerator() {}
    virtual ~BaseGenerator() {}

    MockupParent* GetMockup();

    // Generate the code used to construct the object using either C++ or Python
    virtual bool ConstructionCode(Code&) { return false; }

    // Generate code after the class has been constructed in the source file
    virtual bool AfterConstructionCode(Code&) { return false; }

    // Generate any settings the object needs using either C++ or Python
    virtual bool SettingsCode(Code&) { return false; }

    // Generate code after any children have been constructed
    //
    // Code will be written with indent::auto_keep_whitespace set
    virtual bool AfterChildrenCode(Code&) { return false; }

    // Called if code needs to be generated before the class definition is generated
    virtual bool PreClassHeaderCode(Code&) { return false; }

    // Generate code to add to a C++ header file -- this is normally the class header
    // definition
    virtual bool HeaderCode(Code&) { return false; }

    // Called to add protected members to the class definition
    virtual void AddProtectedHdrMembers(std::set<std::string>&) {}

    // Called when generating a C++ header -- this should return the actual name of the class
    // or it's derived class name. I.e., PanelForm adds wxPanel.
    virtual bool BaseClassNameCode(Code&) { return false; }

    // Generate code to bind the event to a handler -- only override if you need to do
    // something special
    virtual void GenEvent(Code&, NodeEvent*, const std::string&);

    // Add attributes to object, and all properties
    //
    // Return an xrc_ enum (e.g. xrc_sizer_item_created)
    virtual int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) { return xrc_not_supported; }

    // Called by Mockup after all children have been created
    virtual void AfterCreation(wxObject* /*wxobject*/, wxWindow* /*wxparent*/, Node* /* node */, bool /* is_preview */) {}

    // Create an object to use in the Mockup panel (typically a sizer or widget).
    virtual wxObject* CreateMockup(Node* /*node*/, wxObject* /*parent*/) { return nullptr; }

    // Return the required wxXmlResourceHandler
    virtual void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) {}

    // Return true if the Generic version of the control is being used.
    virtual bool IsGeneric(Node*) { return false; }

    // Return the lowest required version of wxUiEditor to support this generator. Override
    // this if the generator or any of it's non-default properties require a newer version of
    // wxUiEditor.
    virtual int GetRequiredVersion(Node* /*node*/);

    // Add any required include files to base source and/or header file
    virtual bool GetIncludes(Node*, std::set<std::string>& /* set_src */, std::set<std::string>& /* set_hdr */)
    {
        return false;
    };

    // Add any required Python libraries that need to be imported
    virtual bool GetPythonImports(Node*, std::set<std::string>& /* set_imports */);

    // Return false if the entire Mockup contents should be recreated due to the property change
    virtual bool OnPropertyChange(wxObject*, Node*, NodeProperty*) { return false; }

    // Called while processing an wxEVT_PG_CHANGING event.
    virtual bool AllowPropertyChange(wxPropertyGridEvent*, NodeProperty*, Node*);

    // Bind wxEVT_LEFT_DOWN to this so that clicking on the widget will select it in the navigation panel
    void OnLeftClick(wxMouseEvent& event);

    // Get the Help menu item text
    virtual tt_string GetHelpText(Node*);

    // Get the HTML filename to browse to. Caller needs to supply the prefix.
    virtual tt_string GetHelpURL(Node*);

    // Get the wxPython Help menu item text
    virtual tt_string GetPythonHelpText(Node*);

    // Get the HTML filename to browse to. E.g., "wx.Button.html"
    virtual tt_string GetPythonURL(Node* node) { return GetPythonHelpText(node) + ".html"; }

    // Change the enable/disable states in the Property Grid Panel based on the current
    // property.
    virtual void ChangeEnableState(wxPropertyGridManager*, NodeProperty*);

    // Call this to retrieve hint text for the property
    virtual std::optional<tt_string> GetHint(NodeProperty*);

    // Called by MainFrame when the user modifies a property. Return false to let MainFrame
    // call PushUndoAction() to push a single prop change to the undo stack. Return true if
    // the generator handles pushing to the undo stack.
    virtual bool ModifyProperty(NodeProperty* /* prop */, tt_string_view /* value */) { return false; }

    // Call this to use different help text then GetPropDeclaration()->GetDescription()
    virtual std::optional<tt_string> GetPropertyDescription(NodeProperty*) { return {}; }

    // Call this to convert wxWidgets constants to friendly names, and to fix conflicting bit
    // flags. Returns true if a change was made. Note that the change is *not* pushed to the
    // undo stack.
    virtual bool VerifyProperty(NodeProperty*);

    // If true, a control can be move left or right or into a new sizer. Override and return
    // false if the control can't do this (such as a book page).
    virtual bool CanChangeParent(Node* node);

    // Return true to automatically add submenu command to add child sizers.
    //
    // You will need to Bind to any commands you add.
    virtual bool PopupMenuAddCommands(NavPopupMenu*, Node*) { return false; }
};
