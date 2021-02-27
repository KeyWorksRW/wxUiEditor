/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for handling wxImage files or art
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

#include "img_props.h"  // ImageProperties -- Handles property grid image properties

class ImageDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    ImageDialogAdapter(ImageProperties& img_props) : wxPGEditorDialogAdapter(), m_img_props(img_props) {}

    bool DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property)) override;

private:
    ImageProperties& m_img_props;
};

class ImageStringProperty : public wxStringProperty
{
public:
    ImageStringProperty(const wxString& label, ImageProperties& img_props) :
        wxStringProperty(label, wxPG_LABEL, img_props.image.wx_str()), m_img_props(img_props)
    {
    }

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override { return new ImageDialogAdapter(m_img_props); }

private:
    ImageProperties& m_img_props;
};
