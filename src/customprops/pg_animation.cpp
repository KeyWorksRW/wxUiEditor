/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom property grid class for animations
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/dir.h>                // wxDir is a class for enumerating the files in a directory
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "pg_animation.h"

#include "node.h"  // Node -- Node class

#include "img_props.h"        // Handles property grid image properties
#include "img_string_prop.h"  // wxSingleChoiceDialogAdapter
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties

wxIMPLEMENT_ABSTRACT_CLASS(PropertyGrid_Animation, wxPGProperty);

PropertyGrid_Animation::PropertyGrid_Animation(const wxString& label, NodeProperty* prop) : wxPGProperty(label, wxPG_LABEL)
{
    m_img_props.node_property = prop;
    m_img_props.SetAnimationType();
    m_value = prop->as_wxString();
    if (prop->HasValue())
    {
        m_img_props.InitValues(prop->as_string());
    }

    wxPGChoices types;

    types.Add(s_type_names[1]);  // Embed

    AddPrivateChild(new wxEnumProperty("type", wxPG_LABEL, types, 0));
    AddPrivateChild(new ImageStringProperty("image", m_img_props));
}

void PropertyGrid_Animation::RefreshChildren()
{
    wxString value = m_value;
    if (value.size())
    {
        m_img_props.InitValues(value.utf8_string());

        if (m_img_props.type == "Embed")
        {
            Item(IndexImage)
                ->SetHelpString(
                    "Specifies the original animation image which will be embedded into a generated class source "
                    "file as an unsigned char array.");
        }
        else
        {
            Item(IndexImage)
                ->SetHelpString("Specifies an external file containing the animation image as an unsigned char array.");
        }
    }

    if (m_old_type != m_img_props.type)
    {
        wxArrayString array_art_ids;
        auto art_dir = Project.ArtDirectory();
        if (art_dir.empty())
            art_dir = "./";
        wxDir dir;
        wxArrayString array_files;
        dir.GetAllFiles(art_dir, &array_files, m_img_props.type == "Header" ? "*.h_img" : "*.gif", wxDIR_FILES);
        if (m_img_props.type == "Embed")
            dir.GetAllFiles(art_dir, &array_files, "*.ani");
        for (auto& iter: array_files)
        {
            wxFileName name(iter);
            array_art_ids.Add(name.GetFullName());
        }
        Item(IndexImage)->SetAttribute(wxPG_ATTR_AUTOCOMPLETE, array_art_ids);
        m_old_type = m_img_props.type;
    }

    Item(IndexType)->SetValue(m_img_props.type.make_wxString());
    Item(IndexImage)->SetValue(m_img_props.image.make_wxString());
}

wxVariant PropertyGrid_Animation::ChildChanged(wxVariant& thisValue, int childIndex, wxVariant& childValue) const
{
    wxString value = thisValue;

    ImageProperties img_props;
    if (value.size())
    {
        img_props.InitValues(value.utf8_string());
    }

    switch (childIndex)
    {
        case IndexType:
            {
                auto index = childValue.GetLong();
                if (index == 0)
                    img_props.type = s_type_names[1];

                if (index >= 0)
                {
                    // If the type has changed, then the image property is no longer valid
                    img_props.image.clear();
                }
                break;
            }

        case IndexImage:
            {
                tt_string name(childValue.GetString().utf8_string());
                if (!name.file_exists())
                {
                    name = Project.ArtDirectory();
                    name.append_filename(childValue.GetString().utf8_string());
                }
                name.make_relative(Project.get_ProjectPath());
                name.backslashestoforward();
                img_props.image.assign(name);
            }
            break;
    }

    value = img_props.CombineValues().make_wxString();
    return value;
}
