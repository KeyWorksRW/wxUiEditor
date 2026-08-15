/////////////////////////////////////////////////////////////////////////////
// Purpose:   EmbeddedImage struct
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [06-20-2026]

#include <wx/image.h>
#include <wx/mstream.h>   // For wxMemoryInputStream
#include <wx/wfstream.h>  // For wxFileInputStream
#include <wx/zstream.h>   // For wxZlibInputStream

#include <sstream>  // For std::ostringstream
#include <tuple>    // for std::ignore

#include "embed_image.h"

#include "mainframe.h"  // for wxGetMainFrame()
#include "pugixml.hpp"  // For XML parsing of SVG files
#include "utils.h"      // For FileNameToVarName()
#include "wxue_namespace/wxue_string.h"

EmbeddedImage::EmbeddedImage(wxue::string_view path, Node* form)
{
    ASSERT(!path.empty());
    ASSERT(m_images.empty());
    m_form = form;
    m_images.emplace_back();
    base_image().filename = path;
    base_image().file_time = base_image().filename.last_write_time();
    const std::optional<wxue::string> result = FileNameToVarName(path.filename());
    base_image().array_name = result.value_or("image_");

    for (char& idx: base_image().array_name)
    {
        if (wxue::is_alnum(idx) || idx == '_')
        {
            continue;
        }
        idx = '_';
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
        const wxDateTime file_time = base_image().filename.last_write_time();
        if (file_time != base_image().file_time)
        {
            if (!UpdateImage(base_image()))
            {
                return wxBitmapBundle();
            }
        }
        const uint64_t org_size = (base_image().array_size >> 32);
        std::vector<char> str(org_size);
        wxMemoryInputStream stream_in(base_image().array_data.data(),
                                      base_image().array_size & 0xFFFFFFFF);
        wxZlibInputStream zlib_strm(stream_in);
        const size_t bytes_read = zlib_strm.Read(str.data(), org_size).LastRead();
        if (bytes_read != org_size)
        {
            return wxBitmapBundle();
        }
        if (base_image().type == wxBITMAP_TYPE_SVG)
        {
            return wxBitmapBundle::FromSVG(str.data(),
                                           override_size == wxDefaultSize ? m_size : override_size);
        }
        // base_image().type == wxBITMAP_TYPE_XPM
        ASSERT(base_image().type == wxBITMAP_TYPE_XPM);

        wxImage image;
        wxMemoryInputStream stream(str.data(), org_size);
        image.LoadFile(stream, wxBITMAP_TYPE_XPM);
        return wxBitmapBundle::FromBitmap(image);
    }

    wxVector<wxBitmap> bitmaps;
    for (auto& iter: m_images)
    {
        const wxDateTime file_time = iter.filename.last_write_time();
        if (file_time != iter.file_time)
        {
            if (!UpdateImage(iter))
            {
                continue;
            }
        }
        wxMemoryInputStream stream(iter.array_data.data(), iter.array_size);
        wxImage image;
        image.LoadFile(stream);
        ASSERT(image.IsOk())
        if (image.IsOk())
        {
            bitmaps.emplace_back(image);
        }
    }
    return wxBitmapBundle::FromBitmaps(bitmaps);
}

bool EmbeddedImage::UpdateImage(ImageInfo& image_info)
{
    if (base_image().type == wxBITMAP_TYPE_SVG)
    {
        ASSERT_MSG(image_info.filename == base_image().filename,
                   "Embedded SVG images should only have a single image")
        // Run the file through an XML parser so that we can remove content that isn't used, as well
        // as removing line breaks, leading spaces, etc.
        pugi::xml_document xml_doc;
        const pugi::xml_parse_result result = xml_doc.load_file_string(base_image().filename);
        if (!result)
        {
            wxMessageDialog(wxGetMainFrame()->getWindow(), result.detailed_msg, "Parsing Error",
                            wxOK | wxICON_ERROR)
                .ShowModal();
            return false;
        }

        pugi::xml_node root = xml_doc.first_child();  // this should be the <svg> element.
        std::ignore = root.remove_attributes();       // we don't need any of the attributes

        // Remove some inkscape nodes that we don't need
        std::ignore = root.remove_child("sodipodi:namedview");
        std::ignore = root.remove_child("metadata");

        // Security: Remove all script tags to prevent potential malware execution
        // Use XPath translate() to convert element names to lowercase for case-insensitive
        // matching. This is more thorough than explicit case enumeration and handles all 32
        // possible case combinations. Performance impact is negligible since this runs once per SVG
        // file load.
        const pugi::xpath_node_set script_nodes =
            xml_doc.select_nodes("//*[translate(name(), 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', "
                                 "'abcdefghijklmnopqrstuvwxyz') = 'script']");
        for (const auto& xpath_node: script_nodes)
        {
            const pugi::xml_node script_node = xpath_node.node();
            pugi::xml_node parent = script_node.parent();
            if (!parent.empty())
            {
                std::ignore = parent.remove_child(script_node);
            }
        }

        std::ostringstream xml_stream;
        xml_doc.save(xml_stream, "", pugi::format_raw | pugi::format_no_declaration);
        const std::string str = xml_stream.str();

        // Include the trailing zero -- we need to read this back as a string, not a data array
        wxMemoryInputStream stream(str.c_str(), str.size() + 1);

        wxMemoryOutputStream memory_stream;
        wxZlibOutputStream save_stream(memory_stream, wxZ_BEST_COMPRESSION);

        const wxFileOffset stream_length = stream.GetLength();
        if (stream_length <= 0)
        {
            return false;
        }
        const size_t raw_size = static_cast<size_t>(stream_length);
        const uint64_t org_size = raw_size;

        if (raw_size > 250 * 1024 * 1024)
        {
            wxMessageDialog(wxGetMainFrame()->getWindow(),
                            wxString::Format("The SVG file is %zu MB. wxUiEditor cannot embed "
                                             "images larger than 250 MB.",
                                             raw_size / (1024 * 1024)),
                            "Image Too Large", wxOK | wxICON_ERROR)
                .ShowModal();
            return false;
        }

        if (raw_size > 25 * 1024 * 1024)
        {
            const int confirm =
                wxMessageDialog(
                    wxGetMainFrame()->getWindow(),
                    wxString::Format(
                        "The SVG file is %zu MB. Are you sure you want to embed this image?",
                        raw_size / (1024 * 1024)),
                    "Confirm Embed", wxYES_NO | wxICON_WARNING)
                    .ShowModal();
            if (confirm != wxID_YES)
            {
                return false;
            }
        }

        if (!CopyStreamData(&stream, &save_stream, stream_length))
        {
            return false;
        }
        save_stream.Close();
        auto compressed_size = static_cast<uint64_t>(memory_stream.TellO());

        const wxStreamBuffer* read_stream = memory_stream.GetOutputStreamBuffer();
        base_image().array_size = (compressed_size | (org_size << 32));
        base_image().array_data.resize(compressed_size);
        memcpy(base_image().array_data.data(), read_stream->GetBufferStart(), compressed_size);
        base_image().file_time = base_image().filename.last_write_time();
        return true;
    }

    wxFFileInputStream stream(image_info.filename);
    if (!stream.IsOk())
    {
        return false;
    }

    wxImageHandler* handler = nullptr;
    const wxList& list = wxImage::GetHandlers();
    for (wxList::compatibility_iterator node = list.GetFirst(); node; node = node->GetNext())
    {
        handler = wxStaticCast(node->GetData(), wxImageHandler);
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

                    const wxStreamBuffer* read_stream = save_stream.GetOutputStreamBuffer();
                    stream.SeekI(0);
                    if (read_stream->GetBufferSize() <= (to_size_t) stream.GetLength())
                    {
                        image_info.array_size = read_stream->GetBufferSize();
                        image_info.array_data.resize(image_info.array_size);
                        memcpy(image_info.array_data.data(), read_stream->GetBufferStart(),
                               image_info.array_size);
                    }
                    else
                    {
                        const wxFileOffset file_size = stream.GetSize();
                        if (file_size < 0)
                        {
                            wxMessageDialog(wxGetMainFrame()->getWindow(),
                                            "The image file cannot be read and appears to be "
                                            "invalid.",
                                            "Invalid Image File", wxOK | wxICON_ERROR)
                                .ShowModal();
                            return false;
                        }
                        image_info.array_size = static_cast<uint64_t>(file_size);
                        image_info.array_data.resize(image_info.array_size);
                        stream.Read(image_info.array_data.data(), image_info.array_size);
                    }
                }
                else
                {
                    stream.SeekI(0);
                    image_info.array_size = stream.GetSize();
                    image_info.array_data.resize(image_info.array_size);
                    stream.Read(image_info.array_data.data(), image_info.array_size);
                }

                return true;
            }
        }
    }
    return false;
}
