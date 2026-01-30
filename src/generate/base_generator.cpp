/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base widget generator class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/event.h>              // Event classes
#include <wx/propgrid/manager.h>   // wxPropertyGridManager
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include <frozen/set.h>

#include "base_generator.h"

#include "code.h"        // Code -- Helper class for generating code
#include "gen_common.h"  // Common component functions
#include "gen_enums.h"
#include "lambdas.h"          // Functions for formatting and storage of lambda events
#include "mainframe.h"        // MainFrame -- Main window frame
#include "mockup_parent.h"    // Top-level MockUp Parent window
#include "node.h"             // Node class
#include "node_decl.h"        // NodeDeclaration class
#include "node_prop.h"        // NodeProperty -- NodeProperty class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties

#include "wxue_namespace/wxue_string.h"  // wxue::string

MockupParent* BaseGenerator::getMockup()
{
    return wxGetFrame().getMockup();
}

void BaseGenerator::OnLeftClick(wxMouseEvent& event)
{
    auto* wxobject = event.GetEventObject();
    auto* node = wxGetFrame().getMockup()->getNode(wxobject);

    if (wxGetFrame().getSelectedNode() != node)
    {
        wxGetFrame().getMockup()->SelectNode(wxobject);
    }
    event.Skip();
}

bool BaseGenerator::AllowIdPropertyChange(wxPropertyGridEvent* event, NodeProperty* /* prop */,
                                          Node* node)
{
    wxue::string newValue = event->GetPropertyValue().GetString().utf8_string();
    if (newValue.empty())
    {
        return true;
    }

    auto* form = node->get_Form();
    if (node->is_Gen(gen_wxMenuItem))
    {
        form = node->get_Parent();
        while (form && !form->is_Gen(gen_wxMenuBar) && !form->is_Gen(gen_MenuBar))
        {
            form = form->get_Parent();
        }

        // This shouldn't happen, but return true just in case
        if (!form)
        {
            return true;
        }
    }
    else if (node->is_Gen(gen_auitool))
    {
        form = node->get_Parent();
        while (form && !form->is_Gen(gen_AuiToolBar) && !form->is_Gen(gen_wxAuiToolBar))
        {
            form = form->get_Parent();
        }

        // This shouldn't happen, but return true just in case
        if (!form)
        {
            return true;
        }
    }
    else if (node->is_Gen(gen_tool) || node->is_Gen(gen_tool_dropdown))
    {
        form = node->get_Parent();
        while (form && !form->is_Gen(gen_ToolBar) && !form->is_Gen(gen_wxToolBar))
        {
            form = form->get_Parent();
        }

        // This shouldn't happen, but return true just in case
        if (!form)
        {
            return true;
        }
    }
    else if (node->is_Gen(gen_ribbonTool) || node->is_Gen(gen_ribbonButton) ||
             node->is_Gen(gen_ribbonGalleryItem))
    {
        form = node->get_Parent();
        while (form && !form->is_Gen(gen_RibbonBar) && !form->is_Gen(gen_wxRibbonBar))
        {
            form = form->get_Parent();
        }

        // This shouldn't happen, but return true just in case
        if (!form)
        {
            return true;
        }
    }

    std::set<wxue::string> ids;

    auto rlambda = [&](Node* child, auto&& rlambda) -> void
    {
        if (child != node && child->HasValue(prop_id) &&
            !child->as_string(prop_id).is_sameprefix("wx"))
        {
            ids.emplace(child->get_PropId());
        }

        for (const auto& iter: child->get_ChildNodePtrs())
        {
            if (iter->HasValue(prop_id) && !iter->as_string(prop_id).is_sameprefix("wx"))
            {
                ids.emplace(iter->get_PropId());
            }
            rlambda(iter.get(), rlambda);
        }
    };

    rlambda(form, rlambda);

    // Same as NodeProperty::get_PropId() -- strip off any assignment
    wxue::string new_id;
    if (auto pos = newValue.find('='); pos != wxue::npos)
    {
        while (pos > 0 && wxue::is_whitespace(newValue[pos - 1]))
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
        auto* property = wxStaticCast(event->GetProperty(), wxFlagsProperty);
        auto variant = event->GetPropertyValue();
        wxue::string newValue = property->ValueToString(variant).utf8_string();
        if (newValue.empty())
        {
            return true;
        }

        auto* parent = node->get_Parent();
        if (newValue == "wxALIGN_TOP" || newValue == "wxALIGN_BOTTOM" ||
            newValue == "wxALIGN_CENTER_VERTICAL")
        {
            if (parent && parent->is_Sizer() &&
                parent->as_string(prop_orientation).contains("wxVERTICAL"))
            {
                event->SetValidationFailureMessage("You can't set vertical alignment when the "
                                                   "parent sizer is oriented vertically.");
                event->Veto();
                return false;
            }
            if (node->as_string(prop_flags).contains("wxEXPAND"))
            {
                event->SetValidationFailureMessage(
                    "You can't set vertical alignment if the wxEXPAND flag is set.");
                event->Veto();
                return false;
            }
        }
        else if (newValue == "wxALIGN_LEFT" || newValue == "wxALIGN_RIGHT" ||
                 newValue == "wxALIGN_CENTER_HORIZONTAL")
        {
            if (parent && parent->is_Sizer() &&
                parent->as_string(prop_orientation).contains("wxHORIZONTAL"))
            {
                event->SetValidationFailureMessage("You can't set horizontal alignment when the "
                                                   "parent sizer is oriented horizontally.");
                event->Veto();
                return false;
            }
            if (node->as_string(prop_flags).contains("wxEXPAND"))
            {
                event->SetValidationFailureMessage(
                    "You can't set horizontal alignment if the wxEXPAND flag is set.");
                event->Veto();
                return false;
            }
        }
    }
    else if (prop->isProp(prop_flags))
    {
        auto* property = wxStaticCast(event->GetProperty(), wxFlagsProperty);
        auto variant = event->GetPropertyValue();
        wxue::string newValue = property->ValueToString(variant).utf8_string();
        if (newValue.empty())
        {
            return true;
        }

        // Remove the original flags so that all we are checking is the changed flag.
        if (node->HasValue(prop_flags))
        {
            auto original = node->as_string(prop_flags);
            original.Replace("|", ", ");
            newValue.Replace(original, "");
        }

        // The newValue may have a flag removed, so this might not be the flag that got unchecked.
        if (newValue.contains("wxEXPAND"))
        {
            if (node->HasValue(prop_alignment))
            {
                const auto& alignment = node->as_string(prop_alignment);
                if (alignment.contains("wxALIGN_LEFT") || alignment.contains("wxALIGN_RIGHT") ||
                    alignment.contains("wxALIGN_CENTER_HORIZONTAL") ||
                    alignment.contains("wxALIGN_TOP") || alignment.contains("wxALIGN_BOTTOM") ||
                    alignment.contains("wxALIGN_CENTER_VERTICAL"))
                {
                    event->SetValidationFailureMessage(
                        "You can't set the wxEXPAND flag if you have either horizontal or vertical "
                        "alignment set.");
                    event->Veto();
                    return false;
                }
            }
        }
    }
    else if (prop->isProp(prop_var_name) || prop->isProp(prop_validator_variable) ||
             prop->isProp(prop_checkbox_var_name) || prop->isProp(prop_radiobtn_var_name))
    {
        auto* property = wxStaticCast(event->GetProperty(), wxStringProperty);
        auto variant = event->GetPropertyValue();
        wxue::string newValue = property->ValueToString(variant).utf8_string();
        if (newValue.empty())
        {
            return true;
        }

        if (!isValidVarName(newValue, Project.get_CodePreference()))
        {
            event->SetValidationFailureMessage(
                "The name you have specified is not a valid variable name.");
            event->Veto();
            return false;
        }
        auto unique_name = node->get_UniqueName(newValue);

        // The above call to get_UniqueName() won't check the current node so if the name is
        // unique, we still need to check within the same node
        bool is_duplicate = !newValue.is_sameas(unique_name);
        if (!is_duplicate)
        {
            if (prop->isProp(prop_var_name))
            {
                if (node->HasValue(prop_validator_variable) &&
                    newValue.is_sameas(node->as_string(prop_validator_variable)))
                {
                    is_duplicate = true;
                }
                else if (node->HasValue(prop_checkbox_var_name) &&
                         newValue.is_sameas(node->as_string(prop_checkbox_var_name)))
                {
                    is_duplicate = true;
                }
                else if (node->HasValue(prop_radiobtn_var_name) &&
                         newValue.is_sameas(node->as_string(prop_radiobtn_var_name)))
                {
                    is_duplicate = true;
                }
            }
            else if (prop->isProp(prop_validator_variable))
            {
                if (node->HasValue(prop_var_name) &&
                    newValue.is_sameas(node->as_string(prop_var_name)))
                {
                    is_duplicate = true;
                }
                else if (node->HasValue(prop_checkbox_var_name) &&
                         newValue.is_sameas(node->as_string(prop_checkbox_var_name)))
                {
                    is_duplicate = true;
                }
                else if (node->HasValue(prop_radiobtn_var_name) &&
                         newValue.is_sameas(node->as_string(prop_radiobtn_var_name)))
                {
                    is_duplicate = true;
                }
            }
            else if (prop->isProp(prop_checkbox_var_name))
            {
                if (node->HasValue(prop_var_name) &&
                    newValue.is_sameas(node->as_string(prop_var_name)))
                {
                    is_duplicate = true;
                }
                else if (node->HasValue(prop_validator_variable) &&
                         newValue.is_sameas(node->as_string(prop_validator_variable)))
                {
                    is_duplicate = true;
                }
                else if (node->HasValue(prop_radiobtn_var_name) &&
                         newValue.is_sameas(node->as_string(prop_radiobtn_var_name)))
                {
                    is_duplicate = true;
                }
            }
            else if (prop->isProp(prop_radiobtn_var_name))
            {
                if (node->HasValue(prop_var_name) &&
                    newValue.is_sameas(node->as_string(prop_var_name)))
                {
                    is_duplicate = true;
                }
                else if (node->HasValue(prop_validator_variable) &&
                         newValue.is_sameas(node->as_string(prop_validator_variable)))
                {
                    is_duplicate = true;
                }
                else if (node->HasValue(prop_checkbox_var_name) &&
                         newValue.is_sameas(node->as_string(prop_checkbox_var_name)))
                {
                    is_duplicate = true;
                }
            }
        }

        if (is_duplicate)
        {
            event->SetValidationFailureMessage(
                "The name you have chosen is already in use by another variable.");
            event->Veto();
            return false;
        }

        // If the event was previously veto'd, and the user corrected the name, then we have to set
        // it here, otherwise it will revert back to the original name before the Veto.

        event->GetProperty()->SetValueFromString(newValue);
    }
    else if (prop->isProp(prop_class_name) && prop->getNode()->is_Form())
    {
        auto* property = wxStaticCast(event->GetProperty(), wxStringProperty);
        auto variant = event->GetPropertyValue();
        wxue::string newValue = property->ValueToString(variant).utf8_string();
        if (newValue.empty())
        {
            return true;
        }

        std::vector<Node*> forms;
        Project.CollectForms(forms);

        for (const auto& iter: forms)
        {
            if (iter == prop->getNode())
            {
                continue;
            }
            if (iter->as_string(prop_class_name).is_sameas(newValue))
            {
                event->SetValidationFailureMessage(
                    "The name you have chosen is already in use by another class.");
                event->Veto();
                return false;
            }
        }
    }
    else if (prop->isProp(prop_label) && (prop->getNode()->is_Gen(gen_propGridItem) ||
                                          prop->getNode()->is_Gen(gen_propGridCategory)))
    {
        auto* property = wxStaticCast(event->GetProperty(), wxStringProperty);
        auto variant = event->GetPropertyValue();
        wxue::string newValue = property->ValueToString(variant).utf8_string();
        const wxue::string& final_name(newValue);
        auto result = node->get_UniqueName(final_name, prop_label);
        if (!newValue.is_sameas(result))
        {
            event->SetValidationFailureMessage(
                "This label is already in use by another PropertyGrid item.");
            event->Veto();
            return false;
        }
    }
    else if (prop->isProp(prop_get_function) || prop->isProp(prop_set_function))
    {
        auto* property = wxStaticCast(event->GetProperty(), wxStringProperty);
        auto variant = event->GetPropertyValue();
        wxue::string newValue = property->ValueToString(variant).utf8_string();

        auto rlambda = [&](Node* check_node, auto&& rlambda) -> bool
        {
            if (check_node != node)
            {
                if (check_node->HasValue(prop_get_function) &&
                    check_node->as_string(prop_get_function) == newValue)
                {
                    event->SetValidationFailureMessage("This function name is already in use.");
                    event->Veto();
                    return false;
                }
                if (check_node->HasValue(prop_set_function) &&
                    check_node->as_string(prop_set_function) == newValue)
                {
                    event->SetValidationFailureMessage("This function name is already in use.");
                    event->Veto();
                    return false;
                }
            }

            return std::ranges::all_of(check_node->get_ChildNodePtrs(),
                                       [&](const auto& iter)
                                       {
                                           return rlambda(iter.get(), rlambda);
                                       });
        };

        if (auto* form = node->get_Form(); form)
        {
            if (!rlambda(form, rlambda))
            {
                return false;
            }
        }
    }

    return true;
}

namespace
{
    const std::map<std::string_view, std::string_view> map_org_widgets = {
        { "BookPage", "wxBookPage" },
        { "PanelForm", "wxPanel" },
        { "RibbonBar", "wxRibbonBar" },
        { "PopupMenu", "wxMenu" },
        { "ToolBar", "wxToolBar" },
        { "AuiToolBar", "wxAuiToolBar" },
        { "auitool", "wxAuiToolBarItem" },
        { "StaticCheckboxBoxSizer", "wxStaticBoxSizer" },
        { "StaticRadioBtnBoxSizer", "wxStaticBoxSizer" },
        { "Check3State", "wxCheckBox" },
        { "MenuBar", "wxMenuBar" },
        { "propGridCategory", "wxPropertyCategory" },
        { "propGridItem", "wxPGProperty" },
        { "propGridPage", "wxPropertyGridPage" },
        { "submenu", "wxMenu" },
        { "tool", "wxToolBarToolBase" },
        { "tool_dropdown", "wxToolBarToolBase" },
    };

    auto getClassHelpName(Node* node) -> std::string
    {
        ASSERT(map_GenNames.contains(node->get_GenName()));
        std::string class_name(map_GenNames.at(node->get_GenName()));
        if (!class_name.starts_with("wx"))
        {
            if (auto iter = map_org_widgets.find(class_name); iter != map_org_widgets.end())
            {
                class_name = iter->second;
            }
        }

        return class_name;
    }
}  // namespace

auto BaseGenerator::GetHelpText(Node* node) -> wxue::string
{
    auto class_name = getClassHelpName(node);

#if defined(_DEBUG)
    if (class_name.size())
    {
        class_name += " (" + GetHelpURL(node) + ')';
    }
#endif  // _DEBUG

    return class_name;
}

wxue::string BaseGenerator::GetPythonHelpText(Node* node)
{
    wxue::string class_name = getClassHelpName(node);
    if (class_name.empty())
    {
        return class_name;
    }

    std::string_view prefix = "wx.";
    if (auto wx_iter = g_map_python_prefix.find(class_name); wx_iter != g_map_python_prefix.end())
    {
        prefix = wx_iter->second;
    }
    wxue::string help_text;
    help_text << prefix << class_name.subview(2);

    return help_text;
}

wxue::string BaseGenerator::GetPythonURL(Node* node)
{
    wxue::string url = GetPythonHelpText(node);
    if (url.empty())
    {
        ASSERT(map_GenNames.contains(node->get_GenName()));
        auto class_name = map_GenNames.at(node->get_GenName());
        if (class_name == "auitool_spacer")
        {
            url = "wx.aui.AuiToolBar.html?highlight=addspacer#wx.aui.AuiToolBar.AddSpacer";
        }
        else if (class_name == "auitool_label")
        {
            url = "wx.aui.AuiToolBar.html?highlight=addlabel#wx.aui.AuiToolBar.AddLabel";
        }
        else if (class_name == "spacer")
        {
            url = "wx.Sizer.html?highlight=addspacer#wx.Sizer.AddSpacer";
        }
        return url;
    }
    url << ".html";
    return url;
}

wxue::string BaseGenerator::GetRubyURL(Node* node)
{
    wxue::string url = GetRubyHelpText(node);
    if (url.empty())
    {
        ASSERT(map_GenNames.contains(node->get_GenName()));
        auto class_name = map_GenNames.at(node->get_GenName());
        if (class_name == "auitool_spacer")
        {
            url = "Wx/AUI/AuiToolBar.html#add_spacer-instance_method";
        }
        else if (class_name == "auitool_label")
        {
            url = "Wx/AUI/AuiToolBar.html#add_label-instance_method";
        }
        else if (class_name == "spacer")
        {
            url = "Wx/Sizer.html#add_spacer-instance_method";
        }
        return url;
    }
    url.Replace("::", "/", true);
    url << ".html";
    return url;
}

wxue::string BaseGenerator::GetRubyHelpText(Node* node)
{
    wxue::string class_name = getClassHelpName(node);
    if (class_name.empty())
    {
        return class_name;
    }

    std::string_view prefix = "Wx::";
    if (auto wx_iter = g_map_ruby_prefix.find(class_name); wx_iter != g_map_ruby_prefix.end())
    {
        prefix = wx_iter->second;
    }
    wxue::string help_text;
    help_text << prefix << class_name.subview(2);

    return help_text;
}

bool BaseGenerator::GetPythonImports(Node* node, std::set<std::string>& set_imports)
{
    auto class_name = node->get_DeclName();
    if (!class_name.starts_with("wx"))
    {
        return false;
    }

    std::string_view prefix = "wx.";
    if (auto wx_iter = g_map_python_prefix.find(class_name); wx_iter != g_map_python_prefix.end())
    {
        prefix = wx_iter->second;
        wxue::string import_lib("import ");
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
        if (auto* pg_parent = prop_grid->GetProperty("alignment"); pg_parent)
        {
            for (unsigned int idx = 0; idx < pg_parent->GetChildCount(); ++idx)
            {
                if (auto* pg_setting = pg_parent->Item(idx); pg_setting)
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
                    else if (label == "wxALIGN_CENTER_HORIZONTAL" ||
                             label == "wxALIGN_CENTER_VERTICAL")
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
        if (auto* pg_setting = prop_grid->GetProperty("private_members"); pg_setting)
        {
            pg_setting->Enable(!changed_prop->as_bool());
        }

        if (auto* pg_setting = prop_grid->GetProperty("pure_virtual_functions"); pg_setting)
        {
            pg_setting->Enable(changed_prop->as_bool());
        }

        if (auto* pg_setting = prop_grid->GetProperty("derived_class_name"); pg_setting)
        {
            pg_setting->Enable(changed_prop->as_bool());
        }

        if (auto* pg_setting = prop_grid->GetProperty("derived_file"); pg_setting)
        {
            pg_setting->Enable(changed_prop->as_bool());
        }
    }
    else if (changed_prop->isProp(prop_python_use_xrc))
    {
        if (auto* pg_setting = prop_grid->GetProperty("python_xrc_file"); pg_setting)
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
        auto* value = prop->as_raw_ptr();
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

std::optional<wxue::string> BaseGenerator::GetHint(NodeProperty* prop)
{
    if (prop->isProp(prop_derived_class_name) && !prop->HasValue())
    {
        // Note that once set, this won't change until the property grid gets recreated.
        return wxue::string(
            !prop->getNode()->as_bool(prop_use_derived_class) ? "requires use_derived_class" : "");
    }
    if (prop->isProp(prop_derived_file) && !prop->HasValue())
    {
        return wxue::string(
            !prop->getNode()->as_bool(prop_use_derived_class) ? "requires use_derived_class" : "");
    }
    if (prop->isProp(prop_python_xrc_file) && !prop->HasValue())
    {
        return wxue::string(
            !prop->getNode()->as_bool(prop_use_derived_class) ? "requires python_use_xrc" : "");
    }
    if (set_output_files.contains(prop->get_PropDeclaration()->get_name()) && !prop->HasValue())
    {
        return wxue::string("change class_name to auto-fill");
    }

    return {};
}

// non-sorted order is critical!

namespace
{
    const std::vector<std::pair<std::string_view, std::string_view>> prefix_pair = {

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

    const std::map<std::string_view, std::string_view> map_convert_classname = {
        { "contextmenuevent", "context_menu_event" },
        { "activityindicator", "activity_indicator" },
        { "simplehtmllistbox", "simple_html_list_box" },
        { "propertysheetdialog", "property_sheet_dialog" },
        { "toolbartoolbase", "tool_bar_tool_base" },
        { "docparentframe", "doc_parent_frame" },
        { "docmdiparentframe", "doc_m_d_i_parent_frame" },
        { "auimdiparentframe", "aui_m_d_i_parent_frame" },
    };

    const std::map<std::string_view, std::string_view> map_convert_nonwx_classname = {
        { "BookPage", "book_ctrl_base" },
        { "dataViewColumn", "data_view_ctrl" },
        { "dataViewListColumn", "data_view_list_ctrl" },
        { "PanelForm", "panel" },
        { "PopupMenu", "menu" },
        { "propGridPage", "property_grid_page" },
        { "RibbonBar", "ribbon_bar" },
        { "RibbonToolBar", "ribbon_tool_bar" },
        { "spacer", "sizer" },
        { "StaticCheckboxBoxSizer", "static_box_sizer" },
        { "StaticRadioBtnBoxSizer", "static_box_sizer" },
        { "ToolBar", "tool_bar" },
        { "AuiToolBar", "aui_tool_bar" },
        { "TreeListCtrlColumn", "tree_list_ctrl" }
    };

}  // namespace

auto BaseGenerator::GetHelpURL(Node* node) -> wxue::string
{
    std::string class_name = getClassHelpName(node);

    if (class_name.starts_with("wx"))
    {
        class_name.erase(0, 2);
        std::ranges::transform(class_name, class_name.begin(),
                               [](unsigned char character)
                               {
                                   return static_cast<char>(std::tolower(character));
                               });
        if (auto iter = map_convert_classname.find(class_name); iter != map_convert_classname.end())
        {
            class_name = iter->second;
        }
        else
        {
            for (const auto& [key, value]: prefix_pair)
            {
                if (auto pos = class_name.find(key); pos != std::string::npos)
                {
                    class_name.replace(pos, key.length(), value);
                }
            }
        }

        wxue::string url = "wx_";
        url << class_name << ".html";
        return url;
    }

    if (auto iter = map_convert_nonwx_classname.find(class_name);
        iter != map_convert_nonwx_classname.end())
    {
        return { iter->second };
    }

    return {};
}

namespace
{
    // These are the control types that cannot have their parent changed
    const auto parentless_types = {

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
}  // namespace

bool BaseGenerator::CanChangeParent(Node* node)
{
    if (node->is_Form())
    {
        return false;
    }

    if (!std::ranges::all_of(parentless_types,
                             [&](const auto& iter)
                             {
                                 return !node->is_Type(iter);
                             }))
    {
        return false;
    }

    return true;
}

auto BaseGenerator::GetRequiredVersion(Node* node) -> int
{
    if (node->HasValue(prop_platforms) && node->as_string(prop_platforms) != "Windows|Unix|Mac")
    {
        return minRequiredVer + 1;
    }
    if (node->HasValue(prop_cpp_conditional) || node->HasValue(prop_python_conditional))
    {
        return minRequiredVer + 2;  // 1.1.1 release
    }

    return minRequiredVer;
}

auto BaseGenerator::isLanguagePropSupported(Node* node, GenLang language, GenEnum::PropName prop)
    -> std::optional<wxue::string>
{
    if (prop == prop_persist && node->as_bool(prop))
    {
        switch (language)
        {
            case GEN_LANG_PERL:
                return "persist is not supported in Perl";
            case GEN_LANG_PYTHON:
                return "persist is not supported in Python";
            case GEN_LANG_XRC:
                return "persist is not supported in XRC";
            default:
                return {};
        }
    }

    return {};
}

auto DeclAddProp(NodeDeclaration* declaration, PropName prop_name, PropType type,
                 std::string_view help, std::string_view def_value) -> PropDeclaration*
{
    auto& properties = declaration->GetPropInfoMap();
    auto* prop_info =
        new PropDeclaration(prop_name, type, PropDeclaration::DefaultValue { def_value },
                            PropDeclaration::HelpText { help });
    declaration->GetCategory().addProperty(prop_name);
    ASSERT(map_PropNames.contains(prop_name));
    properties[std::string(map_PropNames.at(prop_name))] = prop_info;
    return prop_info;
}

void DeclAddOption(PropDeclaration* prop_info, std::string_view name, std::string_view help)
{
    auto& options = prop_info->getOptions();
    auto& opt = options.emplace_back();
    opt.name = name;
    opt.help = help;
}

namespace
{
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
}  // namespace

void DeclAddVarNameProps(NodeDeclaration* declaration, std::string_view def_value)
{
    DeclAddProp(declaration, prop_var_name, type_string, {}, def_value);
    DeclAddProp(declaration, prop_var_comment, type_string_edit_single);

    wxue::string access("protected:");
    if (set_no_class_access.contains(declaration->get_GenName()))
    {
        access = "none";
    }

    auto* prop_info = DeclAddProp(declaration, prop_class_access, type_option, {}, access);
    DeclAddOption(prop_info, "none", "Derived classes do not have access to this item.");
    DeclAddOption(prop_info, "protected:",
                  "In C++, derived classes can access this item. For other languages, the variable "
                  "is accessible by other "
                  "classes and functions.");
    DeclAddOption(prop_info, "public:",
                  "In C++, item is added as a public: class member. For other languages, the "
                  "variable is accessible by "
                  "other classes and functions.");
}

void DeclAddEvent(NodeDeclaration* declaration, const std::string& evt_name,
                  std::string_view event_class, std::string_view help)
{
    declaration->GetCategory().addEvent(evt_name);
    declaration->GetEventInfoMap()[evt_name] = new NodeEventInfo(evt_name, event_class, help);
}

auto BaseGenerator::IsGeneric(Node* /*unused*/) -> bool
{
    return false;
}
