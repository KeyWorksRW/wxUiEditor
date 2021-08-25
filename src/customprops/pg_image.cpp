/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom property grid class for images
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <array>

#include <wx/arrstr.h>             // wxArrayString class
#include <wx/dir.h>                // wxDir is a class for enumerating the files in a directory
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "../art_headers/empty_png.h_img"

#include "pg_image.h"

#include "bitmaps.h"      // Contains various images handling functions
#include "mainapp.h"      // Main application class
#include "node.h"         // Node -- Node class
#include "pjtsettings.h"  // ProjectSettings -- Hold data for currently loaded project
#include "utils.h"        // Utility functions that work with properties

#include "img_string_prop.h"  // wxSingleChoiceDialogAdapter
#include "pg_point.h"         // CustomPointProperty -- Custom property grid class for wxPoint

#include "art_ids.cpp"  // wxART_ strings

wxIMPLEMENT_ABSTRACT_CLASS(PropertyGrid_Image, wxPGProperty);

PropertyGrid_Image::PropertyGrid_Image(const wxString& label, NodeProperty* prop) : wxPGProperty(label, wxPG_LABEL)
{
    m_img_props.node_property = prop;
    m_value = prop->as_wxString();
    if (prop->HasValue())
    {
        m_img_props.InitValues(prop->as_string().c_str());
    }

    wxPGChoices types;

    types.Add(s_type_names[0]);  // Art
    types.Add(s_type_names[1]);  // Embed
    types.Add(s_type_names[2]);  // XPM
    types.Add(s_type_names[3]);  // Header

    AddPrivateChild(new wxEnumProperty("type", wxPG_LABEL, types, 0));
    AddPrivateChild(new ImageStringProperty("image", m_img_props));
    AddPrivateChild(new CustomPointProperty("scale size", prop, CustomPointProperty::type_scale));
    Item(IndexScale)->SetHelpString("Scale the image to this size.");
}

void PropertyGrid_Image::RefreshChildren()
{
    wxString value = m_value;
    if (value.size())
    {
        m_img_props.InitValues(value.utf8_str().data());

        if (m_img_props.type == "Art")
        {
            Item(IndexImage)->SetLabel("id");
            Item(IndexImage)->SetHelpString("Specifies the art ID and optional Client (separated by a | character).");
        }
        else
        {
            Item(IndexImage)->SetLabel("image");
        }

        if (m_img_props.type == "Embed")
        {
            Item(IndexImage)
                ->SetHelpString("Specifies the original image which will be embedded into a generated class source file as "
                                "an unsigned char array.");
        }
        else if (m_img_props.type == "XPM")
        {
            Item(IndexImage)->SetHelpString("Specifies the XPM file to include.");
        }
        else if (m_img_props.type == "Header")
        {
            Item(IndexImage)->SetHelpString("Specifies an external file containing the image as an unsigned char array.");
        }

        if (m_old_image != m_img_props.image || m_old_type != m_img_props.type)
        {
            wxBitmap bmp;
            if (m_img_props.image.size())
            {
                // Get a non-scaled version of the bitmap
                wxImage img = wxGetApp().GetProjectSettings()->GetPropertyBitmap(m_img_props.CombineValues(), false);
                if (img.IsOk())
                {
                    // SetValueImage expects a bitmap with an alpha channel, so if it doesn't have one, make one now.
                    // Note that if this was an XPM file, then the mask will be converted to an alpha channel which is
                    // what we want.

                    if (!img.HasAlpha())
                        img.InitAlpha();
                    bmp = img;
                }
            }

            if (!bmp.IsOk())
                bmp = LoadHeaderImage(empty_png, sizeof(empty_png)).Scale(15, 15);

            Item(IndexImage)->SetValueImage(bmp);
            m_old_image = m_img_props.image;
            // We do NOT set m_old_type here -- that needs to be handled in the next if clause
        }

        if (m_old_type != m_img_props.type)
        {
            wxArrayString array_art_ids;
            if (m_img_props.type == "Art")
            {
                for (auto& iter: set_art_ids)
                {
                    array_art_ids.Add(iter);
                }
            }
            else
            {
                auto art_dir = wxGetApp().GetProject()->prop_as_string(prop_original_art);
                if (art_dir.empty())
                    art_dir = "./";
                wxDir dir;
                wxArrayString array_files;
                dir.GetAllFiles(art_dir, &array_files, m_img_props.type == "XPM" ? "*.xpm" : "*.png", wxDIR_FILES);
                for (size_t pos = 0; pos < array_files.size(); ++pos)
                {
                    ttString* ptr = static_cast<ttString*>(&array_files[pos]);
                    array_art_ids.Add(ptr->filename());
                }
            }

            Item(IndexImage)->SetAttribute(wxPG_ATTR_AUTOCOMPLETE, array_art_ids);
            m_old_type = m_img_props.type;
        }
    }

    Item(IndexType)->SetValue(m_img_props.type.wx_str());
    Item(IndexImage)->SetValue(m_img_props.image.wx_str());
    Item(IndexScale)->SetValue(m_img_props.CombineScale());
}

wxVariant PropertyGrid_Image::ChildChanged(wxVariant& thisValue, int childIndex, wxVariant& childValue) const
{
    ImageProperties img_props;

    auto value = thisValue.GetString();
    if (value.size())
    {
        img_props.InitValues(value.utf8_string().c_str());
    }

    switch (childIndex)
    {
        case IndexType:
            if (auto index = childValue.GetLong(); index >= 0)
            {
                img_props.type = s_type_names[index];

                // If the type has changed, then the image property is no longer valid
                img_props.image.clear();
            }
            break;

        case IndexImage:
            img_props.image.assign_wx(childValue.GetString());
            break;

        case IndexScale:
            {
                ttlib::multistr mstr(childValue.GetString().utf8_string(), ',');
                img_props.SetWidth(mstr[0].atoi());
                img_props.SetHeight(mstr[1].atoi());
            }
            break;
    }

    value = img_props.CombineValues().wx_str();
    return value;
}
