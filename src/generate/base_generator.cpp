/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base widget generator class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "node.h"                  // Node class
#include "node_prop.h"             // NodeProperty -- NodeProperty class
#include <wx/event.h>              // Event classes
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "base_generator.h"

#include "../mainframe.h"             // MainFrame -- Main window frame
#include "../mockup/mockup_parent.h"  // Top-level MockUp Parent window

MockupParent* BaseGenerator::GetMockup()
{
    return wxGetFrame().GetMockup();
}

void BaseGenerator::OnLeftClick(wxMouseEvent& event)
{
    auto wxobject = event.GetEventObject();
    auto node = wxGetFrame().GetMockup()->GetNode(wxobject);

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
        ttString newValue = property->ValueToString(variant);
        if (newValue.IsEmpty())
            return true;

        auto parent = node->GetParent();
        if (newValue == "wxALIGN_TOP" || newValue == "wxALIGN_BOTTOM" || newValue == "wxALIGN_CENTER_VERTICAL")
        {
            if (parent && parent->IsSizer() && parent->prop_as_string(prop_orientation).contains("wxVERTICAL"))
            {
                wxMessageBox("You can't set vertical alignment when the parent sizer is oriented vertically.",
                             "Invalid alignment");
                event->Veto();
                return false;
            }
            else if (node->prop_as_string(prop_flags).contains("wxEXPAND"))
            {
                wxMessageBox("You can't set vertical alignment if the wxEXPAND flag is set.", "Invalid alignment");
                event->Veto();
                return false;
            }
        }
        else if (newValue == "wxALIGN_LEFT" || newValue == "wxALIGN_RIGHT" || newValue == "wxALIGN_CENTER_HORIZONTAL")
        {
            if (parent && parent->IsSizer() && parent->prop_as_string(prop_orientation).contains("wxHORIZONTAL"))
            {
                wxMessageBox("You can't set horizontal alignment when the parent sizer is oriented horizontally.",
                             "Invalid alignment");
                event->Veto();
                return false;
            }
            else if (node->prop_as_string(prop_flags).contains("wxEXPAND"))
            {
                wxMessageBox("You can't set horizontal alignment if the wxEXPAND flag is set.", "Invalid alignment");
                event->Veto();
                return false;
            }
        }
    }
    else if (prop->isProp(prop_flags))
    {
        auto property = wxStaticCast(event->GetProperty(), wxFlagsProperty);
        auto variant = event->GetPropertyValue();
        ttString newValue = property->ValueToString(variant);
        if (newValue.IsEmpty())
            return true;

        // Remove the original flags so that all we are checking is the changed flag.
        if (node->HasValue(prop_flags))
        {
            auto original = node->prop_as_wxString(prop_flags);
            original.Replace("|", ", ");
            newValue.Replace(original, "");
        }

        // The newValue may have a flag removed, so this might not be the flag that got unchecked.
        if (newValue.contains("wxEXPAND"))
        {
            if (node->HasValue(prop_alignment))
            {
                auto& alignment = node->prop_as_string(prop_alignment);
                if (alignment.contains("wxALIGN_LEFT") || alignment.contains("wxALIGN_RIGHT") ||
                    alignment.contains("wxALIGN_CENTER_HORIZONTAL") || alignment.contains("wxALIGN_TOP") ||
                    alignment.contains("wxALIGN_BOTTOM") || alignment.contains("wxALIGN_CENTER_VERTICAL"))
                {
                    wxMessageBox("You can't set the wxEXPAND flag if you have either horizontal or vertical alignment set.",
                                 "Invalid alignment");
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
        ttlib::cstr newValue = property->ValueToString(variant).utf8_string();
        if (newValue.empty())
            return true;
        auto unique_name = node->GetUniqueName(newValue);
        // GetUniqueName() won't check the current node so if the name is unique, we still need to check within the same node
        bool is_duplicate = !newValue.is_sameas(unique_name);
        if (!is_duplicate)
        {
            if (prop->isProp(prop_var_name))
            {
                if (node->HasValue(prop_validator_variable) &&
                    newValue.is_sameas(node->prop_as_string(prop_validator_variable)))
                    is_duplicate = true;
                else if (node->HasValue(prop_checkbox_var_name) &&
                         newValue.is_sameas(node->prop_as_string(prop_checkbox_var_name)))
                    is_duplicate = true;
                else if (node->HasValue(prop_radiobtn_var_name) &&
                         newValue.is_sameas(node->prop_as_string(prop_radiobtn_var_name)))
                    is_duplicate = true;
            }
            else if (prop->isProp(prop_validator_variable))
            {
                if (node->HasValue(prop_var_name) && newValue.is_sameas(node->prop_as_string(prop_var_name)))
                    is_duplicate = true;
                else if (node->HasValue(prop_checkbox_var_name) &&
                         newValue.is_sameas(node->prop_as_string(prop_checkbox_var_name)))
                    is_duplicate = true;
                else if (node->HasValue(prop_radiobtn_var_name) &&
                         newValue.is_sameas(node->prop_as_string(prop_radiobtn_var_name)))
                    is_duplicate = true;
            }
            else if (prop->isProp(prop_checkbox_var_name))
            {
                if (node->HasValue(prop_var_name) && newValue.is_sameas(node->prop_as_string(prop_var_name)))
                    is_duplicate = true;
                else if (node->HasValue(prop_validator_variable) &&
                         newValue.is_sameas(node->prop_as_string(prop_validator_variable)))
                    is_duplicate = true;
                else if (node->HasValue(prop_radiobtn_var_name) &&
                         newValue.is_sameas(node->prop_as_string(prop_radiobtn_var_name)))
                    is_duplicate = true;
            }
            else if (prop->isProp(prop_radiobtn_var_name))
            {
                if (node->HasValue(prop_var_name) && newValue.is_sameas(node->prop_as_string(prop_var_name)))
                    is_duplicate = true;
                else if (node->HasValue(prop_validator_variable) &&
                         newValue.is_sameas(node->prop_as_string(prop_validator_variable)))
                    is_duplicate = true;
                else if (node->HasValue(prop_checkbox_var_name) &&
                         newValue.is_sameas(node->prop_as_string(prop_checkbox_var_name)))
                    is_duplicate = true;
            }
        }

        if (is_duplicate)
        {
            wxMessageBox("The name you have chosen is already in use by another variable.", "Duplicate name");
            event->Veto();
            event->GetProperty()->SetValue(newValue.wx_str());
            return false;
        }

        // If the event was previously veto'd, and the user corrected the name, then we have to set it here,
        // otherwise it will revert back to the original name before the Veto.

        event->GetProperty()->SetValueFromString(newValue, 0);
    }
    return true;
}
