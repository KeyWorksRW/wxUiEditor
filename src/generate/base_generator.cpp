/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base widget generator class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/event.h>              // Event classes
#include <wx/propgrid/manager.h>   // wxPropertyGridManager
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include <frozen/set.h>

#include "base_generator.h"

#include "code.h"             // Code -- Helper class for generating code
#include "gen_common.h"       // Common component functions
#include "lambdas.h"          // Functions for formatting and storage of lamda events
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "node_decl.h"        // NodeDeclaration class
#include "node_prop.h"        // NodeProperty -- NodeProperty class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties

#include "../mockup/mockup_parent.h"  // Top-level MockUp Parent window

MockupParent* BaseGenerator::getMockup()
{
    return wxGetFrame().getMockup();
}

void BaseGenerator::OnLeftClick(wxMouseEvent& event)
{
    auto wxobject = event.GetEventObject();
    auto node = wxGetFrame().getMockup()->getNode(wxobject);

    if (wxGetFrame().getSelectedNode() != node)
    {
        wxGetFrame().getMockup()->SelectNode(wxobject);
    }
    event.Skip();
}

bool BaseGenerator::AllowIdPropertyChange(wxPropertyGridEvent* event, NodeProperty* /* prop */, Node* node)
{
    tt_string newValue = event->GetPropertyValue().GetString().utf8_string();
    if (newValue.empty())
        return true;

    auto form = node->getForm();
    if (node->isGen(gen_wxMenuItem))
    {
        form = node->getParent();
        while (form && !form->isGen(gen_wxMenuBar) && !form->isGen(gen_MenuBar))
        {
            form = form->getParent();
        }

        // This shouldn't happen, but return true just in case
        if (!form)
        {
            return true;
        }
    }
    else if (node->isGen(gen_auitool))
    {
        form = node->getParent();
        while (form && !form->isGen(gen_AuiToolBar) && !form->isGen(gen_wxAuiToolBar))
        {
            form = form->getParent();
        }

        // This shouldn't happen, but return true just in case
        if (!form)
        {
            return true;
        }
    }
    else if (node->isGen(gen_tool) || node->isGen(gen_tool_dropdown))
    {
        form = node->getParent();
        while (form && !form->isGen(gen_ToolBar) && !form->isGen(gen_wxToolBar))
        {
            form = form->getParent();
        }

        // This shouldn't happen, but return true just in case
        if (!form)
        {
            return true;
        }
    }
    else if (node->isGen(gen_ribbonTool) || node->isGen(gen_ribbonButton) || node->isGen(gen_ribbonGalleryItem))
    {
        form = node->getParent();
        while (form && !form->isGen(gen_RibbonBar) && !form->isGen(gen_wxRibbonBar))
        {
            form = form->getParent();
        }

        // This shouldn't happen, but return true just in case
        if (!form)
        {
            return true;
        }
    }

    std::set<tt_string> ids;

    auto rlambda = [&](Node* child, auto&& rlambda) -> void
    {
        if (child != node && child->hasValue(prop_id) && !child->as_string(prop_id).is_sameprefix("wx"))
        {
            ids.emplace(child->getPropId());
        }

        for (const auto& iter: child->getChildNodePtrs())
        {
            if (iter->hasValue(prop_id) && !iter->as_string(prop_id).is_sameprefix("wx"))
            {
                ids.emplace(iter->getPropId());
            }
            rlambda(iter.get(), rlambda);
        }
    };

    rlambda(form, rlambda);

    // Same as NodeProperty::getPropId() -- strip off any assginment
    tt_string new_id;
    if (auto pos = newValue.find('='); pos != tt::npos)
    {
        while (pos > 0 && tt::is_whitespace(newValue[pos - 1]))
        {
            --pos;
        }
        new_id = newValue.substr(0, pos);
    }
    else
    {
        new_id = newValue;
    }

    if (ids.contains(new_id))
    {
        event->SetValidationFailureMessage(
            "You have already used this ID for another control. Please choose a different ID.");
        event->Veto();
        return false;
    }

    return true;
}

bool BaseGenerator::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop, Node* node)
{
    if (prop->isProp(prop_id))
    {
        return AllowIdPropertyChange(event, prop, node);
    }

    if (prop->isProp(prop_alignment))
    {
        auto property = wxStaticCast(event->GetProperty(), wxFlagsProperty);
        auto variant = event->GetPropertyValue();
        tt_string newValue = property->ValueToString(variant).utf8_string();
        if (newValue.empty())
            return true;

        auto parent = node->getParent();
        if (newValue == "wxALIGN_TOP" || newValue == "wxALIGN_BOTTOM" || newValue == "wxALIGN_CENTER_VERTICAL")
        {
            if (parent && parent->isSizer() && parent->as_string(prop_orientation).contains("wxVERTICAL"))
            {
                event->SetValidationFailureMessage(
                    "You can't set vertical alignment when the parent sizer is oriented vertically.");
                event->Veto();
                return false;
            }
            else if (node->as_string(prop_flags).contains("wxEXPAND"))
            {
                event->SetValidationFailureMessage("You can't set vertical alignment if the wxEXPAND flag is set.");
                event->Veto();
                return false;
            }
        }
        else if (newValue == "wxALIGN_LEFT" || newValue == "wxALIGN_RIGHT" || newValue == "wxALIGN_CENTER_HORIZONTAL")
        {
            if (parent && parent->isSizer() && parent->as_string(prop_orientation).contains("wxHORIZONTAL"))
            {
                event->SetValidationFailureMessage(
                    "You can't set horizontal alignment when the parent sizer is oriented horizontally.");
                event->Veto();
                return false;
            }
            else if (node->as_string(prop_flags).contains("wxEXPAND"))
            {
                event->SetValidationFailureMessage("You can't set horizontal alignment if the wxEXPAND flag is set.");
                event->Veto();
                return false;
            }
        }
    }
    else if (prop->isProp(prop_flags))
    {
        auto property = wxStaticCast(event->GetProperty(), wxFlagsProperty);
        auto variant = event->GetPropertyValue();
        tt_string newValue = property->ValueToString(variant).utf8_string();
        if (newValue.empty())
            return true;

        // Remove the original flags so that all we are checking is the changed flag.
        if (node->hasValue(prop_flags))
        {
            auto original = node->as_string(prop_flags);
            original.Replace("|", ", ");
            newValue.Replace(original, "");
        }

        // The newValue may have a flag removed, so this might not be the flag that got unchecked.
        if (newValue.contains("wxEXPAND"))
        {
            if (node->hasValue(prop_alignment))
            {
                auto& alignment = node->as_string(prop_alignment);
                if (alignment.contains("wxALIGN_LEFT") || alignment.contains("wxALIGN_RIGHT") ||
                    alignment.contains("wxALIGN_CENTER_HORIZONTAL") || alignment.contains("wxALIGN_TOP") ||
                    alignment.contains("wxALIGN_BOTTOM") || alignment.contains("wxALIGN_CENTER_VERTICAL"))
                {
                    event->SetValidationFailureMessage(
                        "You can't set the wxEXPAND flag if you have either horizontal or vertical alignment set.");
                    event->Veto();
                    return false;
                }
            }
        }
    }
    else if (prop->isProp(prop_var_name) || prop->isProp(prop_validator_variable) || prop->isProp(prop_checkbox_var_name) ||
             prop->isProp(prop_radiobtn_var_name))
    {
        auto property = wxStaticCast(event->GetProperty(), wxStringProperty);
        auto variant = event->GetPropertyValue();
        tt_string newValue = property->ValueToString(variant).utf8_string();
        if (newValue.empty())
            return true;

        if (!isValidVarName(newValue, Project.getCodePreference()))
        {
            event->SetValidationFailureMessage("The name you have specified is not a valid variable name.");
            event->Veto();
            return false;
        }
        auto unique_name = node->getUniqueName(newValue);

        // The above call to getUniqueName() won't check the current node so if the name is
        // unique, we still need to check within the same node
        bool is_duplicate = !newValue.is_sameas(unique_name);
        if (!is_duplicate)
        {
            if (prop->isProp(prop_var_name))
            {
                if (node->hasValue(prop_validator_variable) && newValue.is_sameas(node->as_string(prop_validator_variable)))
                    is_duplicate = true;
                else if (node->hasValue(prop_checkbox_var_name) &&
                         newValue.is_sameas(node->as_string(prop_checkbox_var_name)))
                    is_duplicate = true;
                else if (node->hasValue(prop_radiobtn_var_name) &&
                         newValue.is_sameas(node->as_string(prop_radiobtn_var_name)))
                    is_duplicate = true;
            }
            else if (prop->isProp(prop_validator_variable))
            {
                if (node->hasValue(prop_var_name) && newValue.is_sameas(node->as_string(prop_var_name)))
                    is_duplicate = true;
                else if (node->hasValue(prop_checkbox_var_name) &&
                         newValue.is_sameas(node->as_string(prop_checkbox_var_name)))
                    is_duplicate = true;
                else if (node->hasValue(prop_radiobtn_var_name) &&
                         newValue.is_sameas(node->as_string(prop_radiobtn_var_name)))
                    is_duplicate = true;
            }
            else if (prop->isProp(prop_checkbox_var_name))
            {
                if (node->hasValue(prop_var_name) && newValue.is_sameas(node->as_string(prop_var_name)))
                    is_duplicate = true;
                else if (node->hasValue(prop_validator_variable) &&
                         newValue.is_sameas(node->as_string(prop_validator_variable)))
                    is_duplicate = true;
                else if (node->hasValue(prop_radiobtn_var_name) &&
                         newValue.is_sameas(node->as_string(prop_radiobtn_var_name)))
                    is_duplicate = true;
            }
            else if (prop->isProp(prop_radiobtn_var_name))
            {
                if (node->hasValue(prop_var_name) && newValue.is_sameas(node->as_string(prop_var_name)))
                    is_duplicate = true;
                else if (node->hasValue(prop_validator_variable) &&
                         newValue.is_sameas(node->as_string(prop_validator_variable)))
                    is_duplicate = true;
                else if (node->hasValue(prop_checkbox_var_name) &&
                         newValue.is_sameas(node->as_string(prop_checkbox_var_name)))
                    is_duplicate = true;
            }
        }

        if (is_duplicate)
        {
            event->SetValidationFailureMessage("The name you have chosen is already in use by another variable.");
            event->Veto();
            return false;
        }

        // If the event was previously veto'd, and the user corrected the name, then we have to set it here,
        // otherwise it will revert back to the original name before the Veto.

        event->GetProperty()->SetValueFromString(newValue);
    }
    else if (prop->isProp(prop_class_name) && prop->getNode()->isForm())
    {
        auto property = wxStaticCast(event->GetProperty(), wxStringProperty);
        auto variant = event->GetPropertyValue();
        tt_string newValue = property->ValueToString(variant).utf8_string();
        if (newValue.empty())
            return true;

        std::vector<Node*> forms;
        Project.CollectForms(forms);

        for (const auto& iter: forms)
        {
            if (iter == prop->getNode())
            {
                continue;
            }
            else if (iter->as_string(prop_class_name).is_sameas(newValue))
            {
                event->SetValidationFailureMessage("The name you have chosen is already in use by another class.");
                event->Veto();
                return false;
            }
        }
    }
    else if (prop->isProp(prop_label) &&
             (prop->getNode()->isGen(gen_propGridItem) || prop->getNode()->isGen(gen_propGridCategory)))
    {
        auto property = wxStaticCast(event->GetProperty(), wxStringProperty);
        auto variant = event->GetPropertyValue();
        tt_string newValue = property->ValueToString(variant).utf8_string();
        tt_string final_name(newValue);
        auto result = node->getUniqueName(final_name, prop_label);
        if (!newValue.is_sameas(result))
        {
            event->SetValidationFailureMessage("This label is already in use by another PropertyGrid item.");
            event->Veto();
            return false;
        }
    }
    else if (prop->isProp(prop_get_function) || prop->isProp(prop_set_function))
    {
        auto property = wxStaticCast(event->GetProperty(), wxStringProperty);
        auto variant = event->GetPropertyValue();
        tt_string newValue = property->ValueToString(variant).utf8_string();

        auto rlambda = [&](Node* check_node, auto&& rlambda) -> bool
        {
            if (check_node != node)
            {
                if (check_node->hasValue(prop_get_function) && check_node->as_string(prop_get_function) == newValue)
                {
                    event->SetValidationFailureMessage("This function name is already in use.");
                    event->Veto();
                    return false;
                }
                else if (check_node->hasValue(prop_set_function) && check_node->as_string(prop_set_function) == newValue)
                {
                    event->SetValidationFailureMessage("This function name is already in use.");
                    event->Veto();
                    return false;
                }
            }

            for (auto& iter: check_node->getChildNodePtrs())
            {
                if (!rlambda(iter.get(), rlambda))
                    return false;
            }
            return true;
        };

        if (auto form = node->getForm(); form)
        {
            if (!rlambda(form, rlambda))
                return false;
        }
    }

    return true;
}

tt_string getClassHelpName(Node* node)
{
    tt_string class_name(map_GenNames[node->getGenName()]);
    if (!class_name.starts_with("wx"))
    {
        if (class_name == "BookPage")
        {
            class_name = map_GenNames[node->getParent()->getGenName()];
        }
        else if (class_name == "PanelForm")
            class_name = "wxPanel";
        else if (class_name == "RibbonBar")
            class_name = "wxRibbonBar";
        else if (class_name == "PopupMenu")
            class_name = "wxMenu";
        else if (class_name == "ToolBar")
            class_name = "wxToolBar";
        else if (class_name == "AuiToolBar")
            class_name = "wxAuiToolBar";
        else if (class_name == "auitool")
            class_name = "wxAuiToolBarItem";
        else if (class_name == "StaticCheckboxBoxSizer" || class_name == "StaticRadioBtnBoxSizer")
            class_name = "wxStaticBoxSizer";
        else if (class_name == "Check3State")
            class_name = "wxCheckBox";
        else if (class_name == "MenuBar")
            class_name = "wxMenuBar";
        else if (class_name == "propGridCategory")
            class_name = "wxPropertyCategory";
        else if (class_name == "propGridItem")
            class_name = "wxPGProperty";
        else if (class_name == "propGridPage")
            class_name = "wxPropertyGridPage";
        else if (class_name == "submenu")
            class_name = "wxMenu";
        else if (class_name == "tool" || class_name == "tool_dropdown")
            class_name = "wxToolBarToolBase";
        else
            class_name.clear();  // Don't return a non-wxWidgets class name
    }

    return class_name;
}

tt_string BaseGenerator::GetHelpText(Node* node)
{
    tt_string class_name = getClassHelpName(node);

#if defined(_DEBUG)
    if (class_name.size())
    {
        class_name << " (" << GetHelpURL(node) << ')';
    }
#endif  // _DEBUG

    return class_name;
}

tt_string BaseGenerator::GetPythonHelpText(Node* node)
{
    tt_string class_name = getClassHelpName(node);
    if (class_name.empty())
    {
        return class_name;
    }

    std::string_view prefix = "wx.";
    if (auto wx_iter = g_map_python_prefix.find(class_name); wx_iter != g_map_python_prefix.end())
    {
        prefix = wx_iter->second;
    }
    tt_string help_text;
    help_text << prefix << class_name.subview(2);

    return help_text;
}

tt_string BaseGenerator::GetPythonURL(Node* node)
{
    tt_string url = GetPythonHelpText(node);
    if (url.empty())
    {
        auto class_name = map_GenNames[node->getGenName()];
        if (tt::is_sameas(class_name, "auitool_spacer"))
        {
            url = "wx.aui.AuiToolBar.html?highlight=addspacer#wx.aui.AuiToolBar.AddSpacer";
        }
        else if (tt::is_sameas(class_name, "auitool_label"))
        {
            url = "wx.aui.AuiToolBar.html?highlight=addlabel#wx.aui.AuiToolBar.AddLabel";
        }
        else if (tt::is_sameas(class_name, "spacer"))
        {
            url = "wx.Sizer.html?highlight=addspacer#wx.Sizer.AddSpacer";
        }
        return url;
    }
    url << ".html";
    return url;
}

tt_string BaseGenerator::GetRubyURL(Node* node)
{
    tt_string url = GetRubyHelpText(node);
    if (url.empty())
    {
        auto class_name = map_GenNames[node->getGenName()];
        if (tt::is_sameas(class_name, "auitool_spacer"))
        {
            url = "Wx/AUI/AuiToolBar.html#add_spacer-instance_method";
        }
        else if (tt::is_sameas(class_name, "auitool_spacer"))
        {
            url = "Wx/AUI/AuiToolBar.html#add_label-instance_method";
        }
        else if (tt::is_sameas(class_name, "spacer"))
        {
            url = "Wx/Sizer.html#add_spacer-instance_method";
        }
        return url;
    }
    url.Replace("::", "/", true);
    url << ".html";
    return url;
}

tt_string BaseGenerator::GetRubyHelpText(Node* node)
{
    tt_string class_name = getClassHelpName(node);
    if (class_name.empty())
    {
        return class_name;
    }

    std::string_view prefix = "Wx::";
    if (auto wx_iter = g_map_ruby_prefix.find(class_name); wx_iter != g_map_ruby_prefix.end())
    {
        prefix = wx_iter->second;
    }
    tt_string help_text;
    help_text << prefix << class_name.subview(2);

    return help_text;
}

bool BaseGenerator::GetPythonImports(Node* node, std::set<std::string>& set_imports)
{
    auto class_name = node->declName();
    if (!class_name.starts_with("wx"))
    {
        return false;
    }

    std::string_view prefix = "wx.";
    if (auto wx_iter = g_map_python_prefix.find(class_name); wx_iter != g_map_python_prefix.end())
    {
        prefix = wx_iter->second;
        tt_string import_lib("import ");
        import_lib << prefix;
        import_lib.pop_back();  // remove the trailing '.'
        set_imports.insert(import_lib);
        return true;
    }
    return false;
}

void BaseGenerator::ChangeEnableState(wxPropertyGridManager* prop_grid, NodeProperty* changed_prop)
{
    // auto changed_node = changed_prop->getNode();
    if (changed_prop->isProp(prop_alignment))
    {
        if (auto pg_parent = prop_grid->GetProperty("alignment"); pg_parent)
        {
            for (unsigned int idx = 0; idx < pg_parent->GetChildCount(); ++idx)
            {
                if (auto pg_setting = pg_parent->Item(idx); pg_setting)
                {
                    auto label = pg_setting->GetLabel();
                    if (label == "wxALIGN_LEFT")
                    {
                        pg_setting->Enable(!changed_prop->as_string().contains("wxALIGN_RIGHT") &&
                                           !changed_prop->as_string().contains("wxALIGN_CENTER"));
                    }
                    else if (label == "wxALIGN_RIGHT")
                    {
                        pg_setting->Enable(!changed_prop->as_string().contains("wxALIGN_LEFT") &&
                                           !changed_prop->as_string().contains("wxALIGN_CENTER"));
                    }
                    else if (label == "wxALIGN_TOP")
                    {
                        pg_setting->Enable(!changed_prop->as_string().contains("wxALIGN_BOTTOM") &&
                                           !changed_prop->as_string().contains("wxALIGN_CENTER"));
                    }
                    else if (label == "wxALIGN_BOTTOM")
                    {
                        pg_setting->Enable(!changed_prop->as_string().contains("wxALIGN_BOTTOM") &&
                                           !changed_prop->as_string().contains("wxALIGN_CENTER"));
                    }
                    else if (label == "wxALIGN_CENTER")
                    {
                        pg_setting->Enable(changed_prop->as_string().empty() ||
                                           changed_prop->as_string().is_sameas("wxALIGN_CENTER"));
                    }
                    else if (label == "wxALIGN_CENTER_HORIZONTAL" || label == "wxALIGN_CENTER_VERTICAL")
                    {
                        pg_setting->Enable(!changed_prop->as_string().contains("wxALIGN_RIGHT") &&
                                           !changed_prop->as_string().contains("wxALIGN_LEFT") &&
                                           !changed_prop->as_string().contains("wxALIGN_BOTTOM") &&
                                           !changed_prop->as_string().contains("wxALIGN_TOP") &&
                                           !changed_prop->as_string().is_sameas("wxALIGN_CENTER"));
                    }
                }
            }
        }
    }
    else if (changed_prop->isProp(prop_use_derived_class))
    {
        if (auto pg_setting = prop_grid->GetProperty("private_members"); pg_setting)
        {
            pg_setting->Enable(!changed_prop->as_bool());
        }

        if (auto pg_setting = prop_grid->GetProperty("derived_class_name"); pg_setting)
        {
            pg_setting->Enable(changed_prop->as_bool());
        }

        if (auto pg_setting = prop_grid->GetProperty("derived_file"); pg_setting)
        {
            pg_setting->Enable(changed_prop->as_bool());
        }
    }
    else if (changed_prop->isProp(prop_python_use_xrc))
    {
        if (auto pg_setting = prop_grid->GetProperty("python_xrc_file"); pg_setting)
        {
            pg_setting->Enable(changed_prop->as_bool());
        }
    }
}

bool BaseGenerator::VerifyProperty(NodeProperty* prop)
{
    bool result = false;

    if (prop->isProp(prop_alignment))
    {
        auto value = prop->as_raw_ptr();
        if (value->contains("wxALIGN_LEFT") &&
            (value->contains("wxALIGN_RIGHT") || value->contains("wxALIGN_CENTER_HORIZONTAL")))
        {
            value->Replace("wxALIGN_LEFT|", "");
            result = true;
        }
        if (value->contains("wxALIGN_TOP") &&
            (value->contains("wxALIGN_BOTTOM") || value->contains("wxALIGN_CENTER_VERTICAL")))
        {
            value->Replace("wxALIGN_TOP|", "");
            result = true;
        }
        if (value->contains("wxALIGN_RIGHT") && value->contains("wxALIGN_CENTER_HORIZONTAL"))
        {
            value->Replace("wxALIGN_RIGHT|", "");
            result = true;
        }
        if (value->contains("wxALIGN_BOTTOM") && value->contains("wxALIGN_CENTER_VERTICAL"))
        {
            value->Replace("wxALIGN_BOTTOM|", "");
            result = true;
        }

        // wxALIGN_CENTER can't be combined with anything
        if (value->contains("wxALIGN_CENTER|"))
        {
            value->Replace("wxALIGN_CENTER|", "");
            result = true;
        }
    }

    return result;
}

static const std::set<GenEnum::PropName> set_output_files = {
    prop_base_file,
    prop_python_file,
    prop_ruby_file,
    prop_xrc_file,
};

std::optional<tt_string> BaseGenerator::GetHint(NodeProperty* prop)
{
    if (prop->isProp(prop_derived_class_name) && !prop->hasValue())
    {
        // Note that once set, this won't change until the property grid gets recreated.
        return tt_string(!prop->getNode()->as_bool(prop_use_derived_class) ? "requires use_derived_class" : "");
    }
    else if (prop->isProp(prop_derived_file) && !prop->hasValue())
    {
        return tt_string(!prop->getNode()->as_bool(prop_use_derived_class) ? "requires use_derived_class" : "");
    }
    else if (prop->isProp(prop_python_xrc_file) && !prop->hasValue())
    {
        return tt_string(!prop->getNode()->as_bool(prop_use_derived_class) ? "requires python_use_xrc" : "");
    }
    else if (set_output_files.contains(prop->getPropDeclaration()->get_name()) && !prop->hasValue())
    {
        return tt_string("change class_name to auto-fill");
    }
    else
    {
        return {};
    }
}

// clang-format off

// non-sorted order is critical!

static std::vector<std::pair<const char*, const char*>> prefix_pair = {

    { "bag", "_bag" },
    { "bar", "_bar" },
    { "bitmap", "_bitmap" },
    { "bookpage", "book_ctrl_base" },
    { "box", "_box" },
    { "bundle", "_bundle" },  // just in case we want to add help for this
    { "button", "_button" },
    { "colour", "_colour" },
    { "column", "_column" },
    { "combo", "_combo" },
    { "ctrl", "_ctrl" },
    { "dialog", "_dialog" },  // stddialog becomes std_dialog
    { "double", "_double" },
    { "event", "_event" },
    { "grid", "_grid" },
    { "item", "_item" },
    { "list", "_list" },
    { "line", "_line" },
    { "manager", "_manager" },
    { "menu", "_menu" },
    { "notebook", "_notebook" },
    { "page", "_page" },
    { "pane", "_pane" },
    { "picker", "_picker" },
    { "simple", "_simple" },
    { "sizer", "_sizer" },
    { "text", "_text" },
    { "tool_bar", "_tool_bar" },  // not that bar will already have been changed to _bar
    { "tree", "_tree" },
    { "validator", "_validator" },
    { "view", "_view" },
    { "window", "_window" },

};
// clang-format on

tt_string BaseGenerator::GetHelpURL(Node* node)
{
    tt_string class_name = getClassHelpName(node);

    if (class_name.starts_with("wx"))
    {
        class_name.erase(0, 2);
        class_name.MakeLower();

        if (class_name == "contextmenuevent")
        {
            class_name = "context_menu_event";
        }
        else if (class_name == "activityindicator")
        {
            class_name = "activity_indicator";
        }
        else if (class_name == "simplehtmllistbox")
        {
            class_name = "simple_html_list_box";
        }
        else if (class_name == "propertysheetdialog")
        {
            class_name = "property_sheet_dialog";
        }
        else if (class_name == "toolbartoolbase")
        {
            class_name = "tool_bar_tool_base";
        }
        else
        {
            for (const auto& [key, value]: prefix_pair)
            {
                if (!class_name.starts_with(key))
                    class_name.Replace(key, value);
            }
        }

        tt_string url = "wx_";
        url << class_name << ".html";
        return url;
    }

    // REVIEW: [Randalphwa - 07-23-2023] some of these are now being handled by getClassHelpName()
    // and will therefore never make it this far.

    else if (class_name == "BookPage")
    {
        return tt_string("wx_book_ctrl_base.html");
    }
    else if (class_name == "dataViewColumn")
    {
        return tt_string("wx_data_view_ctrl.html");
    }
    else if (class_name == "dataViewListColumn")
    {
        return tt_string("wx_data_view_list_ctrl.html");
    }
    else if (class_name == "PanelForm")
    {
        return tt_string("wx_panel.html");
    }
    else if (class_name == "PopupMenu" || class_name == "submenu")
    {
        return tt_string("wx_menu.html");
    }
    else if (class_name == "propGridPage")
    {
        return tt_string("wx_property_grid_page.html");
    }
    else if (class_name == "RibbonBar")
    {
        return tt_string("wx_ribbon_bar.html");
    }
    else if (class_name == "RibbonToolBar")
    {
        return tt_string("wx_ribbon_tool_bar.html");
    }
    else if (class_name == "spacer")
    {
        return tt_string("wx_sizer.html");
    }
    else if (class_name == "StaticCheckboxBoxSizer" || class_name == "StaticRadioBtnBoxSizer")
    {
        return tt_string("wx_static_box_sizer.html");
    }
    else if (class_name == "ToolBar")
    {
        return tt_string("wx_tool_bar.html");
    }
    else if (class_name == "AuiToolBar")
    {
        return tt_string("wx_aui_tool_bar.html");
    }
    else if (class_name == "TreeListCtrlColumn")
    {
        return tt_string("wx_tree_list_ctrl.html");
    }

    return tt_string();
}

// clang-format off

// These are the control types that cannot have their parent changed
static const auto parentless_types = {

    type_aui_tool,
    type_bookpage,
    type_ctx_menu,
    type_data_list,
    type_data_string,
    type_dataviewcolumn,
    type_dataviewlistcolumn,
    type_embed_image,
    type_images,
    type_menu,
    type_menubar,
    type_menubar_form,
    type_menuitem,
    type_page,
    type_ribbonbar,
    type_ribbonbar_form,
    type_ribbonbutton,
    type_ribbonbuttonbar,
    type_ribbongallery,
    type_ribbongalleryitem,
    type_ribbonpage,
    type_ribbonpanel,
    type_ribbontool,
    type_ribbontoolbar,
    type_tool,
    type_wizardpagesimple

};
// clang-format on

bool BaseGenerator::CanChangeParent(Node* node)
{
    if (node->isForm())
    {
        return false;
    }

    for (auto& iter: parentless_types)
    {
        if (node->isType(iter))
        {
            return false;
        }
    }

    return true;
}

int BaseGenerator::GetRequiredVersion(Node* node)
{
    if (node->hasValue(prop_platforms) && node->as_string(prop_platforms) != "Windows|Unix|Mac")
    {
        return minRequiredVer + 1;
    }
    if (node->hasValue(prop_cpp_conditional) || node->hasValue(prop_python_conditional))
    {
        return minRequiredVer + 2;  // 1.1.1 release
    }

    return minRequiredVer;
}

std::optional<tt_string> BaseGenerator::isLanguagePropSupported(Node* node, GenLang language, GenEnum::PropName prop)
{
    if (prop == prop_persist && node->as_bool(prop))
    {
        switch (language)
        {
            case GEN_LANG_PERL:
                return "persist is not supported in Perl";
            case GEN_LANG_RUST:
                return "persist is not supported in Rust";
            case GEN_LANG_PYTHON:
                return "persist is not supported in Python";
            case GEN_LANG_XRC:
                return "persist is not supported in XRC";

#if GENERATE_NEW_LANG_CODE
            case GEN_LANG_FORTRAN:
                return "persist is not supported in Fortran";
            case GEN_LANG_HASKELL:
                return "persist is not supported in Haskell";
            case GEN_LANG_LUA:
                return "persist is not supported in Lua";
#endif  // GENERATE_NEW_LANG_CODE
            default:
                return {};
        }
    }

    return {};
}

PropDeclaration* DeclAddProp(NodeDeclaration* declaration, PropName prop_name, PropType type, std::string_view help,
                             std::string_view def_value)
{
    auto& properties = declaration->GetPropInfoMap();
    auto prop_info = new PropDeclaration(prop_name, type, def_value, help);
    declaration->GetCategory().addProperty(prop_name);
    properties[map_PropNames[prop_name]] = prop_info;
    return prop_info;
}

void DeclAddOption(PropDeclaration* prop_info, std::string_view name, std::string_view help)
{
    auto& options = prop_info->getOptions();
    auto& opt = options.emplace_back();
    opt.name = name;
    opt.help = help;
}

// var_names for these generators will default to "none" for class access
// inline const GenName set_no_class_access[] = {
constexpr auto set_no_class_access = frozen::make_set<GenName>({

    gen_BookPage,
    gen_CloseButton,
    gen_StaticCheckboxBoxSizer,
    gen_StaticRadioBtnBoxSizer,
    gen_TextSizer,
    gen_VerticalBoxSizer,
    gen_auitool,
    gen_auitool_label,
    gen_separator,
    gen_submenu,
    gen_tool,
    gen_tool_dropdown,
    gen_wxBoxSizer,
    gen_wxFlexGridSizer,
    gen_wxGridBagSizer,
    gen_wxGridSizer,
    gen_wxMenuItem,
    gen_wxPanel,
    gen_wxRibbonButtonBar,
    gen_wxRibbonGallery,
    gen_wxRibbonPage,
    gen_wxRibbonPanel,
    gen_wxRibbonToolBar,
    gen_wxStaticBitmap,
    gen_wxStaticBoxSizer,
    gen_wxStaticLine,
    gen_wxStdDialogButtonSizer,
    gen_wxWizardPageSimple,
    gen_wxWrapSizer,

});

void DeclAddVarNameProps(NodeDeclaration* declaration, std::string_view def_value)
{
    DeclAddProp(declaration, prop_var_name, type_string, {}, def_value);
    DeclAddProp(declaration, prop_var_comment, type_string_edit_single);

    tt_string access("protected:");
    if (set_no_class_access.contains(declaration->getGenName()))
    {
        access = "none";
    }

    auto* prop_info = DeclAddProp(declaration, prop_class_access, type_option, {}, access);
    DeclAddOption(prop_info, "none", "Derived classes do not have access to this item.");
    DeclAddOption(prop_info, "protected:",
                  "In C++, derived classes can access this item. For other languages, the variable is accessible by other "
                  "classes and functions.");
    DeclAddOption(prop_info, "public:",
                  "In C++, item is added as a public: class member. For other languages, the variable is accessible by "
                  "other classes and functions.");
}

void DeclAddEvent(NodeDeclaration* declaration, const std::string& evt_name, std::string_view event_class,
                  std::string_view help)
{
    declaration->GetCategory().addEvent(evt_name);
    declaration->GetEventInfoMap()[evt_name] = new NodeEventInfo(evt_name, event_class, help);
}
