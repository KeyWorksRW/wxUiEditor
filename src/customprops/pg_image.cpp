/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom property grid class for images
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>

#include <wx/arrstr.h>             // wxArrayString class
#include <wx/dir.h>                // wxDir is a class for enumerating the files in a directory
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "ui_images.h"

using namespace wxue_img;

#include "pg_image.h"

#include "bitmaps.h"          // Contains various images handling functions
#include "image_handler.h"    // ImageHandler class
#include "img_string_prop.h"  // wxSingleChoiceDialogAdapter
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node -- Node class
#include "pg_point.h"         // CustomPointProperty -- Custom property grid class for wxPoint
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties

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

    if (prop->GetNode()->isGen(gen_embedded_image))
    {
        types.Add(s_type_names[1]);  // Embed
        types.Add(s_type_names[2]);  // SVG
        m_isEmbeddedImage = true;
    }
    else
    {
        // These need to match the array in img_props.h
        types.Add(s_type_names[0]);  // Art
        types.Add(s_type_names[1]);  // Embed
        types.Add(s_type_names[2]);  // SVG
        types.Add(s_type_names[3]);  // XPM
    }

    AddPrivateChild(new wxEnumProperty("type", wxPG_LABEL, types, 0));
    AddPrivateChild(new ImageStringProperty("image", m_img_props));

    AddPrivateChild(new CustomPointProperty("Size", prop, CustomPointProperty::type_SVG));
    Item(IndexSize)->SetHelpString("Default size -- ignored unless it's a SVG file.");
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

        if (m_img_props.type == "Embed" || m_img_props.type == "SVG")
        {
            Item(IndexImage)
                ->SetHelpString("Specifies the original image which will be embedded into a generated class source file as "
                                "an unsigned char array.");
        }
        else if (m_img_props.type == "XPM")
        {
            Item(IndexImage)->SetHelpString("Specifies the XPM file to include.");
        }

        if (m_old_image != m_img_props.image || m_old_type != m_img_props.type)
        {
            wxBitmapBundle bundle;
            if (m_img_props.image.size())
            {
                if (m_img_props.type == "XPM")
                {
                    wxImage img = ProjectImages.GetPropertyBitmap(m_img_props.CombineValues(), false);
                    if (img.IsOk())
                    {
                        // SetValueImage expects a bitmap with an alpha channel, so if it doesn't have one, make one now.
                        // Note that if this was an XPM file, then the mask will be converted to an alpha channel which is
                        // what we want.

                        if (!img.HasAlpha())
                            img.InitAlpha();
                        bundle = wxBitmapBundle::FromBitmap(img);
                    }
                }
                else
                {
                    if (auto img = ProjectImages.GetPropertyImageBundle(m_img_props.CombineValues(),
                                                                        wxGetFrame().GetSelectedNode());
                        img)
                    {
                        if (img->bundle.IsOk())
                        {
                            bundle = img->bundle;
                        }
                    }
                }
            }

            if (!bundle.IsOk())
                bundle = wxBitmapBundle::FromBitmap(LoadHeaderImage(empty_png, sizeof(empty_png)).Scale(15, 15));

            Item(IndexImage)->SetValueImage(bundle);

            m_old_image = m_img_props.image;
            // We do NOT set m_old_type here -- that needs to be handled in the next if clause
        }

        if (m_old_type != m_img_props.type)
        {
            SetAutoComplete();
            m_old_type = m_img_props.type;
        }
    }

    if (!m_isAutoCompleteSet)
    {
        SetAutoComplete();
    }

    Item(IndexType)->SetValue(m_img_props.type.wx_str());
    Item(IndexImage)->SetValue(m_img_props.image.wx_str());
    Item(IndexSize)->SetValue(m_img_props.CombineDefaultSize());
}

void PropertyGrid_Image::SetAutoComplete()
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
        auto art_dir = Project.value(prop_art_directory);
        if (art_dir.empty())
            art_dir = "./";
        wxDir dir;
        wxArrayString array_files;
        wxBusyCursor hourglass;
        if (m_img_props.type == "Embed")
        {
            // For auto-completion, we limit the array to the most common image types
            dir.GetAllFiles(art_dir, &array_files, "*.png", wxDIR_FILES);
            dir.GetAllFiles(art_dir, &array_files, "*.ico", wxDIR_FILES);
            dir.GetAllFiles(art_dir, &array_files, "*.bmp", wxDIR_FILES);
        }
        else if (m_img_props.type == "XPM")
        {
            dir.GetAllFiles(art_dir, &array_files, "*.xpm", wxDIR_FILES);
        }
        else if (m_img_props.type == "SVG")
        {
            dir.GetAllFiles(art_dir, &array_files, "*.svg", wxDIR_FILES);
        }

        for (auto& iter: array_files)
        {
            wxFileName name(iter);
            array_art_ids.Add(name.GetFullName());
        }
    }

    Item(IndexImage)->SetAttribute(wxPG_ATTR_AUTOCOMPLETE, array_art_ids);
    m_isAutoCompleteSet = true;
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
                if (m_isEmbeddedImage && index > 0)
                {
                    // REVIEW: [KeyWorks - 04-19-2022] This will only work if we only allow two iamge types (Embed and SVG)
                    img_props.type = s_type_names[2];  // SVG image type
                }
                else
                {
                    img_props.type = s_type_names[index];
                }

                // If the type has changed, then the image property is no longer valid
                img_props.image.clear();
                if (img_props.type == "SVG")
                {
                    img_props.SetWidth(16);
                    img_props.SetHeight(16);
                }
            }
            break;

        case IndexImage:
            {
                if (img_props.type == "Art")
                {
                    img_props.image.assign_wx(childValue.GetString());
                }
                else
                {
                    tt_wxString name(childValue.GetString());
                    if (name.size())
                    {
                        if (!name.file_exists())
                        {
                            name = Project.ArtDirectory();
                            name.append_filename_wx(childValue.GetString());
                        }
                        name.make_relative_wx(Project.ProjectPath());
                        name.backslashestoforward();
                    }
                    img_props.image.assign_wx(name);
                }
            }
            break;

        case IndexSize:
            {
                auto u8_value = childValue.GetString().utf8_string();
                tt_view_vector mstr(u8_value, ',');
                img_props.SetWidth(mstr[0].atoi());
                img_props.SetHeight(mstr[1].atoi());
            }
            break;
    }

    value = img_props.CombineValues().wx_str();
    return value;
}
