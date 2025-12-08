/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base widget generator class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements BaseGenerator, the interface class for all individual
// widget/sizer generators in wxUiEditor. Each wxWidgets component (wxButton, wxBoxSizer, etc.) has
// a derived class implementing virtual methods: ConstructionCode (object creation), SettingsCode
// (property configuration), AfterChildrenCode (post-child operations), and HeaderCode (C++ class
// member declarations). The class also handles XRC generation (GenXrcObject), mockup preview
// creation (CreateMockup, AfterCreation), include file management (GetIncludes, GetPythonImports),
// and property validation (VerifyProperty, AllowPropertyChange). Generators interact with Code
// class for language-specific output, query Node properties for configuration, and report
// version/language compatibility via GetRequiredVersion and isLanguageVersionSupported. Helper
// functions (DeclAddProp, DeclAddEvent) define property metadata for the UI designer, while
// specialized methods handle mockup updates and context menu customization.

#pragma once

#include <optional>
#include <set>
#include <string>

class Node;
class NodeEvent;
class NodeProperty;

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
    if (node->as_bool(name_prop))                      \
    {                                                  \
        item.append_child(name_child).text().set("1"); \
    }

#define ADD_ITEM_COMMENT(text) item.append_child(pugi::node_comment).set_value(text);

// This is the interface class that all generators derive from.
class BaseGenerator
{
public:
    enum : std::uint8_t
    {
        xrc_not_supported = 0,
        xrc_form_not_supported,
        xrc_sizer_item_created,
        xrc_updated
    };

    BaseGenerator() = default;
    virtual ~BaseGenerator() = default;

    BaseGenerator(const BaseGenerator&) = delete;
    BaseGenerator(BaseGenerator&&) = delete;
    auto operator=(const BaseGenerator&) -> BaseGenerator& = delete;
    auto operator=(BaseGenerator&&) -> BaseGenerator& = delete;

    auto getMockup() -> MockupParent*;

    // Generate the code used to construct the object using either C++, Python or Ruby
    virtual auto ConstructionCode(Code& /*unused*/) -> bool { return false; }

    // Generate code after the class has been constructed in the source file
    virtual auto AfterConstructionCode(Code& /*unused*/) -> bool { return false; }

    // Generate any settings the object needs using either C++, Python or Ruby
    virtual auto SettingsCode(Code& /*unused*/) -> bool { return false; }

    // Generate code after any children have been constructed
    //
    // Code will be written with indent::auto_keep_whitespace set
    virtual auto AfterChildrenCode(Code& /*unused*/) -> bool { return false; }

    // Called if code needs to be generated before the class definition is generated
    virtual auto PreClassHeaderCode(Code& /*unused*/) -> bool { return false; }

    // Generate code to add to a C++ header file after the class ctor, enums, constants, and
    // public members have been added. have been added.
    virtual auto HeaderCode(Code& /*unused*/) -> bool { return false; }

    // Called to add protected members to the class definition
    virtual void AddProtectedHdrMembers(std::set<std::string>& /*unused*/) {}

    // Called when generating a C++ header -- this should return the actual name of the class
    // or it's derived class name. I.e., PanelForm adds wxPanel.
    virtual auto BaseClassNameCode(Code& /*unused*/) -> bool { return false; }

    // Generate code to bind the event to a handler -- only override if you need to do
    // something special
    virtual void GenEvent(Code& code, NodeEvent* event, const std::string& class_name);

    // Add attributes to object, and all properties
    //
    // Return an xrc_ enum (e.g. xrc_sizer_item_created)
    virtual auto GenXrcObject(Node* /*unused*/, pugi::xml_node& /* object */,
                              size_t /* xrc_flags */) -> int
    {
        return xrc_not_supported;
    }

    // Called by Mockup after all children have been created
    virtual void AfterCreation(wxObject* /*wxobject*/, wxWindow* /*wxparent*/, Node* /* node */,
                               bool /* is_preview */)
    {
    }

    // Create an object to use in the Mockup panel (typically a sizer or widget).
    virtual auto CreateMockup(Node* /*node*/, wxObject* /*parent*/) -> wxObject* { return nullptr; }

    // Return the required wxXmlResourceHandler
    virtual void RequiredHandlers(Node* /*unused*/, std::set<std::string>& /* handlers */) {}

    // Return true if the Generic version of the control is being used.
    virtual auto IsGeneric(Node* /*unused*/) -> bool;

    // Return the lowest required version of wxUiEditor to support this generator. Override
    // this if the generator or any of it's non-default properties require a newer version of
    // wxUiEditor.
    virtual auto GetRequiredVersion(Node* /*node*/) -> int;

    // REVIEW: [Randalphwa - 11-22-2023] Currently, GetIncludes() is only called for C++. If
    // all the generators used the language to determine what to add, then it could be used for
    // imports for Python and requires for Ruby as well. I added it now thinking it might work
    // for Python custom controls, but all the generators need to be updated first.

    // Add any required include files to base source and/or header file.
    virtual auto GetIncludes(Node* /*unused*/, std::set<std::string>& /* set_src */,
                             std::set<std::string>& /* set_hdr */, GenLang /* language */) -> bool
    {
        return false;
    };

    // Add any required Python libraries that need to be imported
    virtual auto GetPythonImports(Node* /*unused*/, std::set<std::string>& /* set_imports */)
        -> bool;

    // Add any required libraries or symbols that need to be imported
    virtual auto GetImports(Node* /*unused*/, std::set<std::string>& /* set_imports */,
                            GenLang /* language */) -> bool
    {
        return false;
    }

    // Return false if the entire Mockup contents should be recreated due to the property change
    virtual auto OnPropertyChange(wxObject* /*unused*/, Node* /*unused*/, NodeProperty* /*unused*/)
        -> bool
    {
        return false;
    }

    // Called while processing an wxEVT_PG_CHANGING event.
    virtual auto AllowPropertyChange(wxPropertyGridEvent*, NodeProperty*, Node*) -> bool;

    // Called while processing an wxEVT_PG_CHANGING event.
    virtual auto AllowIdPropertyChange(wxPropertyGridEvent*, NodeProperty*, Node*) -> bool;

    // Bind wxEVT_LEFT_DOWN to this so that clicking on the widget will select it in the navigation
    // panel
    void OnLeftClick(wxMouseEvent& event);

    // Get the Help menu item text
    virtual auto GetHelpText(Node*) -> tt_string;

    // Get the HTML filename to browse to. Caller needs to supply the prefix.
    virtual auto GetHelpURL(Node*) -> tt_string;

    // Get the wxPython Help menu item text
    virtual auto GetPythonHelpText(Node*) -> tt_string;

    // Get the wxPython Help menu item text
    virtual auto GetRubyHelpText(Node*) -> tt_string;

    // Get the HTML filename to browse to. E.g., "wx.Button.html"
    virtual auto GetPythonURL(Node* node) -> tt_string;

    // Get the HTML filename to browse to. E.g., "wx.Button.html"
    virtual auto GetRubyURL(Node* node) -> tt_string;

    // Change the enable/disable states in the Property Grid Panel based on the current
    // property.
    virtual void ChangeEnableState(wxPropertyGridManager*, NodeProperty*);

    // Call this to retrieve hint text for the property
    virtual auto GetHint(NodeProperty*) -> std::optional<tt_string>;

    // Called by MainFrame when the user modifies a property. Return false to let MainFrame
    // call PushUndoAction() to push a single prop change to the undo stack. Return true if
    // the generator handles pushing to the undo stack.
    virtual auto ModifyProperty(NodeProperty* /* prop */, tt_string_view /* value */) -> bool
    {
        return false;
    }

    // Call this to use different help text then get_PropDeclaration()->getDescription()
    virtual auto GetPropertyDescription(NodeProperty* /*unused*/) -> std::optional<tt_string>
    {
        return {};
    }

    // Call this to convert wxWidgets constants to friendly names, and to fix conflicting bit
    // flags. Returns true if a change was made. Note that the change is *not* pushed to the
    // undo stack.
    virtual auto VerifyProperty(NodeProperty*) -> bool;

    // If true, a control can be move left or right or into a new sizer. Override and return
    // false if the control can't do this (such as a book page).
    virtual auto CanChangeParent(Node* node) -> bool;

    // Return true to automatically add submenu command to add child sizers.
    //
    // You will need to Bind to any commands you add.
    virtual auto PopupMenuAddCommands(NavPopupMenu* /*unused*/, Node* /*unused*/) -> bool
    {
        return true;
    }

    // Call this to retrieve any warning text when generating code for the specific language.
    virtual auto GetWarning(Node* /*unused*/, GenLang /* language */) -> std::optional<tt_string>
    {
        return {};
    }

    // result.first == false indicates that the generator cannot construct the object using
    // the current language and version. result.second contains the error message.
    virtual auto isLanguageVersionSupported(GenLang /* language */) -> std::pair<bool, tt_string>
    {
        return { true, {} };
    }

    // result.has_value() == true indicates that the property is not supported using the
    // current language and version. Use result.value() to get the warning message. This will
    // be called *after* the property is set, so the generator can query the property's
    // current value.
    virtual auto isLanguagePropSupported(Node* /*unused*/, GenLang, GenEnum::PropName)
        -> std::optional<tt_string>;

    // Allows a generator, such as the MDI forms to specify additional member variables that should
    // be collected and added to the class declaration. These will either be protected: or private:
    // depending on the private_members property of the form.
    virtual auto CollectMemberVariables(Node* /* node unused */,
                                        std::set<std::string>& /* code_lines unused */) -> void
    {
        // Default does nothing
    }
};

auto DeclAddProp(NodeDeclaration* declaration, PropName prop_name, PropType type,
                 std::string_view help = {}, std::string_view def_value = {}) -> PropDeclaration*;
void DeclAddOption(PropDeclaration* prop_info, std::string_view name, std::string_view help = {});

// This will add prop_var_name, prop_var_comment and prop_class_access
void DeclAddVarNameProps(NodeDeclaration* declaration, std::string_view def_value);

void DeclAddEvent(NodeDeclaration* declaration, const std::string& evt_name,
                  std::string_view event_class, std::string_view help);
