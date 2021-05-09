/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for handling animation files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

#include "anim_props.h"  // ImageProperties -- Handles property grid image properties

class AnimDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    AnimDialogAdapter() : wxPGEditorDialogAdapter() {}

    bool DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property)) override;
};

class AnimStringProperty : public wxStringProperty
{
public:
    AnimStringProperty(const wxString& label, AnimationProperties& img_props) :
        wxStringProperty(label, wxPG_LABEL, img_props.image.wx_str())
    {
    }

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override { return new AnimDialogAdapter(); }
};
