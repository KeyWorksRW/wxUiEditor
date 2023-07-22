/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base widget generator class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/event.h>              // Event classes
#include <wx/propgrid/manager.h>   // wxPropertyGridManager
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

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

MockupParent* BaseGenerator::GetMockup()
{
    return wxGetFrame().GetMockup();
}

void BaseGenerator::OnLeftClick(wxMouseEvent& event)
{
    auto wxobject = event.GetEventObject();
    auto node = wxGetFrame().GetMockup()->getNode(wxobject);

    if (wxGetFrame().GetSelectedNode() != node)
    {
        wxGetFrame().GetMockup()->SelectNode(wxobject);
    }
    event.Skip();
}

bool BaseGenerator::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop, Node* node)
{
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

        if (!isValidVarName(newValue))
        {
            event->SetValidationFailureMessage("The name you have specified is not a valid C++ variable name.");
            event->Veto();
            return false;
        }
        auto unique_name = node->getUniqueName(newValue);
        // getUniqueName() won't check the current node so if the name is unique, we still need to check within the same node
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
            wxGetFrame().SetStatusField("Either change the name, or press ESC to restore the original value.");
            return false;
        }

        // If the event was previously veto'd, and the user corrected the name, then we have to set it here,
        // otherwise it will revert back to the original name before the Veto.

        event->GetProperty()->SetValueFromString(newValue, 0);
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
                wxGetFrame().SetStatusField("Either change the name, or press ESC to restore the original value.");
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
            wxGetFrame().SetStatusField("Either change the name, or press ESC to restore the original value.");
            return false;
        }
    }

    return true;
}

tt_string BaseGenerator::GetHelpText(Node* node)
{
    tt_string class_name(map_GenNames[node->getGenName()]);
    if (!class_name.starts_with("wx"))
    {
        if (class_name == "BookPage")
            class_name = "wxBookCtrl";
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
        else if (class_name == "StaticCheckboxBoxSizer" || class_name == "StaticRadioBtnBoxSizer")
            class_name = "wxStaticBoxSizer";
        else
            class_name.clear();  // Don't return a non-wxWidgets class name
    }

#if defined(_DEBUG)
    if (class_name.size())
    {
        class_name << " (" << GetHelpURL(node) << ')';
    }
#endif  // _DEBUG

    return class_name;
}

extern std::map<std::string_view, std::string_view, std::less<>> g_map_class_prefix;

tt_string BaseGenerator::GetPythonHelpText(Node* node)
{
    auto class_name = node->declName();
    if (!class_name.starts_with("wx"))
    {
        if (class_name == "BookPage")
            class_name = "wxBookCtrl";
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
        else if (class_name == "StaticCheckboxBoxSizer" || class_name == "StaticRadioBtnBoxSizer")
            class_name = "wxStaticBoxSizer";
        else
            return {};
    }

    std::string_view prefix = "wx.";
    if (auto wx_iter = g_map_class_prefix.find(class_name); wx_iter != g_map_class_prefix.end())
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
        return url;
    }
    url << ".html";
    return url;
}

tt_string BaseGenerator::GetRubyHelpText(Node* node)
{
    auto class_name = node->declName();
    if (!class_name.starts_with("wx"))
    {
        if (class_name == "BookPage")
            class_name = "wxBookCtrl";
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
        else if (class_name == "StaticCheckboxBoxSizer" || class_name == "StaticRadioBtnBoxSizer")
            class_name = "wxStaticBoxSizer";
        else
            return {};
    }

    std::string_view prefix = "Wx::";
    tt_string help_text;
    help_text << prefix << class_name.subview(2);

    return help_text;
}

tt_string BaseGenerator::GetRubyURL(Node* node)
{
    tt_string url = GetRubyHelpText(node);
    if (url.empty())
    {
        return url;
    }
    url.Replace("::", "/");
    url << ".html";
    return url;
}

bool BaseGenerator::GetPythonImports(Node* node, std::set<std::string>& set_imports)
{
    auto class_name = node->declName();
    if (!class_name.starts_with("wx"))
    {
        return false;
    }

    std::string_view prefix = "wx.";
    if (auto wx_iter = g_map_class_prefix.find(class_name); wx_iter != g_map_class_prefix.end())
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
    else if (prop->isProp(prop_base_file) && !prop->hasValue())
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
    tt_string class_name(map_GenNames[node->getGenName()]);
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
