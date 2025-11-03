/////////////////////////////////////////////////////////////////////////////
// Purpose:   EmbeddedImage struct
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <memory>  // For std::make_unique

#include <wx/image.h>
#include <wx/mstream.h>   // For wxMemoryInputStream
#include <wx/wfstream.h>  // For wxFileInputStream
#include <wx/zstream.h>   // For wxZlibInputStream

#include "embed_image.h"

#include "mainframe.h"  // for wxGetMainFrame()
#include "pugixml.hpp"  // For XML parsing of SVG files
#include "ttwx.h"       // ttwx namespace helpers for wxStrings
#include "utils.h"      // For FileNameToVarName()

EmbeddedImage::EmbeddedImage(tt_string_view path, Node* form)
{
    ASSERT(path.size());
    ASSERT(m_images.empty());
    m_form = form;
    m_images.push_back(ImageInfo());
    base_image().filename = path;
    base_image().file_time = base_image().filename.last_write_time();
    auto result = FileNameToVarName(path.filename());
    base_image().array_name = result.value_or("image_");

    tt_string check_filename(path.filename());
    check_filename.Replace(".", "_");

    for (size_t idx = 0; idx < base_image().array_name.size(); ++idx)
    {
        if (ttwx::is_alnum(base_image().array_name[idx]) || base_image().array_name[idx] == '_')
        {
            continue;
        }
        base_image().array_name[idx] = '_';
    }
}

void EmbeddedImage::SetEmbedSize(const wxImage& image)
{
    m_size = image.GetSize();
}

// size parameter is only used for SVG files
wxBitmapBundle EmbeddedImage::get_bundle(wxSize override_size)
{
    if (base_image().type == wxBITMAP_TYPE_SVG || base_image().type == wxBITMAP_TYPE_XPM)
    {
        auto file_time = base_image().filename.last_write_time();
        if (file_time != base_image().file_time)
        {
            UpdateImage(base_image());
        }
        uint64_t org_size = (base_image().array_size >> 32);
        auto str = std::make_unique<char[]>(org_size);
        wxMemoryInputStream stream_in(base_image().array_data.get(),
                                      base_image().array_size & 0xFFFFFFFF);
        wxZlibInputStream zlib_strm(stream_in);
        zlib_strm.Read(str.get(), org_size);
        if (base_image().type == wxBITMAP_TYPE_SVG)
        {
            return wxBitmapBundle::FromSVG(str.get(),
                                           override_size == wxDefaultSize ? m_size : override_size);
        }
        else  // base_image().type == wxBITMAP_TYPE_XPM
        {
            ASSERT(base_image().type == wxBITMAP_TYPE_XPM);

            wxImage image;
            wxMemoryInputStream stream(str.get(), org_size);
            image.LoadFile(stream, wxBITMAP_TYPE_XPM);
            return wxBitmapBundle::FromBitmap(image);
        }
    }

    wxVector<wxBitmap> bitmaps;
    for (auto& iter: m_images)
    {
        auto file_time = iter.filename.last_write_time();
        if (file_time != iter.file_time)
        {
            UpdateImage(iter);
        }
        wxMemoryInputStream stream(iter.array_data.get(), iter.array_size);
        wxImage image;
        image.LoadFile(stream);
        ASSERT(image.IsOk())
        if (image.IsOk())
        {
            bitmaps.push_back(image);
        }
    }
    return wxBitmapBundle::FromBitmaps(bitmaps);
}

void EmbeddedImage::UpdateImage(ImageInfo& image_info)
{
    if (base_image().type == wxBITMAP_TYPE_SVG)
    {
        ASSERT_MSG(image_info.filename == base_image().filename,
                   "Embedded SVG images should only have a single image")
        // Run the file through an XML parser so that we can remove content that isn't used, as well
        // as removing line breaks, leading spaces, etc.
        pugi::xml_document doc;
        auto result = doc.load_file_string(base_image().filename);
        if (!result)
        {
            wxMessageDialog(wxGetMainFrame()->getWindow(), result.detailed_msg, "Parsing Error",
                            wxOK | wxICON_ERROR)
                .ShowModal();
            return;
        }

        auto root = doc.first_child();  // this should be the <svg> element.
        root.remove_attributes();       // we don't need any of the attributes

        // Remove some inkscape nodes that we don't need
        root.remove_child("sodipodi:namedview");
        root.remove_child("metadata");

        std::ostringstream xml_stream;
        doc.save(xml_stream, "", pugi::format_raw | pugi::format_no_declaration);
        std::string str = xml_stream.str();

        // Include the trailing zero -- we need to read this back as a string, not a data array
        wxMemoryInputStream stream(str.c_str(), str.size() + 1);

        wxMemoryOutputStream memory_stream;
        wxZlibOutputStream save_strem(memory_stream, wxZ_BEST_COMPRESSION);

        uint64_t org_size = (stream.GetLength() & 0xFFFFFFFF);

        if (!CopyStreamData(&stream, &save_strem, stream.GetLength()))
        {
            return;
        }
        save_strem.Close();
        auto compressed_size = static_cast<uint64_t>(memory_stream.TellO());

        auto read_stream = memory_stream.GetOutputStreamBuffer();
        base_image().array_size = (compressed_size | (org_size << 32));
        base_image().array_data = std::make_unique<unsigned char[]>(compressed_size);
        memcpy(base_image().array_data.get(), read_stream->GetBufferStart(), compressed_size);
        return;
    }

    wxFFileInputStream stream(image_info.filename);
    if (!stream.IsOk())
        return;

    wxImageHandler* handler;
    auto& list = wxImage::GetHandlers();
    for (auto node = list.GetFirst(); node; node = node->GetNext())
    {
        handler = (wxImageHandler*) node->GetData();
        if (handler->CanRead(stream))
        {
            wxImage image;
            if (handler->LoadFile(&image, stream))
            {
                image_info.file_time = image_info.filename.last_write_time();

                // If possible, convert the file to a PNG -- even if the original file is a PNG,
                // since we might end up with better compression.

                if (isConvertibleMime(handler->GetMimeType()))
                {
                    image_info.type = wxBITMAP_TYPE_PNG;

                    wxMemoryOutputStream save_stream;

                    // Maximize compression
                    image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_LEVEL, 9);
                    image.SetOption(wxIMAGE_OPTION_PNG_COMPRESSION_MEM_LEVEL, 9);
                    image.SaveFile(save_stream, "image/png");

                    auto read_stream = save_stream.GetOutputStreamBuffer();
                    stream.SeekI(0);
                    if (read_stream->GetBufferSize() <= (to_size_t) stream.GetLength())
                    {
                        image_info.array_size = read_stream->GetBufferSize();
                        image_info.array_data =
                            std::make_unique<unsigned char[]>(image_info.array_size);
                        memcpy(image_info.array_data.get(), read_stream->GetBufferStart(),
                               image_info.array_size);
                    }
                    else
                    {
                        image_info.array_size = stream.GetSize();
                        image_info.array_data =
                            std::make_unique<unsigned char[]>(image_info.array_size);
                        stream.Read(image_info.array_data.get(), image_info.array_size);
                    }
                }
                else
                {
                    stream.SeekI(0);
                    image_info.array_size = stream.GetSize();
                    image_info.array_data =
                        std::make_unique<unsigned char[]>(image_info.array_size);
                    stream.Read(image_info.array_data.get(), image_info.array_size);
                }

                return;
            }
        }
    }
}
