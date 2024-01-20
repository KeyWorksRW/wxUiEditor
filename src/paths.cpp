/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for directory and file properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// This module handles changes to art_directory, base_directory, and derived_directory

#include "paths.h"

#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "undo_cmds.h"        // InsertNodeAction -- Undoable command classes derived from UndoAction

void AllowDirectoryChange(wxPropertyGridEvent& event, NodeProperty* /* prop */, Node* /* node */)
{
    tt_string newValue = event.GetPropertyValue().GetString().utf8_string();
    if (newValue.empty())
        return;

    newValue.make_absolute();
    newValue.make_relative(Project.getProjectPath());
    newValue.backslashestoforward();

    tt_cwd cwd(true);
    Project.ChangeDir();

    if (!newValue.dir_exists())
    {
        // Displaying the message box can cause a focus change event which will call validation again in the OnIdle()
        // processing. Preserve the focus to avoid validating twice.
        auto focus = wxWindow::FindFocus();

        auto result = wxMessageBox(tt_string() << "The directory \"" << newValue
                                               << "\" does not exist. Do you want to use this name anyway?",
                                   "Directory doesn't exist", wxYES_NO | wxICON_WARNING, wxGetMainFrame());
        if (focus)
        {
            focus->SetFocus();
        }

        if (result != wxYES)
        {
            event.Veto();
            event.SetValidationFailureBehavior(wxPGVFBFlags::MarkCell | wxPGVFBFlags::StayInProperty);
            wxGetFrame().setStatusField("Either change the directory, or press ESC to restore the original value.");
            return;
        }
    }

    // If the event was previously veto'd, and the user corrected the file, then we have to set it here,
    // otherwise it will revert back to the original name before the Veto.

    event.GetProperty()->SetValueFromString(newValue);
}

// Unlike the AllowDirectoryChange() above, this will *not* allow a duplicate prop_base_file filename since the generated
// code will create a linker error due to the duplicate filenames (and the risk of overwriting an already generated file for
// a different class).

void AllowFileChange(wxPropertyGridEvent& event, NodeProperty* prop, Node* node)
{
    if (prop->isProp(prop_base_file) || prop->isProp(prop_python_file) || prop->isProp(prop_xrc_file))
    {
        tt_string newValue = event.GetPropertyValue().GetString().utf8_string();
        if (newValue.empty())
            return;

        newValue.make_absolute();
        newValue.make_relative(Project.getProjectPath());
        newValue.backslashestoforward();

        auto filename = newValue;

        std::vector<Node*> forms;
        Project.CollectForms(forms);
        for (const auto& child: forms)
        {
            if (child == node)
                continue;
            if (prop->isProp(prop_base_file))
            {
                if (child->as_string(prop_base_file).filename() == filename)
                {
                    auto focus = wxWindow::FindFocus();

                    wxMessageBox(wxString() << "The base filename \"" << filename.make_wxString()
                                            << "\" is already in use by " << child->as_string(prop_class_name)
                                            << "\n\nEither change the name, or press ESC to restore the original name.",
                                 "Duplicate base filename", wxICON_STOP);
                    if (focus)
                    {
                        focus->SetFocus();
                    }

                    event.Veto();
                    event.SetValidationFailureBehavior(wxPGVFBFlags::MarkCell | wxPGVFBFlags::StayInProperty);
                    wxGetFrame().setStatusField("Either change the name, or press ESC to restore the original value.");
                    return;
                }
            }
            else if (prop->isProp(prop_python_file))
            {
                if (child->as_string(prop_python_file).filename() == filename)
                {
                    auto focus = wxWindow::FindFocus();

                    wxMessageBox(wxString() << "The python filename \"" << filename.make_wxString()
                                            << "\" is already in use by " << child->as_string(prop_class_name)
                                            << "\n\nEither change the name, or press ESC to restore the original name.",
                                 "Duplicate python filename", wxICON_STOP);
                    if (focus)
                    {
                        focus->SetFocus();
                    }

                    event.Veto();
                    event.SetValidationFailureBehavior(wxPGVFBFlags::MarkCell | wxPGVFBFlags::StayInProperty);
                    wxGetFrame().setStatusField("Either change the name, or press ESC to restore the original value.");
                    return;
                }
            }
            else
            {
                // Currently, XRC files don't have a directory property, so the full path
                // relative to the project file is what we check. It *is* valid to have the
                // same filename provided it is in a different directory.
                if (child->as_string(prop_xrc_file) == filename)
                {
                    auto focus = wxWindow::FindFocus();

                    wxMessageBox(wxString() << "The xrc filename \"" << filename.make_wxString()
                                            << "\" is already in use by " << child->as_string(prop_class_name)
                                            << "\n\nEither change the name, or press ESC to restore the original name.",
                                 "Duplicate xrc filename", wxICON_STOP);

                    if (focus)
                    {
                        focus->SetFocus();
                    }

                    event.Veto();
                    event.SetValidationFailureBehavior(wxPGVFBFlags::MarkCell | wxPGVFBFlags::StayInProperty);
                    wxGetFrame().setStatusField("Either change the name, or press ESC to restore the original value.");
                    return;
                }
            }
        }

        // If the event was previously veto'd, and the user corrected the file, then we have to set it here,
        // otherwise it will revert back to the original name before the Veto.

        event.GetProperty()->SetValueFromString(newValue);
    }
}

void OnPathChanged(wxPropertyGridEvent& event, NodeProperty* prop, Node* node)
{
    // If the user clicked the path button, the current directory may have changed.
    Project.ChangeDir();

    tt_string newValue = event.GetPropertyValue().GetString().utf8_string();
    if (!node->isGen(gen_wxFilePickerCtrl))
    {
        newValue.make_absolute();
        newValue.make_relative(Project.getProjectPath());
        newValue.backslashestoforward();
    }
    // Note that on Windows, even though we changed the property to a forward slash, it will still be displayed
    // with a backslash. However, modifyProperty() will save our forward slash version, so even thought the
    // display isn't correct, it will be stored in the project file correctly.

    event.GetProperty()->SetValueFromString(newValue);
    tt_string value(newValue);
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

void ChangeDerivedDirectory(tt_string& path)
{
    auto& old_path = Project.as_string(prop_derived_directory);
    path.backslashestoforward();
    if (path == "./")
        path.clear();
    if (path.size() && path.back() == '/')
        path.pop_back();

    auto undo_derived = std::make_shared<ModifyProperties>("Derived directory");
    undo_derived->addProperty(Project.getProjectNode()->getPropPtr(prop_derived_directory), path);

    std::vector<Node*> forms;
    Project.CollectForms(forms);

    for (auto& form: forms)
    {
        if (form->as_bool(prop_use_derived_class) && form->hasValue(prop_derived_file))
        {
            tt_string cur_path = form->as_string(prop_derived_file);
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
            cur_path.append_filename(form->as_string(prop_derived_file).filename());
            undo_derived->addProperty(form->getPropPtr(prop_derived_file), cur_path);
        }
    }

    wxGetFrame().PushUndoAction(undo_derived);
}

void ChangeBaseDirectory(tt_string& path)
{
    auto& old_path = Project.as_string(prop_base_directory);
    path.backslashestoforward();
    if (path == "./")
        path.clear();
    if (path.size() && path.back() == '/')
        path.pop_back();

    auto undo_derived = std::make_shared<ModifyProperties>("Base directory");
    undo_derived->addProperty(Project.getProjectNode()->getPropPtr(prop_base_directory), path);

    std::vector<Node*> forms;
    Project.CollectForms(forms);

    for (const auto& form: forms)
    {
        if (form->hasValue(prop_base_file))
        {
            tt_string cur_path = form->as_string(prop_base_directory);
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
            cur_path.append_filename(form->as_string(prop_base_file).filename());
            undo_derived->addProperty(form->getPropPtr(prop_base_file), cur_path);
        }
    }

    wxGetFrame().PushUndoAction(undo_derived);
}
