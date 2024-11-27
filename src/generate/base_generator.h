/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base widget generator class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
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
    if (node->hasValue(name_prop))                                            \
    {                                                                         \
        item.append_child(name_child).text().set(node->as_string(name_prop)); \
    }

#define ADD_ITEM_BOOL(name_prop, name_child)           \
    if (node->as_bool(name_prop))                      \
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

    MockupParent* getMockup();

    // Generate the code used to construct the object using either C++, Python or Ruby
    virtual bool ConstructionCode(Code&) { return false; }

    // Generate code after the class has been constructed in the source file
    virtual bool AfterConstructionCode(Code&) { return false; }

    // Generate any settings the object needs using either C++, Python or Ruby
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
    virtual void GenEvent(Code& code, NodeEvent* event, const std::string& class_name);

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

    // REVIEW: [Randalphwa - 11-22-2023] Currently, GetIncludes() is only called for C++. If
    // all the generators used the language to determine what to add, then it could be used for
    // imports for Python and requires for Ruby as well. I added it now thinking it might work
    // for Python custom controls, but all the generators need to be updated first.

    // Add any required include files to base source and/or header file.
    virtual bool GetIncludes(Node*, std::set<std::string>& /* set_src */, std::set<std::string>& /* set_hdr */,
                             GenLang /* language */)
    {
        return false;
    };

    // Add any required Python libraries that need to be imported
    virtual bool GetPythonImports(Node*, std::set<std::string>& /* set_imports */);

    // Add any required libraries or symbols that need to be imported
    virtual bool GetImports(Node*, std::set<std::string>& /* set_imports */, GenLang /* language */) { return false; }

    // Return false if the entire Mockup contents should be recreated due to the property change
    virtual bool OnPropertyChange(wxObject*, Node*, NodeProperty*) { return false; }

    // Called while processing an wxEVT_PG_CHANGING event.
    virtual bool AllowPropertyChange(wxPropertyGridEvent*, NodeProperty*, Node*);

    // Called while processing an wxEVT_PG_CHANGING event.
    virtual bool AllowIdPropertyChange(wxPropertyGridEvent*, NodeProperty*, Node*);

    // Bind wxEVT_LEFT_DOWN to this so that clicking on the widget will select it in the navigation panel
    void OnLeftClick(wxMouseEvent& event);

    // Get the Help menu item text
    virtual tt_string GetHelpText(Node*);

    // Get the HTML filename to browse to. Caller needs to supply the prefix.
    virtual tt_string GetHelpURL(Node*);

    // Get the wxPython Help menu item text
    virtual tt_string GetPythonHelpText(Node*);

    // Get the wxPython Help menu item text
    virtual tt_string GetRubyHelpText(Node*);

    // Get the HTML filename to browse to. E.g., "wx.Button.html"
    virtual tt_string GetPythonURL(Node* node);

    // Get the HTML filename to browse to. E.g., "wx.Button.html"
    virtual tt_string GetRubyURL(Node* node);

    // Change the enable/disable states in the Property Grid Panel based on the current
    // property.
    virtual void ChangeEnableState(wxPropertyGridManager*, NodeProperty*);

    // Call this to retrieve hint text for the property
    virtual std::optional<tt_string> GetHint(NodeProperty*);

    // Called by MainFrame when the user modifies a property. Return false to let MainFrame
    // call PushUndoAction() to push a single prop change to the undo stack. Return true if
    // the generator handles pushing to the undo stack.
    virtual bool modifyProperty(NodeProperty* /* prop */, tt_string_view /* value */) { return false; }

    // Call this to use different help text then getPropDeclaration()->getDescription()
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
    virtual bool PopupMenuAddCommands(NavPopupMenu*, Node*) { return true; }

    // Call this to retrieve any warning text when generating code for the specific language.
    virtual std::optional<tt_string> GetWarning(Node*, GenLang /* language */) { return {}; }

    // result.first == false indicates that the generator cannot construct the object using
    // the current language and version. result.second contains the error message.
    virtual std::pair<bool, tt_string> isLanguageVersionSupported(GenLang /* language */) { return { true, {} }; }

    // result.has_value() == true indicates that the property is not supported using the
    // current language and version. Use result.value() to get the warning message. This will
    // be called *after* the property is set, so the generator can query the property's
    // current value.
    virtual std::optional<tt_string> isLanguagePropSupported(Node*, GenLang, GenEnum::PropName);
};

PropDeclaration* DeclAddProp(NodeDeclaration* declaration, PropName prop_name, PropType type, std::string_view help = {},
                             std::string_view def_value = {});
void DeclAddOption(PropDeclaration* prop_info, std::string_view name, std::string_view help = {});

// This will add prop_var_name, prop_var_comment and prop_class_access
void DeclAddVarNameProps(NodeDeclaration* declaration, std::string_view def_value);

void DeclAddEvent(NodeDeclaration* declaration, const std::string& evt_name, std::string_view event_class,
                  std::string_view help);
