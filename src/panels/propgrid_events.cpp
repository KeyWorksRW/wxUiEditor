/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid class event handlers
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>             // wxConfig base header
#include <wx/infobar.h>            // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/propgrid/advprops.h>  // wxPropertyGrid Advanced Properties (font, colour, etc.)

#include "propgrid_panel.h"

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "base_panel.h"       // BasePanel -- Code generation panel
#include "cstm_event.h"       // CustomEvent -- Custom Event class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "paths.h"            // Handles *_directory properties
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties

void PropGridPanel::OnAuiNotebookPageChanged(wxAuiNotebookEvent& /* event */)
{
    if (auto panel = wxGetFrame().GetFirstCodePanel(); panel)
    {
        CustomEvent custom_event(EVT_NodeSelected, wxGetFrame().getSelectedNode());
        panel->OnNodeSelected(custom_event);
    }
}

void PropGridPanel::OnEventGridChanged(wxPropertyGridEvent& event)
{
    if (auto it = m_event_map.find(event.GetProperty()); it != m_event_map.end())
    {
        NodeEvent* evt = it->second;
        wxString handler = event.GetPropertyValue();
        auto value = ConvertEscapeSlashes(handler.utf8_string());
        value.trim(tt::TRIM::both);
        wxGetFrame().ChangeEventHandler(evt, value);
    }
}

// This handles the custom event fired when a node's property is changed.
void PropGridPanel::OnNodePropChange(CustomEvent& event)
{
    if (m_isPropChangeSuspended)
    {
        // If the property was modified in the property grid, then we are receiving this event after
        // the node in the property has already been changed. We don't need to process it since we
        // already saw it, but we can use the oppoprtunity to do some additional processing, such as
        // notifying the user that the Mockup can't display the property change.

        OnPostPropChange(event);
        return;
    }

    auto prop = event.GetNodeProperty();
    auto grid_property = m_prop_grid->GetPropertyByLabel(prop->declName().make_wxString());
    if (!grid_property)
        return;

    switch (prop->type())
    {
        case type_float:
            grid_property->SetValue(WXVARIANT(prop->as_float()));
            break;

        case type_int:
        case type_uint:
            grid_property->SetValueFromString(prop->as_string());
            break;

        case type_string:
        case type_string_edit:
            grid_property->SetValueFromString(prop->as_string());
            break;

        case type_string_edit_escapes:
        case type_string_escapes:
        case type_stringlist_escapes:
            grid_property->SetValueFromString(prop->as_escape_text().make_wxString());
            break;

        case type_id:
        case type_option:
        case type_editoption:
            grid_property->SetValueFromString(prop->as_escape_text());
            break;

        case type_bool:
            grid_property->SetValueFromInt(prop->as_string() == "0" ? 0 : 1);
            break;

        case type_bitlist:
            {
                auto value = prop->as_string();
                value.Replace("|", ", ", true);
                if (value == "0")
                    value = "";
                grid_property->SetValueFromString(value);
            }
            break;

        case type_wxPoint:
            {
                // m_prop_grid->SetPropertyValue( grid_property, prop->GetValue() );
                auto aux = prop->as_string();
                aux.Replace(",", ";");
                grid_property->SetValueFromString(aux);
            }
            break;

        case type_wxSize:
            {
                // m_prop_grid->SetPropertyValue( grid_property, prop->GetValue() );
                auto aux = prop->as_string();
                aux.Replace(",", ";");
                grid_property->SetValueFromString(aux);
            }
            break;

        case type_wxColour:
            {
                wxColourPropertyValue def(wxPG_COLOUR_CUSTOM, prop->as_color());
                m_prop_grid->SetPropertyValue(grid_property, def);
            }
            break;

        case type_animation:
        case type_image:
            break;

        default:
            grid_property->SetValueFromString(prop->as_string(), wxPGPropValFormatFlags::FullValue);
    }
    m_prop_grid->Refresh();
}

void PropGridPanel::OnEventGridExpand(wxPropertyGridEvent& event)
{
    m_expansion_map[event.GetPropertyName().utf8_string()] = event.GetProperty()->IsExpanded();

    auto grid_property = m_prop_grid->GetProperty(event.GetProperty()->GetName());
    if (grid_property)
    {
        if (event.GetProperty()->IsExpanded())
        {
            m_prop_grid->Expand(grid_property);
        }
        else
        {
            m_prop_grid->Collapse(grid_property);
        }
    }
}

void PropGridPanel::OnPostPropChange(CustomEvent& event)
{
    if (event.GetNodeProperty()->isProp(prop_border))
    {
        auto info = wxGetFrame().GetPropInfoBar();
        info->Dismiss();
        if (event.GetNodeProperty()->as_string() == "wxBORDER_RAISED")
        {
            info->ShowMessage("The Mockup panel is not able to show a mockup of the raised border.",
                              wxICON_INFORMATION);
        }
    }
    else if (event.GetNodeProperty()->isProp(prop_focus))
    {
        auto node = event.getNode();
        auto form = node->getForm();
        auto list = form->findAllChildProperties(prop_focus);
        size_t count = 0;
        for (auto iter: list)
        {
            if (iter->as_bool())
            {
                ++count;
            }
        }

        if (count > 1)
        {
            wxGetFrame().GetPropInfoBar()->ShowMessage("More than one control has focus set.",
                                                       wxICON_INFORMATION);
        }
        else
        {
            wxGetFrame().GetPropInfoBar()->Dismiss();
        }
    }
    else if (event.GetNodeProperty()->isProp(prop_size) ||
             event.GetNodeProperty()->isProp(prop_minimum_size) ||
             event.GetNodeProperty()->isProp(prop_maximum_size))
    {
        auto node = event.getNode();
        auto new_size = node->as_wxSize(prop_size);
        auto min_size = node->as_wxSize(prop_minimum_size);
        auto max_size = node->as_wxSize(prop_maximum_size);
        if (new_size != wxDefaultSize || min_size != wxDefaultSize || max_size != wxDefaultSize)
        {
            // If any value is -1 then it's not actually set and no comparison is needed
            if (min_size != wxDefaultSize &&
                ((new_size.x != -1 && min_size.x != -1 && new_size.x < min_size.x) ||
                 (new_size.y != -1 && min_size.y != -1 && new_size.y < min_size.y)))
            {
                wxGetFrame().GetPropInfoBar()->ShowMessage(
                    "The size property is smaller than the minimum size property.", wxICON_WARNING);
            }
            else if (max_size != wxDefaultSize &&
                     ((new_size.x != -1 && max_size.x != -1 && new_size.x > max_size.x) ||
                      (new_size.y != -1 && max_size.y != -1 && new_size.y > max_size.y)))
            {
                wxGetFrame().GetPropInfoBar()->ShowMessage(
                    "The size property is larger than the maximum size property.", wxICON_WARNING);
            }
            else if (min_size != wxDefaultSize && max_size != wxDefaultSize &&
                     ((min_size.x != -1 && max_size.x != -1 && min_size.x > max_size.x) ||
                      (min_size.y != -1 && max_size.y != -1 && min_size.y > max_size.y)))
            {
                wxGetFrame().GetPropInfoBar()->ShowMessage(
                    "The minimum size property is larger than the maximum size property.",
                    wxICON_WARNING);
            }
            else
            {
                wxGetFrame().GetPropInfoBar()->Dismiss();
            }
        }
    }
}

void PropGridPanel::OnPropertyGridChanged(wxPropertyGridEvent& event)
{
    auto property = event.GetProperty();

    auto it = m_property_map.find(property);
    if (it == m_property_map.end())
    {
        property = property->GetParent();
        it = m_property_map.find(property);
    }

    if (it == m_property_map.end())
        return;

    auto prop = it->second;
    if (prop->get_name() == prop_code_preference)
    {
        // TODO: [Randalphwa - 10-23-2024] Either code preferences should only show
        // prop_generate_languages, or prop_generate_languages should always update the code
        // preferences. Even better would be to disable the matching generate language bit so that
        // the user can't shut it off.

        modifyProperty(prop, m_prop_grid->GetPropertyValueAsString(property).utf8_string());
        auto grid_iterator = m_prop_grid->GetCurrentPage()->GetIterator(wxPG_ITERATE_CATEGORIES);
        while (!grid_iterator.AtEnd())
        {
            auto grid_property = grid_iterator.GetProperty();
            if (grid_property->GetLabel().Contains("C++"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "C++")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
            else if (grid_property->GetLabel().Contains("Perl"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "Perl")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
            else if (grid_property->GetLabel().Contains("Python"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "Python")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
            else if (grid_property->GetLabel().Contains("Ruby"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "Ruby")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
            else if (grid_property->GetLabel().Contains("Rust"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "Rust")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
            else if (grid_property->GetLabel().Contains("XRC"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "XRC")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
#if GENERATE_NEW_LANG_CODE
            else if (grid_property->GetLabel().Contains("Fortran"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "Fortran")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
            else if (grid_property->GetLabel().Contains("Haskell"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "Haskell")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
            else if (grid_property->GetLabel().Contains("Lua"))
            {
                if (prop->as_string() != "any" && prop->as_string() != "Lua")
                {
                    m_prop_grid->Collapse(grid_property);
                }
                else
                {
                    m_prop_grid->Expand(grid_property);
                }
            }
#endif  // GENERATE_NEW_LANG_CODE

            grid_iterator.Next();
        }

        // This will cause GenerateDlg to initialize the code generation choice to the
        // preferred language, clearing any previously selected language. We can't just use
        // the property, because the dialog needs to know if inherited C++ code should be
        // written.
        auto config = wxConfig::Get();
        config->Write("GenCode", 0);

        wxGetFrame().FireProjectUpdatedEvent();
        wxGetFrame().UpdateLanguagePanels();

        return;
    }

    auto node = prop->getNode();

    switch (prop->type())
    {
        case type_float:
            {
                double val = m_prop_grid->GetPropertyValueAsDouble(property);

                modifyProperty(prop, DoubleToStr(val));
                break;
            }

        case type_code_edit:
        case type_html_edit:
        case type_wxFont:
        case type_string_edit:
        case type_id:
        case type_int:
        case type_include_files:
        case type_uint:
        case type_statbar_fields:
        case type_checklist_item:
            {
                modifyProperty(prop, m_prop_grid->GetPropertyValueAsString(property).utf8_string());
                break;
            }

        case type_option:
        case type_editoption:
            ModifyOptionsProperty(prop, property);
            break;

        case type_string_escapes:
        case type_string_edit_escapes:
            {
                auto value = ConvertEscapeSlashes(
                    m_prop_grid->GetPropertyValueAsString(property).utf8_string());
                modifyProperty(prop, value);
            }
            break;

        case type_stringlist:
#if defined(_WIN32)
            if (prop->isProp(prop_contents))
            {
                // REVIEW: [Randalphwa - 06-26-2023] This will only work if we use quotes to
                // separate items.
                tt_string newValue = property->GetValueAsString().utf8_string();
                // Under Windows 10 using wxWidgets 3.1.3, the last character of the string is
                // partially clipped. Adding a trailing space prevents this clipping.

                if (m_currentSel->isGen(gen_wxRadioBox) && newValue.size())
                {
                    size_t result;
                    for (size_t pos = 0; pos < newValue.size();)
                    {
                        result = newValue.find("\" \"", pos);
                        if (tt::is_found(result))
                        {
                            if (newValue.at(result - 1) != ' ')
                                newValue.insert(result, 1, ' ');
                            pos = result + 3;
                        }
                        else
                        {
                            break;
                        }
                    }

                    result = newValue.find_last_of('"');
                    if (tt::is_found(result))
                    {
                        if (newValue.at(result - 1) != ' ')
                            newValue.insert(result, 1, ' ');
                    }
                    modifyProperty(prop, newValue);
                    break;
                }
            }
#endif  // _WIN32
            modifyProperty(prop, property->GetValueAsString().utf8_string());
            break;

        case type_stringlist_semi:
            modifyProperty(prop, property->GetValueAsString().utf8_string());
            break;

        case type_bool:
            ModifyBoolProperty(prop, property);
            break;

        case type_bitlist:
            ModifyBitlistProperty(prop, property);
            break;

        case type_wxPoint:
        case type_wxSize:
            {
                auto value = event.GetPropertyValue().GetString();
                modifyProperty(prop, value.utf8_string());
            }
            break;

        case type_animation:
        case type_image:
            ModifyEmbeddedProperty(prop, property);
            break;

        case type_file:
            ModifyFileProperty(prop, property);
            break;

        case type_path:
            {
                m_isPropChangeSuspended = true;
                OnPathChanged(event, prop, node);
                m_isPropChangeSuspended = false;
            }
            break;

        default:
            {
                tt_string newValue = property->GetValueAsString().utf8_string();

                if (prop->isProp(prop_var_name))
                {
                    if (newValue.empty())
                    {
                        // An empty name will generate uncompilable code, so we simply switch it to
                        // the default name
                        auto new_name = prop->getPropDeclaration()->getDefaultValue();
                        auto final_name = node->getUniqueName(new_name);
                        newValue = final_name.size() ? final_name : new_name;

                        auto grid_property = m_prop_grid->GetPropertyByLabel("var_name");
                        grid_property->SetValueFromString(newValue.make_wxString());
                    }
                }

                modifyProperty(prop, newValue);

                if (prop->isProp(prop_class_name))
                {
                    if (auto selected_node = wxGetFrame().getSelectedNode();
                        selected_node && selected_node->isForm())
                    {
                        CheckOutputFile(newValue, selected_node);

                        if (Project.getCodePreference() == GEN_LANG_CPLUSPLUS)
                        {
                            if (!selected_node->as_bool(prop_use_derived_class))
                                return;

                            if (!selected_node->hasValue(prop_derived_class_name))
                            {
                                ReplaceDerivedName(
                                    newValue, selected_node->getPropPtr(prop_derived_class_name));
                                ReplaceDerivedFile(
                                    selected_node->as_string(prop_derived_class_name),
                                    selected_node->getPropPtr(prop_derived_file));
                            }
                        }
                    }
                    return;
                }
            }
            break;
    }

    if (node->isGen(gen_wxContextMenuEvent))
    {
        auto event_prop = node->getParent()->getEvent("wxEVT_CONTEXT_MENU");
        if (event_prop)
        {
            event_prop->set_value(node->as_string(prop_handler_name));
        }
    }

    ChangeEnableState(prop);

    if (auto gen = prop->getNode()->getGenerator(); gen)
    {
        auto result = gen->isLanguagePropSupported(prop->getNode(), Project.getCodePreference(),
                                                   prop->get_name());
        if (result.has_value())
        {
            wxGetFrame().ShowInfoBarMsg(result.value());
        }
        else
        {
            wxGetFrame().DismissInfoBar();
        }
    }
    else
    {
        wxGetFrame().DismissInfoBar();
    }
}

void PropGridPanel::OnPropertyGridExpand(wxPropertyGridEvent& event)
{
    m_expansion_map[event.GetPropertyName().utf8_string()] = event.GetProperty()->IsExpanded();

    auto egProp = m_event_grid->GetProperty(event.GetProperty()->GetName());
    if (egProp)
    {
        if (event.GetProperty()->IsExpanded())
        {
            m_event_grid->Expand(egProp);
        }
        else
        {
            m_event_grid->Collapse(egProp);
        }
    }
}

// Only process property changes that we may need to cancel here.
void PropGridPanel::OnPropertyGridChanging(wxPropertyGridEvent& event)
{
    auto property = event.GetProperty();

    auto it = m_property_map.find(property);
    if (it == m_property_map.end())
    {
        property = property->GetParent();
        it = m_property_map.find(property);
    }

    if (it == m_property_map.end())
        return;

    auto prop = it->second;
    auto node = prop->getNode();
    auto generator = node->getGenerator();
    if (generator)
    {
        if (!generator->AllowPropertyChange(&event, prop, node))
        {
            return;
        }
    }

    switch (prop->type())
    {
        case type_file:
            AllowFileChange(event, prop, node);
            break;

        case type_path:
            if (!node->isGen(gen_wxFilePickerCtrl))
            {
                AllowDirectoryChange(event, prop, node);
            }
            break;

        default:
            break;
    }
}

void PropGridPanel::OnPropertyGridItemSelected(wxPropertyGridEvent& event)
{
    auto property = event.GetProperty();
    if (property)
    {
        if (m_notebook_parent->GetSelection() == 0)
        {
            m_selected_prop_name = m_prop_grid->GetPropertyName(property);
            m_pageName = "Properties";
        }
        else
        {
            m_selected_event_name = m_event_grid->GetPropertyName(property);
            m_pageName = "Events";
        }
    }
}
