/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handles property grid image properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-12-2026]

#include <wx/artprov.h>  // wxArtProvider class

#include "image_handler.h"                    // ImageHandler class
#include "img_props.h"                        // ImageProperties
#include "utils.h"                            // Utility functions that work with properties
#include "wxue_namespace/wxue_view_vector.h"  // wxue::ViewVector

void ImageProperties::InitValues(wxue::string_view value)
{
    wxue::ViewVector mstr(value, ';', wxue::TRIM::both);

    if (mstr.size() > IndexImage)
    {
        image = mstr[IndexImage];
    }
    else
    {
        image.clear();
    }

    if (mstr.size() > IndexType)
    {
        type = mstr[IndexType];
        if (type == "Header" && image.extension().is_sameas(".xpm", wxue::CASE::either))
        {
            type = "XPM";
        }

        if ((type == "SVG" || type == "Art") && mstr.size() > IndexImage + 1)
        {
            m_size = GetSizeInfo(mstr[IndexSize]);
        }
        else
        {
            const EmbeddedImage* embed = ProjectImages.GetEmbeddedImage(image);
            if (embed)
            {
                m_size = embed->get_wxSize();
            }
            else if (type == "Art" && mstr.size() > IndexImage)
            {
                wxue::ViewVector art_str(mstr[IndexArtID], '|', wxue::TRIM::both);
                const wxString art_id = art_str[0].wx();
                const wxString client_id = art_str.size() > 1 ? art_str[1].wx() : wxString {};
                const wxBitmap bmp =
                    wxArtProvider::GetBitmap(art_id, wxART_MAKE_CLIENT_ID_FROM_STR(client_id));
                if (bmp.IsOk())
                {
                    m_size = bmp.GetSize();
                    m_def_art_size = m_size;
                }
                else
                {
                    m_size.x = -1;
                    m_size.y = -1;
                }
            }
            else
            {
                m_size.x = -1;
                m_size.y = -1;
            }
        }
    }
}

wxue::string ImageProperties::CombineValues()
{
    wxue::string value;
    image.backslashestoforward();
    value << type << ';' << image;
    if (type == "SVG")
    {
        value << ";[" << m_size.x << ',' << m_size.y << "]";
    }
    else if (type == "Art" && m_size != m_def_art_size)
    {
        value << ";[" << m_size.x << ',' << m_size.y << "]";
    }

    return value;
}

wxString ImageProperties::CombineDefaultSize()
{
    wxString value;
    value << m_size.x << ',' << m_size.y;
    return value;
}
