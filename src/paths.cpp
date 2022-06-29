/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for directory and file properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// This module handles changes to art_directory, base_directory, and derived_directory

#include "ttcwd.h"  // cwd -- Class for storing and optionally restoring the current directory
#include "ttstr.h"  // ttString -- wxString with additional methods similar to ttlib::cstr

#include "paths.h"

#include "mainapp.h"        // Main application class
#include "mainframe.h"      // MainFrame -- Main window frame
#include "node.h"           // Node class
#include "project_class.h"  // Project class
#include "undo_cmds.h"      // InsertNodeAction -- Undoable command classes derived from UndoAction

void AllowDirectoryChange(wxPropertyGridEvent& event, NodeProperty* /* prop */, Node* /* node */)
{
    ttString newValue = event.GetPropertyValue().GetString();
    if (newValue.empty())
        return;

    newValue.make_absolute();
    newValue.make_relative_wx(wxGetApp().GetProjectPath());
    newValue.backslashestoforward();

    ttSaveCwd cwd;
    wxGetApp().GetProjectPath().ChangeDir();

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

        for (const auto& child: project->GetChildNodePtrs())
        {
            if (child.get() == node)
                continue;
            if (child->prop_as_string(prop_base_file).filename() == filename)
            {
                auto focus = wxWindow::FindFocus();

                wxMessageBox(wxString() << "The base filename \"" << filename << "\" is already in use by "
                                        << child->prop_as_string(prop_class_name)
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

void OnPathChanged(wxPropertyGridEvent& event, NodeProperty* prop, Node* /* node */)
{
    // If the user clicked the path button, the current directory may have changed.
    wxGetApp().GetProjectPath().ChangeDir();

    ttString newValue = event.GetPropertyValue().GetString();
    newValue.make_absolute();
    newValue.make_relative_wx(wxGetApp().GetProjectPath());
    newValue.backslashestoforward();

    // Note that on Windows, even though we changed the property to a forward slash, it will still be displayed
    // with a backslash. However, ModifyProperty() will save our forward slash version, so even thought the
    // display isn't correct, it will be stored in the project file correctly.

    event.GetProperty()->SetValueFromString(newValue, 0);
    ttlib::cstr value(newValue.wx_str());
    if (value != prop->as_string())
    {
        if (prop->isProp(prop_derived_directory))
        {
            ChangeDerivedDirectory(value);
        }
        else if (prop->isProp(prop_base_directory))
        {
            ChangeBaseDirectory(value);
        }
        else
        {
            wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
        }
    }
}

void ChangeDerivedDirectory(ttlib::cstr& path)
{
    auto& old_path = wxGetApp().GetProject()->prop_as_string(prop_derived_directory);
    path.backslashestoforward();
    if (path == "./")
        path.clear();
    if (path.size() && path.back() == '/')
        path.pop_back();

    auto undo_derived = std::make_shared<ModifyProperties>("Derived directory");
    undo_derived->AddProperty(wxGetApp().GetProject()->get_prop_ptr(prop_derived_directory), path);

    for (auto& form: wxGetApp().GetProject()->GetChildNodePtrs())
    {
        if (form->prop_as_bool(prop_use_derived_class) && form->HasValue(prop_derived_file))
        {
            ttlib::cstr cur_path = form->prop_as_string(prop_derived_file);
            cur_path.backslashestoforward();
            cur_path.remove_filename();
            if (cur_path.size() && cur_path.back() == '/')
                cur_path.pop_back();

            // if the current directory and the old directory aren't the same, then leave it alone
            if (!old_path.is_sameas(cur_path, tt::CASE::either))
            {
                continue;
            }

            cur_path = path;
            cur_path.append_filename(form->prop_as_string(prop_derived_file).filename());
            undo_derived->AddProperty(form->get_prop_ptr(prop_derived_file), cur_path);
        }
    }

    wxGetFrame().PushUndoAction(undo_derived);
}

void ChangeBaseDirectory(ttlib::cstr& path)
{
    auto& old_path = wxGetApp().GetProject()->prop_as_string(prop_base_directory);
    path.backslashestoforward();
    if (path == "./")
        path.clear();
    if (path.size() && path.back() == '/')
        path.pop_back();

    auto undo_derived = std::make_shared<ModifyProperties>("Base directory");
    undo_derived->AddProperty(wxGetApp().GetProject()->get_prop_ptr(prop_base_directory), path);

    for (const auto& form: wxGetApp().GetProject()->GetChildNodePtrs())
    {
        if (form->HasValue(prop_base_file))
        {
            ttlib::cstr cur_path = form->prop_as_string(prop_base_directory);
            cur_path.backslashestoforward();
            cur_path.remove_filename();
            if (cur_path.size() && cur_path.back() == '/')
                cur_path.pop_back();

            // if the current directory and the old directory aren't the same, then leave it alone
            if (!old_path.is_sameas(cur_path, tt::CASE::either))
            {
                continue;
            }

            cur_path = path;
            cur_path.append_filename(form->prop_as_string(prop_base_file).filename());
            undo_derived->AddProperty(form->get_prop_ptr(prop_base_file), cur_path);
        }
    }

    wxGetFrame().PushUndoAction(undo_derived);
}
