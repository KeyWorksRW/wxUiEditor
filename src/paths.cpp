/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for directory and file properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// This module handles changes to art_directory, base_directory, and derived_directory

#include "ttstr.h"  // ttString -- wxString with additional methods similar to ttlib::cstr

#include "paths.h"

#include "mainapp.h"    // Main application class
#include "mainframe.h"  // MainFrame -- Main window frame
#include "node.h"       // Node class

void AllowDirectoryChange(wxPropertyGridEvent& event, NodeProperty* /* prop */, Node* /* node */)
{
    ttString newValue = event.GetPropertyValue().GetString();
    if (newValue.empty())
        return;

    newValue.make_absolute();
    newValue.make_relative_wx(wxGetApp().GetProjectPath());
    newValue.backslashestoforward();

    if (!newValue.dir_exists())
    {
        // Displaying the message box can cause a focus change event which will call validation again in the OnIdle()
        // processing. Preserve the focus to avoid validating twice.
        auto focus = wxWindow::FindFocus();

        auto result = wxMessageBox(ttlib::cstr() << "The directory \"" << newValue.wx_str()
                                                 << "\" does not exist. Do you want to use this name anyway?",
                                   "Directory doesn't exist", wxYES_NO | wxICON_WARNING, wxGetApp().GetMainFrame());
        if (focus)
        {
            focus->SetFocus();
        }

        if (result != wxYES)
        {
            event.Veto();
            event.SetValidationFailureBehavior(wxPG_VFB_MARK_CELL | wxPG_VFB_STAY_IN_PROPERTY);
            wxGetFrame().SetStatusField("Either change the directory, or press ESC to restore the original value.");
            return;
        }
    }

    // If the event was previously veto'd, and the user corrected the file, then we have to set it here,
    // otherwise it will revert back to the original name before the Veto.

    event.GetProperty()->SetValueFromString(newValue, 0);
}

// Unlike the AllowDirectoryChange() above, this will *not* allow a duplicate prop_base_file filename since the generated
// code will create a linker error due to the duplicate filenames (and the risk of overwriting an already generated file for
// a different class).

void AllowFileChange(wxPropertyGridEvent& event, NodeProperty* prop, Node* node)
{
    if (prop->isProp(prop_base_file))
    {
        ttString newValue = event.GetPropertyValue().GetString();
        if (newValue.empty())
            return;

        newValue.make_absolute();
        newValue.make_relative_wx(wxGetApp().GetProjectPath());
        newValue.backslashestoforward();

        auto filename = newValue.sub_cstr();
        auto project = wxGetApp().GetProject();
        for (size_t child_idx = 0; child_idx < project->GetChildCount(); ++child_idx)
        {
            if (project->GetChild(child_idx) == node)
                continue;
            if (project->GetChild(child_idx)->prop_as_string(prop_base_file).filename() == filename)
            {
                auto focus = wxWindow::FindFocus();

                wxMessageBox(wxString() << "The base filename \"" << filename << "\" is already in use by "
                                        << project->GetChild(child_idx)->prop_as_string(prop_class_name)
                                        << "\n\nEither change the name, or press ESC to restore the original name.",
                             "Duplicate base filename", wxICON_STOP);
                if (focus)
                {
                    focus->SetFocus();
                }

                event.Veto();
                event.SetValidationFailureBehavior(wxPG_VFB_MARK_CELL | wxPG_VFB_STAY_IN_PROPERTY);
                wxGetFrame().SetStatusField("Either change the name, or press ESC to restore the original value.");
                return;
            }
        }

        // If the event was previously veto'd, and the user corrected the file, then we have to set it here,
        // otherwise it will revert back to the original name before the Veto.

        event.GetProperty()->SetValueFromString(newValue, 0);
    }
}
