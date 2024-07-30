/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom property grid class for images
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>

#include <wx/arrstr.h>  // wxArrayString class
#include <wx/artprov.h>
#include <wx/dir.h>                // wxDir is a class for enumerating the files in a directory
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "ui_images.h"

using namespace wxue_img;

#include "pg_image.h"

#include "img_string_prop.h"  // wxSingleChoiceDialogAdapter
#include "txt_string_prop.h"  // EditStringDialogAdapter -- Derived wxStringProperty class for single-line text

#include "bitmaps.h"          // Contains various images handling functions
#include "image_handler.h"    // ImageHandler class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node -- Node class
#include "pg_point.h"         // CustomPointProperty -- Custom property grid class for wxPoint
#include "preferences.h"      // Preferences -- Stores user preferences
#include "project_handler.h"  // ProjectHandler class

#include "art_ids.cpp"  // wxART_ strings

#define EMBED_INDEX 0
#define SVG_INDEX   1
#define ART_INDEX   2
#define XPM_INDEX   3

wxIMPLEMENT_ABSTRACT_CLASS(PropertyGrid_Image, wxPGProperty);

PropertyGrid_Image::PropertyGrid_Image(const wxString& label, NodeProperty* prop) : wxPGProperty(label, wxPG_LABEL)
{
    m_img_props.node_property = prop;
    m_value = prop->as_wxString();
    if (prop->hasValue())
    {
        m_img_props.InitValues(prop->as_string());
    }
    else if (UserPrefs.is_SvgImages())
    {
        m_img_props.type = s_type_names[SVG_INDEX];  // SVG
    }

    wxPGChoices types;

    if (prop->getNode()->isGen(gen_embedded_image))
    {
        types.Add(s_type_names[EMBED_INDEX]);
        types.Add(s_type_names[SVG_INDEX]);
        m_isEmbeddedImage = true;
    }
    else
    {
        // These need to match the array in img_props.h
        types.Add(s_type_names[EMBED_INDEX]);
        types.Add(s_type_names[SVG_INDEX]);
        types.Add(s_type_names[ART_INDEX]);
        types.Add(s_type_names[XPM_INDEX]);
    }

    AddPrivateChild(new wxEnumProperty("type", wxPG_LABEL, types, 0));
    Item(IndexType)->SetHelpString("The type of image to use.");

    AddPrivateChild(new ImageStringProperty("image", m_img_props));

    if (m_isEmbeddedImage)
    {
        if (m_img_props.type == s_type_names[SVG_INDEX])
        {
            AddPrivateChild(new CustomPointProperty("Size", prop, CustomPointProperty::type_SVG));
        }
        else
        {
            AddPrivateChild(new CustomPointProperty("Original Size (ignored)", prop, CustomPointProperty::type_BITMAP));
        }
    }
    else
    {
        if (m_img_props.type == s_type_names[SVG_INDEX])
        {
            AddPrivateChild(new CustomPointProperty("Size", prop, CustomPointProperty::type_SVG));
        }
        else if (m_img_props.type == s_type_names[ART_INDEX])
        {
            AddPrivateChild(new CustomPointProperty("Size", prop, CustomPointProperty::type_ART));
        }
        else
        {
            AddPrivateChild(new CustomPointProperty("Original Size (ignored)", prop, CustomPointProperty::type_BITMAP));
        }
    }

    Item(IndexSize)->SetHelpString("Default size -- ignored unless it's an SVG or ART file.");
}

void PropertyGrid_Image::RefreshChildren()
{
    wxString value = m_value;
    if (value.size())
    {
        m_img_props.InitValues(value.utf8_string());

        if (m_img_props.type == "SVG" || m_img_props.type == "Art")
        {
            Item(IndexSize)->SetLabel("Size");
        }
        else
        {
            Item(IndexSize)->SetLabel("Original Size (ignored)");
        }

        if (m_img_props.type == "Art")
        {
            Item(IndexImage)->SetLabel("id");
            Item(IndexImage)->SetHelpString("Specifies the art ID and optional Client (separated by a | character).");
        }
        else if (m_img_props.type == "Embed" || m_img_props.type == "SVG")
        {
            Item(IndexImage)->SetLabel("image");
            Item(IndexImage)
                ->SetHelpString("Specifies the original image which will be embedded into a generated class source file as "
                                "an unsigned char array.");
        }
        else if (m_img_props.type == "XPM")
        {
            Item(IndexImage)->SetLabel("image");
            Item(IndexImage)->SetHelpString("Specifies the XPM file to include.");
        }

        if (m_old_image != m_img_props.image || m_old_type != m_img_props.type)
        {
            wxBitmapBundle bundle;
            if (m_img_props.image.size())
            {
                if (m_img_props.type != "XPM")
                {
                    auto* embed = ProjectImages.FindEmbedded(m_img_props.CombineValues());
                    if (embed)
                    {
                        bundle = embed->get_bundle(m_img_props.GetSize());
                    }
                }
                else  // XPM
                {
                    wxImage img = ProjectImages.GetPropertyBitmap(m_img_props.CombineValues(), false);
                    if (img.IsOk())
                    {
                        // SetValueImage expects a bitmap with an alpha channel, so if it doesn't have one, make one now.
                        // Note that if this was an XPM file, then the mask will be converted to an alpha channel which
                        // is what we want.

                        if (!img.HasAlpha())
                            img.InitAlpha();
                        bundle = wxBitmapBundle::FromBitmap(img);
                    }
                }
            }

            if (!bundle.IsOk())
                bundle = wxue_img::bundle_empty_png();

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

    Item(IndexType)->SetValue(m_img_props.type.make_wxString());
    Item(IndexImage)->SetValue(m_img_props.image.make_wxString());

    // CombineDefaultSize uses m_size
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
        auto art_dir = Project.as_string(prop_art_directory);
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
    if (UserPrefs.is_SvgImages())
    {
        img_props.type = s_type_names[SVG_INDEX];
        img_props.SetWidth(24);
        img_props.SetHeight(24);
    }

    auto value = thisValue.GetString();
    if (value.size())
    {
        img_props.InitValues(value.utf8_string());
    }

    switch (childIndex)
    {
        case IndexType:
            if (auto index = childValue.GetLong(); index >= 0)
            {
                if (m_isEmbeddedImage && index > 0)
                {
                    img_props.type = s_type_names[SVG_INDEX];  // SVG image type
                }
                else
                {
                    img_props.type = s_type_names[index];
                }

                // If the type has changed, then the image property is no longer valid
                img_props.image.clear();
                if (img_props.type == "SVG")
                {
                    img_props.SetWidth(24);
                    img_props.SetHeight(24);
                }
                else if (img_props.type == "Art")
                {
                    auto copy = value;
                    // img_props.SetWidth(24);
                    // img_props.SetHeight(24);
                }
            }
            break;

        case IndexImage:
            {
                if (img_props.type == "Art")
                {
                    auto mstr = childValue.GetString().utf8_string();
                    tt_view_vector art_str(mstr, '|', tt::TRIM::both);
                    wxString art_id = art_str[0].make_wxString();
                    auto bmp = wxArtProvider::GetBitmap(art_id, wxART_MAKE_CLIENT_ID_FROM_STR(art_str[1].make_wxString()));
                    if (bmp.IsOk())
                    {
                        img_props.SetSize(bmp.GetSize());
                    }
                    else
                    {
                        img_props.SetSize(wxDefaultSize);
                    }
                    img_props.image.assign_wx(childValue.GetString());
                }
                else
                {
                    tt_string name(childValue.GetString().utf8_string());
                    if (name.size())
                    {
                        if (!name.file_exists())
                        {
                            name = Project.ArtDirectory();
                            name.append_filename(childValue.GetString().utf8_string());
                        }
                        name.make_relative(Project.getProjectPath());
                        name.backslashestoforward();
                    }
                    img_props.image = name;
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

    value = img_props.CombineValues().make_wxString();
    return value;
}
