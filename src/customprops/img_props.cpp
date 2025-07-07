/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handles property grid image properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/artprov.h>  // wxArtProvider class

#include "image_handler.h"   // ImageHandler class
#include "img_props.h"       // ImageProperties
#include "tt_view_vector.h"  // tt_view_vector -- Read/Write line-oriented strings/files
#include "utils.h"           // Utility functions that work with properties

void ImageProperties::InitValues(tt_string_view value)
{
    tt_view_vector mstr(value, ';', tt::TRIM::both);

    if (mstr.size() > IndexImage)
        image = mstr[IndexImage];
    else
        image.clear();

    if (mstr.size() > IndexType)
    {
        type = mstr[IndexType];
        if (type == "Header" && image.extension().is_sameas(".xpm", tt::CASE::either))
            type = "XPM";

        if ((type == "SVG" || type == "Art") && mstr.size() > IndexImage + 1)
        {
            m_size = GetSizeInfo(mstr[IndexSize]);
        }
        else
        {
            auto* embed = ProjectImages.GetEmbeddedImage(image);
            if (embed)
            {
                m_size = embed->size;
            }
            else if (type == "Art" && mstr.size() > IndexImage)
            {
                tt_view_vector art_str(mstr[IndexArtID], '|', tt::TRIM::both);
                wxString art_id = art_str[0].make_wxString();
                auto bmp = wxArtProvider::GetBitmap(
                    art_id, wxART_MAKE_CLIENT_ID_FROM_STR(art_str[1].make_wxString()));
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

tt_string ImageProperties::CombineValues()
{
    tt_string value;
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
