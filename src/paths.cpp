/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for directory and file properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// This module handles changes to art_directory, base_directory, and derived_directory

#include <ttcwd_wx.h>  // cwd -- Class for storing and optionally restoring the current directory

#include "paths.h"

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
    newValue.make_relative_wx(GetProject()->GetProjectPath());
    newValue.backslashestoforward();

    ttSaveCwd cwd;
    GetProject()->GetProjectPath().ChangeDir();

    if (!newValue.dir_exists())
    {
        // Displaying the message box can cause a focus change event which will call validation again in the OnIdle()
        // processing. Preserve the focus to avoid validating twice.
        auto focus = wxWindow::FindFocus();

        auto result = wxMessageBox(ttlib::cstr() << "The directory \"" << newValue.wx_str()
                                                 << "\" does not exist. Do you want to use this name anyway?",
                                   "Directory doesn't exist", wxYES_NO | wxICON_WARNING, GetMainFrame());
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
    if (prop->isProp(prop_base_file) || prop->isProp(prop_python_file) || prop->isProp(prop_xrc_file))
    {
        ttString newValue = event.GetPropertyValue().GetString();
        if (newValue.empty())
            return;

        newValue.make_absolute();
        newValue.make_relative_wx(GetProject()->GetProjectPath());
        newValue.backslashestoforward();

        auto filename = newValue.sub_cstr();
        auto project = GetProject();

        std::vector<Node*> forms;
        project->CollectForms(forms);
        for (const auto& child: forms)
        {
            if (child == node)
                continue;
            if (prop->isProp(prop_base_file))
            {
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
            else if (prop->isProp(prop_python_file))
            {
                if (child->as_string(prop_python_file).filename() == filename)
                {
                    auto focus = wxWindow::FindFocus();

                    wxMessageBox(wxString() << "The python filename \"" << filename << "\" is already in use by "
                                            << child->prop_as_string(prop_class_name)
                                            << "\n\nEither change the name, or press ESC to restore the original name.",
                                 "Duplicate python filename", wxICON_STOP);
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
            else
            {
                // Currently, XRC files don't have a directory property, so the full path
                // relative to the project file is what we check. It *is* valid to have the
                // same filename provided it is in a different directory.
                if (child->prop_as_string(prop_xrc_file) == filename)
                {
                    auto focus = wxWindow::FindFocus();

                    wxMessageBox(wxString() << "The xrc filename \"" << filename << "\" is already in use by "
                                            << child->prop_as_string(prop_class_name)
                                            << "\n\nEither change the name, or press ESC to restore the original name.",
                                 "Duplicate xrc filename", wxICON_STOP);

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
        }

        // If the event was previously veto'd, and the user corrected the file, then we have to set it here,
        // otherwise it will revert back to the original name before the Veto.

        event.GetProperty()->SetValueFromString(newValue, 0);
    }
}

void OnPathChanged(wxPropertyGridEvent& event, NodeProperty* prop, Node* /* node */)
{
    // If the user clicked the path button, the current directory may have changed.
    GetProject()->GetProjectPath().ChangeDir();

    ttString newValue = event.GetPropertyValue().GetString();
    newValue.make_absolute();
    newValue.make_relative_wx(GetProject()->GetProjectPath());
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
    auto& old_path = GetProject()->prop_as_string(prop_derived_directory);
    path.backslashestoforward();
    if (path == "./")
        path.clear();
    if (path.size() && path.back() == '/')
        path.pop_back();

    auto undo_derived = std::make_shared<ModifyProperties>("Derived directory");
    undo_derived->AddProperty(GetProject()->get_prop_ptr(prop_derived_directory), path);

    std::vector<Node*> forms;
    GetProject()->CollectForms(forms);

    for (auto& form: forms)
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
    auto& old_path = GetProject()->prop_as_string(prop_base_directory);
    path.backslashestoforward();
    if (path == "./")
        path.clear();
    if (path.size() && path.back() == '/')
        path.pop_back();

    auto undo_derived = std::make_shared<ModifyProperties>("Base directory");
    undo_derived->AddProperty(GetProject()->get_prop_ptr(prop_base_directory), path);

    std::vector<Node*> forms;
    GetProject()->CollectForms(forms);

    for (const auto& form: forms)
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
